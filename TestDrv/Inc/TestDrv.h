#pragma once

#include "../../Mod/Inc/Mod.h"

#ifndef TESTDRV_API
#define TESTDRV_API DLL_IMPORT
LINK_LIB(TestDrv)
#endif

class TESTDRV_API UTestRenderDevice : public UModRenderDevice{
	DECLARE_CLASS(UTestRenderDevice, UModRenderDevice, 0, TestDrv);
public:
	virtual UBOOL Init();
};
