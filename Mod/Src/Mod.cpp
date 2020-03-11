#include "../Inc/Mod.h"

IMPLEMENT_PACKAGE(Mod)

/*
 * PatchVTable
 */
void* PatchVTable(void** VTable, INT Index, void* Func){
	DWORD OldProtect;

	if(!VirtualProtect(VTable + Index, sizeof(void*), PAGE_EXECUTE_READWRITE, &OldProtect)){
		GLog->Logf(NAME_Error, "Unable to patch vtable: VirtualProtect failed with error code %i", GetLastError());

		return NULL;
	}

	void* OldFunc = VTable[Index];

	VTable[Index] = Func;

	return OldFunc;
}
