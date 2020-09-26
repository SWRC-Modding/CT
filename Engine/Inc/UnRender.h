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
	UViewport*     Viewport;
	FRenderTarget* RenderTarget;
	FSceneNode*    Parent;
	INT            Recursion;

	FMatrix WorldToCamera,
	        CameraToWorld,
	        CameraToScreen,
	        ScreenToCamera,
	        WorldToScreen,
	        ScreenToWorld;
	FVector ViewOrigin,
	        CameraX,
	        CameraY;

	FLOAT   Determinant; // WorldToScreen.Determinant();

	// Constructors
	FSceneNode(UViewport* InViewport, FRenderTarget* InRenderTarget);
	FSceneNode(FSceneNode* InParent);

	FPlane Project(const FVector& V); // Projects a point from the scene node's coordinate system into screen space.
	FVector Deproject(const FPlane& P); // Deprojects a point from screen space into the scene node's coordinate system.
	UBOOL IsChildNode();
	UBOOL IsParentNode();

	// Virtual functions
	virtual ~FSceneNode();
	virtual FSceneNode* GetLodSceneNode(){ return this; } // Determines the scenenode to use for determining LOD.
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

//
// FLevelSceneNode
//
class ENGINE_API FLevelSceneNode : public FSceneNode{
public:
	ULevel* Level;
	UModel* Model;
	AActor* ViewActor;
	INT     ViewZone;
	INT     InvisibleZone;
	DWORD   StencilMask;

	// Constructors/destructor.
	FLevelSceneNode(UViewport* InViewport, FRenderTarget* InRenderTarget);
	FLevelSceneNode(FLevelSceneNode* InParent, INT InViewZone, const FMatrix& LocalToParent);

	// Overrides
	virtual ~FLevelSceneNode();
	virtual void Render(FRenderInterface* RI);
	virtual FLevelSceneNode* GetLevelSceneNode(){ return this; }
	virtual UBOOL CanUseHardwareOcclusion();

	// Virtual functions
	virtual FConvexVolume GetViewFrustum();
	virtual UBOOL FilterActor(AActor* Actor);
    virtual UBOOL FilterAttachment(AActor* AttachedActor){ return 1; }
	virtual UBOOL FilterProjector(AProjector* Actor);
};

//
// FCameraSceneNode
//
class ENGINE_API FCameraSceneNode : public FLevelSceneNode{
public:
	FRotator ViewRotation;
	FLOAT    ViewFOV;

	// Constructor.
	FCameraSceneNode(UViewport* InViewport,
	                 FRenderTarget* InRenderTarget,
	                 AActor* CameraActor,
	                 const FVector& CameraLocation,
	                 const FRotator& CameraRotation,
	                 FLOAT CameraFOV,
	                 int);

	// Overrides
	virtual ~FCameraSceneNode();
	virtual void Render(FRenderInterface* RI);
	virtual FCameraSceneNode* GetCameraSceneNode(){ return this; }

	// Virtual functions
	virtual void UpdateMatrices(); // Allows changing parameters on the fly.
};

//
// FPlayerSceneNode
//
class ENGINE_API FPlayerSceneNode : public FCameraSceneNode{
public:
	// Constructor.
	FPlayerSceneNode(UViewport* InViewport,
	                 FRenderTarget* InRenderTarget,
	                 AActor* CameraActor,
	                 const FVector& CameraLocation,
	                 const FRotator& CameraRotation,
	                 FLOAT CameraFOV);

	// Overrides
	virtual void Render(FRenderInterface* RI);
};

//
// FActorSceneNode
//
class ENGINE_API FActorSceneNode : public FCameraSceneNode{
public:
	AActor*	RenderActor;

	// Constructor.
	FActorSceneNode(UViewport* InViewport,
	                FRenderTarget* InRenderTarget,
	                AActor* InActor,
	                AActor* CameraActor,
	                const FVector& CameraLocation,
	                const FRotator& CameraRotation,
	                FLOAT CameraFOV,
					int);

	// Overrides
	virtual void Render(FRenderInterface* RI);
	virtual FActorSceneNode* GetActorSceneNode(){ return this; }
};

//
// FSkySceneNode
//
class ENGINE_API FSkySceneNode : public FLevelSceneNode{
public:
	// Constructor.
	FSkySceneNode(FLevelSceneNode* InParent, INT InViewZone);

	// FSceneNode interface.
	virtual FSkySceneNode* GetSkySceneNode(){ return this; }
};

//
// FMirrorSceneNode
//
class ENGINE_API FMirrorSceneNode : public FLevelSceneNode{
public:
	INT	MirrorSurface;

	// Constructor.
	FMirrorSceneNode(FLevelSceneNode* InParent, const FPlane& MirrorPlane, INT InViewZone, INT InMirrorSurface);

	// Overrides
	virtual FMirrorSceneNode* GetMirrorSceneNode(){ return this; }
};

//
// FWarpZoneSceneNode
//
class ENGINE_API FWarpZoneSceneNode : public FLevelSceneNode{
public:
	// Constructor.
	FWarpZoneSceneNode(FLevelSceneNode* InParent, AWarpZoneInfo* WarpZone);

	// Overrides
	virtual FWarpZoneSceneNode* GetWarpZoneSceneNode(){ return this; }
};

#endif

/*------------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------------*/
