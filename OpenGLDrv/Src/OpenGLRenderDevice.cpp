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

	new(GetClass(), "DistortionEffects",    RF_Public) UBoolProperty(CPP_PROPERTY(CanDoDistortionEffects), "Options", CPF_Config);
	new(GetClass(), "UseDesktopResolution", RF_Public) UBoolProperty(CPP_PROPERTY(bUseDesktopResolution),  "Options", CPF_Config);
	new(GetClass(), "KeepAspectRatio",      RF_Public) UBoolProperty(CPP_PROPERTY(bKeepAspectRatio),       "Options", CPF_Config);
	new(GetClass(), "BilinearFramebuffer",  RF_Public) UBoolProperty(CPP_PROPERTY(bBilinearFramebuffer),   "Options", CPF_Config);
	new(GetClass(), "AutoReloadShaders",    RF_Public) UBoolProperty(CPP_PROPERTY(bAutoReloadShaders),     "Options", CPF_Config);
	new(GetClass(), "TextureFilter",        RF_Public) UByteProperty(CPP_PROPERTY(TextureFilter),          "Options", CPF_Config);
	new(GetClass(), "TextureAnisotropy",    RF_Public) UIntProperty (CPP_PROPERTY(TextureAnisotropy),      "Options", CPF_Config);
	new(GetClass(), "VSync",                RF_Public) UBoolProperty(CPP_PROPERTY(bVSync),                 "Options", CPF_Config);
	new(GetClass(), "AdaptiveVSync",        RF_Public) UBoolProperty(CPP_PROPERTY(bAdaptiveVSync),         "Options", CPF_Config);
	new(GetClass(), "FirstRun",             RF_Public) UBoolProperty(CPP_PROPERTY(bFirstRun),              "",        CPF_Config);
	new(GetClass(), "DebugOpenGL",          RF_Public) UBoolProperty(CPP_PROPERTY(bDebugOpenGL),           "",        CPF_Config);
	new(GetClass(), "ShaderDir",            RF_Public) UStrProperty (CPP_PROPERTY(ShaderDir),              "",        CPF_Config);
}

void UOpenGLRenderDevice::MakeCurrent(){
	if(!IsCurrent())
		wglMakeCurrent(DeviceContext, OpenGLContext);
}

bool UOpenGLRenderDevice::IsCurrent(){
	return OpenGLContext != NULL && wglGetCurrentContext() == OpenGLContext;
}

