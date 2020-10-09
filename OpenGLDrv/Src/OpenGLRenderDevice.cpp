#include "../Inc/OpenGLRenderDevice.h"

#include "GL/glew.h"

IMPLEMENT_CLASS(UOpenGLRenderDevice)

UBOOL UOpenGLRenderDevice::SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes, UBOOL bSaveSize){
	HWND hwnd = static_cast<HWND>(Viewport->GetWindow());
	check(hwnd);
	HDC hdc = GetDC(hwnd);
	check(hdc);

	PIXELFORMATDESCRIPTOR pfd = {
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

	ChoosePixelFormat(hdc, &pfd);

	INT PixelFormat = ChoosePixelFormat( hdc, &pfd );
	Parse(appCmdLine(), "PIXELFORMAT=", PixelFormat);
	check(PixelFormat);

	debugf(NAME_Init, TEXT("Using pixel format %i"), PixelFormat);

	verify(SetPixelFormat(hdc, PixelFormat, &pfd));

	HGLRC hglrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hglrc);
	GLenum GlewStatus = glewInit();

	if(GlewStatus != GLEW_OK)
		appErrorf("GLEW failed to initialize: %s", glewGetErrorString(GlewStatus));

	debugf(NAME_Init, "GL_VENDOR      : %s", glGetString(GL_VENDOR));
	debugf(NAME_Init, "GL_RENDERER    : %s", glGetString(GL_RENDERER));
	debugf(NAME_Init, "GL_VERSION     : %s", glGetString(GL_VERSION));

	return 1;
}
