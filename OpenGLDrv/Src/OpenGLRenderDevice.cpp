#include "../Inc/OpenGLRenderDevice.h"

#include "GL/glew.h"
#include "OpenGLResource.h"

#define MIN_OPENGL_MAJOR_VERSION 4
#define MIN_OPENGL_MINOR_VERSION 5

IMPLEMENT_CLASS(UOpenGLRenderDevice)

static const TCHAR* DefaultVertexShader   = "void vs_main(void){\n"
                                                "\tgl_Position = Transform * vec4(InPosition, 1.0);\n"
                                            "}\n";
static const TCHAR* DefaultFragmentShader = "void fs_main(void){\n"
                                                "\tFragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
                                            "}\n";

static const TCHAR* FramebufferVertexShader   = "void vs_main(void){\n"
			                                        "\tTexCoord0 = InTexCoord0;\n"
			                                        "\tgl_Position = vec4(InPosition.xy, 0.5, 1.0);\n"
			                                    "}\n";
static const TCHAR* FramebufferFragmentShader = "void fs_main(void){\n"
			                                        "\tFragColor = texture2D(Texture0, TexCoord0);\n"
			                                    "}\n";

UOpenGLRenderDevice::UOpenGLRenderDevice() : RenderInterface(this),
                                             ScreenRenderTarget(0, 0, TEXF_RGBA8, false, true){
	LoadShaders();
}

void UOpenGLRenderDevice::StaticConstructor(){
	SupportsZBIAS = 1;
	bFirstRun = 1;
	ShaderDir = "../Shaders";

	new(GetClass(), "UseDesktopResolution", RF_Public) UBoolProperty(CPP_PROPERTY(bUseDesktopResolution), "Options", CPF_Config);
	new(GetClass(), "KeepAspectRatio", RF_Public) UBoolProperty(CPP_PROPERTY(bKeepAspectRatio), "Options", CPF_Config);
	new(GetClass(), "FirstRun", RF_Public) UBoolProperty(CPP_PROPERTY(bFirstRun), "", CPF_Config);
	new(GetClass(), "ShaderDir", RF_Public) UStrProperty(CPP_PROPERTY(ShaderDir), "", CPF_Config);
}

void UOpenGLRenderDevice::MakeCurrent(){
	guardFunc;

	if(!IsCurrent()){
		wglMakeCurrent(DeviceContext, OpenGLContext);
		GIsOpenGL = 1;
	}

	unguard;
}

bool UOpenGLRenderDevice::IsCurrent(){
	return OpenGLContext != NULL && wglGetCurrentContext() == OpenGLContext;
}

