#include "RtxDrv.h"

IMPLEMENT_PACKAGE(RtxDrv)
IMPLEMENT_CLASS(URtxRenderDevice)

UBOOL URtxRenderDevice::Init()
{
	UClient* Client = UTexture::__Client;
	Client->Shadows = 0;
	Client->FrameFXDisabled = 1;
	Client->BloomQuality = 0;
	Client->BumpmappingQuality = 0;
	return Super::Init();
}

FRenderInterface* URtxRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize)
{
	RenderInterface.Impl = Super::Lock(Viewport, HitData, HitSize);

	return RenderInterface.Impl ? &RenderInterface : NULL;
}

void URtxRenderDevice::Unlock(FRenderInterface* RI)
{
	Super::Unlock(static_cast<FRtxRenderInterface*>(RI)->Impl);
}
