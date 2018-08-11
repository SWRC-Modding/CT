#pragma once

#ifndef MODMPGAME_NATIVE_DEFS
	#ifndef MODMPGAME_API
		LINK_LIB(ModMPGame)
	#endif //MODMPGAME_API
#else
	#error "Don't include ModMPGameClasses.h directly, include ModMPGame.h instead!"
#endif //MODMPGAME_NATIVE_DEFS

#include "../../Engine/Inc/Engine.h"
#include "ModMPGameClasses.h"