#pragma once

#include "OpenGLDrv.h"
#include "OpenGLRenderInterface.h"
#include "Shader.h"
#include "opengl.h"

class FOpenGLResource;

#define SHADER_FILE_EXTENSION ".glsl"
#define SHADER_MACROS_FILE_EXTENSION ".glslmacros"

enum ETextureFilter{
	TF_Nearest,
	TF_Bilinear,
	TF_Trilinear
};

class OPENGLDRV_API UOpenGLRenderDevice : public URenderDevice{
	DECLARE_CLASS(UOpenGLRenderDevice,URenderDevice,CLASS_Config,OpenGLDrv)
	friend class FOpenGLResource;
	friend class FOpenGLTexture;
	friend class FOpenGLRenderInterface;
public:
	UBOOL          bUseDesktopResolution;
	UBOOL          bKeepAspectRatio;
	UBOOL          bBilinearFramebuffer;
	UBOOL          bAutoReloadShaders;
	ETextureFilter TextureFilter;
	INT            TextureAnisotropy;

	// Default shader code
	static FString VertexShaderVarsText;
	static FString FragmentShaderVarsText;
	static FString FixedFunctionShaderText;

	// Error shader
	static FShaderGLSL ErrorShader;

	UOpenGLRenderDevice();
	void StaticConstructor();

	bool IsCurrent();
	void MakeCurrent();
	void UnSetRes(UViewport* Viewport);
	FOpenGLResource* GetCachedResource(QWORD CacheId);
	FShaderGLSL* GetShader(UHardwareShader* HardwareShader);

	static void SetHardwareShaderMacros(UHardwareShaderMacros* Macros);
	static void ExpandShaderMacros(FString* Text);

	// Overrides
	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar);
	virtual UBOOL Init();
	virtual UBOOL SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes = 0, UBOOL bSaveSize = 1);
	virtual void Exit(UViewport* Viewport);
	virtual void Flush(UViewport* Viewport);
	virtual void FlushResource(QWORD CacheId);
	virtual UBOOL ResourceCached(QWORD CacheId);
	virtual FMemCount ResourceMem(FRenderResource*, UObject*){ return FMemCount(); }
	virtual FMemCount ResourceMemTotal(){ return FMemCount(); }
	virtual void UpdateGamma(UViewport* Viewport){}
	virtual void RestoreGamma(){}
	virtual UBOOL VSyncEnabled(){ return bVSync; }
	virtual void EnableVSync(bool bEnable);
	virtual FRenderInterface* Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize);
	virtual void Unlock(FRenderInterface* RI);
	virtual void Present(UViewport* Viewport);
	virtual void ReadPixels(UViewport* Viewport, FColor* Pixels);
	virtual void SetEmulationMode(EHardwareEmulationMode Mode){}
	virtual FRenderCaps* GetRenderCaps();
	virtual void RenderMovie(UViewport* Viewport);
	virtual FMovie* GetNewMovie(ECodecType Codec, FString Filename, UBOOL UseSound, INT FrameRate, int);
	virtual int GetStateCaching(){ return 0; }
	virtual int SetStateCaching(int){ return 0; }
	virtual int RefreshStates(){ return 0; }
	virtual INT DoesSupportFSAA(INT Level){ return 0; }
	virtual void TakeScreenshot(const TCHAR* Name, UViewport* Viewport, INT Width, INT Height);
	virtual UBOOL SupportsTextureFormat(ETextureFormat){ return 0; }

	void* GetScratchBuffer(INT Size){
		if(Scratch.Num() < Size)
			Scratch.Set(Size);

		return Scratch.GetData();
	}

	void LoadShaders();

private:
	HDC                                 DeviceContext;
	HGLRC                               OpenGLContext;
	FOpenGLRenderInterface              RenderInterface;

	FShaderGLSL                         FixedFunctionShader;

	FAuxRenderTarget                    Backbuffer;
	unsigned int                        BackbufferDepthStencil; // Shared with UFrameFX::WorkingTarget

	UBOOL                               bFirstRun;
	UBOOL                               bVSync;
	UBOOL                               bAdaptiveVSync;
	UBOOL                               bDebugOpenGL;
	UBOOL                               bShowDebugNotifications;
	UBOOL                               bIsFullscreen;
	INT                                 SavedViewportWidth;
	INT                                 SavedViewportHeight;

	FOpenGLIndexBuffer*                 DynamicIndexBuffer;
	FOpenGLVertexStream*                DynamicVertexStream;

	TArray<BYTE>                        Scratch;
	FOpenGLResource*                    ResourceHash[4096];

	FStringNoInit                       ShaderDir;

	TMap<FString, SQWORD>               ShaderFileTimes;
	TMap<UHardwareShader*, FShaderGLSL> GLShaderByHardwareShader;

	void AddResource(FOpenGLResource* Resource);
	void RemoveResource(FOpenGLResource* Resource);

	FOpenGLIndexBuffer* GetDynamicIndexBuffer(INT IndexSize);
	FOpenGLVertexStream* GetDynamicVertexStream();

	FStringTemp MakeShaderFilename(const FString& ShaderName, const TCHAR* Extension);
	bool ShaderFileNeedsReload(const char* Filename);
	bool LoadShader(FShaderGLSL* Shader);
	void SaveShader(FShaderGLSL* Shader);
	bool LoadShaderMacroText();
	void SaveShaderMacroText();

	bool LoadShaderText(const FFilename& Filename, FString* Out);
	void SaveShaderText(const FFilename& Filename, const FString& Text);

	// Shader conversion

	static UHardwareShaderMacros* HardwareShaderMacros;
	static TMap<FString, FString> ShaderMacros;
	static TArray<FString>        ExpandedMacros; // Used to check for circular references in macros

	static void ParseGLSLMacros(const FString& Text);
	static FStringTemp GLSLShaderFromD3DHardwareShader(UHardwareShader* Shader);
	static bool ConvertD3DAssemblyToGLSL(const TCHAR* Text, FString* Out, bool* UsesFog);

	// Movie playback

	HWND CurrentMovieWindow;

	void HandleMovieWindow(UViewport* Viewport);

	// Opengl functions

	UBOOL SupportsWGLSwapIntervalTear;
	UBOOL SupportsEXTFilterAnisotropic;

	void LoadWGLFuncs();
	void LoadGLFuncs();

	HMODULE OpenGL32Dll;

public:
#define WGL_FUNC(name, ret, args) ret(OPENGL_CALL*wgl ## name)args;
	WGL_BASE_FUNCS
	WGL_FUNCS
#undef WGL_FUNC
#define GL_FUNC(name, ret, args) ret(OPENGL_CALL*gl ## name)args;
	GL_BASE_FUNCS
	GL_FUNCS
#undef GL_FUNC
};
