#pragma once

#include "OpenGLDrv.h"
#include "OpenGLRenderInterface.h"
#include "Shader.h"

class FOpenGLResource;

#define FRAGMENT_SHADER_FILE_EXTENSION ".fs"
#define VERTEX_SHADER_FILE_EXTENSION ".vs"

class OPENGLDRV_API UOpenGLRenderDevice : public URenderDevice{
	DECLARE_CLASS(UOpenGLRenderDevice,URenderDevice,CLASS_Config,OpenGLDrv)
public:
	UBOOL                  bUseDesktopResolution;
	UBOOL                  bKeepAspectRatio;

	HDC                    DeviceContext;
	HGLRC                  OpenGLContext;
	FOpenGLRenderInterface RenderInterface;
	FRenderCaps            RenderCaps;

	// Resources
	FAuxRenderTarget       ScreenRenderTarget;
	FShaderGLSL            FixedFunctionShader;
	FShaderGLSL            FramebufferShader;

	// Default shader code
	static FString            VertexShaderVarsText;
	static FString            FragmentShaderVarsText;
	static FString            FixedFunctionVertexShaderText;
	static FString            FixedFunctionFragmentShaderText;
	static FString            FramebufferVertexShaderText;
	static FString            FramebufferFragmentShaderText;

	UOpenGLRenderDevice();
	void StaticConstructor();

	void MakeCurrent();
	bool IsCurrent();
	void UnSetRes();
	FOpenGLResource* GetCachedResource(QWORD CacheId);
	FShaderGLSL* GetShader(UHardwareShader* HardwareShader);

	static void SetHardwareShaderMacros(UHardwareShaderMacros* Macros);

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
	virtual void ReadPixels(UViewport* Viewport, FColor* Pixels){}
	virtual void SetEmulationMode(EHardwareEmulationMode Mode){}
	virtual FRenderCaps* GetRenderCaps();
	virtual void RenderMovie(UViewport* Viewport){}
	virtual FMovie* GetNewMovie(ECodecType Codec, FString Filename, UBOOL UseSound, INT FrameRate, int){ return NULL; }
	virtual int GetStateCaching(){ return 0; }
	virtual int SetStateCaching(int){ return 0; }
	virtual int RefreshStates(){ return 0; }
	virtual INT DoesSupportFSAA(INT Level){ return 0; }
	virtual void TakeScreenshot(const char*, class UViewport*, int, int){}
	virtual UBOOL SupportsTextureFormat(ETextureFormat){ return 0; }

	void* GetScratchBuffer(INT Size){
		if(Scratch.Num() < Size)
			Scratch.Set(Size);

		return Scratch.GetData();
	}

	void LoadShaders();

private:
	UBOOL                     bFirstRun;
	UBOOL                     bFixCanvasScaling;
	INT                       TextureAnisotropy;
	UBOOL                     bVSync;
	UBOOL                     bAdaptiveVSync;
	UBOOL                     bUseOffscreenFramebuffer;
	UBOOL                     bDebugOpenGL;
	UBOOL                     bIsFullscreen;
	INT                       SavedViewportWidth;
	INT                       SavedViewportHeight;

	FOpenGLIndexBuffer*       DynamicIndexBuffer16;
	FOpenGLIndexBuffer*       DynamicIndexBuffer32;
	FOpenGLVertexStream*      DynamicVertexStream;
	TMap<DWORD, unsigned int> VAOsByDeclId;

	TArray<BYTE>              Scratch;
	FOpenGLResource*          ResourceHash[4096];

	FStringNoInit             ShaderDir;

	TMap<UHardwareShader*, FShaderGLSL> GLShaderByHardwareShader;

	friend class FOpenGLResource;
	friend class FOpenGLRenderInterface;

	void AddResource(FOpenGLResource* Resource);
	void RemoveResource(FOpenGLResource* Resource);

	unsigned int GetVAO(const FStreamDeclaration* Declarations, INT NumStreams);
	FOpenGLIndexBuffer* GetDynamicIndexBuffer(INT IndexSize);
	FOpenGLVertexStream* GetDynamicVertexStream();

	bool LoadVertexShader(FShaderGLSL* Shader);
	bool LoadFragmentShader(FShaderGLSL* Shader);
	void SaveVertexShader(FShaderGLSL* Shader);
	void SaveFragmentShader(FShaderGLSL* Shader);

	FStringTemp MakeShaderFilename(FShaderGLSL* Shader, const TCHAR* Extension);
	void SaveShaderText(const FFilename& Filename, const FString& Text);

	// Shader conversion

	static UHardwareShaderMacros* HardwareShaderMacros;
	static TMap<FString, FString> HardwareShaderMacroText;

	static void ClearHardwareShaderMacros();
	static void ExpandHardwareShaderMacros(FString* ShaderText);
	static FStringTemp GLSLVertexShaderFromD3DVertexShader(UHardwareShader* Shader);
	static FStringTemp GLSLFragmentShaderFromD3DPixelShader(UHardwareShader* Shader);
	static bool ConvertD3DAssemblyToGLSL(const TCHAR* Text, FString* Out);
};
