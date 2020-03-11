#include "../../Core/Inc/Core.h"

#ifndef MOD_API
#define MOD_API DLL_IMPORT
LINK_LIB(Mod)
#endif

/**
 * Patches the given vtable with a custom function and returns the old function.
 * @param The vtable to patch. It can be obtained like so: *reinterpret_cast<void***>(PointerToObject).
 * @param Index The index of the function in the vtable. *NOT* the byte offset.
 * @param Func The function which is inserted into VTable at Index.
 * @return The function pointer that was at Index previously. NULL if there was an error.
 */
MOD_API void* PatchVTable(void** VTable, INT Index, void* Func);