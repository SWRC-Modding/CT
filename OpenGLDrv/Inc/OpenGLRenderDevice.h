#pragma once

#include "OpenGLDrv.h"
#include "OpenGLRenderInterface.h"
#include "Shader.h"

class FOpenGLResource;

#define FRAGMENT_SHADER_FILE_EXTENSION ".fs"
#define VERTEX_SHADER_FILE_EXTENSION ".vs"
#define SHADER_MACROS_FILE_EXTENSION ".glsl"

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
	BYTE           TextureFilter; // ETextureFilter
	INT            TextureAnisotropy;

	// Default shader code
	static FString VertexShaderVarsText;
	static FString FragmentShaderVarsText;
	static FString FixedFunctionVertexShaderText;
	static FString FixedFunctionFragmentShaderText;

	// Error shader
	static FShaderGLSL ErrorShader;

	UOpenGLRenderDevice();
	void StaticConstructor();

	void MakeCurrent();
	bool IsCurrent();
	void UnSetRes();
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
	HDC                       DeviceContext;
	HGLRC                     OpenGLContext;
	FOpenGLRenderInterface    RenderInterface;
	FRenderCaps               RenderCaps;

	FShaderGLSL               FixedFunctionShader;

	FAuxRenderTarget          Backbuffer;
	unsigned int              BackbufferDepthStencil; // Shared with UFrameFX::WorkingTarget

	UBOOL                     bFirstRun;
	UBOOL                     bVSync;
	UBOOL                     bAdaptiveVSync;
	UBOOL                     bDebugOpenGL;
	UBOOL                     bIsFullscreen;
	INT                       SavedViewportWidth;
	INT                       SavedViewportHeight;

	FOpenGLIndexBuffer*       DynamicIndexBuffer16;
	FOpenGLIndexBuffer*       DynamicIndexBuffer32;
	FOpenGLVertexStream*      DynamicVertexStream;

	TArray<BYTE>              Scratch;
	FOpenGLResource*          ResourceHash[4096];

	FStringNoInit             ShaderDir;

	TMap<FString, SQWORD>               ShaderFileTimes;
	TMap<UHardwareShader*, FShaderGLSL> GLShaderByHardwareShader;

	void AddResource(FOpenGLResource* Resource);
	void RemoveResource(FOpenGLResource* Resource);

	FOpenGLIndexBuffer* GetDynamicIndexBuffer(INT IndexSize);
	FOpenGLVertexStream* GetDynamicVertexStream();

	bool ShaderFileNeedsReload(const char* Filename);
	bool LoadVertexShader(FShaderGLSL* Shader);
	bool LoadFragmentShader(FShaderGLSL* Shader);
	bool LoadShaderMacroText();
	void SaveVertexShader(FShaderGLSL* Shader);
	void SaveFragmentShader(FShaderGLSL* Shader);
	void SaveShaderMacroText();

	FStringTemp MakeShaderFilename(const FString& ShaderName, const TCHAR* Extension);
	bool LoadShaderText(const FFilename& Filename, FString* Out);
	void SaveShaderText(const FFilename& Filename, const FString& Text);

	// Shader conversion

	static UHardwareShaderMacros* HardwareShaderMacros;
	static TMap<FString, FString> HardwareShaderMacroText;
	static TArray<FString>        ExpandedMacros; // Used to check for circular references in macros

	static void ParseGLSLMacros(const FString& Text);
	static FStringTemp GLSLVertexShaderFromD3DVertexShader(UHardwareShader* Shader);
	static FStringTemp GLSLFragmentShaderFromD3DPixelShader(UHardwareShader* Shader);
	static bool ConvertD3DAssemblyToGLSL(const TCHAR* Text, FString* Out, bool VertexFog);

	// Movie playback

	HWND CurrentMovieWindow;

	void HandleMovieWindow(UViewport* Viewport);
};
