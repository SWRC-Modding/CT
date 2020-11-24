#include "../Inc/OpenGLRenderDevice.h"

#include "GL/glew.h"
#include "OpenGLResource.h"

#define MIN_OPENGL_MAJOR_VERSION 4
#define MIN_OPENGL_MINOR_VERSION 5

IMPLEMENT_CLASS(UOpenGLRenderDevice)

HGLRC UOpenGLRenderDevice::CurrentContext = NULL;

UOpenGLRenderDevice::UOpenGLRenderDevice() : RenderInterface(this),
                                             ScreenRenderTarget(0, 0, TEXF_RGBA8, false, true){}

void UOpenGLRenderDevice::StaticConstructor(){
	SupportsZBIAS = 1;
}

void UOpenGLRenderDevice::MakeCurrent(){
	guardFunc;

	if(!IsCurrent()){
		wglMakeCurrent(DeviceContext, OpenGLContext);
		CurrentContext = OpenGLContext;
		GIsOpenGL = 1;
	}

	unguard;
}

bool UOpenGLRenderDevice::IsCurrent(){
	checkSlow(wglGetCurrentContext() == CurrentContext);

	return CurrentContext == OpenGLContext;
}

void UOpenGLRenderDevice::UnSetRes(){
	guardFunc;

	if(OpenGLContext){
		if(IsCurrent()){
			CurrentContext = NULL;
			wglMakeCurrent(NULL, NULL);
			GIsOpenGL = 0;
		}

		wglDeleteContext(OpenGLContext);
		OpenGLContext = NULL;
	}

	unguard;
}

void UOpenGLRenderDevice::RequireExt(const TCHAR* Name){
	guardFunc;

	GLint numExtensions = 0;

	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

	for(GLint i = 0; i < numExtensions; ++i){
		if(appStrcmp(Name, (const char*)glGetStringi(GL_EXTENSIONS, i)) == 0){
			debugf(NAME_Init, "Device supports: %s", Name);

			return;
		}
	}

	appErrorf("Required OpenGL extension '%s' is not supported", Name);

	unguard;
}

void UOpenGLRenderDevice::AddResource(FOpenGLResource* Resource){
	checkSlow(Resource->HashIndex == INDEX_NONE);
	checkSlow(Resource->HashNext == NULL);

	Resource->HashIndex = GetResourceHashIndex(Resource->CacheId);
	Resource->HashNext = ResourceHash[Resource->HashIndex];
	ResourceHash[Resource->HashIndex] = Resource;
}

void UOpenGLRenderDevice::RemoveResource(FOpenGLResource* Resource){
	checkSlow(Resource->RenDev == this);
	checkSlow(Resource->HashIndex != INDEX_NONE);

	FOpenGLResource** Temp = &ResourceHash[Resource->HashIndex];

	while(*Temp){
		if(*Temp == Resource){
			*Temp = (*Temp)->HashNext;

			break;
		}

		Temp = &(*Temp)->HashNext;
	}
}

FOpenGLResource* UOpenGLRenderDevice::GetCachedResource(QWORD CacheId){
	INT HashIndex = GetResourceHashIndex(CacheId);
	FOpenGLResource* Resource = ResourceHash[HashIndex];

	while(Resource){
		if(Resource->CacheId == CacheId)
			return Resource;

		Resource = Resource->HashNext;
	}

	return NULL;
}

void UOpenGLRenderDevice::Destroy(){
	Super::Destroy();

	// The following resources must be manually freed since they are not contained in the resource hash

	if(DefaultShader)
		delete DefaultShader;

	if(FramebufferShader)
		delete FramebufferShader;
}

