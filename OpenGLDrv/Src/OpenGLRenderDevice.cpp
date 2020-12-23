#include "../Inc/OpenGLRenderDevice.h"

#include "GL/glew.h"
#include "GL/wglew.h"
#include "OpenGLResource.h"

#define MIN_OPENGL_MAJOR_VERSION 4
#define MIN_OPENGL_MINOR_VERSION 5

IMPLEMENT_CLASS(UOpenGLRenderDevice)

UOpenGLRenderDevice::UOpenGLRenderDevice() : RenderInterface(this),
                                             ScreenRenderTarget(0, 0, TEXF_RGBA8, false, true){
	LoadShaders();
}

void UOpenGLRenderDevice::StaticConstructor(){
	SupportsZBIAS = 1;
	UseStencil = 1;
	bFirstRun = 1;
	ShaderDir = "../Shaders";

	new(GetClass(), "UseDesktopResolution", RF_Public) UBoolProperty(CPP_PROPERTY(bUseDesktopResolution), "Options", CPF_Config);
	new(GetClass(), "KeepAspectRatio", RF_Public) UBoolProperty(CPP_PROPERTY(bKeepAspectRatio), "Options", CPF_Config);
	new(GetClass(), "FirstRun", RF_Public) UBoolProperty(CPP_PROPERTY(bFirstRun), "", CPF_Config);
	new(GetClass(), "VSync", RF_Public) UBoolProperty(CPP_PROPERTY(bVSync), "", CPF_Config);
	new(GetClass(), "DebugOpenGL", RF_Public) UBoolProperty(CPP_PROPERTY(bDebugOpenGL), "", CPF_Config);
	new(GetClass(), "ShaderDir", RF_Public) UStrProperty(CPP_PROPERTY(ShaderDir), "", CPF_Config);
}

