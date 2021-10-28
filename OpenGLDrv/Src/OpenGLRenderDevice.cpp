#include "../Inc/OpenGLRenderDevice.h"

#include "GL/glew.h"
#include "GL/wglew.h"
#include "OpenGLResource.h"

#define MIN_OPENGL_MAJOR_VERSION 4
#define MIN_OPENGL_MINOR_VERSION 5

IMPLEMENT_CLASS(UOpenGLRenderDevice)

FShaderGLSL UOpenGLRenderDevice::ErrorShader(FStringTemp("ERRORSHADER"),
                                             FStringTemp("void main(void){ gl_Position = LocalToScreen * vec4(InPosition.xyz, 1.0); }\n"),
                                             FStringTemp("void main(void){ FragColor = vec4(1.0, 0.0, 1.0, 1.0); }\n"));

UOpenGLRenderDevice::UOpenGLRenderDevice() : RenderInterface(this),
                                             FixedFunctionShader(FStringTemp("FixedFunction")),
                                             Backbuffer(0, 0, TEXF_RGBA8, false, false){}

void UOpenGLRenderDevice::StaticConstructor(){
	SupportsCubemaps       = 1;
	SupportsZBIAS          = 1;
	CanDoDistortionEffects = 1;
	bBilinearFramebuffer   = 1;
	TextureFilter          = TF_Bilinear;
	TextureAnisotropy      = 8;
	bFirstRun              = 1;
	ShaderDir              = FStringTemp("OpenGLShaders");

	new(GetClass(), "DistortionEffects",      RF_Public) UBoolProperty(CPP_PROPERTY(CanDoDistortionEffects),  "Options", CPF_Config);
	new(GetClass(), "UseDesktopResolution",   RF_Public) UBoolProperty(CPP_PROPERTY(bUseDesktopResolution),   "Options", CPF_Config);
	new(GetClass(), "KeepAspectRatio",        RF_Public) UBoolProperty(CPP_PROPERTY(bKeepAspectRatio),        "Options", CPF_Config);
	new(GetClass(), "BilinearFramebuffer",    RF_Public) UBoolProperty(CPP_PROPERTY(bBilinearFramebuffer),    "Options", CPF_Config);
	new(GetClass(), "AutoReloadShaders",      RF_Public) UBoolProperty(CPP_PROPERTY(bAutoReloadShaders),      "Options", CPF_Config);
	new(GetClass(), "TextureFilter",          RF_Public) UByteProperty(CPP_PROPERTY(TextureFilter),           "Options", CPF_Config);
	new(GetClass(), "TextureAnisotropy",      RF_Public) UIntProperty (CPP_PROPERTY(TextureAnisotropy),       "Options", CPF_Config);
	new(GetClass(), "VSync",                  RF_Public) UBoolProperty(CPP_PROPERTY(bVSync),                  "Options", CPF_Config);
	new(GetClass(), "AdaptiveVSync",          RF_Public) UBoolProperty(CPP_PROPERTY(bAdaptiveVSync),          "Options", CPF_Config);
	new(GetClass(), "FirstRun",               RF_Public) UBoolProperty(CPP_PROPERTY(bFirstRun),               "",        CPF_Config);
	new(GetClass(), "DebugOpenGL",            RF_Public) UBoolProperty(CPP_PROPERTY(bDebugOpenGL),            "",        CPF_Config);
	new(GetClass(), "ShowDebugNotifications", RF_Public) UBoolProperty(CPP_PROPERTY(bShowDebugNotifications), "",        CPF_Config);
	new(GetClass(), "ShaderDir",              RF_Public) UStrProperty (CPP_PROPERTY(ShaderDir),               "",        CPF_Config);
}

bool UOpenGLRenderDevice::IsCurrent(){
	return OpenGLContext != NULL && wglGetCurrentContext() == OpenGLContext;
}

void UOpenGLRenderDevice::MakeCurrent(){
	checkSlow(OpenGLContext);

	if(!IsCurrent())
		wglMakeCurrent(DeviceContext, OpenGLContext);
}

void UOpenGLRenderDevice::UnSetRes(UViewport* Viewport){
	guardFunc;

	if(OpenGLContext){
		MakeCurrent();
		Flush(Viewport);
		RenderInterface.Exit();
		wglMakeCurrent(DeviceContext, OpenGLContext);
		wglDeleteContext(OpenGLContext);
		OpenGLContext = NULL;
	}

	unguard;
}

