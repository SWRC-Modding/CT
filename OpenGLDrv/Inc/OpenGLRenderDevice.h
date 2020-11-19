#pragma once

#include "OpenGLDrv.h"
#include "OpenGLRenderInterface.h"

class FOpenGLResource;
class FOpenGLShaderProgram;

class OPENGLDRV_API UOpenGLRenderDevice : public URenderDevice{
	DECLARE_CLASS(UOpenGLRenderDevice,URenderDevice,CLASS_Config,OpenGLDrv)
public:
	HDC                    DeviceContext;
	HGLRC                  OpenGLContext;
	FOpenGLRenderInterface RenderInterface;
	FRenderCaps            RenderCaps;

	FOpenGLShaderProgram*  FramebufferShader;
	FOpenGLResource*       ResourceHash[4096];

	static HGLRC CurrentContext;

	UOpenGLRenderDevice();

	void StaticConstructor();

	void MakeCurrent();
	bool IsCurrent();
	void UnSetRes();
	void RequireExt(const TCHAR* Name);
	FOpenGLResource* GetCachedResource(QWORD CacheId);

	// Overrides
	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar){ return 0; }
	virtual UBOOL Init(){ PRINT_FUNC; return 1; }
	virtual UBOOL SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes = 0, UBOOL bSaveSize = 1);
	virtual void Exit(UViewport* Viewport);
	virtual void Flush(UViewport* Viewport);
	virtual void FlushResource(QWORD CacheId);
	virtual UBOOL ResourceCached(QWORD CacheId);
	virtual FMemCount ResourceMem(FRenderResource*, UObject*){ PRINT_FUNC; return FMemCount(); }
	virtual FMemCount ResourceMemTotal(){ PRINT_FUNC; return FMemCount(); }
	virtual void UpdateGamma(UViewport* Viewport){ PRINT_FUNC; }
	virtual void RestoreGamma(){ PRINT_FUNC; }
	virtual UBOOL VSyncEnabled(){ PRINT_FUNC; return 0; }
	virtual void EnableVSync(bool bEnable){ PRINT_FUNC; }
	virtual FRenderInterface* Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize);
	virtual void Unlock(FRenderInterface* RI){ PRINT_FUNC; }
	virtual void Present(UViewport* Viewport);
	virtual void ReadPixels(UViewport* Viewport, FColor* Pixels){ PRINT_FUNC; }
	virtual void SetEmulationMode(EHardwareEmulationMode Mode){ PRINT_FUNC; }
	virtual FRenderCaps* GetRenderCaps();
	virtual void RenderMovie(UViewport* Viewport){ PRINT_FUNC; }
	virtual FMovie* GetNewMovie(ECodecType Codec, FString Filename, UBOOL UseSound, INT FrameRate, int){ PRINT_FUNC; return NULL; }
	virtual int GetStateCaching(){ PRINT_FUNC; return 0; }
	virtual int SetStateCaching(int){ PRINT_FUNC; return 0; }
	virtual int RefreshStates(){ PRINT_FUNC; return 0; }
	virtual UBOOL DoesSupportFSAA(int){ PRINT_FUNC; return 0; }
	virtual void TakeScreenshot(const char*, class UViewport*, int, int){ PRINT_FUNC; }
	virtual UBOOL SupportsTextureFormat(ETextureFormat){ PRINT_FUNC; return 0; }

private:
	unsigned int ScreenVAO;

	friend class FOpenGLResource;

	void AddResource(FOpenGLResource* Resource);
	void RemoveResource(FOpenGLResource* Resource);
};
