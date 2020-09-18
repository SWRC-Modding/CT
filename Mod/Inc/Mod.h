#pragma once

#include "../../D3DDrv/Inc/D3DDrv.h"

#ifndef MOD_API
#define MOD_API DLL_IMPORT
LINK_LIB(Mod)
#endif

#include "ModClasses.h"

/*
 * Patches the given vtable with a custom function and returns the old function.
 * The vtable can be obtained like so: *reinterpret_cast<void***>(PointerToObject).
 * NOTE: This changes the vtable for all objects of the same class.
 * Returns the function pointer that was at Index previously. NULL if there was an error.
 */
MOD_API void* PatchVTable(void* Object, INT Index, void* NewFunc);

/*
 * ModRenderDevice.
 * - Fixes bumpmapping crashes by converting the bumpmaps to a compatible format
 */
class MOD_API UModRenderDevice : public UD3DRenderDevice{
	DECLARE_CLASS(UModRenderDevice, UD3DRenderDevice, 0, Mod)
public:
	static UObject* FOVChanger;

	virtual UBOOL Init();
	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar);
};