void UOpenGLRenderDevice::MakeCurrent(){
	guardFunc;

	if(!IsCurrent()){
		wglMakeCurrent(DeviceContext, OpenGLContext);
		// NOTE: This must be set to 0 in order to avoid inconsistencies with RGBA and BGRA colors.
		GIsOpenGL = 0;
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
	if(ParseCommand(&Cmd, "LOADSHADERS")){
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

		RenderInterface.EnableZTest(1);
		RenderInterface.SetShader(&FixedFunctionShader);
		RenderInterface.SetCullMode(CM_CW);
		RenderInterface.SetFillMode(FM_Solid);
		EnableVSync(bVSync != 0);

		// Set default values for unspecified vertex attributes

		glVertexAttrib4f(FVF_Diffuse, 1.0f, 1.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_Specular, 1.0f, 1.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord0, 0.0f, 0.0f, 0.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord1, 0.0f, 0.0f, 0.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord2, 0.0f, 0.0f, 0.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord3, 0.0f, 0.0f, 0.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord4, 0.0f, 0.0f, 0.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord5, 0.0f, 0.0f, 0.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord6, 0.0f, 0.0f, 0.0f, 1.0f);
		glVertexAttrib4f(FVF_TexCoord7, 0.0f, 0.0f, 0.0f, 1.0f);
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
	if(Viewport && Viewport->Actor)
		Viewport->Actor->FrameFX->FreeRenderTargets();

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

void UOpenGLRenderDevice::EnableVSync(bool bEnable){
	if(WGL_EXT_swap_control){
		bVSync = bEnable;
		wglSwapIntervalEXT(bEnable ? 1 : 0);
	}else{
		bVSync = 0;
	}
}

FRenderInterface* UOpenGLRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize){
	check(RenderInterface.CurrentState == &RenderInterface.SavedStates[0]);

	MakeCurrent();

	RenderInterface.LockedViewport = Viewport;

	// Render target and default shader might be deleted when Flush is called so check for that and set them again

	if(!RenderInterface.CurrentState->RenderTarget)
		RenderInterface.SetRenderTarget(&ScreenRenderTarget, false);

	FShaderGLSL* DefaultShader = &FixedFunctionShader;
	FOpenGLShader* CurrentShader = RenderInterface.CurrentState->Shader;

	checkSlow(!CurrentShader || CurrentShader->CacheId == DefaultShader->GetCacheId());

	if(!CurrentShader || (CurrentShader->Revision != DefaultShader->GetRevision())) // Update default shader in case it was reloaded
		RenderInterface.SetShader(DefaultShader);

	RenderInterface.SetupPerFrameShaderConstants();
	RenderInterface.PushState();

	return &RenderInterface;
}

void UOpenGLRenderDevice::Unlock(FRenderInterface* RI){
	RI->PopState();
	RenderInterface.LockedViewport = NULL;
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

	FOpenGLTexture* Framebuffer = RenderInterface.CurrentState->RenderTarget;

	if(Framebuffer){
		RenderInterface.PushState();
		glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

		INT FramebufferWidth = Framebuffer->Width;
		INT FramebufferHeight = Framebuffer->Height;
		INT ViewportWidth;
		INT ViewportHeight;

		if(bIsFullscreen){
			ViewportWidth  = SavedViewportWidth;
			ViewportHeight = SavedViewportHeight;
		}else{
			ViewportWidth  = Viewport->SizeX;
			ViewportHeight = Viewport->SizeY;
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

		Framebuffer->BindTexture(0);
		RenderInterface.SetFillMode(FM_Solid);
		RenderInterface.EnableZTest(0);
		RenderInterface.SetDynamicStream(VS_FixedFunction, &FullscreenQuad);
		RenderInterface.SetShader(&FramebufferShader);
		RenderInterface.DrawPrimitive(PT_TriangleStrip, 0, 2);
		Framebuffer->BindRenderTarget();
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
	// Init default shaders with the default implementation

	FixedFunctionShader.SetName("FixedFunction");
	FixedFunctionShader.SetVertexShaderText(CommonShaderHeaderText + VertexShaderVarsText + FixedFunctionVertexShaderText);
	FixedFunctionShader.SetFragmentShaderText(CommonShaderHeaderText + FragmentShaderVarsText + FixedFunctionFragmentShaderText);
	LoadShader(&FixedFunctionShader);

	FramebufferShader.SetName("Framebuffer");
	FramebufferShader.SetVertexShaderText(CommonShaderHeaderText + VertexShaderVarsText + FramebufferVertexShaderText);
	FramebufferShader.SetFragmentShaderText(CommonShaderHeaderText + FragmentShaderVarsText + FramebufferFragmentShaderText);
	LoadShader(&FramebufferShader);
}

void UOpenGLRenderDevice::LoadShader(FShaderGLSL* Shader){
	FStringTemp ShaderText(0);
	FString Filename = ShaderDir * Shader->GetName() + VERTEX_SHADER_FILE_EXTENSION;

	GFileManager->MakeDirectory(*ShaderDir);

	if(appLoadFileToString(ShaderText, *Filename))
		Shader->SetVertexShaderText(ShaderText);
	else
		appSaveStringToFile(Shader->GetVertexShaderText(), *Filename);

	Filename = ShaderDir * Shader->GetName() + FRAGMENT_SHADER_FILE_EXTENSION;

	if(appLoadFileToString(ShaderText, *Filename))
		Shader->SetFragmentShaderText(ShaderText);
	else
		appSaveStringToFile(Shader->GetFragmentShaderText(), *Filename);
}

// Default shader code

FString UOpenGLRenderDevice::CommonShaderHeaderText(
	"#version 450\n\n"
	"// Global shared uniforms\n\n"
	"layout(std140, binding = 0) uniform Globals{\n"
#define UNIFORM_BLOCK_MEMBER(type, name) "\t" #type " " #name ";\n"
	UNIFORM_BLOCK_CONTENTS
#undef UNIFORM_BLOCK_MEMBER
	"};\n\n"
	"// Textures\n\n"
	"layout(binding = 0) uniform sampler2D Texture0;\n"
	"layout(binding = 1) uniform sampler2D Texture1;\n"
	"layout(binding = 2) uniform sampler2D Texture2;\n"
	"layout(binding = 3) uniform sampler2D Texture3;\n"
	"layout(binding = 4) uniform sampler2D Texture4;\n"
	"layout(binding = 5) uniform sampler2D Texture5;\n"
	"layout(binding = 6) uniform sampler2D Texture6;\n"
	"layout(binding = 7) uniform sampler2D Texture7;\n\n", true);

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
	"out vec4 StageTexCoords[8];\n\n"
	"void main(void){\n"
		"\tPosition = (LocalToWorld * vec4(InPosition, 1.0)).xyz;\n"
		"\tNormal = (LocalToWorld * vec4(InNormal, 0.0)).xyz;\n"
		"\tDiffuse = InDiffuse;\n"
		"\tSpecular = InSpecular;\n"
		"\tTexCoord0 = InTexCoord0;\n"
		"\tTexCoord1 = InTexCoord1;\n"
		"\tTexCoord2 = InTexCoord2;\n"
		"\tTexCoord3 = InTexCoord3;\n"
		"\tTexCoord4 = InTexCoord4;\n"
		"\tTexCoord5 = InTexCoord5;\n"
		"\tTexCoord6 = InTexCoord6;\n"
		"\tTexCoord7 = InTexCoord7;\n"
		"\n"
		"\tStageTexCoords = vec4[](TexCoord0, TexCoord0, TexCoord0, TexCoord0, TexCoord0, TexCoord0, TexCoord0, TexCoord0);\n"
		"\n"
		"\tgl_Position = Transform * vec4(InPosition, 1.0);\n"
	"}\n", true);

FString UOpenGLRenderDevice::FixedFunctionFragmentShaderText(
	"layout(location = 0)  uniform int NumStages;\n"
	"layout(location = 1)  uniform int StageColorArgs[16];\n"
	"layout(location = 17) uniform int StageColorOps[8];\n"
	"layout(location = 25) uniform int StageAlphaArgs[16];\n"
	"layout(location = 41) uniform int StageAlphaOps[8];\n"
	"layout(location = 49) uniform vec4 ConstantColor;\n"
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
	"vec4 color_arg(int StageIndex, int Arg){\n"
		"\tswitch(StageColorArgs[StageIndex * 2 + Arg]){\n"
		"\tcase CA_Previous:\n"
		"\t	return FragColor;\n"
		"\tcase CA_Diffuse:\n"
		"\t	return Diffuse;\n"
		"\tcase CA_Constant:\n"
		"\t	return ConstantColor;\n"
		"\tcase CA_Texture0:\n"
		"\t	return texture2D(Texture0, StageTexCoords[StageIndex].xy);\n"
		"\tcase CA_Texture1:\n"
		"\t	return texture2D(Texture1, StageTexCoords[StageIndex].xy);\n"
		"\tcase CA_Texture2:\n"
		"\t	return texture2D(Texture2, StageTexCoords[StageIndex].xy);\n"
		"\tcase CA_Texture3:\n"
		"\t	return texture2D(Texture3, StageTexCoords[StageIndex].xy);\n"
		"\tcase CA_Texture4:\n"
		"\t	return texture2D(Texture4, StageTexCoords[StageIndex].xy);\n"
		"\tcase CA_Texture5:\n"
		"\t	return texture2D(Texture5, StageTexCoords[StageIndex].xy);\n"
		"\tcase CA_Texture6:\n"
		"\t	return texture2D(Texture6, StageTexCoords[StageIndex].xy);\n"
		"\tcase CA_Texture7:\n"
		"\t	return texture2D(Texture7, StageTexCoords[StageIndex].xy);\n"
		"\t}\n"
		"\n"
		"\treturn vec4(1.0, 1.0, 1.0, 1.0);\n"
	"}\n"
	"\n"
	"#define COP_Arg1             0\n"
	"#define COP_Arg2             1\n"
	"#define COP_Modulate         2\n"
	"#define COP_Add              3\n"
	"#define COP_Subtract         4\n"
	"#define COP_AlphaBlend       5\n"
	"#define COP_AddAlphaModulate 6\n"
	"\n"
	"vec4 color_op(int StageIndex, vec4 Arg1, vec4 Arg2){\n"
		"\tswitch(StageColorOps[StageIndex]){\n"
		"\t	case COP_Arg1:\n"
			"\t\treturn Arg1;\n"
		"\t	case COP_Arg2:\n"
			"\t\treturn Arg2;\n"
		"\t	case COP_Modulate:\n"
			"\t\treturn Arg1 * Arg2;\n"
		"\t	case COP_Add:\n"
			"\t\treturn Arg1 + Arg2;\n"
		"\t	case COP_Subtract:\n"
			"\t\treturn Arg1 - Arg2;\n"
		"\t	case COP_AlphaBlend:\n"
			"\t\treturn mix(Arg1, Arg2, Arg1.a);\n"
		"\t	case COP_AddAlphaModulate:\n"
			"\t\treturn Arg1 + Arg2 * Arg1.a;\n"
		"\t}\n"
		"\n"
		"\treturn vec4(1.0, 1.0, 1.0, 1.0);\n"
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
		"\t	return Arg1;\n"
		"\tcase AOP_Arg2:\n"
		"\t	return Arg2;\n"
		"\tcase AOP_Modulate:\n"
		"\t	return Arg1 * Arg2;\n"
		"\tcase AOP_Add:\n"
		"\t	return Arg1 + Arg2;\n"
		"\tcase AOP_Blend:\n"
		"\t	return mix(Arg1, Arg2, Arg1);\n"
		"\t}\n"
		"\n"
		"\treturn 1.0;\n"
	"}\n"
	"\n"
	"void shader_stage(int StageIndex){\n"
		"\tvec4 ColorArg1 = color_arg(StageIndex, 0);\n"
		"\tvec4 ColorArg2 = color_arg(StageIndex, 1);\n"
		"\n"
		"\tFragColor.rgb = color_op(StageIndex, ColorArg1, ColorArg2).rgb;\n"
		"\n"
		"\tfloat AlphaArg1 = color_arg(StageIndex, 0).a;\n"
		"\tfloat AlphaArg2 = color_arg(StageIndex, 1).a;\n"
		"\n"
		"\tFragColor.a = alpha_op(StageIndex, AlphaArg1, AlphaArg2);\n"
	"}\n"
	"\n"
	"// Main\n"
	"\n"
	"void main(void){\n"
		"\tFragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
		"\n"
		"\tfor(int i = 0; i < NumStages; ++i)\n"
		"\t	shader_stage(i);\n"
	"}\n", true);

FString UOpenGLRenderDevice::FramebufferVertexShaderText(
	"void main(void){\n"
		"\tTexCoord0 = InTexCoord0;\n"
		"\tgl_Position = vec4(InPosition.xy, 0.5, 1.0);\n"
	"}\n", true);

FString UOpenGLRenderDevice::FramebufferFragmentShaderText(
	"void main(void){\n"
		"\tFragColor = texture2D(Texture0, TexCoord0.xy);\n"
	"}\n", true);
