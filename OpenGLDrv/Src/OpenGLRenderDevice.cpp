#include "../Inc/OpenGLRenderDevice.h"

#include "GL/glew.h"

#define MIN_OPENGL_MAJOR_VERSION 4
#define MIN_OPENGL_MINOR_VERSION 5

IMPLEMENT_CLASS(UOpenGLRenderDevice)

HGLRC UOpenGLRenderDevice::CurrentContext = NULL;

void UOpenGLRenderDevice::StaticConstructor(){

}

void UOpenGLRenderDevice::MakeCurrent(){
	PRINT_FUNC;
	guardFunc;

	if(CurrentContext != OpenGLContext){
		wglMakeCurrent(DeviceContext, OpenGLContext);
		CurrentContext = OpenGLContext;
		GIsOpenGL = 1;
	}

	unguard;
}

void UOpenGLRenderDevice::UnSetRes(){
	PRINT_FUNC;
	guardFunc;

	if(OpenGLContext){
		if(CurrentContext == OpenGLContext){
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

UBOOL UOpenGLRenderDevice::SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes, UBOOL bSaveSize){
	PRINT_FUNC;
	guardFunc;

	UnSetRes();

	HWND hwnd = static_cast<HWND>(Viewport->GetWindow());
	check(hwnd);
	DeviceContext = GetDC(hwnd);
	check(DeviceContext);

	if(ColorBytes == 2)
		Use16bit = 1;
	else if(ColorBytes == 4)
		Use16bit = 0;

	ColorBytes = Use16bit ? 2 : 4;

	PIXELFORMATDESCRIPTOR Pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), // size
		1,                             // version
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,                 // color type
		ColorBytes <= 2 ? 16 : 32,     // prefered color depth
		0, 0, 0, 0, 0, 0,              // color bits (ignored)
		0,                             // alpha buffer
		0,                             // alpha bits (ignored)
		0,                             // accumulation buffer
		0, 0, 0, 0,                    // accum bits (ignored)
		ColorBytes <= 2 ? 16 : 24,     // depth buffer
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
	debugf(NAME_Init, "%i-bit depth buffer", DepthBits);
	debugf(NAME_Init, "%i-bit stencil buffer", StencilBits);

	// Check for required extensions
	RequireExt("GL_ARB_texture_compression");
	RequireExt("GL_EXT_texture_compression_s3tc");

	return 1;

	unguard;
}

void UOpenGLRenderDevice::Exit(UViewport* Viewport){
	PRINT_FUNC;
	UnSetRes();
}

FRenderInterface* UOpenGLRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize){
	PRINT_FUNC;

	MakeCurrent();
	glViewport(0, 0, Viewport->SizeX, Viewport->SizeY);

	return &RenderInterface;
}

void UOpenGLRenderDevice::Present(UViewport* Viewport){
	PRINT_FUNC;

	if(OpenGLContext == CurrentContext)
		verify(SwapBuffers(DeviceContext));
}

FRenderCaps* UOpenGLRenderDevice::GetRenderCaps(){
	PRINT_FUNC;

	RenderCaps.MaxSimultaneousTerrainLayers = 1;
	RenderCaps.PixelShaderVersion = 0;
	RenderCaps.HardwareTL = 1;

	return &RenderCaps;
}
