#pragma once

#ifndef MOD_NATIVE_DEFS
	#ifndef MOD_API
		LINK_LIB(Mod)
	#endif //MOD_API
#else
	#error "Don't include ModClasses.h directly, include Mod.h instead!"
#endif //MOD_NATIVE_DEFS

#include "../../Engine/Inc/Engine.h"
#include "ModClasses.h"