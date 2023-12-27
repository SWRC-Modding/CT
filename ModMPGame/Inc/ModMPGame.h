#include "Engine.h"
#include "CTGame.h"

#ifndef MODMPGAME_API
#define MODMPGAME_API DLL_IMPORT
LINK_LIB(ModMPGame)
#endif

#include "ModMPGameClasses.h"

MODMPGAME_API APlayerController* GetLocalPlayerController();
