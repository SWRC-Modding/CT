#pragma once

#include "../../Engine/Inc/Engine.h"
#include "opengl.h"

#ifndef OPENGLDRV_API
#define OPENGLDRV_API DLL_IMPORT
LINK_LIB(OpenGLDrv)
#endif

class UOpenGLRenderDevice;
