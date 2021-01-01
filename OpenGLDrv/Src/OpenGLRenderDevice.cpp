#include "../Inc/OpenGLRenderDevice.h"

#include "GL/glew.h"
#include "GL/wglew.h"
#include "OpenGLResource.h"

#define MIN_OPENGL_MAJOR_VERSION 4
#define MIN_OPENGL_MINOR_VERSION 5

IMPLEMENT_CLASS(UOpenGLRenderDevice)

UOpenGLRenderDevice::UOpenGLRenderDevice() : RenderInterface(this),
                                             ScreenRenderTarget(0, 0, TEXF_RGBA8, false, true){}

void UOpenGLRenderDevice::StaticConstructor(){
	SupportsCubemaps = 1;
	SupportsZBIAS = 1;
	bFirstRun = 1;
	bFixCanvasScaling = 1;
	bUseOffscreenFramebuffer = 1;
	ShaderDir = "OpenGLShaders";

	new(GetClass(), "UseDesktopResolution", RF_Public) UBoolProperty(CPP_PROPERTY(bUseDesktopResolution), "Options", CPF_Config);
	new(GetClass(), "KeepAspectRatio", RF_Public) UBoolProperty(CPP_PROPERTY(bKeepAspectRatio), "Options", CPF_Config);
	new(GetClass(), "VSync", RF_Public) UBoolProperty(CPP_PROPERTY(bVSync), "Options", CPF_Config);
	new(GetClass(), "AdaptiveVSync", RF_Public) UBoolProperty(CPP_PROPERTY(bAdaptiveVSync), "Options", CPF_Config);
	new(GetClass(), "FirstRun", RF_Public) UBoolProperty(CPP_PROPERTY(bFirstRun), "", CPF_Config);
	new(GetClass(), "FixCanvasScaling", RF_Public) UBoolProperty(CPP_PROPERTY(bFixCanvasScaling), "", CPF_Config);
	new(GetClass(), "UseOffscreenFramebuffer", RF_Public) UBoolProperty(CPP_PROPERTY(bUseOffscreenFramebuffer), "", CPF_Config);
	new(GetClass(), "DebugOpenGL", RF_Public) UBoolProperty(CPP_PROPERTY(bDebugOpenGL), "", CPF_Config);
	new(GetClass(), "ShaderDir", RF_Public) UStrProperty(CPP_PROPERTY(ShaderDir), "", CPF_Config);
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

FShaderGLSL* UOpenGLRenderDevice::GetShader(UHardwareShader* HardwareShader){
	FShaderGLSL* Shader = GLShaderByHardwareShader.Find(HardwareShader);

	if(!Shader){
		Shader = &GLShaderByHardwareShader[HardwareShader];

		if(HardwareShader->GetFName() != NAME_InGameTempName){
			FString ShaderName = HardwareShader->GetPathName();

			Shader->SetName(ShaderName.Substitute(".", "\\").Substitute(".", "\\"));
		}

		Shader->SetVertexShaderText(HardwareShader->VertexShaderText);
		Shader->SetFragmentShaderText(HardwareShader->PixelShaderText);
		LoadShader(Shader);
	}

	return Shader;
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
					glVertexArrayAttribFormat(VAO, Function, GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE, Offset);
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

	if(type == GL_DEBUG_TYPE_ERROR && appIsDebuggerPresent())
		appDebugBreak();
}

typedef void(__fastcall*UCanvasUpdateFunc)(UCanvas*, DWORD);

UCanvasUpdateFunc UCanvasUpdateOriginal = NULL;

void __fastcall UCanvasUpdateOverride(UCanvas* Self, DWORD Edx){
	GIsOpenGL = 1;
	checkSlow(UCanvasUpdateOriginal);
	UCanvasUpdateOriginal(Self, Edx);
	GIsOpenGL = 0;
}

UBOOL UOpenGLRenderDevice::Init(){
	// NOTE: This must be set to 0 in order to avoid inconsistencies with RGBA and BGRA colors.
	GIsOpenGL = 0;

	/*
	 * Setting GIsOpenGL to 0 causes an issue with UCanvas where it doesn't properly fit the screen and fonts are rendered in lower resolution.
	 * This is fixed by patching the vtable with an override function that sets GIsOpenGL to 1 for the duration of the call to UCanvas::Update.
	 */
	if(bFixCanvasScaling && !UCanvasUpdateOriginal)
		UCanvasUpdateOriginal = static_cast<UCanvasUpdateFunc>(PatchDllClassVTable("Engine.dll", "UCanvas", NULL, 31, UCanvasUpdateOverride));

	FixedFunctionShader.SetName("FixedFunction");
	FixedFunctionShader.SetVertexShaderText(CommonShaderHeaderText + VertexShaderVarsText + FixedFunctionVertexShaderText);
	FixedFunctionShader.SetFragmentShaderText(CommonShaderHeaderText + FragmentShaderVarsText + FixedFunctionFragmentShaderText);
	FramebufferShader.SetName("Framebuffer");
	FramebufferShader.SetVertexShaderText(FramebufferVertexShaderText);
	FramebufferShader.SetFragmentShaderText(FramebufferFragmentShaderText);

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

		INT PfdFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		INT DepthBits = 24;
		INT StencilBits = UseStencil ? 8 : 0;

		if(bUseOffscreenFramebuffer){
			PfdFlags |= PFD_DEPTH_DONTCARE;
			DepthBits = 0;
			StencilBits = 0;
			debugf("Using offscreen framebuffer");
		}else{
			debugf("Not using offscreen framebuffer");

			if(bKeepAspectRatio)
				debugf("KeepAspectRatio is set to true but has no effect");
		}

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
			TArray<int> Attributes;

			Attributes.Add(WGL_CONTEXT_MAJOR_VERSION_ARB);
			Attributes.Add(MIN_OPENGL_MAJOR_VERSION);
			Attributes.Add(WGL_CONTEXT_MINOR_VERSION_ARB);
			Attributes.Add(MIN_OPENGL_MINOR_VERSION);
			Attributes.Add(WGL_CONTEXT_FLAGS_ARB);
			Attributes.Add(WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB);

			if(bDebugOpenGL){
				debugf("OpenGL debugging enabled");
				Attributes.Last() |= WGL_CONTEXT_DEBUG_BIT_ARB;
			}

			Attributes.Add(WGL_CONTEXT_PROFILE_MASK_ARB);
			Attributes.Add(WGL_CONTEXT_CORE_PROFILE_BIT_ARB);
			Attributes.Add(0);

			OpenGLContext = wglCreateContextAttribsARB(DeviceContext, NULL, Attributes.GetData());

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

		// Setup initial state

		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_POLYGON_OFFSET_FILL);

		RenderInterface.Init();
		RenderInterface.EnableZTest(1);
		RenderInterface.EnableZWrite(1);
		RenderInterface.SetStencilOp(CF_Always, 0x0, 0xFF, SO_Keep, SO_Keep, SO_Keep, 0xFF);
		RenderInterface.EnableStencilTest(UseStencil);
		RenderInterface.SetShader(&FixedFunctionShader);
		RenderInterface.SetCullMode(CM_CW);
		RenderInterface.SetFillMode(FM_Solid);
		EnableVSync(bVSync != 0);

		// Set default values for unspecified vertex attributes

		glVertexAttrib4f(FVF_Diffuse, 1.0f, 1.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_Specular, 1.0f, 1.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord0, 0.0f, 0.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord1, 0.0f, 0.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord2, 0.0f, 0.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord3, 0.0f, 0.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord4, 0.0f, 0.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord5, 0.0f, 0.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord6, 0.0f, 0.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord7, 0.0f, 0.0f, 1.0f, 1.0f);
	}else{
		MakeCurrent();
	}

	// Set window size
	if(Fullscreen){
		HMONITOR    Monitor = MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO Info    = {sizeof(MONITORINFO)};

		verify(GetMonitorInfoA(Monitor, &Info));

		INT Width  = Info.rcMonitor.right - Info.rcMonitor.left;
		INT Height = Info.rcMonitor.bottom - Info.rcMonitor.top;

		if(bUseDesktopResolution || !bUseOffscreenFramebuffer){
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

	if(bUseOffscreenFramebuffer){
		// Resize screen render target if necessary
		if(ScreenRenderTarget.Width != NewX || ScreenRenderTarget.Height != NewY){
			ScreenRenderTarget.Width = NewX;
			ScreenRenderTarget.Height = NewY;
			++ScreenRenderTarget.Revision;
		}

		RenderInterface.SetViewport(0, 0, NewX, NewY);
	}else{
		if(bIsFullscreen)
			RenderInterface.SetViewport(0, 0, SavedViewportWidth, SavedViewportHeight);
		else
			RenderInterface.SetViewport(0, 0, NewX, NewY);
	}


	return 1;

	unguard;
}

void UOpenGLRenderDevice::Exit(UViewport* Viewport){
	Flush(Viewport);
	UnSetRes();
}

void UOpenGLRenderDevice::Flush(UViewport* Viewport){
	RenderInterface.CurrentState->RenderTarget = NULL;
	RenderInterface.CurrentState->Shader = NULL;
	RenderInterface.CurrentState->IndexBuffer = NULL;
	RenderInterface.CurrentState->NumVertexStreams = 0;

	if(Viewport && Viewport->Actor)
		Viewport->Actor->FrameFX->FreeRenderTargets();

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

void UOpenGLRenderDevice::EnableVSync(bool bEnable){
	if(WGL_EXT_swap_control){
		bVSync = bEnable;

		INT Interval;

		if(bEnable){
			if(WGL_EXT_swap_control_tear && bAdaptiveVSync)
				Interval = -1;
			else
				Interval = 1;
		}else{
			Interval = 0;
		}

		wglSwapIntervalEXT(Interval);
	}else{
		bVSync = 0;
	}
}

FRenderInterface* UOpenGLRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize){
	check(RenderInterface.CurrentState == &RenderInterface.SavedStates[0]);

	MakeCurrent();

	RenderInterface.LockedViewport = Viewport;

	if(bUseOffscreenFramebuffer)
		RenderInterface.SetRenderTarget(&ScreenRenderTarget, false);

	RenderInterface.SetShader(&FixedFunctionShader); // TODO: Move to FOpenGLRenderInterface::SetMaterial
	RenderInterface.SetupPerFrameShaderConstants();
	RenderInterface.PushState();

	return &RenderInterface;
}

void UOpenGLRenderDevice::Unlock(FRenderInterface* RI){
	RI->PopState();
	RenderInterface.LockedViewport = NULL;
}

void UOpenGLRenderDevice::Present(UViewport* Viewport){
	checkSlow(IsCurrent());

	FOpenGLTexture* Framebuffer = RenderInterface.CurrentState->RenderTarget;

	if(Framebuffer){
		RenderInterface.PushState();
		RenderInterface.SetRenderTarget(NULL, false);

		INT FramebufferWidth = Framebuffer->Width;
		INT FramebufferHeight = Framebuffer->Height;
		INT ScreenWidth;
		INT ScreenHeight;

		if(bIsFullscreen){
			ScreenWidth  = SavedViewportWidth;
			ScreenHeight = SavedViewportHeight;
		}else{
			ScreenWidth  = Viewport->SizeX;
			ScreenHeight = Viewport->SizeY;
		}

		FLOAT XScale = 1.0f;
		FLOAT YScale = 1.0f;

		if(bKeepAspectRatio){
			FLOAT ViewportAspectRatio = static_cast<FLOAT>(ScreenWidth) / ScreenHeight;
			FLOAT FramebufferAspectRatio = static_cast<FLOAT>(FramebufferWidth) / FramebufferHeight;

			if(FramebufferAspectRatio < ViewportAspectRatio){
				FLOAT Scale = static_cast<FLOAT>(ScreenHeight) / FramebufferHeight;

				XScale = FramebufferWidth * Scale / ScreenWidth;
			}else{
				FLOAT Scale = static_cast<FLOAT>(ScreenWidth) / FramebufferWidth;

				YScale = FramebufferHeight * Scale / ScreenHeight;
			}

			// Clear black bars
			RenderInterface.Clear(1, FColor(0, 0, 0), 0, 0.0f, 0, 0);
		}

		INT ViewportWidth = static_cast<INT>(ScreenWidth * XScale);
		INT ViewportHeight = static_cast<INT>(ScreenHeight * YScale);

		RenderInterface.SetViewport(ScreenWidth / 2 - ViewportWidth / 2, ScreenHeight / 2 - ViewportHeight / 2, ViewportWidth, ViewportHeight);
		Framebuffer->BindTexture(0);
		RenderInterface.SetFillMode(FM_Solid);
		RenderInterface.EnableZTest(0);
		RenderInterface.EnableStencilTest(0);
		RenderInterface.SetShader(&FramebufferShader);
		RenderInterface.DrawPrimitive(PT_TriangleStrip, 0, 2);
		RenderInterface.PopState();
	}

	SwapBuffers(DeviceContext);

	check(glGetError() == GL_NO_ERROR);
}

FRenderCaps* UOpenGLRenderDevice::GetRenderCaps(){
	RenderCaps.MaxSimultaneousTerrainLayers = 1;
	RenderCaps.PixelShaderVersion = 0;
	RenderCaps.HardwareTL = 1;

	return &RenderCaps;
}

void UOpenGLRenderDevice::LoadShaders(){
	LoadShader(&FixedFunctionShader);
	LoadShader(&FramebufferShader);

	for(TMap<UHardwareShader*, FShaderGLSL>::TIterator It(GLShaderByHardwareShader); It; ++It)
		LoadShader(&It.Value());
}

void UOpenGLRenderDevice::LoadShader(FShaderGLSL* Shader){
	if(Shader->GetName()[0] == '\0')
		return;

	FStringTemp ShaderText(0);
	FFilename Filename = ShaderDir * Shader->GetName() + VERTEX_SHADER_FILE_EXTENSION;

	GFileManager->MakeDirectory(*(Filename.GetPath() + "\\"), 1);

	if(GFileManager->FileSize(*Filename) > 0 && appLoadFileToString(ShaderText, *Filename))
		Shader->SetVertexShaderText(ShaderText);
	else
		appSaveStringToFile(Shader->GetVertexShaderText(), *Filename);

	Filename = ShaderDir * Shader->GetName() + FRAGMENT_SHADER_FILE_EXTENSION;

	GFileManager->MakeDirectory(*(Filename.GetPath() + "\\"), 1);

	if(GFileManager->FileSize(*Filename) > 0 && appLoadFileToString(ShaderText, *Filename))
		Shader->SetFragmentShaderText(ShaderText);
	else
		appSaveStringToFile(Shader->GetFragmentShaderText(), *Filename);
}

// Default shader code

#define UNIFORM_BLOCK_MEMBER(type, name) "\t" #type " " #name ";\n"
#define SHADER_HEADER \
	"#version 450 core\n\n" \
	"// Global shared uniforms\n\n" \
	"layout(std140, binding = 0) uniform Globals{\n" \
		UNIFORM_BLOCK_CONTENTS \
	"};\n\n"

FString UOpenGLRenderDevice::CommonShaderHeaderText(
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
	"layout(binding = 8)  uniform samplerCube CubeMap0;\n"
	"layout(binding = 9)  uniform samplerCube CubeMap1;\n"
	"layout(binding = 10) uniform samplerCube CubeMap2;\n"
	"layout(binding = 11) uniform samplerCube CubeMap3;\n"
	"layout(binding = 12) uniform samplerCube CubeMap4;\n"
	"layout(binding = 13) uniform samplerCube CubeMap5;\n"
	"layout(binding = 14) uniform samplerCube CubeMap6;\n"
	"layout(binding = 15) uniform samplerCube CubeMap7;\n\n", true);

FString UOpenGLRenderDevice::VertexShaderVarsText(
	"// Vertex attributes\n\n"
	"layout(location = 0)  in vec3 InPosition;\n"
	"layout(location = 1)  in vec3 InNormal;\n"
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
	"layout(location = 12) in vec3 InTangent;\n"
	"layout(location = 13) in vec3 InBinormal;\n\n"
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
	"out vec3 Binormal;\n\n", true);

FString UOpenGLRenderDevice::FragmentShaderVarsText(
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
	"out vec4 FragColor;\n\n", true);

FString UOpenGLRenderDevice::FixedFunctionVertexShaderText(
	"// Shader specific uniforms\n"
	"\n"
	"layout(location = 0)  uniform int NumStages;\n"
	"layout(location = 1)  uniform int TexCoordCount;\n"
	"layout(location = 2)  uniform int StageTexCoordSources[8];\n"
	"layout(location = 10) uniform mat4 StageTexMatrices[8];\n"
	"layout(location = 18) uniform int StageColorArgs[16];\n"
	"layout(location = 34) uniform int StageColorOps[8];\n"
	"layout(location = 42) uniform int StageAlphaArgs[16];\n"
	"layout(location = 58) uniform int StageAlphaOps[8];\n"
	"layout(location = 66) uniform vec4 ConstantColor;\n"
	"layout(location = 67) uniform float AlphaRef;\n"
	"\n"
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
		"\tPosition = (LocalToWorld * vec4(InPosition, 1.0)).xyz;\n"
		"\tNormal = (LocalToWorld * vec4(InNormal, 0.0)).xyz;\n"
		"\tDiffuse = InDiffuse;\n"
		"\tSpecular = InSpecular;\n"
		"\n"
		"\tswitch(TexCoordCount){\n"
		"\tcase 2:\n"
			"\t\tfor(int i = 0; i < NumStages; ++i)\n"
				"\t\t\tStageTexCoords[i] = StageTexMatrices[i] * vec4(tex_coord_source(StageTexCoordSources[i]).xy, 1.0, 1.0);\n"
			"\t\tbreak;\n"
		"\tcase 3:\n"
			"\t\tfor(int i = 0; i < NumStages; ++i)\n"
				"\t\t\tStageTexCoords[i] = StageTexMatrices[i] * vec4(tex_coord_source(StageTexCoordSources[i]).xyz, 1.0);\n"
			"\t\tbreak;\n"
		"\tcase 4:\n"
			"\t\tfor(int i = 0; i < NumStages; ++i)\n"
				"\t\t\tStageTexCoords[i] = StageTexMatrices[i] * tex_coord_source(StageTexCoordSources[i]);\n"
		"\t}\n"
		"\n"
		"\tgl_Position = LocalToScreen * vec4(InPosition, 1.0);\n"
	"}\n", true);

FString UOpenGLRenderDevice::FixedFunctionFragmentShaderText(
	"// Shader specific uniforms\n"
	"\n"
	"layout(location = 0)  uniform int NumStages;\n"
	"layout(location = 1)  uniform int TexCoordCount;\n"
	"layout(location = 2)  uniform int StageTexCoordSources[8];\n"
	"layout(location = 10) uniform mat4 StageTexMatrices[8];\n"
	"layout(location = 18) uniform int StageColorArgs[16];\n"
	"layout(location = 34) uniform int StageColorOps[8];\n"
	"layout(location = 42) uniform int StageAlphaArgs[16];\n"
	"layout(location = 58) uniform int StageAlphaOps[8];\n"
	"layout(location = 66) uniform vec4 ConstantColor;\n"
	"layout(location = 67) uniform float AlphaRef;\n"
	"\n"
	"in vec4 StageTexCoords[8];\n"
	"\n"
	"#define CA_Previous 0\n"
	"#define CA_Diffuse  1\n"
	"#define CA_Constant 2\n"
	"#define CA_Texture0 3\n"
	"#define CA_Texture1 4\n"
	"#define CA_Texture2 5\n"
	"#define CA_Texture3 6\n"
	"#define CA_Texture4 7\n"
	"#define CA_Texture5 8\n"
	"#define CA_Texture6 9\n"
	"#define CA_Texture7 10\n"
	"\n"
	"#define CM_CubeMap (1 << 30)\n"
	"#define CM_Invert  (1 << 31)\n"
	"\n"
	"vec4 color_arg(int StageIndex, int Arg, bool AlphaArgs){\n"
		"\tint ColorArg = AlphaArgs ? StageAlphaArgs[StageIndex * 2 + Arg] : StageColorArgs[StageIndex * 2 + Arg];\n"
		"\tvec4 Result;\n"
		"\n"
		"\tswitch(ColorArg & 0xFF){\n"
		"\tcase CA_Previous:\n"
			"\t\tResult = FragColor;\n"
			"\t\tbreak;\n"
		"\tcase CA_Diffuse:\n"
			"\t\tResult = Diffuse;\n"
			"\t\tbreak;\n"
		"\tcase CA_Constant:\n"
			"\t\tResult = ConstantColor;\n"
			"\t\tbreak;\n"
		"\tcase CA_Texture0:\n"
			"\t\tif((ColorArg & CM_CubeMap) == 0)\n"
				"\t\t\tResult = texture(Texture0, StageTexCoords[StageIndex].xy) * GlobalColor;\n"
			"\t\telse\n"
				"\t\t\tResult = texture(CubeMap0, StageTexCoords[StageIndex].xyz) * GlobalColor;\n"
			"\t\tbreak;\n"
		"\tcase CA_Texture1:\n"
			"\t\tif((ColorArg & CM_CubeMap) == 0)\n"
				"\t\t\tResult = texture(Texture1, StageTexCoords[StageIndex].xy) * GlobalColor;\n"
			"\t\telse\n"
				"\t\t\tResult = texture(CubeMap1, StageTexCoords[StageIndex].xyz) * GlobalColor;\n"
			"\t\tbreak;\n"
		"\tcase CA_Texture2:\n"
			"\t\tif((ColorArg & CM_CubeMap) == 0)\n"
				"\t\t\tResult = texture(Texture2, StageTexCoords[StageIndex].xy) * GlobalColor;\n"
			"\t\telse\n"
				"\t\t\tResult = texture(CubeMap2, StageTexCoords[StageIndex].xyz) * GlobalColor;\n"
			"\t\tbreak;\n"
		"\tcase CA_Texture3:\n"
			"\t\tif((ColorArg & CM_CubeMap) == 0)\n"
				"\t\t\tResult = texture(Texture3, StageTexCoords[StageIndex].xy) * GlobalColor;\n"
			"\t\telse\n"
				"\t\t\tResult = texture(CubeMap3, StageTexCoords[StageIndex].xyz) * GlobalColor;\n"
			"\t\tbreak;\n"
		"\tcase CA_Texture4:\n"
			"\t\tif((ColorArg & CM_CubeMap) == 0)\n"
				"\t\t\tResult = texture(Texture4, StageTexCoords[StageIndex].xy) * GlobalColor;\n"
			"\t\telse\n"
				"\t\t\tResult = texture(CubeMap4, StageTexCoords[StageIndex].xyz) * GlobalColor;\n"
			"\t\tbreak;\n"
		"\tcase CA_Texture5:\n"
			"\t\tif((ColorArg & CM_CubeMap) == 0)\n"
				"\t\t\tResult = texture(Texture5, StageTexCoords[StageIndex].xy) * GlobalColor;\n"
			"\t\telse\n"
				"\t\t\tResult = texture(CubeMap5, StageTexCoords[StageIndex].xyz) * GlobalColor;\n"
			"\t\tbreak;\n"
		"\tcase CA_Texture6:\n"
			"\t\tif((ColorArg & CM_CubeMap) == 0)\n"
				"\t\t\tResult = texture(Texture6, StageTexCoords[StageIndex].xy) * GlobalColor;\n"
			"\t\telse\n"
				"\t\t\tResult = texture(CubeMap6, StageTexCoords[StageIndex].xyz) * GlobalColor;\n"
			"\t\tbreak;\n"
		"\tcase CA_Texture7:\n"
			"\t\tif((ColorArg & CM_CubeMap) == 0)\n"
				"\t\t\tResult = texture(Texture7, StageTexCoords[StageIndex].xy) * GlobalColor;\n"
			"\t\telse\n"
				"\t\t\tResult = texture(CubeMap7, StageTexCoords[StageIndex].xyz) * GlobalColor;\n"
			"\t\tbreak;\n"
		"\tdefault:\n"
			"\t\tResult = vec4(1.0, 1.0, 1.0, 1.0);\n"
		"\t}\n"
		"\n"
		"\tif((ColorArg & CM_Invert) != 0)\n"
			"\t\tResult = 1.0 - Result;\n"
		"\n"
		"\treturn Result;\n"
	"}\n"
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
	"vec4 color_op(int StageIndex, vec4 Arg1, vec4 Arg2){\n"
		"\tswitch(StageColorOps[StageIndex]){\n"
		"\tcase COP_Arg1:\n"
			"\t\treturn Arg1;\n"
		"\tcase COP_Arg2:\n"
			"\t\treturn Arg2;\n"
		"\tcase COP_Modulate:\n"
			"\t\treturn Arg1 * Arg2;\n"
		"\tcase COP_Modulate2X:\n"
			"\t\treturn Arg1 * Arg2 * 2.0;\n"
		"\tcase COP_Modulate4X:\n"
			"\t\treturn Arg1 * Arg2 * 4.0;\n"
		"\tcase COP_Add:\n"
			"\t\treturn Arg1 + Arg2;\n"
		"\tcase COP_Subtract:\n"
			"\t\treturn Arg1 - Arg2;\n"
		"\tcase COP_AlphaBlend:\n"
			"\t\treturn mix(Arg1, Arg2, Arg1.a);\n"
		"\tcase COP_AddAlphaModulate:\n"
			"\t\treturn Arg1 + Arg2 * Arg1.a;\n"
		"\tdefault:\n"
			"\t\treturn vec4(1.0, 1.0, 1.0, 1.0);\n"
		"\t}\n"
	"}\n"
	"\n"
	"#define AOP_Arg1     0\n"
	"#define AOP_Arg2     1\n"
	"#define AOP_Modulate 2\n"
	"#define AOP_Add      3\n"
	"#define AOP_Blend    4\n"
	"\n"
	"float alpha_op(int StageIndex, float Arg1, float Arg2){\n"
		"\tswitch(StageAlphaOps[StageIndex]){\n"
		"\tcase AOP_Arg1:\n"
			"\t\treturn Arg1;\n"
		"\tcase AOP_Arg2:\n"
			"\t\treturn Arg2;\n"
		"\tcase AOP_Modulate:\n"
			"\t\treturn Arg1 * Arg2;\n"
		"\tcase AOP_Add:\n"
			"\t\treturn Arg1 + Arg2;\n"
		"\tcase AOP_Blend:\n"
			"\t\treturn mix(Arg1, Arg2, Arg2);\n"
		"\tdefault:\n"
			"\t\treturn 1.0;\n"
		"\t}\n"
	"}\n"
	"\n"
	"void shader_stage(int StageIndex){\n"
		"\tvec4 ColorArg1 = color_arg(StageIndex, 0, false);\n"
		"\tvec4 ColorArg2 = color_arg(StageIndex, 1, false);\n"
		"\n"
		"\tColorArg1.a = color_arg(StageIndex, 0, true).a;\n"
		"\tColorArg2.a = color_arg(StageIndex, 1, true).a;\n"
		"\n"
		"\tFragColor.a = alpha_op(StageIndex, ColorArg1.a, ColorArg2.a);\n"
		"\tFragColor.rgb = color_op(StageIndex, ColorArg1, ColorArg2).rgb;\n"
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
		"\tif(FragColor.a <= AlphaRef)\n"
			"\t\tdiscard;\n"
	"}\n", true);

FString UOpenGLRenderDevice::FramebufferVertexShaderText(
	SHADER_HEADER
	"out vec2 TexCoord;\n\n"
	"void main(void){\n"
		"\tconst vec4[] Vertices = vec4[](vec4(1.0, 1.0, 1.0, 1.0),\n"
		"\t                               vec4(-1.0, 1.0, 0.0, 1.0),\n"
		"\t                               vec4(1.0, -1.0, 1.0, 0.0),\n"
		"\t                               vec4(-1.0, -1.0, 0.0, 0.0));\n\n"
		"\tgl_Position = vec4(Vertices[gl_VertexID].xy, 0.5, 1.0);\n"
		"\tTexCoord = Vertices[gl_VertexID].zw;\n"
	"}\n", true);

FString UOpenGLRenderDevice::FramebufferFragmentShaderText(
	SHADER_HEADER
	"layout(binding = 0) uniform sampler2D Screen;\n\n"
	"in vec2 TexCoord;\n\n"
	"out vec4 FragColor;\n\n"
	"void main(void){\n"
		"\tFragColor = texture2D(Screen, TexCoord.xy);\n"
	"}\n", true);

#undef UNIFORM_BLOCK_MEMBER
#undef SHADER_HEADER
