#include "../../Engine/Inc/Engine.h"
#include "../../CTGame/Inc/CTGame.h"

#ifndef MODMPGAME_API
#define MODMPGAME_API DLL_IMPORT
LINK_LIB(ModMPGame)
#endif

#include "ModMPGameClasses.h"

MODMPGAME_API APlayerController* GetLocalPlayerController();
