/*=============================================================================
	Engine.h: Unreal engine public header file.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_ENGINE
#define _INC_ENGINE

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

ENGINE_API extern class UEngine*	GEngine;
ENGINE_API extern class FMemStack	GEngineMem;
ENGINE_API extern class FMemCache	GCache;
ENGINE_API extern FLOAT 			GEngineDeltaTime;
ENGINE_API extern FLOAT 			GEngineTime;

/*-----------------------------------------------------------------------------
	Engine public includes.
-----------------------------------------------------------------------------*/

#include "UnObj.h"				// Standard object definitions.
#include "UnPrim.h"				// Primitive class.
#include "UnModel.h"			// Model class.
#include "UnTex.h"				// Texture and palette.
#include "UnAnim.h"
#include "EngineClasses.h"		// All actor classes.
#include "UnURL.h"				// Uniform resource locators.
#include "UnLevel.h"			// Level object.
#include "UnIn.h"				// Input system.
#include "UnPlayer.h"			// Player class.
#include "UnEngine.h"			// Unreal engine.
#include "UnGame.h"				// Unreal game engine.
#include "UnCamera.h"			// Viewport subsystem.
#include "UnMesh.h"				// Mesh objects.
#include "UnSkeletalMesh.h"		// Skeletal model objects.
#include "UnActor.h"			// Actor inlines.
#include "UnAudio.h"			// Audio code.
#include "UnDynBsp.h"			// Dynamic Bsp objects.
#include "UnScrTex.h"			// Scripted textures.
#include "UnRenderIterator.h"	// Enhanced Actor Render Interface
#include "UnRenderIteratorSupport.h"
#include "UnRenDev.h"
#include "UnPath.h"

ENGINE_API extern class UCubemapManager* 		GCubemapManager;
ENGINE_API extern class FEngineStats 			GEngineStats;
ENGINE_API extern class UGlobalTempObjects* 	GGlobalTempObjects;
ENGINE_API extern class FMatineeTools 			GMatineeTools;
ENGINE_API extern class FPathBuilder 			GPathBuilder;
ENGINE_API extern class FRebuildTools 			GRebuildTools;
ENGINE_API extern class FStatGraph* 			GStatGraph;
ENGINE_API extern class FTempLineBatcher* 		GTempLineBatcher;
ENGINE_API extern class FTerrainTools 			GTerrainTools;

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
#endif