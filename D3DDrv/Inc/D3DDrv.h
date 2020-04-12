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

class D3DDRV_API UD3DRenderDevice : public URenderDevice{
	DECLARE_CLASS(UD3DRenderDevice,URenderDevice,CLASS_Config,D3DDrv);
public:
	char Padding1[17920];
	class IDirect3D8*       Direct3D8;
	class IDirect3DDevice8* Direct3DDevice8;
	char Padding2[47868];

	//Overrides
	virtual int Exec(const char*, FOutputDevice&);
	virtual int Init();
	virtual int SetRes(class UViewport*, int, int, int, int, int);
	virtual void Exit(class UViewport*);
	virtual void Flush(class UViewport*);
	virtual void FlushResource(unsigned __int64);
	virtual int ResourceCached(unsigned __int64);
	virtual struct FMemCount ResourceMem(class FRenderResource*, class UObject*);
	virtual struct FMemCount ResourceMemTotal();
	virtual void UpdateGamma(class UViewport*);
	virtual void RestoreGamma();
	virtual int VSyncEnabled();
	virtual void EnableVSync(bool);
	virtual class FRenderInterface* Lock(class UViewport*, unsigned char*, int*);
	virtual void Unlock(class FRenderInterface*);
	virtual void Present(class UViewport*);
	virtual void ReadPixels(class UViewport*, class FColor*);
	virtual void SetEmulationMode(enum EHardwareEmulationMode);
	virtual struct FRenderCaps* GetRenderCaps();
	virtual void RenderMovie(class UViewport *);
	virtual class FMovie* GetNewMovie(enum ECodecType, class FString, int, int, int);
	virtual int GetStateCaching();
	virtual int SetStateCaching(int);
	virtual int RefreshStates();
	virtual int DoesSupportFSAA(int);
	virtual void TakeScreenshot(const char*, class UViewport*, int, int);
	virtual int SupportsTextureFormat(enum ETextureFormat);
};

#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif

#endif //D3DDRV_NATIVE_DEFS