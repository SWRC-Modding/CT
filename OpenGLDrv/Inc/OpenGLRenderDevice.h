#pragma once

#include "OpenGLDrv.h"
#include "OpenGLRenderInterface.h"

class OPENGLDRV_API UOpenGLRenderDevice : public URenderDevice{
	DECLARE_CLASS(UOpenGLRenderDevice,URenderDevice,CLASS_Config,OpenGLDrv)
public:
	FOpenGLRenderInterface RenderInterface;

	// Overrides
	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar){ return 0; }
	virtual UBOOL Init(){ return 1; }
	virtual UBOOL SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes = 0, UBOOL bSaveSize = 1);
	virtual void Exit(UViewport* Viewport){}
	virtual void Flush(UViewport* Viewport){}
	virtual void FlushResource(QWORD CacheId){}
	virtual UBOOL ResourceCached(QWORD CacheId){ return 0; }
	virtual struct FMemCount ResourceMem(FRenderResource*, UObject*){ return FMemCount(); }
	virtual struct FMemCount ResourceMemTotal(){ return FMemCount(); }
	virtual void UpdateGamma(UViewport* Viewport){}
	virtual void RestoreGamma(){}
	virtual UBOOL VSyncEnabled(){ return 0; }
	virtual void EnableVSync(bool bEnable){}
	virtual FRenderInterface* Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize){ return NULL; }
	virtual void Unlock(FRenderInterface* RI){}
	virtual void Present(UViewport* Viewport){}
	virtual void ReadPixels(UViewport* Viewport, FColor* Pixels){}
	virtual void SetEmulationMode(EHardwareEmulationMode Mode){}
	virtual FRenderCaps* GetRenderCaps(){ return NULL; }
	virtual void RenderMovie(UViewport* Viewport){}
	virtual FMovie* GetNewMovie(ECodecType Codec, FString Filename, UBOOL UseSound, INT FrameRate, int){ return NULL; }
	virtual int GetStateCaching(){ return 0; }
	virtual int SetStateCaching(int){ return 0; }
	virtual int RefreshStates(){ return 0; }
	virtual UBOOL DoesSupportFSAA(int){ return 0; }
	virtual void TakeScreenshot(const char*, class UViewport*, int, int){}
	virtual UBOOL SupportsTextureFormat(ETextureFormat){ return 0; }
};
