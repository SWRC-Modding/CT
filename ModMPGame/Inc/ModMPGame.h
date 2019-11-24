#include "../../Engine/Inc/Engine.h"

#ifndef MODMPGAME_API
LINK_LIB(ModMPGame)
#endif

#include "ModMPGameClasses.h"

MODMPGAME_API APlayerController* GetLocalPlayerController();
