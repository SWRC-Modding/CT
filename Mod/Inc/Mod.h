#include "../../Core/Inc/Core.h"

#ifndef MOD_API
#define MOD_API DLL_IMPORT
LINK_LIB(Mod)
#endif

#include "ModClasses.h"

/*
 * Patches the given vtable with a custom function and returns the old function.
 * NOTE: This changes the vtable for all objects of the same class.
 * Returns the function pointer that was at Index previously. NULL if there was an error.
 */
MOD_API void* PatchVTable(void* Object, INT Index, void* NewFunc);
/*
 * Patches the vtable for a class that is exported from a dll.
 * VTableName is the name of the particular vtable to patch in case of multiple virtual inheritance.
 */
MOD_API void* PatchDllClassVTable(const TCHAR* DllName, const TCHAR* ClassName, const TCHAR* VTableName, INT Index, void* NewFunc);
