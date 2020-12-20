#pragma once

#include "OpenGLDrv.h"
#include "OpenGLRenderInterface.h"
#include "Shader.h"

class FOpenGLResource;
class FOpenGLShader;
class FOpenGLRenderTarget;

#define FRAGMENT_SHADER_FILE_EXTENSION ".fsh"
#define VERTEX_SHADER_FILE_EXTENSION ".vsh"
#define MAX_SHADER_SUBROUTINES 32

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

	UOpenGLRenderDevice();
	void StaticConstructor();

	void MakeCurrent();
	bool IsCurrent();
	void UnSetRes();
	FOpenGLResource* GetCachedResource(QWORD CacheId);

	// Overrides
	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar);
	virtual UBOOL Init(){ return 1; }
	virtual UBOOL SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes = 0, UBOOL bSaveSize = 1);
	virtual void Exit(UViewport* Viewport);
	virtual void Flush(UViewport* Viewport);
	virtual void FlushResource(QWORD CacheId);
	virtual UBOOL ResourceCached(QWORD CacheId);
	virtual FMemCount ResourceMem(FRenderResource*, UObject*){ return FMemCount(); }
	virtual FMemCount ResourceMemTotal(){ return FMemCount(); }
	virtual void UpdateGamma(UViewport* Viewport){}
	virtual void RestoreGamma(){}
	virtual UBOOL VSyncEnabled(){ return 0; }
	virtual void EnableVSync(bool bEnable){}
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

	TMap<FString, FString>    VertexShaderTextByName;
	TMap<FString, FString>    FragmentShaderTextByName;

	static FString            CommonShaderHeaderText;
	static FString            VertexShaderVarsText;
	static FString            FragmentShaderVarsText;
	static FString            FixedFunctionVertexShaderText;
	static FString            FixedFunctionFragmentShaderText;
	static FString            FramebufferVertexShaderText;
	static FString            FramebufferFragmentShaderText;

	friend class FOpenGLResource;
	friend class FOpenGLRenderInterface;

	void AddResource(FOpenGLResource* Resource);
	void RemoveResource(FOpenGLResource* Resource);

	unsigned int GetVAO(const FStreamDeclaration* Declarations, INT NumStreams);
	FOpenGLIndexBuffer* GetDynamicIndexBuffer(INT IndexSize);
	FOpenGLVertexStream* GetDynamicVertexStream();

	void LoadShader(FShaderGLSL* Shader);
};
