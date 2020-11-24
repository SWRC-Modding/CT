#pragma once

#include "../../Engine/Inc/Engine.h"

#ifndef OPENGLDRV_API
#define OPENGLDRV_API DLL_IMPORT
LINK_LIB(OpenGLDrv)
#endif

// REMOVE!!!
#define PRINT_FUNC //debugf(NAME_Log, "######################################################################################" __FUNCTION__)
