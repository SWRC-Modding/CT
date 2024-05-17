#include "RtxDrv.h"

IMPLEMENT_PACKAGE(RtxDrv)
IMPLEMENT_CLASS(URtxRenderDevice)

FRenderInterface* URtxRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize)
{
	RenderInterface.Impl = Super::Lock(Viewport, HitData, HitSize);

	return RenderInterface.Impl ? &RenderInterface : NULL;
}

void URtxRenderDevice::Unlock(FRenderInterface* RI)
{
	Super::Unlock(static_cast<FRtxRenderInterface*>(RI)->Impl);
}