UBOOL UOpenGLRenderDevice::SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes, UBOOL bSaveSize){
	guardFunc;

	check(RenderInterface.CurrentState == RenderInterface.SavedStates);
	check(RenderInterface.SavedStateIndex == 0);

	HWND hwnd = static_cast<HWND>(Viewport->GetWindow());
	check(hwnd);
	UBOOL Was16Bit = Use16bit;

	if(ColorBytes == 2)
		Use16bit = 1;
	else if(ColorBytes == 4)
		Use16bit = 0;

	// Create new context if there isn't one already or if the desired color depth has changed.
	if(!OpenGLContext || Was16Bit != Use16bit){
		Flush(Viewport);
		UnSetRes();

		DeviceContext = GetDC(hwnd);
		check(DeviceContext);

		ColorBytes = Use16bit ? 2 : 4;

		debugf("SetRes: %ix%i, %i-bit, Fullscreen: %i", NewX, NewY, ColorBytes * 8, Fullscreen);

		PIXELFORMATDESCRIPTOR Pfd = {
			sizeof(PIXELFORMATDESCRIPTOR), // size
			1,                             // version
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DEPTH_DONTCARE, // flags
			PFD_TYPE_RGBA,                 // color type
			ColorBytes <= 2 ? 16 : 32,     // preferred color depth
			0, 0, 0, 0, 0, 0,              // color bits (ignored)
			0,                             // alpha buffer
			0,                             // alpha bits (ignored)
			0,                             // accumulation buffer
			0, 0, 0, 0,                    // accum bits (ignored)
			0,                             // depth buffer
			0,                             // stencil buffer
			0,                             // auxiliary buffers
			PFD_MAIN_PLANE,                // main layer
			0,                             // reserved
			0, 0, 0,                       // layer, visible, damage masks
		};

		INT PixelFormat = ChoosePixelFormat(DeviceContext, &Pfd);
		Parse(appCmdLine(), "PIXELFORMAT=", PixelFormat);
		check(PixelFormat);

		debugf(NAME_Init, "Using pixel format %i", PixelFormat);

		verify(SetPixelFormat(DeviceContext, PixelFormat, &Pfd));

		OpenGLContext = wglCreateContext(DeviceContext);

		MakeCurrent();

		glewExperimental = GL_TRUE;
		GLenum GlewStatus = glewInit();

		if(GlewStatus != GLEW_OK)
			appErrorf("GLEW failed to initialize: %s", glewGetErrorString(GlewStatus));

		debugf(NAME_Init, "GL_VENDOR      : %s", glGetString(GL_VENDOR));
		debugf(NAME_Init, "GL_RENDERER    : %s", glGetString(GL_RENDERER));
		debugf(NAME_Init, "GL_VERSION     : %s", glGetString(GL_VERSION));

		GLint MajorVersion;
		GLint MinorVersion;

		glGetIntegerv(GL_MAJOR_VERSION, &MajorVersion);
		glGetIntegerv(GL_MINOR_VERSION, &MinorVersion);

		if(MajorVersion < MIN_OPENGL_MAJOR_VERSION || (MajorVersion == MIN_OPENGL_MAJOR_VERSION && MinorVersion < MIN_OPENGL_MINOR_VERSION))
			appErrorf("OpenGL %i.%i is required but got %i.%i", MIN_OPENGL_MAJOR_VERSION, MIN_OPENGL_MINOR_VERSION, MajorVersion, MinorVersion);

		GLint DepthBits;
		GLint StencilBits;

		glGetIntegerv(GL_DEPTH_BITS, &DepthBits);
		glGetIntegerv(GL_STENCIL_BITS, &StencilBits);

		debugf(NAME_Init, "%i-bit color buffer", ColorBytes * 8);

		// Check for required extensions
		RequireExt("GL_ARB_texture_compression");
		RequireExt("GL_EXT_texture_compression_s3tc");

		glEnable(GL_DEPTH_TEST);

		// Create default shader for drawing everything that doesn't use a custom one

		DefaultShader = new FOpenGLShaderProgram(this, MakeCacheID(CID_RenderShader));
		DefaultShader->VertexShader = new FOpenGLShader(this, MakeCacheID(CID_RenderShader), OST_Vertex);
		DefaultShader->FragmentShader = new FOpenGLShader(this, MakeCacheID(CID_RenderShader), OST_Fragment);

		DefaultShader->VertexShader->Cache(
			"void main(void){\n"
			"    gl_Position = Transform * vec4(InPosition, 1.0);\n"
			"}\n"
		);
		DefaultShader->FragmentShader->Cache(
			"void main(void){\n"
			"    FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
			"}\n"
		);
		DefaultShader->Cache(DefaultShader->VertexShader, DefaultShader->FragmentShader);
		DefaultShader->VertexShader = NULL;
		DefaultShader->FragmentShader = NULL;
		RemoveResource(DefaultShader); // HACK: Removing the shader from the hash to prevent it from being destroyed when Flush is called

		// Create shader for displaying an off-screen framebuffer

		FramebufferShader = new FOpenGLShaderProgram(this, MakeCacheID(CID_RenderShader));
		FramebufferShader->VertexShader = new FOpenGLShader(this, MakeCacheID(CID_RenderShader), OST_Vertex);
		FramebufferShader->FragmentShader = new FOpenGLShader(this, MakeCacheID(CID_RenderShader), OST_Fragment);

		FramebufferShader->VertexShader->Cache(
			"void main(void){\n"
			"    const vec4[4] vertices = vec4[](vec4(1.0, 1.0, 1.0, 1.0),\n"
			"                                    vec4(-1.0, 1.0, 0.0, 1.0),\n"
			"                                    vec4(1.0, -1.0, 1.0, 0.0),\n"
			"                                    vec4(-1.0, -1.0, 0.0, 0.0));\n"
			"    TexCoord0 = vertices[gl_VertexID].zw;\n"
			"    gl_Position = vec4(vertices[gl_VertexID].xy, 0.5, 1.0);\n"
			"}\n"
		);
		FramebufferShader->FragmentShader->Cache(
			"void main(void){\n"
			"    FragColor = texture2D(Texture0, TexCoord0);\n"
			"}\n"
		);
		FramebufferShader->Cache(FramebufferShader->VertexShader, FramebufferShader->FragmentShader);
		FramebufferShader->VertexShader = NULL;
		FramebufferShader->FragmentShader = NULL;
		RemoveResource(FramebufferShader); // HACK: Removing the shader from the hash to prevent it from being destroyed when Flush is called
		glCreateVertexArrays(1, &ScreenVAO);
	}else{
		MakeCurrent();
	}

	// Resize screen render target if necessary
	if(ScreenRenderTarget.Width != NewX || ScreenRenderTarget.Height != NewY){
		ScreenRenderTarget.Width = NewX;
		ScreenRenderTarget.Height = NewY;
		++ScreenRenderTarget.Revision;
	}

	RenderInterface.SetRenderTarget(&ScreenRenderTarget, false);

	if(Fullscreen){
		HMONITOR    Monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO Info    = {sizeof(MONITORINFO)};

		verify(GetMonitorInfoA(Monitor, &Info));

		INT Width  = Info.rcMonitor.right - Info.rcMonitor.left;
		INT Height = Info.rcMonitor.bottom - Info.rcMonitor.top;

		Viewport->ResizeViewport(BLIT_Fullscreen | BLIT_OpenGL, Width, Height);
	}else{
		Viewport->ResizeViewport(BLIT_OpenGL, NewX, NewY);
	}

	return 1;

	unguard;
}

