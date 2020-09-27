#pragma once

#include "../../Mod/Inc/Mod.h"

#ifndef TESTDRV_API
#define TESTDRV_API DLL_IMPORT
LINK_LIB(TestDrv)
#endif

class TESTDRV_API UTestRenderDevice : public UModRenderDevice{
	DECLARE_CLASS(UTestRenderDevice, UModRenderDevice, 0, TestDrv);
public:
	class FTestRenderInterface* RenderInterface;

	virtual UBOOL Init();
	virtual void Exit(UViewport* Viewport);
	virtual FRenderInterface* Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize);
	virtual void Unlock(FRenderInterface* RI);
	virtual UBOOL SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes, UBOOL bSaveSize);
	virtual void Present(UViewport* Viewport);
};
