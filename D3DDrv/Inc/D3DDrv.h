#ifndef D3DDRV_NATIVE_DEFS
#define D3DDRV_NATIVE_DEFS

#include "../../Engine/Inc/Engine.h"

#if SUPPORTS_PRAGMA_PACK
#pragma pack (push,4)
#endif

#ifndef D3DDRV_API
#define D3DDRV_API DLL_IMPORT
LINK_LIB(D3DDrv)
#endif

struct IDirect3D8;
struct IDirect3DDevice8;

class D3DDRV_API UD3DRenderDevice : public URenderDevice{
	DECLARE_CLASS(UD3DRenderDevice,URenderDevice,CLASS_Config,D3DDrv);
public:
	char Padding1[17920];
	IDirect3D8*       Direct3D8;
	IDirect3DDevice8* Direct3DDevice8;
	char Padding2[47868];

	//Overrides
	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar);
	virtual UBOOL Init();
	virtual UBOOL SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes = 0, UBOOL bSaveSize = true);
	virtual void Exit(UViewport* Viewport);
	virtual void Flush(UViewport* Viewport);
	virtual void FlushResource(QWORD CacheId);
	virtual UBOOL ResourceCached(QWORD CacheId);
	virtual struct FMemCount ResourceMem(FRenderResource*, UObject*);
	virtual struct FMemCount ResourceMemTotal();
	virtual void UpdateGamma(UViewport* Viewport);
	virtual void RestoreGamma();
	virtual UBOOL VSyncEnabled();
	virtual void EnableVSync(bool bEnable);
	virtual FRenderInterface* Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize);
	virtual void Unlock(FRenderInterface* RI);
	virtual void Present(UViewport* Viewport);
	virtual void ReadPixels(UViewport* Viewport, FColor* Pixels);
	virtual void SetEmulationMode(EHardwareEmulationMode Mode);
	virtual FRenderCaps* GetRenderCaps();
	virtual void RenderMovie(UViewport* Viewport);
	virtual FMovie* GetNewMovie(ECodecType Codec, FString Filename, UBOOL UseSound, INT FrameRate, int);
	virtual int GetStateCaching();
	virtual int SetStateCaching(int);
	virtual int RefreshStates();
	virtual UBOOL DoesSupportFSAA(int);
	virtual void TakeScreenshot(const char*, class UViewport*, int, int);
	virtual UBOOL SupportsTextureFormat(ETextureFormat);
};

#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif

#endif // D3DDRV_NATIVE_DEFS
