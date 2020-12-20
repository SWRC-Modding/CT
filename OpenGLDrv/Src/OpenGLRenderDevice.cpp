#include "../Inc/OpenGLRenderDevice.h"

#include "GL/glew.h"
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

void GLAPIENTRY MessageCallback(GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar* message,
                                const void* userParam){
	if(type == GL_DEBUG_TYPE_ERROR)
		appMsgf(3, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s", "** GL ERROR **", type, severity, message);
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

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(MessageCallback, NULL);

		debugf(NAME_Init, "GL_VENDOR      : %s", glGetString(GL_VENDOR));
		debugf(NAME_Init, "GL_RENDERER    : %s", glGetString(GL_RENDERER));
		debugf(NAME_Init, "GL_VERSION     : %s", glGetString(GL_VERSION));

		GLint MajorVersion;
		GLint MinorVersion;

		glGetIntegerv(GL_MAJOR_VERSION, &MajorVersion);
		glGetIntegerv(GL_MINOR_VERSION, &MinorVersion);

		if(MajorVersion < MIN_OPENGL_MAJOR_VERSION || (MajorVersion == MIN_OPENGL_MAJOR_VERSION && MinorVersion < MIN_OPENGL_MINOR_VERSION))
			appErrorf("OpenGL %i.%i is required but got %i.%i", MIN_OPENGL_MAJOR_VERSION, MIN_OPENGL_MINOR_VERSION, MajorVersion, MinorVersion);

		debugf(NAME_Init, "%i-bit color buffer", ColorBytes * 8);

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

		// Set default values for unspecified vertex attributes

		glVertexAttrib4f(FVF_Diffuse, 1.0f, 1.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_Specular, 1.0f, 1.0f, 1.0f, 1.0f);
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

FRenderInterface* UOpenGLRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize){
	check(RenderInterface.CurrentState == &RenderInterface.SavedStates[0]);

	MakeCurrent();

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

		glBindTextureUnit(0, Framebuffer->ColorAttachment);
		RenderInterface.SetFillMode(FM_Solid);
		RenderInterface.EnableZTest(0);
		RenderInterface.SetDynamicStream(VS_FixedFunction, &FullscreenQuad);
		RenderInterface.SetShader(&FramebufferShader);
		RenderInterface.DrawPrimitive(PT_TriangleStrip, 0, 2);
		Framebuffer->Bind();
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
		"\tTexCoord7 = InTexCoord7;\n\n"
		"\tgl_Position = Transform * vec4(InPosition, 1.0);\n"
	"}\n", true);

FString UOpenGLRenderDevice::FixedFunctionFragmentShaderText(
"// Shader specific uniforms\n"
"\n"
"layout(location = 0) uniform float AlphaRef;\n"
"layout(location = 1) uniform vec4 ConstantColor;\n"
"\n"
"// Subroutine types\n"
"\n"
"subroutine vec4 TexCoordSource(void);\n"
"subroutine vec4 DiffuseColor(void);\n"
"subroutine vec4 CombinerMask(void);\n"
"subroutine vec4 MaterialColor(void);\n"
"subroutine vec4 OutputColor(void);\n"
"\n"
"// Subroutine uniforms\n"
"layout(location = 0) subroutine uniform TexCoordSource texture0_coord;\n"
"layout(location = 1) subroutine uniform TexCoordSource texture1_coord;\n"
"layout(location = 2) subroutine uniform TexCoordSource texture2_coord;\n"
"layout(location = 3) subroutine uniform TexCoordSource texture3_coord;\n"
"layout(location = 4) subroutine uniform TexCoordSource texture4_coord;\n"
"layout(location = 5) subroutine uniform TexCoordSource texture5_coord;\n"
"layout(location = 6) subroutine uniform TexCoordSource texture6_coord;\n"
"layout(location = 7) subroutine uniform TexCoordSource texture7_coord;\n"
"layout(location = 8) subroutine uniform DiffuseColor diffuse_color;\n"
"layout(location = 9) subroutine uniform DiffuseColor combiner_material1;\n"
"layout(location = 10) subroutine uniform DiffuseColor combiner_material2;\n"
"layout(location = 11) subroutine uniform DiffuseColor combiner_mask_color;\n"
"layout(location = 12) subroutine uniform CombinerMask combiner_mask;\n"
"layout(location = 13) subroutine uniform MaterialColor material_color;\n"
"layout(location = 14) subroutine uniform OutputColor final_color;\n"
"\n"
"// TexCoordSource\n"
"\n"
"layout(index = 0) subroutine(TexCoordSource) vec4 tex_coord0(void){ return TexCoord0; }\n"
"layout(index = 1) subroutine(TexCoordSource) vec4 tex_coord1(void){ return TexCoord1; }\n"
"layout(index = 2) subroutine(TexCoordSource) vec4 tex_coord2(void){ return TexCoord2; }\n"
"layout(index = 3) subroutine(TexCoordSource) vec4 tex_coord3(void){ return TexCoord3; }\n"
"layout(index = 4) subroutine(TexCoordSource) vec4 tex_coord4(void){ return TexCoord4; }\n"
"layout(index = 5) subroutine(TexCoordSource) vec4 tex_coord5(void){ return TexCoord5; }\n"
"layout(index = 6) subroutine(TexCoordSource) vec4 tex_coord6(void){ return TexCoord6; }\n"
"layout(index = 7) subroutine(TexCoordSource) vec4 tex_coord7(void){ return TexCoord7; }\n"
"\n"
"// DiffuseColor\n"
"\n"
"layout(index = 8) subroutine(DiffuseColor) vec4 constant_color(void){\n"
"\treturn ConstantColor;\n"
"}\n"
"\n"
"layout(index = 9) subroutine(DiffuseColor) vec4 vertex_color(void){\n"
"\treturn Diffuse;\n"
"}\n"
"\n"
"layout(index = 10) subroutine(DiffuseColor) vec4 texture0_color(void){ return texture2D(Texture0, texture0_coord().xy); }\n"
"layout(index = 11) subroutine(DiffuseColor) vec4 texture1_color(void){ return texture2D(Texture1, texture1_coord().xy); }\n"
"layout(index = 12) subroutine(DiffuseColor) vec4 texture2_color(void){ return texture2D(Texture2, texture2_coord().xy); }\n"
"layout(index = 13) subroutine(DiffuseColor) vec4 texture3_color(void){ return texture2D(Texture3, texture3_coord().xy); }\n"
"layout(index = 14) subroutine(DiffuseColor) vec4 texture4_color(void){ return texture2D(Texture4, texture4_coord().xy); }\n"
"layout(index = 15) subroutine(DiffuseColor) vec4 texture5_color(void){ return texture2D(Texture5, texture5_coord().xy); }\n"
"layout(index = 16) subroutine(DiffuseColor) vec4 texture6_color(void){ return texture2D(Texture6, texture6_coord().xy); }\n"
"layout(index = 17) subroutine(DiffuseColor) vec4 texture7_color(void){ return texture2D(Texture7, texture7_coord().xy); }\n"
"\n"
"// CombinerMask\n"
"\n"
"layout(index = 18) subroutine(CombinerMask) vec4 combiner_mask_default(void){\n"
"\treturn combiner_mask_color();\n"
"}\n"
"\n"
"layout(index = 19) subroutine(CombinerMask) vec4 combiner_mask_inverted(void){\n"
"\treturn 1.0 - combiner_mask_color();\n"
"}\n"
"\n"
"// MaterialColor\n"
"\n"
"layout(index = 20) subroutine(MaterialColor) vec4 combiner_main(void){\n"
"\tvec4 col1 = combiner_material1();\n"
"\tvec4 col2 = combiner_material2();\n"
"\tvec4 mask = combiner_mask();\n"
"\n"
"\treturn mix(col1, col2, mask.w);\n"
"}\n"
"\n"
"layout(index = 21) subroutine(MaterialColor) vec4 default_material_color(void){\n"
"\treturn diffuse_color();\n"
"}\n"
"\n"
"// OutputColor\n"
"\n"
"layout(index = 22) subroutine(OutputColor) vec4 final_diffuse_color(void){\n"
"\treturn material_color() * Diffuse;\n"
"}\n"
"\n"
"layout(index = 23) subroutine(OutputColor) vec4 final_diffuse_color_alpha_test(void){\n"
"\tvec4 col = material_color() * Diffuse;\n"
"\n"
"\tif(col.w <= AlphaRef)\n"
"\t	discard;\n"
"\n"
"\treturn col;\n"
"}\n"
"\n"
"// Main\n"
"\n"
"void main(void){\n"
"\tFragColor = final_color();\n"
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
