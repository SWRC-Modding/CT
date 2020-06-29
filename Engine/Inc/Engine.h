/*=============================================================================
	Engine.h: Unreal engine public header file.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

/*----------------------------------------------------------------------------
	API.
----------------------------------------------------------------------------*/

#ifndef ENGINE_API
#define ENGINE_API DLL_IMPORT
#endif

/*-----------------------------------------------------------------------------
	Dependencies.
-----------------------------------------------------------------------------*/

#include "../../Core/inc/Core.h"

LINK_LIB(Engine)

/*-----------------------------------------------------------------------------
	Global variables.
-----------------------------------------------------------------------------*/

ENGINE_API extern class UEngine*  GEngine;
ENGINE_API extern class FMemStack GEngineMem;
ENGINE_API extern class FMemCache GCache;
ENGINE_API extern FLOAT           GEngineDeltaTime;
ENGINE_API extern FLOAT           GEngineTime;

/*-----------------------------------------------------------------------------
	Size of the world.
-----------------------------------------------------------------------------*/

#define WORLD_MAX       524288.0   /* Maximum size of the world */
#define HALF_WORLD_MAX  262144.0   /* Half the maximum size of the world */
#define HALF_WORLD_MAX1 262143.0   /* Half the maximum size of the world - 1*/
#define MIN_ORTHOZOOM   250.0      /* Limit of 2D viewport zoom in */
#define MAX_ORTHOZOOM   16000000.0 /* Limit of 2D viewport zoom out */

/*-----------------------------------------------------------------------------
	Engine public includes.
-----------------------------------------------------------------------------*/

#include "UnObj.h"                   // Standard object definitions.
#include "UnRenderResource.h"
#include "UnPrim.h"                  // Primitive class.
#include "UnModel.h"                 // Model class.
#include "UnMaterial.h"
#include "UnTex.h"                   // Texture and palette.
#include "UnAnim.h"
#include "EngineClasses.h"           // All actor classes.
#include "UnURL.h"                   // Uniform resource locators.
#include "UnLevel.h"                 // Level object.
#include "UnIn.h"                    // Input system.
#include "UnPlayer.h"                // Player class.
#include "UnEngine.h"                // Unreal engine.
#include "UnGame.h"                  // Unreal game engine.
#include "UnCamera.h"                // Viewport subsystem.
#include "UnMesh.h"                  // Mesh objects.
#include "UnSkeletalMesh.h"          // Skeletal model objects.
#include "UnActor.h"                 // Actor inlines.
#include "UnAudio.h"                 // Audio code.
#include "UnDynBsp.h"                // Dynamic Bsp objects.
#include "UnScrTex.h"                // Scripted textures.
#include "UnRenderIterator.h"        // Enhanced Actor Render Interface
#include "UnRenderIteratorSupport.h"
#include "UnRenDev.h"
#include "UnRenderUtil.h"
#include "UnPath.h"
#include "UnCDKey.h"                 // CD key validation.
#include "UnNet.h"

ENGINE_API extern class UCubemapManager*    GCubemapManager;
ENGINE_API extern class FEngineStats        GEngineStats;
ENGINE_API extern class UGlobalTempObjects* GGlobalTempObjects;
ENGINE_API extern class FMatineeTools       GMatineeTools;
ENGINE_API extern class FPathBuilder        GPathBuilder;
ENGINE_API extern class FRebuildTools       GRebuildTools;
ENGINE_API extern class FStatGraph*         GStatGraph;
ENGINE_API extern class FTempLineBatcher*   GTempLineBatcher;
ENGINE_API extern class FTerrainTools       GTerrainTools;

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