void UOpenGLRenderDevice::Exit(UViewport* Viewport){
	PRINT_FUNC;
	Flush(Viewport);
	UnSetRes();
}

void UOpenGLRenderDevice::Flush(UViewport* Viewport){
	RenderInterface.FlushResources();

	for(INT i = 0; i < ARRAY_COUNT(ResourceHash); ++i){
		FOpenGLResource* Resource = ResourceHash[i];

		while(Resource){
			delete Resource;
			Resource = ResourceHash[i];
		}
	}

	appMemzero(ResourceHash, sizeof(ResourceHash));
}

void UOpenGLRenderDevice::FlushResource(QWORD CacheId){
	FOpenGLResource* Resource = GetCachedResource(CacheId);

	if(Resource)
		delete Resource;
}

UBOOL UOpenGLRenderDevice::ResourceCached(QWORD CacheId){
	return GetCachedResource(CacheId) != NULL;
}

FRenderInterface* UOpenGLRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize){
	PRINT_FUNC;

	check(RenderInterface.SavedStateIndex == 0);

	// Makes stuff easier to see when rendering as wireframe
	// TODO: REMOVE!!!
	for(TObjectIterator<APlayerController> It; It; ++It)
		It->ShowFlags &= ~SHOW_Backdrop;

	MakeCurrent();
	RenderInterface.SetViewport(0, 0, Viewport->SizeX, Viewport->SizeY);

	// Render target might be deleted when Flush is called so check for that and set it again
	if(!RenderInterface.CurrentState->RenderTarget)
		RenderInterface.SetRenderTarget(&ScreenRenderTarget, false);

	return &RenderInterface;
}

void UOpenGLRenderDevice::Present(UViewport* Viewport){
	checkSlow(IsCurrent());

	// TODO: REMOVE!!!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	FOpenGLRenderTarget* Framebuffer = RenderInterface.CurrentState->RenderTarget;

	check(Framebuffer);

	FramebufferShader->Bind();
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	glBindTextureUnit(0, Framebuffer->ColorAttachment);
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(ScreenVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);
	SwapBuffers(DeviceContext);

	Framebuffer->Bind();

	// TODO: REMOVE!!!
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	check(glGetError() == GL_NO_ERROR);
}

FRenderCaps* UOpenGLRenderDevice::GetRenderCaps(){
	RenderCaps.MaxSimultaneousTerrainLayers = 1;
	RenderCaps.PixelShaderVersion = 0;
	RenderCaps.HardwareTL = 1;

	return &RenderCaps;
}
