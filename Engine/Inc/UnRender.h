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
// FDynamicLight
//

class ENGINE_API FDynamicLight{
public:
	AActor* Actor;
	INT     Revision;

	FPlane  Color;
	FVector Position;
	FVector Direction;
	FLOAT   Radius;

	UBOOL   Dynamic;
	UBOOL   Changed;
	INT     SortKey;

	INT     Padding[2]; // PADDING!!!

	FLOAT   Alpha;

	// Constructor
	FDynamicLight(AActor* InActor = NULL);

	// Update - Calculates render data for the light.
	void Update();

	// SampleIntensity
	FLOAT SampleIntensity(const FVector& SamplePosition, const FVector& SampleNormal, bool);

	// SampleLight
	FColor SampleLight(const FVector& SamplePosition, const FVector& SampleNormal);
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

/*-----------------------------------------------------------------------------
	Hit proxies.
-----------------------------------------------------------------------------*/

// Hit a Bsp surface.
struct ENGINE_API HBspSurf : public HHitProxy{
	DECLARE_HIT_PROXY(HBspSurf,HHitProxy)

	INT iSurf;

	HBspSurf(INT iInSurf) : iSurf(iInSurf){}
};

// Hit an actor.
struct ENGINE_API HActor : public HHitProxy{
	DECLARE_HIT_PROXY(HActor,HHitProxy)

	AActor* Actor;

	HActor(AActor* InActor) : Actor(InActor){}
	virtual AActor* GetActor(){ return Actor; }
};

// Hit a brush vertex.
struct HBrushVertex : public HHitProxy{
	DECLARE_HIT_PROXY(HBrushVertex,HHitProxy)

	ABrush* Brush;
	FVector Location;

	HBrushVertex(ABrush* InBrush, FVector InLocation) : Brush(InBrush), Location(InLocation){}
};

// Hit ray descriptor.
struct ENGINE_API HCoords : public HHitProxy{
	DECLARE_HIT_PROXY(HCoords,HHitProxy)

	FVector Origin;
	FVector Direction;

	HCoords(FCameraSceneNode* InFrame){
		FLOAT X = (FLOAT) ((InFrame->Viewport->HitX + InFrame->Viewport->HitXL / 2) - (InFrame->Viewport->SizeX / 2)) / (InFrame->Viewport->SizeX / 2.0f);
		FLOAT Y = (FLOAT) ((InFrame->Viewport->HitY + InFrame->Viewport->HitYL / 2) - (InFrame->Viewport->SizeY / 2)) / -(InFrame->Viewport->SizeY / 2.0f);

		Origin = InFrame->ViewOrigin;
		Direction = InFrame->Deproject(FPlane(X, Y, 0.0f, InFrame->Viewport->IsOrtho() ? 1.0f : NEAR_CLIPPING_PLANE)) - InFrame->ViewOrigin;
	}
};

// Hit terrain.
struct ENGINE_API HTerrain : public HHitProxy{
	DECLARE_HIT_PROXY(HTerrain,HHitProxy)

	ATerrainInfo* TerrainInfo;
	HTerrain(ATerrainInfo* InTerrainInfo) : TerrainInfo(InTerrainInfo){}

	virtual AActor* GetActor(){ return (AActor*)TerrainInfo; }
};

struct ENGINE_API HTerrainToolLayer : public HHitProxy{
	DECLARE_HIT_PROXY(HTerrainToolLayer,HHitProxy)

	ATerrainInfo* TerrainInfo;
	UTexture* AlphaMap; // The texture that will be painted on
	INT LayerNum;

	HTerrainToolLayer(ATerrainInfo* InTerrainInfo, INT InLayerNum, UTexture* InAlphaMap) : TerrainInfo(InTerrainInfo),
	                                                                                       LayerNum(InLayerNum),
	                                                                                       AlphaMap(InAlphaMap){}
};

class ASceneManager;
class UMatAction;
class UMatSubAction;

struct ENGINE_API HMatineeTimePath : public HHitProxy{
	DECLARE_HIT_PROXY(HMatineeTimePath,HHitProxy)

	ASceneManager* SceneManager;