void UOpenGLRenderDevice::UnSetRes(){
	guardFunc;

	if(OpenGLContext){
		if(IsCurrent()){
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

unsigned int UOpenGLRenderDevice::GetVAO(const FStreamDeclaration* Declarations, INT NumStreams){
	// Check if there is an existing VAO for this format by hashing the shader declarations
	GLuint& VAO = VAOsByDeclId[appMemCrc(Declarations, sizeof(FStreamDeclaration) * NumStreams)];

	// Create and setup VAO if none was found matching the vertex format
	if(!VAO){
		glCreateVertexArrays(1, &VAO);

		for(INT StreamIndex = 0; StreamIndex < NumStreams; ++StreamIndex){
			const FStreamDeclaration& Decl = Declarations[StreamIndex];
			GLuint Offset = 0;

			for(INT i = 0; i < Decl.NumComponents; ++i){
				BYTE Function = Decl.Components[i].Function; // EFixedVertexFunction
				BYTE Type     = Decl.Components[i].Type;     // EComponentType

				checkSlow(Function < FVF_MAX);
				checkSlow(Type < CT_MAX);

				switch(Type){
				case CT_Float4:
					glVertexArrayAttribFormat(VAO, Function, 4, GL_FLOAT, GL_FALSE, Offset);
					Offset += sizeof(FLOAT) * 4;
					break;
				case CT_Float3:
					glVertexArrayAttribFormat(VAO, Function, 3, GL_FLOAT, GL_FALSE, Offset);
					Offset += sizeof(FLOAT) * 3;
					break;
				case CT_Float2:
					glVertexArrayAttribFormat(VAO, Function, 2, GL_FLOAT, GL_FALSE, Offset);
					Offset += sizeof(FLOAT) * 2;
					break;
				case CT_Float1:
					glVertexArrayAttribFormat(VAO, Function, 1, GL_FLOAT, GL_FALSE, Offset);
					Offset += sizeof(FLOAT);
					break;
				case CT_Color:
					glVertexArrayAttribFormat(VAO, Function, 4, GL_UNSIGNED_BYTE, GL_TRUE, Offset);
					Offset += sizeof(FColor);
					break;
				default:
					appErrorf("Unexpected EComponentType (%i)", Type);
				}

				glEnableVertexArrayAttrib(VAO, Function);
				glVertexArrayAttribBinding(VAO, Function, StreamIndex);
			}
		}
	}

	return VAO;
}

FOpenGLIndexBuffer* UOpenGLRenderDevice::GetDynamicIndexBuffer(INT IndexSize){
	if(IndexSize == sizeof(DWORD)){
		if(!DynamicIndexBuffer32)
			DynamicIndexBuffer32 = new FOpenGLIndexBuffer(this, MakeCacheID(CID_RenderIndices), true);

		return DynamicIndexBuffer32;
	}else{
		if(!DynamicIndexBuffer16)
			DynamicIndexBuffer16 = new FOpenGLIndexBuffer(this, MakeCacheID(CID_RenderIndices), true);

		return DynamicIndexBuffer16;
	}
}

FOpenGLVertexStream* UOpenGLRenderDevice::GetDynamicVertexStream(){
	if(!DynamicVertexStream)
		DynamicVertexStream = new FOpenGLVertexStream(this, MakeCacheID(CID_RenderVertices), true);

	return DynamicVertexStream;
}

UBOOL UOpenGLRenderDevice::Exec(const TCHAR* Cmd, FOutputDevice& Ar){
	if(ParseCommand(&Cmd, "LOADSHADERS")){
		Ar.Log("Reloading shaders from disk");
		LoadShaders();

		return 1;
	}

	return 0;
}

UBOOL UOpenGLRenderDevice::SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes, UBOOL bSaveSize){
	guardFunc;

	if(bFirstRun){
		// Use desktop resolution when using the OpenGL renderer for the first time
		bUseDesktopResolution = 1;
		bFirstRun = 0;
		SaveConfig();
	}

	HWND hwnd = static_cast<HWND>(Viewport->GetWindow());
	UBOOL Was16Bit = Use16bit;

	if(ColorBytes == 2)
		Use16bit = 1;
	else
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
			Use16bit ? 16 : 32,            // preferred color depth
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

		// Setup initial state

		RenderInterface.EnableZTest(1);
		RenderInterface.SetShader(GetShader(SHADER_Default));
		RenderInterface.SetCullMode(CM_CW);
		RenderInterface.SetFillMode(FM_Wireframe); // TODO: Change to FM_Solid
	}else{
		MakeCurrent();
	}

	// Set window size
	if(Fullscreen){
		HMONITOR    Monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO Info    = {sizeof(MONITORINFO)};

		verify(GetMonitorInfoA(Monitor, &Info));

		INT Width  = Info.rcMonitor.right - Info.rcMonitor.left;
		INT Height = Info.rcMonitor.bottom - Info.rcMonitor.top;

		if(bUseDesktopResolution){
			NewX = Width;
			NewY = Height;
		}

		Viewport->ResizeViewport(BLIT_Fullscreen | BLIT_OpenGL, Width, Height);
		SavedViewportWidth = Viewport->SizeX;
		SavedViewportHeight = Viewport->SizeY;
		Viewport->SizeX = NewX;
		Viewport->SizeY = NewY;
		bIsFullscreen = 1;

		if(bSaveSize){
			UClient* Client = Viewport->GetOuterUClient();

			check(Client);

			// NOTE: We have to do it like this since the bSaveSize parameter for ResizeViewport seems to have no effect
			Client->FullscreenViewportX = Viewport->SizeX;
			Client->FullscreenViewportY = Viewport->SizeY;
			Client->SaveConfig();
		}
	}else{
		Viewport->ResizeViewport(BLIT_OpenGL, NewX, NewY);
		bIsFullscreen = 0;
	}

	// Resize screen render target if necessary
	if(ScreenRenderTarget.Width != NewX || ScreenRenderTarget.Height != NewY){
		ScreenRenderTarget.Width = NewX;
		ScreenRenderTarget.Height = NewY;
		++ScreenRenderTarget.Revision;
	}

	RenderInterface.SetRenderTarget(&ScreenRenderTarget, false);
	RenderInterface.SetViewport(0, 0, NewX, NewY);

	return 1;

	unguard;
}

void UOpenGLRenderDevice::Exit(UViewport* Viewport){
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

	DynamicIndexBuffer32 = NULL;
	DynamicIndexBuffer16 = NULL;
	DynamicVertexStream  = NULL;

	for(TMap<DWORD, unsigned int>::TIterator It(VAOsByDeclId); It; ++It){
		if(It.Value())
			glDeleteVertexArrays(1, &It.Value());
	}

	VAOsByDeclId.Empty();
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
	check(RenderInterface.CurrentState == &RenderInterface.SavedStates[0]);

	// Makes stuff easier to see when rendering as wireframe
	// TODO: REMOVE!!!
	for(TObjectIterator<APlayerController> It; It; ++It)
		It->ShowFlags &= ~SHOW_Backdrop;

	MakeCurrent();

	// Render target and default shader might be deleted when Flush is called so check for that and set them again

	if(!RenderInterface.CurrentState->RenderTarget)
		RenderInterface.SetRenderTarget(&ScreenRenderTarget, false);

	FShaderGLSL* DefaultShader = GetShader(SHADER_Default);
	FOpenGLShader* CurrentShader = RenderInterface.CurrentState->Shader;

	checkSlow(!CurrentShader || CurrentShader->CacheId == DefaultShader->GetCacheId());

	if(!CurrentShader || (CurrentShader->Revision != DefaultShader->GetRevision())) // Update default shader in case it was reloaded
		RenderInterface.SetShader(DefaultShader);

	RenderInterface.PushState();

	return &RenderInterface;
}

void UOpenGLRenderDevice::Unlock(FRenderInterface* RI){
	RI->PopState();
}

class FFullscreenQuadVertexStream : public FVertexStream{
public:
	struct Vertex{
		FLOAT X;
		FLOAT Y;
		FLOAT U;
		FLOAT V;
	} Vertices[4];

	FFullscreenQuadVertexStream(FLOAT XScale, FLOAT YScale){
		Vertices[0].X = 1.0f * XScale;
		Vertices[0].Y = 1.0f * YScale;
		Vertices[0].U = 1.0f;
		Vertices[0].V = 1.0f;
		Vertices[1].X = -1.0f * XScale;
		Vertices[1].Y = 1.0f * YScale;
		Vertices[1].U = 0.0f;
		Vertices[1].V = 1.0f;
		Vertices[2].X = 1.0f * XScale;
		Vertices[2].Y = -1.0f * YScale;
		Vertices[2].U = 1.0f;
		Vertices[2].V = 0.0f;
		Vertices[3].X = -1.0f * XScale;
		Vertices[3].Y = -1.0f * YScale;
		Vertices[3].U = 0.0f;
		Vertices[3].V = 0.0f;
	}

	virtual INT GetStride(){ return sizeof(Vertex); }
	virtual INT GetSize(){ return sizeof(Vertices); }

	virtual INT GetComponents(FVertexComponent* Components){
		Components[0].Function = FVF_Position;
		Components[0].Type     = CT_Float2;
		Components[1].Function = FVF_TexCoord0;
		Components[1].Type     = CT_Float2;

		return 2;
	}

	virtual void GetStreamData(void* Dest){ appMemcpy(Dest, Vertices, sizeof(Vertices)); }
};

void UOpenGLRenderDevice::Present(UViewport* Viewport){
	checkSlow(IsCurrent());

	FOpenGLRenderTarget* Framebuffer = RenderInterface.CurrentState->RenderTarget;

	check(Framebuffer);

	RenderInterface.PushState();
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

	// TODO: Add width and height members to FOpenGLRenderTarget...
	INT FramebufferWidth = RenderInterface.CurrentState->ViewportWidth;
	INT FramebufferHeight = RenderInterface.CurrentState->ViewportHeight;
	INT ViewportWidth;
	INT ViewportHeight;

	if(bIsFullscreen){
		ViewportWidth  = SavedViewportWidth;
		ViewportHeight = SavedViewportHeight;
	}else{
		ViewportWidth  =  Viewport->SizeX;
		ViewportHeight =  Viewport->SizeY;
	}

	RenderInterface.SetViewport(0, 0, ViewportWidth, ViewportHeight);

	FLOAT XScale = 1.0f;
	FLOAT YScale = 1.0f;

	if(bKeepAspectRatio){
		FLOAT ViewportAspectRatio = static_cast<FLOAT>(ViewportWidth) / ViewportHeight;
		FLOAT FramebufferAspectRatio = static_cast<FLOAT>(FramebufferWidth) / FramebufferHeight;

		if(FramebufferAspectRatio < ViewportAspectRatio){
			FLOAT Scale = static_cast<FLOAT>(ViewportHeight) / FramebufferHeight;

			XScale = FramebufferWidth * Scale / ViewportWidth;
		}else{
			FLOAT Scale = static_cast<FLOAT>(ViewportWidth) / FramebufferWidth;

			YScale = FramebufferHeight * Scale / ViewportHeight;
		}

		// Clear black bars
		RenderInterface.Clear(1, FColor(0, 0, 0));
	}

	FFullscreenQuadVertexStream FullscreenQuad(XScale, YScale);

	glBindTextureUnit(0, Framebuffer->ColorAttachment);
	RenderInterface.SetFillMode(FM_Solid);
	RenderInterface.EnableZTest(0);
	RenderInterface.SetDynamicStream(VS_FixedFunction, &FullscreenQuad);
	RenderInterface.SetShader(GetShader(SHADER_Framebuffer));
	RenderInterface.DrawPrimitive(PT_TriangleStrip, 0, 2);
	SwapBuffers(DeviceContext);
	Framebuffer->Bind();
	RenderInterface.PopState();

	check(glGetError() == GL_NO_ERROR);
}

FRenderCaps* UOpenGLRenderDevice::GetRenderCaps(){
	RenderCaps.MaxSimultaneousTerrainLayers = 1;
	RenderCaps.PixelShaderVersion = 0;
	RenderCaps.HardwareTL = 1;

	return &RenderCaps;
}

void UOpenGLRenderDevice::LoadShaders(){
	// Init default shaders with the default implementation
	#define SHADER(x) \
		DefaultShaders[SHADER_ ## x].SetVertexShaderText(FString(x ## VertexShader, true)); \
		DefaultShaders[SHADER_ ## x].SetFragmentShaderText(FString(x ## FragmentShader, true));
	DEFAULT_SHADERS
	#undef SHADER
	// Load shaders from disk or save them if they don't exist
	#define SHADER(x) LoadShader(&DefaultShaders[SHADER_ ## x], #x);
	DEFAULT_SHADERS
	#undef SHADER
}

void UOpenGLRenderDevice::LoadShader(FShaderGLSL* Shader, const TCHAR* Name){
	FStringTemp ShaderText(0);
	FString Filename = ShaderDir * Name + ".vsh";

	GFileManager->MakeDirectory(*ShaderDir);

	if(appLoadFileToString(ShaderText, *Filename))
		Shader->SetVertexShaderText(ShaderText);
	else
		appSaveStringToFile(Shader->GetVertexShaderText(), *Filename);

	Filename = ShaderDir * Name + ".fsh";

	if(appLoadFileToString(ShaderText, *Filename))
		Shader->SetFragmentShaderText(ShaderText);
	else
		appSaveStringToFile(Shader->GetFragmentShaderText(), *Filename);
}
