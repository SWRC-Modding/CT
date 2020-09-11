#include "../Inc/TestDrv.h"
#include "../../Mod/Inc/Mod.h"

IMPLEMENT_PACKAGE(TestDrv)

/*
 * TestRenderDevice
 */

UBOOL UTestRenderDevice::Init(){
	GLog->Log(NAME_Init, "Initializing TestRenderDevice");

	return Super::Init();
}

IMPLEMENT_CLASS(UTestRenderDevice)