	HMatineeTimePath(ASceneManager* InSceneManager) : SceneManager(InSceneManager){}
};

struct ENGINE_API HMatineeScene : public HHitProxy{
	DECLARE_HIT_PROXY(HMatineeScene,HHitProxy)

	ASceneManager* SceneManager;

	HMatineeScene(ASceneManager* InSceneManager) : SceneManager(InSceneManager){}
};

struct ENGINE_API HMatineeAction : public HHitProxy{
	DECLARE_HIT_PROXY(HMatineeAction,HHitProxy)

	ASceneManager* SM;
	UMatAction* MatAction;

	HMatineeAction(ASceneManager* InSM, UMatAction* InMatAction) : SM(InSM), MatAction(InMatAction){}
};

struct ENGINE_API HMatineeSubAction : public HHitProxy{
	DECLARE_HIT_PROXY(HMatineeSubAction,HHitProxy)

	UMatAction* MatAction;
	UMatSubAction* MatSubAction;

	HMatineeSubAction(UMatSubAction* InMatSubAction, UMatAction* InMatAction) : MatSubAction(InMatSubAction), MatAction(InMatAction){}
};

struct ENGINE_API HMaterialTree : public HHitProxy{
	DECLARE_HIT_PROXY(HMaterialTree,HHitProxy);

	UMaterial* Material;
	DWORD hWnd; // The HWND of the texture properties dialog

	HMaterialTree(UMaterial* InMaterial, DWORD InHwnd) : Material(InMaterial), hWnd(InHwnd){}
};

// Hit an axis indicator on a gizmo
struct HGizmoAxis : public HHitProxy{
	DECLARE_HIT_PROXY(HGizmoAxis,HHitProxy)

	AActor* Actor;
	INT Axis;

	HGizmoAxis(AActor* InActor, INT InAxis) : Actor(InActor), Axis(InAxis){}
	virtual AActor* GetActor(){ return Actor; }
};

// Hit an actor vertex.
struct HActorVertex : public HHitProxy{
	DECLARE_HIT_PROXY(HActorVertex,HHitProxy)

	AActor* Actor;
	FVector Location;

	HActorVertex(AActor* InActor, FVector InLocation ) : Actor(InActor), Location(InLocation){}
	virtual AActor* GetActor(){ return Actor; }
};

// Hit a bezier control point
struct HBezierControlPoint : public HHitProxy{
	DECLARE_HIT_PROXY(HBezierControlPoint,HHitProxy)

	UMatAction* MA;
	UBOOL bStart; // Is this the starting(=0) or ending(=1) control point?

	HBezierControlPoint(UMatAction* InMA, UBOOL InStart) : MA(InMA), bStart(InStart){}
	UBOOL operator==(const HBezierControlPoint& BCP) const{ return (MA == BCP.MA && bStart == BCP.bStart); }
};

// The following hit proxies are from Editor.dll

// Hit a texture view.
struct HTextureView : public HHitProxy{
	DECLARE_HIT_PROXY(HTextureView,HHitProxy)

	UMaterial* Material;
	INT ViewX, ViewY;

	HTextureView(UMaterial* InMaterial, INT InX, INT InY) : Material(InMaterial), ViewX(InX), ViewY(InY){}
	void Click(const FHitCause& Cause);
};

// Hit a global pivot.
struct HGlobalPivot : public HHitProxy{
	DECLARE_HIT_PROXY(HGlobalPivot,HHitProxy)

	FVector Location;

	HGlobalPivot(FVector InLocation) : Location(InLocation){}
};

// Hit a browser texture.
struct HBrowserMaterial : public HHitProxy{
	DECLARE_HIT_PROXY(HBrowserMaterial,HHitProxy)

	UMaterial* Material;

	HBrowserMaterial(UMaterial* InMaterial) : Material(InMaterial){}
};

// Hit the backdrop.
struct HBackdrop : public HHitProxy{
	DECLARE_HIT_PROXY(HBackdrop,HHitProxy)

	FVector Location;

	HBackdrop(FVector InLocation) : Location(InLocation){}
	void Click(const FHitCause& Cause);
};

#endif

/*------------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------------*/
