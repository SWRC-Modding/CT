#pragma once

#include "Core.h"

/*
 * Patches the given vtable with a custom function and returns the old function.
 * NOTE: This changes the vtable for all objects of the same class.
 * Returns the function pointer that was at Index previously. NULL if there was an error.
 */
inline void* PatchVTable(void* Object, INT Index, void* NewFunc)
{
	DWORD OldProtect;
	void** VTable = *reinterpret_cast<void***>(Object);

	if(!VirtualProtect(VTable + Index, sizeof(void*), PAGE_READWRITE, &OldProtect))
	{
		debugf(NAME_Error, "Failed to patch vtable: VirtualProtect failed with error code %i", GetLastError());

		return NULL;
	}

	void* OldFunc = VTable[Index];
	VTable[Index] = NewFunc;

	VirtualProtect(VTable + Index, sizeof(void*), OldProtect, &OldProtect);

	return OldFunc;
}

/*
 * Patches the vtable for a class that is exported from a dll.
 * VTableName is the name of the particular vtable to patch in case of multiple virtual inheritance.
 */
inline void* PatchDllClassVTable(const TCHAR* DllName, const TCHAR* ClassName, const TCHAR* VTableName, INT Index, void* NewFunc)
{
	void* Handle = appGetDllHandle(DllName);

	if(!Handle)
	{
		debugf(NAME_Error, "Failed to patch vtable for class '%s': Failed to load library '%s'", ClassName, DllName);

		return NULL;
	}

	FString DllExportName = (FStringTemp("??_7") + ClassName + "@@6B" + (VTableName ? FString(VTableName) + "@@@" : "@"));
	void** VTable = static_cast<void**>(appGetDllExport(Handle, *DllExportName));

	if(!VTable)
	{
		debugf(NAME_Error, "Failed to patch vtable for class '%s': Dll export for vtable '%s' not found", ClassName, VTableName);

		return NULL;
	}

	appFreeDllHandle(Handle); // Decrementing reference count just in case. This might cause a crash later if the dll wasn't already loaded which is required.

	return PatchVTable(&VTable, Index, NewFunc);
}

/*
 * Redirects the function at the source address to the target function by inserting a relative jump instruction.
 * The original function can't be called anymore.
 */
inline void RedirectFunction(void* Src, void* Target)
{
	DWORD OldProtect;
	if(VirtualProtect(Src, 5, PAGE_EXECUTE_READWRITE, &OldProtect))
	{
		*(BYTE*)Src = 0xE9; // JMP
		*((DWORD*)((BYTE*)Src + 1)) = (DWORD)Target - (DWORD)Src - 5;
		VirtualProtect(Src, 5, OldProtect, &OldProtect);
	}
	else
	{
		debugf(NAME_Error, "Failed to redirect function at %p to %p", Src, Target);
	}
}
