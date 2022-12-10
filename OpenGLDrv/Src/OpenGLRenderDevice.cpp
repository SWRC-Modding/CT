#include "OpenGLRenderDevice.h"
#include "OpenGLResource.h"

#define MIN_OPENGL_MAJOR_VERSION 4
#define MIN_OPENGL_MINOR_VERSION 5

IMPLEMENT_PACKAGE(OpenGLDrv)
IMPLEMENT_CLASS(UOpenGLRenderDevice)

UOpenGLRenderDevice::UOpenGLRenderDevice() : ErrorShader(this),
                                             RenderInterface(this),
                                             Backbuffer(0, 0, TEXF_RGBA8, false, false){}

void UOpenGLRenderDevice::StaticConstructor(){
	SupportsCubemaps       = 1;
	SupportsZBIAS          = 1;
	CanDoDistortionEffects = 1;
	bBilinearFramebuffer   = 1;
	TextureFilter          = TF_Trilinear;
	TextureAnisotropy      = 16;
	bFirstRun              = 1;
	ShaderDir              = FStringTemp("OpenGLShaders");

	new(GetClass(), "DistortionEffects",      RF_Public) UBoolProperty(CPP_PROPERTY(CanDoDistortionEffects),  "Options", CPF_Config);
	new(GetClass(), "UseDesktopResolution",   RF_Public) UBoolProperty(CPP_PROPERTY(bUseDesktopResolution),   "Options", CPF_Config);
	new(GetClass(), "KeepAspectRatio",        RF_Public) UBoolProperty(CPP_PROPERTY(bKeepAspectRatio),        "Options", CPF_Config);
	new(GetClass(), "BilinearFramebuffer",    RF_Public) UBoolProperty(CPP_PROPERTY(bBilinearFramebuffer),    "Options", CPF_Config);
	new(GetClass(), "SaveShadersToDisk",      RF_Public) UBoolProperty(CPP_PROPERTY(bSaveShadersToDisk),      "Options", CPF_Config);
	new(GetClass(), "AutoReloadShaders",      RF_Public) UBoolProperty(CPP_PROPERTY(bAutoReloadShaders),      "Options", CPF_Config);
	new(GetClass(), "UseTrilinear",           RF_Public) UBoolProperty(CPP_PROPERTY(bUseTrilinear),           "Options", CPF_Config);
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

const FOpenGLShader* UOpenGLRenderDevice::GetShaderForMaterial(UMaterial* Material){
	// HACK: The 30 bit padding space after UMaterial::Validated is used to store a flag, indicating whether the material has a shader or not and an index to retrieve it.

	if(Material->Validated){
		if(Material->UseFallback) // Material has a shader, so quickly look it up by index
			return &ShadersByMaterial.GetPairs()[reinterpret_cast<INT*>(Material)[24] >> 3].Value;

		return NULL;
	}

	Material->Validated = 1;

	if(Material->IsIn(UObject::GetTransientPackage()))
		return NULL;

	FString MaterialPathName = Material->GetPathName();
	FString ShaderPath = MaterialPathName.Substitute(".", "\\").Substitute(".", "\\");
	FString ShaderText;
	bool ShaderLoaded = LoadShaderIfChanged(ShaderPath, ShaderText);
	FOpenGLCachedShader* Shader = ShadersByMaterial.Find(*MaterialPathName);

	if(Material->IsA<UHardwareShader>()){
		// Generate GLSL from D3D shader if no shader exists yet for the material or the source file is empty or deleted.
		if((!Shader && !ShaderLoaded) || (ShaderLoaded && ShaderText.Len() == 0)){
			ShaderText = GLSLShaderFromD3DHardwareShader(static_cast<UHardwareShader*>(Material));

			if(bSaveShadersToDisk)
				SaveShader(ShaderPath, ShaderText);
		}
	}else{
		// Source file is empty or deleted, so free the shader and set the flag indicating that the material should use the shader generator instead.
		// TODO: Handle this like the hardware shaders and write the generated shader code to the file in that case instead of using the flag.
		if(Shader && ShaderLoaded && ShaderText.Len() == 0){
			Shader->Free();
			Shader = NULL;
			reinterpret_cast<INT*>(Material)[24] = (reinterpret_cast<INT*>(Material)[24] & 0x3) | 0x4;
		}else if(!ShaderLoaded && reinterpret_cast<INT*>(Material)[24] & 0x4){ // The material previously used a shader but now doesn't anymore so return NULL
			Shader = NULL;
		}
	}

	if(ShaderText.Len() > 0){
		if(!Shader){
			Shader = &ShadersByMaterial[*MaterialPathName];
			Shader->RenDev = this;
			Shader->Index = ShadersByMaterial.Num() - 1;
		}

		Shader->Compile(*ShaderText, *MaterialPathName);
		reinterpret_cast<INT*>(Material)[24] = (reinterpret_cast<INT*>(Material)[24] & 0x3) | (Shader->Index << 3);
	}

	Material->UseFallback = Shader != NULL;

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
	if(ParseCommand(&Cmd, "OPENGL")){
		if(ParseCommand(&Cmd, "RELOADSHADERS")){
			Ar.Log("Reloading shaders from disk");

			if(LoadShaderMacroText()) // Load the macros here. the shaders themselves are loaded on demand.
				ShaderFileTimes.Empty(); // Macros may have changed, so force reload all shaders

			UMaterial::ClearFallbacks();

			return 1;
		}else if(ParseCommand(&Cmd, "AUTORELOADSHADERS")){
			bAutoReloadShaders = !bAutoReloadShaders;

			if(bAutoReloadShaders)
				Ar.Log("Automatic shader reloading enabled");
			else
				Ar.Log("Automatic shader reloading disabled");

			return 1;
		}
	}

	return 0;
}

static OPENGL_MESSAGE_CALLBACK(OpenGLMessageCallback){
	const char* SourceStr;

	switch(Source){
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

	switch(Type){
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

	switch(Severity){
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

	if(Severity != GL_DEBUG_SEVERITY_NOTIFICATION || ((UOpenGLRenderDevice*)UserParam)->bShowDebugNotifications)
		debugf("GL CALLBACK: source=%s, type=%s, severity=%s - %s", SourceStr, TypeStr, SeverityStr, Message);

	if(Type == GL_DEBUG_TYPE_ERROR){
		GLog->Flush();

		if(appIsDebuggerPresent())
			appDebugBreak();
	}
}

UBOOL UOpenGLRenderDevice::Init(){
	// Init SWRCFix if it exists. Hacky but RenderDevice is always loaded at startup...
	HMODULE ModDLL = LoadLibraryA("Mod.dll");

	if(ModDLL){
		void(CDECL*InitSWRCFix)(void) = reinterpret_cast<void(CDECL*)(void)>(GetProcAddress(ModDLL, "InitSWRCFix"));

		if(InitSWRCFix)
			InitSWRCFix();
	}

	// NOTE: This must be set to 0 in order to avoid inconsistencies with RGBA and BGRA colors.
	GIsOpenGL = 0;

	// Load wgl and basic opengl functions
	checkSlow(!OpenGL32Dll);

	OpenGL32Dll = LoadLibraryA("opengl32.dll");

	if(!OpenGL32Dll)
		appErrorf("Unable to load opengl32.dll");

#define WGL_FUNC(name, ret, args) \
	wgl ## name = reinterpret_cast<ret(OPENGL_CALL*)args>(GetProcAddress(OpenGL32Dll, "wgl" #name)); \
	if(!wgl ## name) \
		appErrorf("Unable to load function '%s' from opengl32.dll", "wgl" #name);
	WGL_BASE_FUNCS
#undef WGL_FUNC

#define GL_FUNC(name, ret, args) \
	gl ## name = reinterpret_cast<ret(OPENGL_CALL*)args>(GetProcAddress(OpenGL32Dll, "gl" #name)); \
	if(!gl ## name) \
		appErrorf("Unable to load function '%s' from opengl32.dll", "gl" #name);
	GL_BASE_FUNCS
#undef GL_FUNC

	// Initialize hardware shader macros

	SetHardwareShaderMacros(CastChecked<UHardwareShaderMacros>(GEngine->HBumpShaderMacros));
	LoadShaderMacroText();

	if(bSaveShadersToDisk)
		SaveShaderMacroText();

	if(bUseTrilinear)
		TextureFilter = TF_Trilinear;

	foreachobj(UMaterial, It){
		It->UseFallback = 0;
		It->Validated = 0;
		reinterpret_cast<INT*>(*It)[24] &= 0x3; // Clear 30 bit padding after UMaterial::Validated that is used by the OpenGL renderer
	}

	checkSlow(UTexture::__Client);
	appMemcpy(TextureLODSet, UTexture::__Client->TextureLODSet, LODSET_MAX);

	// Disable squad shadows for now since there's an unexplained crash in USkeletalMeshInstance::ApplyAnimation if they're enabled
	GEngine->Exec("SET WINDOWSCLIENT SHADOWS FALSE", *GWarn);

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

		verify(wglMakeCurrent(DeviceContext, TempContext));

		LoadWGLExtFuncs();

		if(wglCreateContextAttribsARB){
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
				wglDeleteContext(TempContext);
				LoadWGLExtFuncs();
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

		LoadGLFuncs();

		if(bDebugOpenGL){
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(OpenGLMessageCallback, this);
		}

		// Check for extensions

		if(wglGetExtensionsStringARB){
			const char* WGLExtensions = wglGetExtensionsStringARB(DeviceContext);

			if(WGLExtensions)
				SupportsWGLSwapIntervalTear = appStrstr(reinterpret_cast<const TCHAR*>(WGLExtensions), "WGL_EXT_swap_control_tear") != NULL;
		}

		GLint NumExtensions = 0;
		UBOOL SupportsDXTCompression = 0;

		glGetIntegerv(GL_NUM_EXTENSIONS, &NumExtensions);

		for(GLint i = 0; i < NumExtensions; ++i){
			const TCHAR* Extension = reinterpret_cast<const TCHAR*>(glGetStringi(GL_EXTENSIONS, i));

			if(!SupportsEXTFilterAnisotropic &&
			   (appStrcmp(Extension, "GL_ARB_texture_filter_anisotropic") == 0 || appStrcmp(Extension, "GL_EXT_texture_filter_anisotropic") == 0)){
				SupportsEXTFilterAnisotropic = 1;
			}else if(!SupportsDXTCompression && appStrcmp(Extension, "GL_EXT_texture_compression_s3tc") == 0){
				SupportsDXTCompression = 1;
			}
		}

		if(!SupportsDXTCompression)
			appErrorf("OpenGL driver does not support required extension for DXT compression: GL_EXT_texture_compression_s3tc");

		LoadGLExtFuncs();

		// Initialize render interface

		RenderInterface.Init(NewX, NewY);
		EnableVSync(bVSync != 0);

		// Set default values for unspecified vertex attributes

		glVertexAttrib4f(FVF_Position,  0.0f, 0.0f, 0.0f, 0.0f);
		glVertexAttrib4f(FVF_Normal,    0.0f, 0.0f, 0.0f, 0.0f);
		glVertexAttrib4f(FVF_Diffuse,   0.0f, 0.0f, 0.0f, 0.0f);
		glVertexAttrib4f(FVF_Specular,  1.0f, 1.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord0, 0.0f, 0.0f, 0.0f, 0.0f);
		glVertexAttrib4f(FVF_TexCoord1, 0.0f, 0.0f, 0.0f, 0.0f);
		glVertexAttrib4f(FVF_TexCoord2, 0.0f, 0.0f, 0.0f, 0.0f);
		glVertexAttrib4f(FVF_TexCoord3, 0.0f, 0.0f, 0.0f, 0.0f);
		glVertexAttrib4f(FVF_TexCoord4, 0.0f, 0.0f, 0.0f, 0.0f);
		glVertexAttrib4f(FVF_TexCoord5, 0.0f, 0.0f, 0.0f, 0.0f);
		glVertexAttrib4f(FVF_TexCoord6, 0.0f, 0.0f, 0.0f, 0.0f);
		glVertexAttrib4f(FVF_TexCoord7, 0.0f, 0.0f, 0.0f, 0.0f);
		glVertexAttrib4f(FVF_Tangent,   0.0f, 0.0f, 0.0f, 0.0f);
		glVertexAttrib4f(FVF_Binormal,  0.0f, 0.0f, 0.0f, 0.0f);

		// Initialize shaders

		ErrorShader.Compile("#ifdef VERTEX_SHADER\n"
		                    "void main(void){\n"
		                    "\tgl_Position = LocalToScreen * vec4(InPosition.xyz, 1.0);\n"
		                    "}\n"
		                    "#elif defined(FRAGMENT_SHADER)\n"
		                    "void main(void){\n"
		                    "\tFragColor = vec4(1.0, 0.0, 1.0, 1.0);\n"
		                    "}\n"
		                    "#else\n"
		                    "#error Shader type not implemented\n"
		                    "#endif\n",
		                    "ERRORSHADER");
		check(ErrorShader.IsValid());

		/*
		 * Mirror image since the FrameFx render targets expect d3d coordinates
		 * This means that glBlitNamedFramebuffer needs to be called with y and height exchanged in UOpenGLRenderDevice::Present
		 */
		glClipControl(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
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

	checkSlow(OpenGL32Dll);
	FreeLibrary(OpenGL32Dll);
	OpenGL32Dll = NULL;
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

	DynamicIndexBuffer  = NULL;
	DynamicVertexStream = NULL;

	UMaterial::ClearFallbacks();
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
	if(wglSwapIntervalEXT){
		bVSync = bEnable;

		INT Interval;

		if(bEnable)
			Interval = bAdaptiveVSync && SupportsWGLSwapIntervalTear ? -1 : 1;
		else
			Interval = 0;

		wglSwapIntervalEXT(Interval);
	}else{
		bVSync = 0;
	}
}

FRenderInterface* UOpenGLRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize){
	MakeCurrent();

	if(bAutoReloadShaders){
		if(LoadShaderMacroText())
			ShaderFileTimes.Empty(); // Macros may have changed, so force reload all shaders

		UMaterial::ClearFallbacks();
	}

	BYTE* ClientTextureLODSet = UTexture::__Client->TextureLODSet;

	if(appMemcmp(TextureLODSet, ClientTextureLODSet, LODSET_MAX) != 0){
		appMemcpy(TextureLODSet, ClientTextureLODSet, LODSET_MAX);
		Flush(Viewport);
	}

	RenderInterface.Locked(Viewport);
	RenderInterface.SetRenderTarget(&Backbuffer, false);

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
		FPlane Black(0.0f, 0.0f, 0.0f, 1.0f);
		glClearNamedFramebufferfv(GL_NONE, GL_COLOR, GL_NONE, reinterpret_cast<GLfloat*>(&Black));

		INT FramebufferWidth = Viewport->SizeX;
		INT FramebufferHeight = Viewport->SizeY;
		INT ScreenWidth = SavedViewportWidth;
		INT ScreenHeight = SavedViewportHeight;

		FLOAT ScreenAspectRatio = static_cast<FLOAT>(ScreenWidth) / ScreenHeight;
		FLOAT FramebufferAspectRatio = static_cast<FLOAT>(FramebufferWidth) / FramebufferHeight;

		if(FramebufferAspectRatio < ScreenAspectRatio){ // Vertical black bars
			ViewportWidth = FramebufferWidth * (static_cast<FLOAT>(ScreenHeight) / FramebufferHeight);
			ViewportHeight = ScreenHeight;
		}else{ // Horizontal black bars
			ViewportWidth = ScreenWidth;
			ViewportHeight = FramebufferHeight * (static_cast<FLOAT>(ScreenWidth) / FramebufferWidth);
		}

		ViewportX = (ScreenWidth - ViewportWidth) / 2;
		ViewportY = (ScreenHeight - ViewportHeight) / 2;
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
		                       ViewportX, ViewportY + ViewportHeight, ViewportX + ViewportWidth, ViewportY,
		                       GL_COLOR_BUFFER_BIT,
		                       bBilinearFramebuffer ? GL_LINEAR : GL_NEAREST);
		SwapBuffers(DeviceContext);
	}

	checkSlow(glGetError() == GL_NO_ERROR);
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
	static FRenderCaps RenderCaps(4, 14, 1);

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

FStringTemp UOpenGLRenderDevice::MakeShaderFilename(const FString& ShaderName, const TCHAR* Extension){
	return FStringTemp(appBaseDir()) * ShaderDir * ShaderName + Extension;
}

bool UOpenGLRenderDevice::ShaderFileNeedsReload(const char* Filename){
	SQWORD CurrentFileTime = GFileManager->GetGlobalTime(Filename);
	SQWORD PreviousFileTime = ShaderFileTimes[Filename];

	return CurrentFileTime != PreviousFileTime;
}

bool UOpenGLRenderDevice::LoadShaderIfChanged(const FString& Name, FString& Out){
	FFilename Filename = MakeShaderFilename(Name, SHADER_FILE_EXTENSION);

	// If macros have updated the shader code needs to be reloaded even if the file wasn't updated since it might use some macros
	if(ShaderFileNeedsReload(*Filename))
		return LoadShaderText(Filename, &Out);

	return false;
}

void UOpenGLRenderDevice::SaveShader(const FString& Name, const FString& Text){
	SaveShaderText(MakeShaderFilename(Name, SHADER_FILE_EXTENSION), Text);
}

bool UOpenGLRenderDevice::LoadShaderMacroText(){
	FFilename Filename = MakeShaderFilename("Macros", SHADER_MACROS_FILE_EXTENSION);
	SQWORD CurrentFileTime = GFileManager->GetGlobalTime(*Filename);

	if(CurrentFileTime == ShaderMacroFileTime)
		return false;

	FString Macros;

	if(appLoadFileToString(Macros, *Filename)){
		ShaderMacroFileTime = CurrentFileTime;
		debugf("Loaded %s", *Filename.GetCleanFilename());
		ParseGLSLMacros(Macros);

		return true;
	}

	// Reset file time in case the file was deleted
	if(GFileManager->FileSize(*Filename) < 0){
		ShaderMacroFileTime = 0;

		return true;
	}

	return false;
}

void UOpenGLRenderDevice::SaveShaderMacroText(){
	FString Macros;

	for(TMap<FString, FString>::TIterator It(ShaderMacros); It; ++It){
		FString MacroText = It.Value();

		// Trim leading empty lines
		for(INT i = 0; i < MacroText.Len(); ++i){
			if(!appIsSpace(MacroText[i])){
				while(i > 0 && MacroText[i - 1] != '\n') // Preserve leading whitespace on first line
					--i;

				MacroText = MacroText.Right(MacroText.Len() - i);
				break;
			}
		}

		// Trim trailing empty lines
		for(INT i = MacroText.Len() - 1; i >= 0; --i){
			if(!appIsSpace(MacroText[i])){
				MacroText = MacroText.Left(i + 1);
				break;
			}
		}

		Macros += "@" + It.Key() + "\n" + MacroText + "\n\n";
	}

	SaveShaderText(MakeShaderFilename("Macros", SHADER_MACROS_FILE_EXTENSION), Macros);
}

bool UOpenGLRenderDevice::LoadShaderText(const FFilename& Filename, FString* Out){
	if(appLoadFileToString(*Out, *Filename)){
		ShaderFileTimes[*Filename] = GFileManager->GetGlobalTime(*Filename);
		debugf("Loaded %s", *Filename.GetCleanFilename());

		return true;
	}

	// Reset file time in case the file was deleted
	if(GFileManager->FileSize(*Filename) < 0){
		ShaderFileTimes[*Filename] = 0;

		return true;
	}

	return false;
}

void UOpenGLRenderDevice::SaveShaderText(const FFilename& Filename, const FString& Text){
	GFileManager->MakeDirectory(*(Filename.GetPath() + "\\"), 1);

	if(appSaveStringToFile(Text, *Filename, GFileManager))
		ShaderFileTimes[*Filename] = GFileManager->GetGlobalTime(*Filename);
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
			FLOAT ScreenAspectRatio = static_cast<FLOAT>(ScreenWidth) / ScreenHeight;
			FLOAT FramebufferAspectRatio = static_cast<FLOAT>(FramebufferWidth) / FramebufferHeight;

			if(FramebufferAspectRatio < ScreenAspectRatio){
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

void UOpenGLRenderDevice::LoadWGLExtFuncs(){
#define WGL_FUNC(name, ret, args) \
	wgl ## name = reinterpret_cast<ret(OPENGL_CALL*)args>(wglGetProcAddress("wgl" #name));
	WGL_EXT_FUNCS
#undef WGL_FUNC
}

void UOpenGLRenderDevice::LoadGLFuncs(){
#define GL_FUNC(name, ret, args) \
	gl ## name = reinterpret_cast<ret(OPENGL_CALL*)args>(wglGetProcAddress("gl" #name)); \
	if(!gl ## name) \
		appErrorf("Unable to load required opengl function '%s'", "gl" # name);
	GL_FUNCS
#undef GL_FUNC
}

void UOpenGLRenderDevice::LoadGLExtFuncs(){
#define GL_FUNC(name, ret, args) \
	gl ## name = reinterpret_cast<ret(OPENGL_CALL*)args>(wglGetProcAddress("gl" #name));
	GL_EXT_FUNCS
#undef GL_FUNC
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
"#define VERTEX_SHADER\n\n"
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
"#define FRAGMENT_SHADER\n\n"
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
"\t\t\tResult = texture(Texture ## n, Coords.xy * TextureInfos[n].UVScale); \\\n"
"\t\telse \\\n"
"\t\t\tResult = texture(Cubemap ## n, Coords.xyz * TextureInfos[n].UVScale); \\\n"
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
"vec4 apply_fog(vec4 BaseColor){ return vec4(mix(FogColor.rgb, BaseColor.rgb, Fog), BaseColor.a); }\n\n");
