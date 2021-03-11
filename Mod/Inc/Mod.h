#pragma once

#include "../../Engine/Inc/Engine.h"

#ifndef MOD_API
#define MOD_API DLL_IMPORT
LINK_LIB(Mod)
#endif

#include "ModClasses.h"

// Global init function exported with C-Linkage so that other modules can easily check for it without requiring a hard link
extern "C" MOD_API void CDECL InitSWRCFix(void);
