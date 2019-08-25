/*=============================================================================
	UnRender.h: Rendering functions and structures
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _UNRENDER_H_
#define _UNRENDER_H_

/*------------------------------------------------------------------------------------
	Forward declarations.
------------------------------------------------------------------------------------*/

class FSceneNode;

/*------------------------------------------------------------------------------------
	Includes.
------------------------------------------------------------------------------------*/

#include "UnRenDev.h"

/*------------------------------------------------------------------------------------
	Defines.
------------------------------------------------------------------------------------*/

#define ORTHO_LOW_DETAIL 40000.0f

/*------------------------------------------------------------------------------------
	FSceneNode.
------------------------------------------------------------------------------------*/

//
// A scene frame is a temporary object representing a portion of
// the view of the world to render.
//
class ENGINE_API FSceneNode{
public:

	UViewport*		Viewport;
	FRenderTarget*	RenderTarget;
	FSceneNode*		Parent;
	INT				Recursion;

	FMatrix		WorldToCamera,
				CameraToWorld,
				CameraToScreen,
				ScreenToCamera,
				WorldToScreen,
				ScreenToWorld;
	FVector		ViewOrigin,
				CameraX,
				CameraY;

	FLOAT		Determinant;		// WorldToScreen.Determinant();

	// Constructor/destructor
	FSceneNode(UViewport* InViewport,FRenderTarget* InRenderTarget);
	FSceneNode(FSceneNode* InParent);
	virtual ~FSceneNode();

	// Project - Projects a point from the scene node's coordinate system into screen space.
	FPlane Project(FVector V);

	// Deproject - Deprojects a point from screen space into the scene node's coordinate system.
	FVector Deproject(FPlane P);

	// GetLodSceneNode - Determines the scenenode to use for determining LOD.
	virtual FSceneNode* GetLodSceneNode(){ return this; }

	// Render
	virtual void Render(FRenderInterface* RI) = 0;

	// Subinterfaces.
	virtual class FLevelSceneNode* GetLevelSceneNode(){ return NULL; }
	virtual class FCameraSceneNode* GetCameraSceneNode(){ return NULL; }
	virtual class FActorSceneNode* GetActorSceneNode(){ return NULL; }
	virtual class FSkySceneNode* GetSkySceneNode(){ return NULL; }
	virtual class FMirrorSceneNode* GetMirrorSceneNode(){ return NULL; }
	virtual class FWarpZoneSceneNode* GetWarpZoneSceneNode(){ return NULL; }
	virtual UBOOL CanUseHardwareOcclusion(){ return 0; }
};

#endif

/*------------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------------*/