void UOpenGLRenderDevice::AddResource(FOpenGLResource* Resource){
	checkSlow(Resource->HashIndex == INDEX_NONE);
	checkSlow(Resource->HashNext == NULL);
	checkSlow(GetCachedResource(Resource->CacheId) == NULL);

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

FShaderGLSL* UOpenGLRenderDevice::GetShader(UHardwareShader* HardwareShader){
	FShaderGLSL* Shader = GLShaderByHardwareShader.Find(HardwareShader);

	if(!Shader){
		Shader = &GLShaderByHardwareShader[HardwareShader];
		// Explicitly create object because TMap does not call the constructor!!!
		*Shader = FShaderGLSL(FStringTemp(HardwareShader->GetPathName()).Substitute(".", "\\").Substitute(".", "\\"));

		if(!HardwareShader->IsIn(UObject::GetTransientPackage())){
			// Convert d3d shader assembly to glsl or load existing shader from disk

			if(!LoadVertexShader(Shader)){
				Shader->SetVertexShaderText(GLSLVertexShaderFromD3DVertexShader(HardwareShader));
				SaveVertexShader(Shader);
			}

			if(!LoadFragmentShader(Shader)){
				Shader->SetFragmentShaderText(GLSLFragmentShaderFromD3DPixelShader(HardwareShader));
				SaveFragmentShader(Shader);
			}
		}else{
			Shader->SetVertexShaderText(GLSLVertexShaderFromD3DVertexShader(HardwareShader));
			Shader->SetFragmentShaderText(GLSLFragmentShaderFromD3DPixelShader(HardwareShader));
		}
	}

	return Shader;
}

FOpenGLIndexBuffer* UOpenGLRenderDevice::GetDynamicIndexBuffer(INT IndexSize){
	if(!DynamicIndexBuffer)
		DynamicIndexBuffer = new FOpenGLIndexBuffer(this, MakeCacheID(CID_RenderIndices), true);

	return DynamicIndexBuffer;
}

FOpenGLVertexStream* UOpenGLRenderDevice::GetDynamicVertexStream(){
	if(!DynamicVertexStream)
		DynamicVertexStream = new FOpenGLVertexStream(this, MakeCacheID(CID_RenderVertices), true);

	return DynamicVertexStream;
}

UBOOL UOpenGLRenderDevice::Exec(const TCHAR* Cmd, FOutputDevice& Ar){
	if(ParseCommand(&Cmd, "RELOADSHADERS")){
		Ar.Log("Reloading shaders from disk");
		LoadShaders();

		return 1;
	}

	return 0;
}

void GLAPIENTRY OpenGLMessageCallback(GLenum source,
                                      GLenum type,
                                      GLuint id,
                                      GLenum severity,
                                      GLsizei length,
                                      const GLchar* message,
                                      const void* userParam){
	const char* SourceStr;

	switch(source){
	case GL_DEBUG_SOURCE_API:
		SourceStr = "API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		SourceStr = "window system";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		SourceStr = "shader compiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		SourceStr = "third party";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		SourceStr = "application";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		SourceStr = "other";
		break;
	default:
		SourceStr = "unknown";
	}

	const char* TypeStr;

	switch(type){
	case GL_DEBUG_TYPE_ERROR:
		TypeStr = "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		TypeStr = "deprecated behavior";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		TypeStr = "undefined behavior";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		TypeStr = "portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		TypeStr = "performance";
		break;
	case GL_DEBUG_TYPE_OTHER:
		TypeStr = "other";
		break;
	case GL_DEBUG_TYPE_MARKER:
		TypeStr = "marker";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		TypeStr = "push group";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		TypeStr = "pop group";
		break;
	default:
		TypeStr = "unknown";
	}

	const char* SeverityStr;

	switch(severity){
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		SeverityStr = "notification";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		SeverityStr = "high";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		SeverityStr = "medium";
		break;
	case GL_DEBUG_SEVERITY_LOW:
		SeverityStr = "low";
		break;
	default:
		SeverityStr = "unknown";
	}

	if(severity != GL_DEBUG_SEVERITY_NOTIFICATION)
		debugf("GL CALLBACK: source=%s, type=%s, severity=%s - %s", SourceStr, TypeStr, SeverityStr, message);

	if(type == GL_DEBUG_TYPE_ERROR){
		GLog->Flush();

		if(appIsDebuggerPresent())
			appDebugBreak();
	}
}

UBOOL UOpenGLRenderDevice::Init(){
	// Init SWRCFix if it exists. Hacky but RenderDevice is always loaded at startup...
	void* ModDLL = appGetDllHandle("Mod.dll");

	if(ModDLL){
		void(CDECL*InitSWRCFix)(void) = static_cast<void(CDECL*)(void)>(appGetDllExport(ModDLL, "InitSWRCFix"));

		if(InitSWRCFix)
			InitSWRCFix();
	}

	// NOTE: This must be set to 0 in order to avoid inconsistencies with RGBA and BGRA colors.
	GIsOpenGL = 0;

	SetHardwareShaderMacros(CastChecked<UHardwareShaderMacros>(GEngine->HBumpShaderMacros));

	FixedFunctionShader.SetVertexShaderText(FixedFunctionVertexShaderText);
	FixedFunctionShader.SetFragmentShaderText(FixedFunctionFragmentShaderText);

	LoadShaders();

	return 1;
}

UBOOL UOpenGLRenderDevice::SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes, UBOOL bSaveSize){
	guardFunc;

	if(bFirstRun){
		// Use desktop resolution when using the OpenGL renderer for the first time
		bUseDesktopResolution = 1;
		bFirstRun = 0;
		SaveConfig();
	}

	HWND Window = static_cast<HWND>(Viewport->GetWindow());
	INT FullscreenWidth = NewX;
	INT FullscreenHeight = NewY;

	// Set NewX and NewY to screen resolution if bUseDesktopResolution is true
	if(Fullscreen){
		HMONITOR    Monitor = MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO Info    = {sizeof(MONITORINFO)};

		GetMonitorInfoA(Monitor, &Info);

		FullscreenWidth  = Info.rcMonitor.right - Info.rcMonitor.left;
		FullscreenHeight = Info.rcMonitor.bottom - Info.rcMonitor.top;

		if(bUseDesktopResolution){
			NewX = FullscreenWidth;
			NewY = FullscreenHeight;
		}
	}

	UBOOL Was16Bit = Use16bit;

	if(ColorBytes == 2)
		Use16bit = 1;
	else
		Use16bit = 0;

	// Create new context if there isn't one already or if the desired color depth has changed.
	if(!OpenGLContext || Was16Bit != Use16bit){
		UnSetRes(Viewport);

		DeviceContext = GetDC(Window);
		check(DeviceContext);

		ColorBytes = Use16bit ? 2 : 4;

		debugf("SetRes: %ix%i, %i-bit, Fullscreen: %i", NewX, NewY, ColorBytes * 8, Fullscreen);

		INT PfdFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DEPTH_DONTCARE;
		INT DepthBits = 0;
		INT StencilBits = 0;

		PIXELFORMATDESCRIPTOR Pfd = {
			sizeof(PIXELFORMATDESCRIPTOR), // size
			1,                             // version
			PfdFlags,                      // flags
			PFD_TYPE_RGBA,                 // color type
			ColorBytes * 8,                // preferred color depth
			0, 0, 0, 0, 0, 0,              // color bits (ignored)
			0,                             // alpha buffer
			0,                             // alpha bits (ignored)
			0,                             // accumulation buffer
			0, 0, 0, 0,                    // accum bits (ignored)
			DepthBits,                     // depth buffer
			StencilBits,                   // stencil buffer
			0,                             // auxiliary buffers
			PFD_MAIN_PLANE,                // main layer
			0,                             // reserved
			0, 0, 0,                       // layer, visible, damage masks
		};

		INT PixelFormat = ChoosePixelFormat(DeviceContext, &Pfd);

		debugf(NAME_Init, "Using pixel format %i", PixelFormat);
		debugf(NAME_Init, "%i-bit color buffer", ColorBytes * 8);

		SetPixelFormat(DeviceContext, PixelFormat, &Pfd);

		HGLRC TempContext = wglCreateContext(DeviceContext);

		if(!TempContext)
			appErrorf("Failed to create OpenGL context");

		wglMakeCurrent(DeviceContext, TempContext);

		glewExperimental = GL_TRUE;

		GLenum GlewStatus = glewInit();

		if(GlewStatus != GLEW_OK)
			appErrorf("GLEW failed to initialize: %s", glewGetErrorString(GlewStatus));

		if(WGLEW_ARB_create_context){
			int Attribs[16];
			int NumAttribs = 0;

			Attribs[NumAttribs++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
			Attribs[NumAttribs++] = MIN_OPENGL_MAJOR_VERSION;
			Attribs[NumAttribs++] = WGL_CONTEXT_MINOR_VERSION_ARB;
			Attribs[NumAttribs++] = MIN_OPENGL_MINOR_VERSION;
			Attribs[NumAttribs++] = WGL_CONTEXT_FLAGS_ARB;
			Attribs[NumAttribs++] = WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

			if(bDebugOpenGL){
				debugf("OpenGL debugging enabled");
				Attribs[NumAttribs - 1] |= WGL_CONTEXT_DEBUG_BIT_ARB;
			}

			Attribs[NumAttribs++] = WGL_CONTEXT_PROFILE_MASK_ARB;
			Attribs[NumAttribs++] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
			Attribs[NumAttribs++] = 0;

			OpenGLContext = wglCreateContextAttribsARB(DeviceContext, NULL, Attribs);

			if(OpenGLContext){
				wglMakeCurrent(DeviceContext, OpenGLContext);

				GlewStatus = glewInit();

				if(GlewStatus != GLEW_OK)
					appErrorf("GLEW failed to initialize: %s", glewGetErrorString(GlewStatus));

				wglDeleteContext(TempContext);
			}else{
				debugf(NAME_Warning, "wglCreateContextAttribsARB failed");
				// Use the temp context as a fallback. It might still work if it supports the required OpenGL version.
				OpenGLContext = TempContext;
			}
		}else{
			debugf("WGL_ARB_create_context not supported");
			OpenGLContext = TempContext;
		}

		debugf(NAME_Init, "GL_VENDOR      : %s", glGetString(GL_VENDOR));
		debugf(NAME_Init, "GL_RENDERER    : %s", glGetString(GL_RENDERER));
		debugf(NAME_Init, "GL_VERSION     : %s", glGetString(GL_VERSION));

		// Check for minimum required OpenGL version

		GLint MajorVersion;
		GLint MinorVersion;

		glGetIntegerv(GL_MAJOR_VERSION, &MajorVersion);
		glGetIntegerv(GL_MINOR_VERSION, &MinorVersion);

		if(MajorVersion < MIN_OPENGL_MAJOR_VERSION || (MajorVersion == MIN_OPENGL_MAJOR_VERSION && MinorVersion < MIN_OPENGL_MINOR_VERSION))
			appErrorf("OpenGL %i.%i is required but got %i.%i", MIN_OPENGL_MAJOR_VERSION, MIN_OPENGL_MINOR_VERSION, MajorVersion, MinorVersion);

		if(bDebugOpenGL){
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(OpenGLMessageCallback, NULL);
		}

		// Check for required extensions

		#define CHECK_EXT(ext) \
			if(!GLEW_ ## ext) \
				appErrorf("Required OpenGL extension '%s' is not supported", #ext);

		CHECK_EXT(ARB_direct_state_access);
		CHECK_EXT(ARB_vertex_array_bgra);
		CHECK_EXT(ARB_texture_compression);
		CHECK_EXT(EXT_texture_compression_s3tc);

		#undef CHECK_EXT

		RenderInterface.Init(NewX, NewY);
		EnableVSync(bVSync != 0);

		// Set default values for unspecified vertex attributes

		glVertexAttrib4f(FVF_Position,  0.0f, 0.0f, 0.0f, 1.0f);
		glVertexAttrib4f(FVF_Normal,    0.0f, 0.0f, 1.0f, 0.0f);
		glVertexAttrib4f(FVF_Diffuse,   0.0f, 0.0f, 0.0f, 0.0f);
		glVertexAttrib4f(FVF_Specular,  1.0f, 1.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord0, 0.0f, 0.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord1, 0.0f, 0.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord2, 0.0f, 0.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord3, 0.0f, 0.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord4, 0.0f, 0.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord5, 0.0f, 0.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord6, 0.0f, 0.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord7, 0.0f, 0.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_Tangent,   0.0f, 0.0f, 1.0f, 0.0f);
		glVertexAttrib4f(FVF_Binormal,  0.0f, 0.0f, 1.0f, 0.0f);
	}else{
		MakeCurrent();
		glDeleteRenderbuffers(1, &BackbufferDepthStencil);
		BackbufferDepthStencil = GL_NONE;
		RenderInterface.SetViewport(0, 0, NewX, NewY);
	}

	if(Fullscreen){
		Viewport->ResizeViewport(BLIT_Fullscreen | BLIT_OpenGL, FullscreenWidth, FullscreenHeight);
		SavedViewportWidth = Viewport->SizeX;
		SavedViewportHeight = Viewport->SizeY;
		Viewport->SizeX = NewX;
		Viewport->SizeY = NewY;
		bIsFullscreen = 1;

		if(bSaveSize){
			UClient* Client = Viewport->GetOuterUClient();

			// NOTE: We have to do it like this since the bSaveSize parameter for ResizeViewport seems to have no effect
			Client->FullscreenViewportX = Viewport->SizeX;
			Client->FullscreenViewportY = Viewport->SizeY;
			Client->SaveConfig();
		}
	}else{
		Viewport->ResizeViewport(BLIT_OpenGL, NewX, NewY);
		bIsFullscreen = 0;
	}

	// Create shared depth/stencil buffer
	glCreateRenderbuffers(1, &BackbufferDepthStencil);
	glNamedRenderbufferStorage(BackbufferDepthStencil, GL_DEPTH24_STENCIL8, NewX, NewY);

	// Resize screen render target if necessary
	if(Backbuffer.Width != NewX || Backbuffer.Height != NewY){
		Backbuffer.Width = NewX;
		Backbuffer.Height = NewY;
		++Backbuffer.Revision;
	}

	return 1;

	unguard;
}

void UOpenGLRenderDevice::Exit(UViewport* Viewport){
	UnSetRes(Viewport);
	GIsOpenGL = 0;
}

void UOpenGLRenderDevice::Flush(UViewport* Viewport){
	checkSlow(IsCurrent());
	RenderInterface.Flush();
	Scratch.Empty();

	if(Viewport && Viewport->Actor && Viewport->Actor->FrameFX)
		Viewport->Actor->FrameFX->FreeRenderTargets();

	for(INT i = 0; i < ARRAY_COUNT(ResourceHash); ++i){
		FOpenGLResource* Resource = ResourceHash[i];

		while(Resource){
			delete Resource;
			Resource = ResourceHash[i];
		}
	}

	appMemzero(ResourceHash, sizeof(ResourceHash));
	GLShaderByHardwareShader.Empty();

	DynamicIndexBuffer  = NULL;
	DynamicVertexStream = NULL;
}

void UOpenGLRenderDevice::FlushResource(QWORD CacheId){
	FOpenGLResource* Resource = GetCachedResource(CacheId);

	if(Resource)
		delete Resource;
}

UBOOL UOpenGLRenderDevice::ResourceCached(QWORD CacheId){
	return GetCachedResource(CacheId) != NULL;
}

void UOpenGLRenderDevice::EnableVSync(bool bEnable){
	if(WGL_EXT_swap_control){
		bVSync = bEnable;

		INT Interval;

		if(bEnable)
			Interval = bAdaptiveVSync && WGL_EXT_swap_control_tear ? -1 : 1;
		else
			Interval = 0;

		wglSwapIntervalEXT(Interval);
	}else{
		bVSync = 0;
	}
}

FRenderInterface* UOpenGLRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize){
	MakeCurrent();
	RenderInterface.Locked(Viewport);
	RenderInterface.SetRenderTarget(&Backbuffer, false);

	if(bAutoReloadShaders)
		LoadShaders();

	return &RenderInterface;
}

void UOpenGLRenderDevice::Unlock(FRenderInterface* RI){
	checkSlow(RI == &RenderInterface);
	RenderInterface.Unlocked();
}

void UOpenGLRenderDevice::Present(UViewport* Viewport){
	checkSlow(IsCurrent());

	HandleMovieWindow(Viewport);

	INT ViewportX;
	INT ViewportY;
	INT ViewportWidth;
	INT ViewportHeight;

	if(bKeepAspectRatio && bIsFullscreen){
		// Clear black bars.
		// This shouldn't have to happen every frame but for some reason the old pixels are still visible outside of the draw region
		// after a resolution change, even if the buffer was cleared in SetRes.
		FPlane Black(0.0f, 0.0f, 0.0f, 1.0f);
		glClearNamedFramebufferfv(GL_NONE, GL_COLOR, GL_NONE, reinterpret_cast<GLfloat*>(&Black));

		INT FramebufferWidth = Viewport->SizeX;
		INT FramebufferHeight = Viewport->SizeY;
		INT ScreenWidth = SavedViewportWidth;
		INT ScreenHeight = SavedViewportHeight;

		FLOAT XScale = 1.0f;
		FLOAT YScale = 1.0f;
		FLOAT ViewportAspectRatio = static_cast<FLOAT>(ScreenWidth) / ScreenHeight;
		FLOAT FramebufferAspectRatio = static_cast<FLOAT>(FramebufferWidth) / FramebufferHeight;

		if(FramebufferAspectRatio < ViewportAspectRatio){
			FLOAT Scale = static_cast<FLOAT>(ScreenHeight) / FramebufferHeight;

			XScale = FramebufferWidth * Scale / ScreenWidth;
		}else{
			FLOAT Scale = static_cast<FLOAT>(ScreenWidth) / FramebufferWidth;

			YScale = FramebufferHeight * Scale / ScreenHeight;
		}

		ViewportWidth = static_cast<INT>(ScreenWidth * XScale);
		ViewportHeight = static_cast<INT>(ScreenHeight * YScale);
		ViewportX = ScreenWidth / 2 - ViewportWidth / 2;
		ViewportY = ScreenHeight / 2 - ViewportHeight / 2;
	}else{
		ViewportX = 0;
		ViewportY = 0;

		if(bIsFullscreen){
			ViewportWidth = SavedViewportWidth;
			ViewportHeight = SavedViewportHeight;
		}else{
			ViewportWidth = Viewport->SizeX;
			ViewportHeight = Viewport->SizeY;
		}
	}

	FOpenGLTexture* BackbufferTexture = static_cast<FOpenGLTexture*>(GetCachedResource(Backbuffer.GetCacheId()));

	if(BackbufferTexture){
		glBlitNamedFramebuffer(BackbufferTexture->FBO, GL_NONE,
		                       0, 0, BackbufferTexture->Width, BackbufferTexture->Height,
		                       ViewportX, ViewportHeight, ViewportX + ViewportWidth, ViewportY,
		                       GL_COLOR_BUFFER_BIT,
		                       bBilinearFramebuffer ? GL_LINEAR : GL_NEAREST);

		SwapBuffers(DeviceContext);
	}

	check(glGetError() == GL_NO_ERROR);
}

void UOpenGLRenderDevice::ReadPixels(UViewport* Viewport, FColor* Pixels){
	if(Viewport && Pixels){
		RenderInterface.PushState();
		RenderInterface.SetRenderTarget(&Backbuffer, true);
		glReadPixels(0, 0, Viewport->SizeX, Viewport->SizeY, GL_BGRA, GL_UNSIGNED_BYTE, Pixels);
		RenderInterface.PopState();
	}
}

FRenderCaps* UOpenGLRenderDevice::GetRenderCaps(){
	static FRenderCaps RenderCaps(3, 14, 1);

	return &RenderCaps;
}

void UOpenGLRenderDevice::RenderMovie(UViewport* Viewport){
	FMovie* Movie = NULL;

	if(Viewport->Actor && Viewport->Actor->myHUD && Viewport->Actor->myHUD->Movie)
		Movie = Viewport->Actor->myHUD->Movie->fMovie;

	if(!Movie)
		return;

	bool NeedLock = RenderInterface.LockedViewport == NULL;

	if(NeedLock)
		Lock(Viewport, NULL, NULL);

	// TODO: Implement

	if(NeedLock)
		Unlock(&RenderInterface);
}

FMovie* UOpenGLRenderDevice::GetNewMovie(ECodecType Codec, FString Filename, UBOOL UseSound, INT FrameRate, int){
	if(Codec == CODEC_RoQ)
		return new FRoQMovie(Filename, UseSound, FrameRate);

	return NULL;
}

void UOpenGLRenderDevice::TakeScreenshot(const TCHAR* Name, UViewport* Viewport, INT Width, INT Height){
	check(Width == Viewport->SizeX);
	check(Height == Viewport->SizeY);
	URenderDevice::TakeScreenshot(Name, Viewport, Width, Height);
}

void UOpenGLRenderDevice::LoadShaders(){
	if(!LoadVertexShader(&FixedFunctionShader))
		SaveVertexShader(&FixedFunctionShader);

	if(!LoadFragmentShader(&FixedFunctionShader))
		SaveFragmentShader(&FixedFunctionShader);

	for(TMap<UHardwareShader*, FShaderGLSL>::TIterator It(GLShaderByHardwareShader); It; ++It){
		if(!It.Key()->IsIn(UObject::GetTransientPackage())){
			if(!LoadVertexShader(&It.Value()))
				SaveVertexShader(&It.Value());

			if(!LoadFragmentShader(&It.Value()))
				SaveFragmentShader(&It.Value());
		}
	}

	if(!LoadShaderMacroText())
		SaveShaderMacroText();
}

FStringTemp UOpenGLRenderDevice::MakeShaderFilename(const FString& ShaderName, const TCHAR* Extension){
	return ShaderDir * ShaderName + Extension;
}

bool UOpenGLRenderDevice::ShaderFileNeedsReload(const char* Filename){
	SQWORD CurrentFileTime = GFileManager->GetGlobalTime(Filename);
	SQWORD PreviousFileTime = ShaderFileTimes[Filename];

	return CurrentFileTime == 0 || CurrentFileTime != PreviousFileTime;
}

bool UOpenGLRenderDevice::LoadVertexShader(FShaderGLSL* Shader){
	FStringTemp ShaderText(0);
	FFilename Filename = MakeShaderFilename(Shader->GetName(), VERTEX_SHADER_FILE_EXTENSION);

	if(*Shader->GetVertexShaderText() && !ShaderFileNeedsReload(*Filename))
		return true;

	if(LoadShaderText(Filename, &ShaderText)){
		Shader->SetVertexShaderText(ShaderText);

		return true;
	}

	return false;
}

bool UOpenGLRenderDevice::LoadFragmentShader(FShaderGLSL* Shader){
	FStringTemp ShaderText(0);
	FFilename Filename = MakeShaderFilename(Shader->GetName(), FRAGMENT_SHADER_FILE_EXTENSION);

	if(*Shader->GetFragmentShaderText() && !ShaderFileNeedsReload(*Filename))
		return true;

	if(LoadShaderText(Filename, &ShaderText)){
		Shader->SetFragmentShaderText(ShaderText);

		return true;
	}

	return false;
}

bool UOpenGLRenderDevice::LoadShaderText(const FFilename& Filename, FString* Out){
	if(GFileManager->FileSize(*Filename) > 0 && appLoadFileToString(*Out, *Filename)){
		ShaderFileTimes[*Filename] = GFileManager->GetGlobalTime(*Filename);

		return true;
	}

	return false;
}

bool UOpenGLRenderDevice::LoadShaderMacroText(){
	FFilename Filename = MakeShaderFilename("Macros", SHADER_MACROS_FILE_EXTENSION);

	if(!ShaderFileNeedsReload(*Filename))
		return true;

	FString Macros;

	if(LoadShaderText(Filename, &Macros)){
		ParseGLSLMacros(Macros);

		// Increment revision of all shaders since they might use the macros and need to be updated
		for(TMap<UHardwareShader*, FShaderGLSL>::TIterator It(GLShaderByHardwareShader); It; ++It)
			++It.Value().Revision;

		++FixedFunctionShader.Revision;

		return true;
	}

	return false;
}

void UOpenGLRenderDevice::SaveShaderText(const FFilename& Filename, const FString& Text){
	GFileManager->MakeDirectory(*(Filename.GetPath() + "\\"), 1);

	if(appSaveStringToFile(Text, *Filename, GFileManager))
		ShaderFileTimes[*Filename] = GFileManager->GetGlobalTime(*Filename);
}

void UOpenGLRenderDevice::SaveVertexShader(FShaderGLSL* Shader){
	SaveShaderText(MakeShaderFilename(Shader->GetName(), VERTEX_SHADER_FILE_EXTENSION), FString(Shader->GetVertexShaderText(), true));
}

void UOpenGLRenderDevice::SaveFragmentShader(FShaderGLSL* Shader){
	SaveShaderText(MakeShaderFilename(Shader->GetName(), FRAGMENT_SHADER_FILE_EXTENSION), FString(Shader->GetFragmentShaderText(), true));
}

void UOpenGLRenderDevice::SaveShaderMacroText(){
	FString Macros;

	for(TMap<FString, FString>::TIterator It(ShaderMacros); It; ++It)
		Macros += "@" + It.Key() + "\n" + It.Value() + "\n";

	SaveShaderText(MakeShaderFilename("Macros", SHADER_MACROS_FILE_EXTENSION), Macros);
}

/*
 * HACK:
 * Movies like the intros and extras are played in a separate window. We resize and position it on top of the game window.
 * Works alright, except for the mouse cursor which is visible in the center of the window in fullscreen mode.
 */
void UOpenGLRenderDevice::HandleMovieWindow(UViewport* Viewport){
	if(Viewport->GetOuterUClient()->IsMoviePlaying()){
		HWND MovieWindow = FindWindowA(NULL, "ActiveMovie Window");

		if(CurrentMovieWindow != MovieWindow){
			CurrentMovieWindow = MovieWindow;

			if(!CurrentMovieWindow)
				return;

			SetWindowLongA(CurrentMovieWindow, GWL_STYLE, GetWindowLongA(CurrentMovieWindow, GWL_STYLE) & ~(WS_CAPTION | WS_THICKFRAME));
		}else if(!CurrentMovieWindow){
			return;
		}

		HWND ViewportWindow = static_cast<HWND>(Viewport->GetWindow());
		RECT Rect;

		GetClientRect(ViewportWindow, &Rect);
		MapWindowPoints(ViewportWindow, NULL, reinterpret_cast<POINT*>(&Rect), 2); // Map client rect to screen coordinates

		if(GetAsyncKeyState(VK_LBUTTON) || GetAsyncKeyState(VK_RBUTTON)){
			POINT CursorPos;
			GetCursorPos(&CursorPos);

			// Stop movie if mouse was clicked in client area
			if(CursorPos.x >= Rect.left && CursorPos.x <= Rect.right &&
			   CursorPos.y >= Rect.top && CursorPos.y <= Rect.bottom){
				Viewport->GetOuterUClient()->StopMovie();

				return;
			}
		}

		// Correct aspect ratio of video player in fullscreen to fit inside black bars
		if(bIsFullscreen && bKeepAspectRatio){
			INT FramebufferWidth = Viewport->SizeX;
			INT FramebufferHeight = Viewport->SizeY;
			INT ScreenWidth = SavedViewportWidth;
			INT ScreenHeight = SavedViewportHeight;
			FLOAT XScale = 1.0f;
			FLOAT YScale = 1.0f;
			FLOAT ViewportAspectRatio = static_cast<FLOAT>(ScreenWidth) / ScreenHeight;
			FLOAT FramebufferAspectRatio = static_cast<FLOAT>(FramebufferWidth) / FramebufferHeight;

			if(FramebufferAspectRatio < ViewportAspectRatio){
				FLOAT Scale = static_cast<FLOAT>(ScreenHeight) / FramebufferHeight;

				XScale = FramebufferWidth * Scale / ScreenWidth;
			}else{
				FLOAT Scale = static_cast<FLOAT>(ScreenWidth) / FramebufferWidth;

				YScale = FramebufferHeight * Scale / ScreenHeight;
			}

			INT XDiff = static_cast<INT>((ScreenWidth - XScale * ScreenWidth) * 0.5f);
			INT YDiff = static_cast<INT>((ScreenHeight - YScale * ScreenHeight) * 0.5f);

			Rect.left += XDiff;
			Rect.right -= XDiff;
			Rect.top += YDiff;
			Rect.bottom -= YDiff;
		}

		SetWindowPos(CurrentMovieWindow, HWND_TOPMOST, Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, SWP_SHOWWINDOW);
		SetFocus(ViewportWindow); // Focus on viewport window to let the engine handle any keyboard input
	}else{
		CurrentMovieWindow = NULL;
	}
}

// Default shader code

#define UNIFORM_BLOCK_MEMBER(type, name) "\t" STRINGIFY(type) " " STRINGIFY(name) ";\n"
#define UNIFORM_STRUCT_MEMBER(type, name) type name;
#define SHADER_HEADER \
	"#version 450 core\n\n" \
	"// EShaderLight\n" \
	"#define SL_Directional 0\n" \
	"#define SL_Point       1\n" \
	"#define SL_Spot        2\n\n" \
	"// Global shared uniforms\n\n" \
	"layout(std140, binding = 0) uniform Globals{\n" \
		UNIFORM_BLOCK_CONTENTS \
	"};\n\n" \
	"float saturate(float v){ return min(max(v, 0.0), 1.0); }\n" \
	"vec2  saturate(vec2  v){ return min(max(v, 0.0), 1.0); }\n" \
	"vec3  saturate(vec3  v){ return min(max(v, 0.0), 1.0); }\n" \
	"vec4  saturate(vec4  v){ return min(max(v, 0.0), 1.0); }\n\n"

FString UOpenGLRenderDevice::VertexShaderVarsText(
SHADER_HEADER
"// Vertex attributes\n\n"
"layout(location = 0)  in vec4 InPosition;\n"
"layout(location = 1)  in vec4 InNormal;\n"
"layout(location = 2)  in vec4 InDiffuse;\n"
"layout(location = 3)  in vec4 InSpecular;\n"
"layout(location = 4)  in vec4 InTexCoord0;\n"
"layout(location = 5)  in vec4 InTexCoord1;\n"
"layout(location = 6)  in vec4 InTexCoord2;\n"
"layout(location = 7)  in vec4 InTexCoord3;\n"
"layout(location = 8)  in vec4 InTexCoord4;\n"
"layout(location = 9)  in vec4 InTexCoord5;\n"
"layout(location = 10) in vec4 InTexCoord6;\n"
"layout(location = 11) in vec4 InTexCoord7;\n"
"layout(location = 12) in vec4 InTangent;\n"
"layout(location = 13) in vec4 InBinormal;\n\n"
"// Variables\n\n"
"out vec3 Position;\n"
"out vec3 Normal;\n"
"out vec4 Diffuse;\n"
"out vec4 Specular;\n"
"out vec4 TexCoord0;\n"
"out vec4 TexCoord1;\n"
"out vec4 TexCoord2;\n"
"out vec4 TexCoord3;\n"
"out vec4 TexCoord4;\n"
"out vec4 TexCoord5;\n"
"out vec4 TexCoord6;\n"
"out vec4 TexCoord7;\n"
"out vec3 Tangent;\n"
"out vec3 Binormal;\n"
"out float Fog;\n\n"
"float calculate_fog(float Dist){ return saturate((FogEnd - Dist) / (FogEnd - FogStart)); }\n\n");

FString UOpenGLRenderDevice::FragmentShaderVarsText(
SHADER_HEADER
"// Textures\n\n"
"layout(binding = 0) uniform sampler2D Texture0;\n"
"layout(binding = 1) uniform sampler2D Texture1;\n"
"layout(binding = 2) uniform sampler2D Texture2;\n"
"layout(binding = 3) uniform sampler2D Texture3;\n"
"layout(binding = 4) uniform sampler2D Texture4;\n"
"layout(binding = 5) uniform sampler2D Texture5;\n"
"layout(binding = 6) uniform sampler2D Texture6;\n"
"layout(binding = 7) uniform sampler2D Texture7;\n\n"
"// Cube maps\n\n"
"layout(binding = 8)  uniform samplerCube Cubemap0;\n"
"layout(binding = 9)  uniform samplerCube Cubemap1;\n"
"layout(binding = 10) uniform samplerCube Cubemap2;\n"
"layout(binding = 11) uniform samplerCube Cubemap3;\n"
"layout(binding = 12) uniform samplerCube Cubemap4;\n"
"layout(binding = 13) uniform samplerCube Cubemap5;\n"
"layout(binding = 14) uniform samplerCube Cubemap6;\n"
"layout(binding = 15) uniform samplerCube Cubemap7;\n\n"
"#define SAMPLE_TEX_FUNC(n) \\\n"
"\tvec4 sample_texture ## n(vec4 Coords){ \\\n"
"\t\tvec4 Result; \\\n"
"\t\tif(!TextureInfos[n].IsCubemap) \\\n"
"\t\t\tResult = texture(Texture ## n, Coords.xy); \\\n"
"\t\telse \\\n"
"\t\t\tResult = texture(Cubemap ## n, Coords.xyz); \\\n"
"\t\tif(TextureInfos[n].IsBumpmap) \\\n"
"\t\t\tResult.rg = (Result.rg - 0.5) * 2; \\\n"
"\t\treturn Result; \\\n"
"\t}\n\n"
"SAMPLE_TEX_FUNC(0)\n"
"SAMPLE_TEX_FUNC(1)\n"
"SAMPLE_TEX_FUNC(2)\n"
"SAMPLE_TEX_FUNC(3)\n"
"SAMPLE_TEX_FUNC(4)\n"
"SAMPLE_TEX_FUNC(5)\n"
"SAMPLE_TEX_FUNC(6)\n"
"SAMPLE_TEX_FUNC(7)\n\n"
"// Variables\n\n"
"in vec3 Position;\n"
"in vec3 Normal;\n"
"in vec4 Diffuse;\n"
"in vec4 Specular;\n"
"in vec4 TexCoord0;\n"
"in vec4 TexCoord1;\n"
"in vec4 TexCoord2;\n"
"in vec4 TexCoord3;\n"
"in vec4 TexCoord4;\n"
"in vec4 TexCoord5;\n"
"in vec4 TexCoord6;\n"
"in vec4 TexCoord7;\n"
"in vec3 Tangent;\n"
"in vec3 Binormal;\n"
"in float Fog;\n"
"out vec4 FragColor;\n\n"
"void alpha_test(vec4  c){ if(c.a <= AlphaRef) discard; }\n"
"vec3 apply_fog(vec3 BaseColor){ return mix(FogColor.rgb, BaseColor, Fog); }\n\n");

FString UOpenGLRenderDevice::FixedFunctionVertexShaderText(
"out vec3 AmbientFactor;\n"
"\n"
"void main(void){\n"
"\tPosition = (LocalToWorld * vec4(InPosition.xyz, 1.0)).xyz;\n"
"\tNormal = (LocalToWorld * vec4(InNormal.xyz, 0.0)).xyz;\n"
"\tDiffuse = InDiffuse;\n"
"\tSpecular = InSpecular;\n"
"\tFog = calculate_fog((LocalToCamera * vec4(InPosition.xyz, 1.0)).z);\n"
"\tgl_Position = LocalToScreen * vec4(InPosition.xyz, 1.0);\n"
"\tAmbientFactor = UseStaticLighting ? Specular.rgb : vec3(1.0);\n"
"}\n");

FString UOpenGLRenderDevice::FixedFunctionFragmentShaderText(
"in vec3 AmbientFactor;\n"
"\n"
"vec3 light_color(void){\n"
"\tvec3 DiffuseLight = Diffuse.rgb;\n"
"\tvec3 NormalizedNormal = normalize(Normal);\n"
"\n"
"\tfor(int i = 0; i < 4; ++i){\n"
"\t\tfloat LightFactor;\n"
"\n"
"\t\tif(Lights[i].Type == SL_Directional){\n"
"\t\t\tLightFactor = saturate(dot(-Lights[i].Direction.xyz, NormalizedNormal));\n"
"\t\t}else{\n"
"\t\t\tvec3 Dir = Lights[i].Position.xyz - Position;\n"
"\t\t\tvec3 NormalizedDir = normalize(Dir);\n"
"\t\t\tfloat Dist = length(Dir);\n"
"\t\t\t\n"
"\t\t\tLightFactor = saturate(dot(NormalizedDir, NormalizedNormal));\n"
"\n"
"\t\t\tif(Lights[i].Type == SL_Spot)\n"
"\t\t\t\tLightFactor *= saturate((dot(NormalizedDir, normalize(-Lights[i].Direction.xyz)) - Lights[i].Cone * 0.9) / 0.03); // Values hand-tuned to match d3d\n"
"\n"
"\t\t\t// Attenuation\n"
"\t\t\tLightFactor *= 1.0 / (Lights[i].Constant + Lights[i].Linear * Dist + Lights[i].Quadratic * (Dist * Dist));\n"
"\t\t}\n"
"\n"
"\t\tDiffuseLight += Lights[i].Color.rgb * LightFactor;\n"
"\t}\n"
"\n"
"\treturn AmbientLightColor.rgb * AmbientFactor + DiffuseLight;\n"
"}\n"
"\n"
"void main(void){\n"
"\tFragColor = vec4(1);\n"
"\n"
"\tif(UseDynamicLighting)\n"
"\t\tFragColor.rgb *= light_color();\n"
"\telse if(UseStaticLighting)\n"
"\t\tFragColor.rgb *= Diffuse.rgb;\n"
"}\n");

#undef UNIFORM_STRUCT_MEMBER
#undef UNIFORM_BLOCK_MEMBER
#undef SHADER_HEADER