void UOpenGLRenderDevice::UnSetRes(){
	guardFunc;

	if(OpenGLContext){
		MakeCurrent();
		RenderInterface.Exit();
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

static bool IsMatrixShaderConstant(BYTE ConstantType){
	return ConstantType == EVC_WorldToScreenMatrix ||
	       ConstantType == EVC_ObjectToScreenMatrix ||
	       ConstantType == EVC_ObjectToWorldMatrix ||
	       ConstantType == EVC_CameraToWorldMatrix ||
	       ConstantType == EVC_WorldToCameraMatrix ||
	       ConstantType == EVC_WorldToObjectMatrix ||
	       ConstantType == EVC_ObjectToCameraMatrix;
}

FShaderGLSL* UOpenGLRenderDevice::GetShader(UHardwareShader* HardwareShader){
	FShaderGLSL* Shader = GLShaderByHardwareShader.Find(HardwareShader);

	if(!Shader){
		Shader = &GLShaderByHardwareShader[HardwareShader];
		// Explicitly create object because TMap does not call the constructor!!!
		*Shader = FShaderGLSL(FStringTemp(HardwareShader->GetPathName()).Substitute(".", "\\").Substitute(".", "\\"));

		// Cache number of vertex and pixel shader constants

		for(INT i = MAX_VERTEX_SHADER_CONSTANTS - 1; i >= 0; --i){
			if(HardwareShader->VSConstants[i].Type != EVC_Unused){
				HardwareShader->NumVSConstants = i + 1;

				if(IsMatrixShaderConstant(HardwareShader->VSConstants[i].Type))
					HardwareShader->NumVSConstants += 3;

				break;
			}
		}

		for(INT i = MAX_PIXEL_SHADER_CONSTANTS - 1; i >= 0; --i){
			if(HardwareShader->PSConstants[i].Type != EVC_Unused){
				HardwareShader->NumPSConstants = i + 1;

				if(IsMatrixShaderConstant(HardwareShader->PSConstants[i].Type))
					HardwareShader->NumPSConstants += 3;

				break;
			}
		}

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
	debugf("GL CALLBACK: type = 0x%x, severity = 0x%x, message = %s", type, severity, message);

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
	UBOOL Was16Bit = Use16bit;

	if(ColorBytes == 2)
		Use16bit = 1;
	else
		Use16bit = 0;

	// Create new context if there isn't one already or if the desired color depth has changed.
	if(!OpenGLContext || Was16Bit != Use16bit){
		Flush(Viewport);
		UnSetRes();

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
		glVertexAttrib4f(FVF_Diffuse,   0.0f, 0.0f, 0.0f, 1.0f);
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

	// Set window size
	if(Fullscreen){
		HMONITOR    Monitor = MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY);
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
	Flush(Viewport);
	UnSetRes();
	GIsOpenGL = 0;
}

void UOpenGLRenderDevice::Flush(UViewport* Viewport){
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

	DynamicIndexBuffer32 = NULL;
	DynamicIndexBuffer16 = NULL;
	DynamicVertexStream  = NULL;
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
	RenderCaps.MaxSimultaneousTerrainLayers = 1;
	RenderCaps.PixelShaderVersion = 0;
	RenderCaps.HardwareTL = 1;

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

	for(TMap<FString, FString>::TIterator It(HardwareShaderMacroText); It; ++It)
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
	"float calculate_fog(float Dist){ return 1 - saturate((FogEnd - Dist) / (FogEnd - FogStart)); }\n\n");

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
	"vec4 apply_fog(vec4 BaseColor){ return vec4(mix(BaseColor.rgb, FogColor.rgb, Fog * BaseColor.a), BaseColor.a); }\n\n");

#define FIXED_FUNCTION_UNIFORMS \
	"// Shader specific uniforms\n\n" \
	"layout(location = 0)  uniform int NumStages;\n" \
	"layout(location = 1)  uniform int StageTexCoordCount[8];\n" \
	"layout(location = 9)  uniform int StageTexCoordSources[8];\n" \
	"layout(location = 17) uniform mat4 StageTexMatrices[8];\n" \
	"layout(location = 25) uniform int StageColorArgs[16];\n" \
	"layout(location = 41) uniform int StageColorOps[8];\n" \
	"layout(location = 49) uniform int StageAlphaArgs[16];\n" \
	"layout(location = 65) uniform int StageAlphaOps[8];\n" \
	"layout(location = 73) uniform vec4 ConstantColor;\n" \
	"layout(location = 74) uniform bool LightingEnabled;\n" \
	"layout(location = 75) uniform float LightFactor;\n\n"

FString UOpenGLRenderDevice::FixedFunctionVertexShaderText(
	FIXED_FUNCTION_UNIFORMS
	"out vec4 StageTexCoords[8];\n"
	"\n"
	"#define TCS_Stream0                         0\n"
	"#define TCS_Stream1                         1\n"
	"#define TCS_Stream2                         2\n"
	"#define TCS_Stream3                         3\n"
	"#define TCS_Stream4                         4\n"
	"#define TCS_Stream5                         5\n"
	"#define TCS_Stream6                         6\n"
	"#define TCS_Stream7                         7\n"
	"#define TCS_WorldCoords                     8\n"
	"#define TCS_CameraCoords                    9\n"
	"#define TCS_CubeWorldSpaceReflection        10\n"
	"#define TCS_CubeCameraSpaceReflection       11\n"
	"#define TCS_ProjectorCoords                 12\n"
	"#define TCS_NoChange                        13\n"
	"#define TCS_SphereWorldSpaceReflection      14\n"
	"#define TCS_SphereCameraSpaceReflection     15\n"
	"#define TCS_CubeWorldSpaceNormal            16\n"
	"#define TCS_CubeCameraSpaceNormal           17\n"
	"#define TCS_SphereWorldSpaceNormal          18\n"
	"#define TCS_SphereCameraSpaceNormal         19\n"
	"#define TCS_BumpSphereCameraSpaceNormal     20\n"
	"#define TCS_BumpSphereCameraSpaceReflection 21\n"
	"\n"
	"vec4 tex_coord_source(int Source){\n"
		"\tswitch(Source){\n"
		"\tcase TCS_Stream0:\n"
			"\t\treturn InTexCoord0;\n"
		"\tcase TCS_Stream1:\n"
			"\t\treturn InTexCoord1;\n"
		"\tcase TCS_Stream2:\n"
			"\t\treturn InTexCoord2;\n"
		"\tcase TCS_Stream3:\n"
			"\t\treturn InTexCoord3;\n"
		"\tcase TCS_Stream4:\n"
			"\t\treturn InTexCoord4;\n"
		"\tcase TCS_Stream5:\n"
			"\t\treturn InTexCoord5;\n"
		"\tcase TCS_Stream6:\n"
			"\t\treturn InTexCoord6;\n"
		"\tcase TCS_Stream7:\n"
			"\t\treturn InTexCoord7;\n"
		"\tcase TCS_WorldCoords:\n"
		"\tcase TCS_CameraCoords:\n"
			"\t\treturn vec4(Position, 1.0);\n"
		"\tcase TCS_CubeWorldSpaceReflection:\n"
		"\tcase TCS_CubeCameraSpaceReflection:\n"
			"\t\treturn vec4(reflect(normalize(Position - CameraToWorld[3].xyz), normalize(Normal)), 1.0);\n"
		"\tcase TCS_ProjectorCoords:\n"
			"\t\treturn vec4(Normal, 1.0);\n"
		"\t}\n"
		"\n"
		"\treturn vec4(0.0, 0.0, 1.0, 1.0);\n"
	"}\n"
	"\n"
	"void main(void){\n"
		"\tPosition = (LocalToWorld * vec4(InPosition.xyz, 1.0)).xyz;\n"
		"\tNormal = (LocalToWorld * vec4(InNormal.xyz, 0.0)).xyz;\n"
		"\tDiffuse = InDiffuse;\n"
		"\tSpecular = InSpecular;\n"
		"\tFog = calculate_fog((LocalToCamera * vec4(InPosition.xyz, 1.0)).z);\n"
		"\n"
		"\tfor(int i = 0; i < NumStages; ++i){\n"
			"\t\tswitch(StageTexCoordCount[i]){\n"
			"\t\tcase 2:\n"
				"\t\t\tStageTexCoords[i] = StageTexMatrices[i] * vec4(tex_coord_source(StageTexCoordSources[i]).xy, 1.0, 1.0);\n"
				"\t\t\tbreak;\n"
			"\t\tcase 3:\n"
				"\t\t\tStageTexCoords[i] = StageTexMatrices[i] * vec4(tex_coord_source(StageTexCoordSources[i]).xyz, 1.0);\n"
				"\t\t\tbreak;\n"
			"\t\tcase 4:\n"
				"\t\t\tStageTexCoords[i] = StageTexMatrices[i] * tex_coord_source(StageTexCoordSources[i]);\n"
			"\t\t}\n"
		"\t}\n"
		"\n"
		"\tgl_Position = LocalToScreen * vec4(InPosition.xyz, 1.0);\n"
	"}\n");

FString UOpenGLRenderDevice::FixedFunctionFragmentShaderText(
	FIXED_FUNCTION_UNIFORMS
	"in vec4 StageTexCoords[8];\n"
	"\n"
	"vec4  Temp1;\n"
	"vec4  Temp2;\n"
	"float LightInfluence = 1.0; // For self-illumination\n"
	"\n"
	"// EColorArg\n"
	"\n"
	"#define CA_Diffuse  0\n"
	"#define CA_Constant 1\n"
	"#define CA_Previous 2\n"
	"#define CA_Temp1    3\n"
	"#define CA_Temp2    4\n"
	"#define CA_Texture0 5\n"
	"#define CA_Texture1 6\n"
	"#define CA_Texture2 7\n"
	"#define CA_Texture3 8\n"
	"#define CA_Texture4 9\n"
	"#define CA_Texture5 10\n"
	"#define CA_Texture6 11\n"
	"#define CA_Texture7 12\n"
	"\n"
	"// EColorArgModifier\n"
	"\n"
	"#define CAM_Invert (1 << 31)\n"
	"\n"
	"vec4 color_arg(int ColorArg, int StageIndex){\n"
	"	vec4 Result;\n"
	"\n"
	"	switch(ColorArg & 0xFF){\n"
	"	case CA_Diffuse:\n"
	"		Result = Diffuse;\n"
	"		break;\n"
	"	case CA_Constant:\n"
	"		Result = ConstantColor;\n"
	"		break;\n"
	"	case CA_Previous:\n"
	"		Result = FragColor;\n"
	"		break;\n"
	"	case CA_Temp1:\n"
	"		Result = Temp1;\n"
	"		break;\n"
	"	case CA_Temp2:\n"
	"		Result = Temp2;\n"
	"		break;\n"
	"	case CA_Texture0:\n"
	"		Result = sample_texture0(StageTexCoords[StageIndex]) * GlobalColor;\n"
	"		break;\n"
	"	case CA_Texture1:\n"
	"		Result = sample_texture1(StageTexCoords[StageIndex]) * GlobalColor;\n"
	"		break;\n"
	"	case CA_Texture2:\n"
	"		Result = sample_texture2(StageTexCoords[StageIndex]) * GlobalColor;\n"
	"		break;\n"
	"	case CA_Texture3:\n"
	"		Result = sample_texture3(StageTexCoords[StageIndex]) * GlobalColor;\n"
	"		break;\n"
	"	case CA_Texture4:\n"
	"		Result = sample_texture4(StageTexCoords[StageIndex]) * GlobalColor;\n"
	"		break;\n"
	"	case CA_Texture5:\n"
	"		Result = sample_texture5(StageTexCoords[StageIndex]) * GlobalColor;\n"
	"		break;\n"
	"	case CA_Texture6:\n"
	"		Result = sample_texture6(StageTexCoords[StageIndex]) * GlobalColor;\n"
	"		break;\n"
	"	case CA_Texture7:\n"
	"		Result = sample_texture7(StageTexCoords[StageIndex]) * GlobalColor;\n"
	"		break;\n"
	"	default:\n"
	"		Result = GlobalColor;\n"
	"	}\n"
	"\n"
	"	if((ColorArg & CAM_Invert) != 0)\n"
	"		Result = 1.0 - Result;\n"
	"\n"
	"	return Result;\n"
	"}\n"
	"\n"
	"// EColorOp\n"
	"\n"
	"#define COP_Arg1             0\n"
	"#define COP_Arg2             1\n"
	"#define COP_Modulate         2\n"
	"#define COP_Modulate2X       3\n"
	"#define COP_Modulate4X       4\n"
	"#define COP_Add              5\n"
	"#define COP_Subtract         6\n"
	"#define COP_AlphaBlend       7\n"
	"#define COP_AddAlphaModulate 8\n"
	"\n"
	"// EColorOpModifier\n"
	"\n"
	"#define COPM_SaveTemp1 (1 << 30)\n"
	"#define COPM_SaveTemp2 (1 << 31)\n"
	"\n"
	"vec4 color_op(int StageIndex, vec4 Arg1, vec4 Arg2){\n"
		"\tint ColorOp = StageColorOps[StageIndex];\n"
		"\tvec4 Result;\n"
		"\n"
		"\tswitch(ColorOp & 0xFF){\n"
		"\tcase COP_Arg1:\n"
			"\t\tResult = Arg1;\n"
			"\t\tbreak;\n"
		"\tcase COP_Arg2:\n"
			"\t\tResult = Arg2;\n"
			"\t\tbreak;\n"
		"\tcase COP_Modulate:\n"
			"\t\tResult = Arg1 * Arg2;\n"
			"\t\tbreak;\n"
		"\tcase COP_Modulate2X:\n"
			"\t\tResult = Arg1 * Arg2 * 2.0;\n"
			"\t\tbreak;\n"
		"\tcase COP_Modulate4X:\n"
			"\t\tResult = Arg1 * Arg2 * 4.0;\n"
			"\t\tbreak;\n"
		"\tcase COP_Add:\n"
			"\t\tResult = Arg1 + Arg2;\n"
			"\t\tbreak;\n"
		"\tcase COP_Subtract:\n"
			"\t\tResult = Arg1 - Arg2;\n"
			"\t\tbreak;\n"
		"\tcase COP_AlphaBlend:\n"
			"\t\tResult = mix(Arg1, Arg2, Arg1.a);\n"
			"\t\tbreak;\n"
		"\tcase COP_AddAlphaModulate:\n"
			"\t\tResult = Arg1 + Arg2 * Arg1.a;\n"
			"\t\tbreak;\n"
		"\tdefault:\n"
			"\t\tResult = vec4(1.0, 1.0, 1.0, 1.0);\n"
		"\t}\n"
		"\n"
		"\tif((ColorOp & COPM_SaveTemp1) != 0)\n"
			"\t\tTemp1 = Result;\n"
		"\n"
		"\tif((ColorOp & COPM_SaveTemp2) != 0)\n"
			"\t\tTemp2 = Result;\n"
		"\n"
		"\n"
		"\treturn Result;\n"
	"}\n"
	"\n"
	"// EAlphaOp\n"
	"\n"
	"#define AOP_Arg1     0\n"
	"#define AOP_Arg2     1\n"
	"#define AOP_Modulate 2\n"
	"#define AOP_Add      3\n"
	"#define AOP_Blend    4\n"
	"\n"
	"// EAlphaOpModifier\n"
	"\n"
	"#define AOPM_LightInfluence (1 << 31)\n"
	"\n"
	"float alpha_op(int StageIndex, float Arg1, float Arg2){\n"
		"\tint AlphaOp = StageAlphaOps[StageIndex];\n"
		"\tfloat Result;\n"
		"\n"
		"\tswitch(AlphaOp & 0xFF){\n"
		"\tcase AOP_Arg1:\n"
			"\t\tResult = Arg1;\n"
			"\t\tbreak;\n"
		"\tcase AOP_Arg2:\n"
			"\t\tResult = Arg2;\n"
			"\t\tbreak;\n"
		"\tcase AOP_Modulate:\n"
			"\t\tResult = Arg1 * Arg2;\n"
			"\t\tbreak;\n"
		"\tcase AOP_Add:\n"
			"\t\tResult = Arg1 + Arg2;\n"
			"\t\tbreak;\n"
		"\tcase AOP_Blend:\n"
			"\t\tResult = mix(Arg1, Arg2, Arg2);\n"
			"\t\tbreak;\n"
		"\tdefault:\n"
			"\t\tResult = 1.0;\n"
		"\t}\n"
		"\n"
		"\tif((AlphaOp & AOPM_LightInfluence) != 0)\n"
			"\t\tLightInfluence = 1.0 - Result;\n"
		"\n"
		"\treturn Result;\n"
	"}\n"
	"\n"
	"void shader_stage(int StageIndex){\n"
		"\tvec4 ColorArg1 = color_arg(StageColorArgs[StageIndex * 2], StageIndex);\n"
		"\tvec4 ColorArg2 = color_arg(StageColorArgs[StageIndex * 2 + 1], StageIndex);\n"
		"\n"
		"\tColorArg1.a = color_arg(StageAlphaArgs[StageIndex * 2], StageIndex).a;\n"
		"\tColorArg2.a = color_arg(StageAlphaArgs[StageIndex * 2 + 1], StageIndex).a;\n"
		"\n"
		"\tFragColor.a = alpha_op(StageIndex, ColorArg1.a, ColorArg2.a);\n"
		"\tFragColor.rgb = color_op(StageIndex, ColorArg1, ColorArg2).rgb;\n"
	"}\n"
	"\n"
	"// Lighting\n"
	"\n"
	"vec4 light_color(){\n"
		"\tvec4 LightColor = Diffuse + AmbientLightColor;\n"
		"\tvec3 NormalizedNormal = normalize(Normal);\n"
		"\n"
		"\tfor(int i = 0; i < NumLights; ++i){\n"
			"\t\tif(Lights[i].Type == 0){\n"
				"\t\t\tLightColor += Lights[i].Color * max(dot(Lights[i].Direction.xyz, NormalizedNormal), 0.0);\n"
			"\t\t}else{\n"
				"\t\t\tvec3 Dir = Lights[i].Position.xyz - Position;\n"
				"\t\t\tfloat Dist = length(Dir);\n"
				"\n"
				"\t\t\tif(Dist <= Lights[i].Radius)\n"
					"\t\t\t\tLightColor += Lights[i].Color * max(dot(normalize(Dir), NormalizedNormal), 0.0) * (1.0 - Dist * Lights[i].InvRadius);\n"
			"\t\t}\n"
		"\t}\n"
		"\n"
		"\treturn LightColor * LightFactor * Specular;\n"
	"}\n"
	"\n"
	"// Main\n"
	"\n"
	"void main(void){\n"
		"\tFragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
		"\n"
		"\tfor(int i = 0; i < NumStages; ++i)\n"
			"\t\tshader_stage(i);\n"
		"\n"
		"\talpha_test(FragColor);\n"
		"\n"
		"\tif(LightingEnabled)\n"
			"\t\tFragColor.rgb = mix(FragColor.rgb, FragColor.rgb * light_color().rgb, LightInfluence);\n"
		"\n"
		"\tif(FogEnabled)\n"
			"\t\tFragColor = apply_fog(FragColor);\n"
	"}\n");

#undef UNIFORM_STRUCT_MEMBER
#undef UNIFORM_BLOCK_MEMBER
#undef SHADER_HEADER
