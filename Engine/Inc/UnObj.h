/*=============================================================================
	UnObj.h: Standard Unreal object definitions.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*----------------------------------------------------------------------------
	Forward declarations.
----------------------------------------------------------------------------*/

// All engine classes.
class		UBitmap;
class			UTexture;
class				UFont;
class		UPalette;
class		USound;
class		UMusic;
class		UPrimitive;
class			UMesh;
class			UModel;
class		UPolys;
class		ULevelBase;
class			ULevel;
class			UPendingLevel;
class		UPlayer;
class			UViewport;
class			UNetConnection;
class		UConsole;

// Other classes.
struct FTextureInfo;
class  AActor;
class  ABrush;
class  FDecal;
class  ADecal;
class  FScan;

/*-----------------------------------------------------------------------------
	UBspNode.
-----------------------------------------------------------------------------*/

// Flags associated with a Bsp node.
enum EBspNodeFlags
{
	// Flags.
	NF_NotCsg			= 0x01, // Node is not a Csg splitter, i.e. is a transparent poly.
	NF_ShootThrough		= 0x02, // Can shoot through (for projectile solid ops).
	NF_NotVisBlocking   = 0x04, // Node does not block visibility, i.e. is an invisible collision hull.
	NF_PolyOccluded		= 0x08, // Node's poly was occluded on the previously-drawn frame.
	NF_BoxOccluded		= 0x10, // Node's bounding box was occluded.
	NF_BrightCorners	= 0x10, // Temporary.
	NF_IsNew 		 	= 0x20, // Editor: Node was newly-added.
	NF_IsFront     		= 0x40, // Filter operation bounding-sphere precomputed and guaranteed to be front.
	NF_IsBack      		= 0x80, // Guaranteed back.

	// Combinations of flags.
	NF_NeverMove		= 0, // Bsp cleanup must not move nodes with these tags.
};

//
// Identifies a unique convex volume in the world.
//
struct ENGINE_API FPointRegion
{
	// Variables.
	class AZoneInfo* Zone;			// Zone actor.
	INT				 iLeaf;			// Bsp leaf.
	BYTE             ZoneNumber;	// Zone number.

	// Constructors.
	FPointRegion()
	{}
	FPointRegion( class AZoneInfo* InLevel )
	:	Zone(InLevel), iLeaf(INDEX_NONE), ZoneNumber(0)
	{}
	FPointRegion( class AZoneInfo* InZone, INT InLeaf, BYTE InZoneNumber )
	:	Zone(InZone), iLeaf(InLeaf), ZoneNumber(InZoneNumber)
	{}
};

//
// FBspNode defines one node in the Bsp, including the front and back
// pointers and the polygon data itself.  A node may have 0 or 3 to (MAX_NODE_VERTICES-1)
// vertices. If the node has zero vertices, it's only used for splitting and
// doesn't contain a polygon (this happens in the editor).
//
// vNormal, vTextureU, vTextureV, and others are indices into the level's
// vector table.  iFront,iBack should be INDEX_NONE to indicate no children.
//
// If iPlane==INDEX_NONE, a node has no coplanars.  Otherwise iPlane
// is an index to a coplanar polygon in the Bsp.  All polygons that are iPlane
// children can only have iPlane children themselves, not fronts or backs.
//
class FBspNode // 64 bytes
{
public:
	enum {MAX_NODE_VERTICES=16};	// Max vertices in a Bsp node, pre clipping.
	enum {MAX_FINAL_VERTICES=24};	// Max vertices in a Bsp node, post clipping.
	enum {MAX_ZONES=64};			// Max zones per level.

	// Persistent information.
	FPlane			Plane;			// 16 Plane the node falls into (X, Y, Z, W).
	QWORD			ZoneMask;		// 8  Bit mask for all zones at or below this node (up to 64).
	INT				iVertPool;		// 4  Index of first vertex in vertex pool, =iTerrain if NumVertices==0 and NF_TerrainFront.
	INT				iSurf;			// 4  Index to surface information.

	// iBack:  4  Index to node in front (in direction of Normal).
	// iFront: 4  Index to node in back  (opposite direction as Normal).
	// iPlane: 4  Index to next coplanar poly in coplanar list.
	union { INT iBack; INT iChild[1]; };
	        INT iFront;
			INT iPlane;

	INT				iCollisionBound;// 4  Collision bound.
	INT				iRenderBound;	// 4  Rendering bound.
	BYTE			iZone[2];		// 2  Visibility zone in 1=front, 0=back.
	BYTE			NumVertices;	// 1  Number of vertices in node.
	BYTE			NodeFlags;		// 1  Node flags.
	INT				iLeaf[2];		// 8  Leaf in back and front, INDEX_NONE=not a leaf.

	// Functions.
	UBOOL IsCsg( DWORD ExtraFlags=0 ) const
	{
		return (NumVertices>0) && !(NodeFlags & (NF_IsNew | NF_NotCsg | ExtraFlags));
	}
	UBOOL ChildOutside( INT iChild, UBOOL Outside, DWORD ExtraFlags=0 ) const
	{
		return iChild ? (Outside || IsCsg(ExtraFlags)) : (Outside && !IsCsg(ExtraFlags));
	}
	ENGINE_API friend FArchive& operator<<( FArchive& Ar, FBspNode& N );
};

//
// Properties of a zone.
//
class ENGINE_API FZoneProperties
{
public:
	// Variables.
	AZoneInfo*	ZoneActor;		// Optional actor defining the zone's property.
	FLOAT		LastRenderTime;	// Most recent level TimeSeconds when rendered.
	QWORD		Connectivity;	// (Connect[i]&(1<<j))==1 if zone i is adjacent to zone j.
	QWORD		Visibility;		// (Connect[i]&(1<<j))==1 if zone i can see zone j.

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FZoneProperties& P )
	{
		guard(FZoneProperties<<);
		return Ar << *(UObject**)&P.ZoneActor << P.Connectivity << P.Visibility;
		Ar << P.LastRenderTime;
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	UBspLeaves.
-----------------------------------------------------------------------------*/

//
// Information about a convex volume.
//
class FLeaf
{
public:
	// Variables.
	INT iZone;          // The zone this convex volume is in.
	INT iPermeating;    // Lights permeating this volume considering shadowing.
	INT iVolumetric;    // Volumetric lights hitting this region, no shadowing.
	QWORD VisibleZones; // Bit mask of visible zones from this convex volume.

	// Functions.
	FLeaf()
	{}
	FLeaf( INT iInZone, INT InPermeating, INT InVolumetric, QWORD InVisibleZones )
	:	iZone(iInZone), iPermeating(InPermeating), iVolumetric(InVolumetric), VisibleZones(InVisibleZones)
	{}
	friend FArchive& operator<<( FArchive& Ar, FLeaf& L )
	{
		guard(FLeaf<<);
		return Ar << AR_INDEX(L.iZone) << AR_INDEX(L.iPermeating) << AR_INDEX(L.iVolumetric) << L.VisibleZones;
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	UBspSurf.
-----------------------------------------------------------------------------*/

//
// One Bsp polygon.  Lists all of the properties associated with the
// polygon's plane.  Does not include a point list; the actual points
// are stored along with Bsp nodes, since several nodes which lie in the
// same plane may reference the same poly.
//
class FBspSurf
{
public:

	// Persistent info.
	UTexture*	Texture;		// 4 Texture map.
	DWORD		PolyFlags;		// 4 Polygon flags.
	INT			pBase;			// 4 Polygon & texture base point index (where U,V==0,0).
	INT			vNormal;		// 4 Index to polygon normal.
	INT			vTextureU;		// 4 Texture U-vector index.
	INT			vTextureV;		// 4 Texture V-vector index.
	INT			iLightMap;		// 4 Light mesh.
	INT			iBrushPoly;		// 4 Editor brush polygon index.
	SWORD		PanU;			// 2 U-Panning value.
	SWORD		PanV;			// 2 V-Panning value.
	ABrush*		Actor;			// 4 Brush actor owning this Bsp surface.
	TArray<FDecal>	Decals;		// 12 Array decals on this surface
	TArray<INT>	Nodes;			// 12 Nodes which make up this surface

	// Functions.
	ENGINE_API friend FArchive& operator<<( FArchive& Ar, FBspSurf& Surf );
};

// Flags describing effects and properties of a Bsp polygon.
enum EPolyFlags
{
	// Regular in-game flags.
	PF_Invisible		= 0x00000001,	// Poly is invisible.
	PF_Masked			= 0x00000002,	// Poly should be drawn masked.
	PF_Translucent	 	= 0x00000004,	// Poly is transparent.
	PF_NotSolid			= 0x00000008,	// Poly is not solid, doesn't block.
	PF_Environment   	= 0x00000010,	// Poly should be drawn environment mapped.
	PF_ForceViewZone	= 0x00000010,	// Force current iViewZone in OccludeBSP (reuse Environment flag)
	PF_Semisolid	  	= 0x00000020,	// Poly is semi-solid = collision solid, Csg nonsolid.
	PF_Modulated 		= 0x00000040,	// Modulation transparency.
	PF_FakeBackdrop		= 0x00000080,	// Poly looks exactly like backdrop.
	PF_TwoSided			= 0x00000100,	// Poly is visible from both sides.
	PF_AutoUPan		 	= 0x00000200,	// Automatically pans in U direction.
	PF_AutoVPan 		= 0x00000400,	// Automatically pans in V direction.
	PF_NoSmooth			= 0x00000800,	// Don't smooth textures.
	PF_BigWavy 			= 0x00001000,	// Poly has a big wavy pattern in it.
	PF_SpecialPoly		= 0x00001000,	// Game-specific poly-level render control (reuse BigWavy flag)
	PF_SmallWavy		= 0x00002000,	// Small wavy pattern (for water/enviro reflection).
	PF_Flat				= 0x00004000,	// Flat surface.
	PF_LowShadowDetail	= 0x00008000,	// Low detaul shadows.
	PF_NoMerge			= 0x00010000,	// Don't merge poly's nodes before lighting when rendering.
	PF_CloudWavy		= 0x00020000,	// Polygon appears wavy like clouds.
	PF_DirtyShadows		= 0x00040000,	// Dirty shadows.
	PF_BrightCorners	= 0x00080000,	// Brighten convex corners.
	PF_SpecialLit		= 0x00100000,	// Only speciallit lights apply to this poly.
	PF_Gouraud			= 0x00200000,	// Gouraud shaded.
	PF_NoBoundRejection = 0x00200000,	// Disable bound rejection in OccludeBSP (reuse Gourard flag)
	PF_Unlit			= 0x00400000,	// Unlit.
	PF_HighShadowDetail	= 0x00800000,	// High detail shadows.
	PF_Portal			= 0x04000000,	// Portal between iZones.
	PF_Mirrored			= 0x08000000,	// Reflective surface.

	// Editor flags.
	PF_Memorized     	= 0x01000000,	// Editor: Poly is remembered.
	PF_Selected      	= 0x02000000,	// Editor: Poly is selected.
	PF_Highlighted      = 0x10000000,	// Editor: Poly is highlighted.   
	PF_FlatShaded		= 0x40000000,	// FPoly has been split by SplitPolyWithPlane.   

	// Internal.
	PF_EdProcessed 		= 0x40000000,	// FPoly was already processed in editorBuildFPolys.
	PF_EdCut       		= 0x80000000,	// FPoly has been split by SplitPolyWithPlane.  
	PF_RenderFog		= 0x40000000,	// Render with fogmapping.
	PF_Occlude			= 0x80000000,	// Occludes even if PF_NoOcclude.
	PF_RenderHint       = 0x01000000,   // Rendering optimization hint.

	// Combinations of flags.
	PF_NoOcclude		= PF_Masked | PF_Translucent | PF_Invisible | PF_Modulated,
	PF_NoEdit			= PF_Memorized | PF_Selected | PF_EdProcessed | PF_NoMerge | PF_EdCut,
	PF_NoImport			= PF_NoEdit | PF_NoMerge | PF_Memorized | PF_Selected | PF_EdProcessed | PF_EdCut,
	PF_AddLast			= PF_Semisolid | PF_NotSolid,
	PF_NoAddToBSP		= PF_EdCut | PF_EdProcessed | PF_Selected | PF_Memorized,
	PF_NoShadows		= PF_Unlit | PF_Invisible | PF_Environment | PF_FakeBackdrop,
	PF_Transient		= PF_Highlighted,
};

/*-----------------------------------------------------------------------------
	FLightMapIndex.
-----------------------------------------------------------------------------*/

//
// A shadow occlusion mask.
//
class ENGINE_API FShadowMask
{
public:
	AActor* Owner;
	TArray<BYTE> ShadowData;
	friend FArchive& operator<<( FArchive& Ar, FShadowMask& M );
	/*{
		return Ar << *(UObject**)&M.Owner << M.ShadowData;
	}*/
};

//
// Describes the mesh-based lighting applied to a Bsp poly.
//
class ENGINE_API FLightMapIndex
{
public:
	INT		DataOffset;
	INT		iLightActors;
	FVector Pan;
	FLOAT	UScale, VScale;
	INT     UClamp, VClamp;
	BYTE	UBits, VBits;
	friend FArchive& operator<<( FArchive& Ar, FLightMapIndex& I )
	{
		guard(FLightMapIndex<<);
		Ar << I.DataOffset;
		Ar << I.Pan;
		Ar << AR_INDEX(I.UClamp) << AR_INDEX(I.VClamp);
		Ar << I.UScale << I.VScale;
		Ar << I.iLightActors;
		return Ar;
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	UPolys.
-----------------------------------------------------------------------------*/

// Results from FPoly.SplitWithPlane, describing the result of splitting
// an arbitrary FPoly with an arbitrary plane.
enum ESplitType
{
	SP_Coplanar		= 0, // Poly wasn't split, but is coplanar with plane
	SP_Front		= 1, // Poly wasn't split, but is entirely in front of plane
	SP_Back			= 2, // Poly wasn't split, but is entirely in back of plane
	SP_Split		= 3, // Poly was split into two new editor polygons
};

//
// A general-purpose polygon used by the editor.  An FPoly is a free-standing
// class which exists independently of any particular level, unlike the polys
// associated with Bsp nodes which rely on scads of other objects.  FPolys are
// used in UnrealEd for internal work, such as building the Bsp and performing
// boolean operations.
//
class ENGINE_API FPoly
{
public:
#if 0 //WDM -- remove this...?
	// The changes to these values allow things like the 2D shape editor and brush clipping
	// to create large polys with many sides if they need to.
	//
	enum {MAX_VERTICES=32}; // Maximum vertices an FPoly may have.
#else
	enum {MAX_VERTICES=16}; // Maximum vertices an FPoly may have.
#endif
	enum {VERTEX_THRESHOLD=MAX_VERTICES-2}; // Threshold for splitting into two.

	FVector     Base;        	// Base point of polygon.
	FVector     Normal;			// Normal of polygon.
	FVector     TextureU;		// Texture U vector.
	FVector     TextureV;		// Texture V vector.
	FVector     Vertex[MAX_VERTICES]; // Actual vertices.
	DWORD       PolyFlags;		// FPoly & Bsp poly bit flags (PF_).
	ABrush*		Actor;			// Brush where this originated, or NULL.
	UTexture*	Texture;		// Texture map.
	FName		ItemName;		// Item name.
	INT			NumVertices;	// Number of vertices.
	INT			iLink;			// iBspSurf, or brush fpoly index of first identical polygon, or MAXWORD.
	INT			iBrushPoly;		// Index of editor solid's polygon this originated from.
	SWORD		PanU,PanV;		// Texture panning values.
	INT			SavePolyIndex;	// Used by multiple vertex editing to keep track of original PolyIndex into owner brush
	UBOOL		bFaceDragSel;

	// Custom functions.
	void  Init				();
	void  Reverse			();
	void  SplitInHalf		(FPoly *OtherHalf);
	void  Transform			(const FModelCoords &Coords, const FVector &PreSubtract,const FVector &PostAdd, FLOAT Orientation);
	int   Fix				();
	int   CalcNormal		( UBOOL bSilent = 0 );
	int   SplitWithPlane	(const FVector &Base,const FVector &Normal,FPoly *FrontPoly,FPoly *BackPoly,int VeryPrecise) const;
	int   SplitWithNode		(const UModel *Model,INT iNode,FPoly *FrontPoly,FPoly *BackPoly,int VeryPrecise) const;
	int   SplitWithPlaneFast(const FPlane Plane,FPoly *FrontPoly,FPoly *BackPoly) const;
	int   Split				(const FVector &Normal, const FVector &Base, int NoOverflow=0 );
	int   RemoveColinears	();
	int   Finalize			(int NoError);
	int   Faces				(const FPoly &Test) const;
	FLOAT Area				();

	// Serializer.
	ENGINE_API friend FArchive& operator<<( FArchive& Ar, FPoly& Poly );

	// Inlines.
	int IsBackfaced( const FVector &Point ) const
		{return ((Point-Base) | Normal) < 0.f;}
	int IsCoplanar( const FPoly &Test ) const
		{return Abs((Base - Test.Base)|Normal)<0.01f && Abs(Normal|Test.Normal)>0.9999f;}
};

//
// List of FPolys.
//
class ENGINE_API UPolys : public UObject{
	DECLARE_CLASS(UPolys,UObject,CLASS_RuntimeStatic,Engine)

	// Elements.
	TTransArray<FPoly> Element;

	// Constructors.
	UPolys() : Element(this){}

	// UObject interface.
	void Serialize( FArchive& Ar )
	{
		guard(UPolys::Serialize);
		Super::Serialize( Ar );
		if( Ar.IsTrans() )
		{
			Ar << Element;
		}
		else
		{
			Element.CountBytes( Ar );
			INT DbNum=Element.Num(), DbMax=DbNum;
			Ar << DbNum << DbMax;
			if( Ar.IsLoading() )
			{
				Element.Empty( DbNum );
				Element.AddZeroed( DbNum );
			}
			for( INT i=0; i<Element.Num(); i++ )
				Ar << Element[i];
		}
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	FVerts.
-----------------------------------------------------------------------------*/

//
// One vertex associated with a Bsp node's polygon.  Contains a vertex index
// into the level's FPoints table, and a unique number which is common to all
// other sides in the level which are cospatial with this side.
//
class FVert{
public:
	// Variables.
	INT 	pVertex;	// Index of vertex.
	INT		iSide;		// If shared, index of unique side. Otherwise INDEX_NONE.

	// Functions.
	friend FArchive& operator<< (FArchive &Ar, FVert &Vert)
	{
		guard(FVert<<);
		return Ar << AR_INDEX(Vert.pVertex) << AR_INDEX(Vert.iSide);
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	FDecals.
-----------------------------------------------------------------------------*/

//
// Decal associated with a surface
//
class FDecal
{
public:
	// Variables.
	FVector Vertices[4];	// Vertices of decal on surface (offset from the surface base).
	ADecal*	Actor;			// ADecal actor associated with this decal.
	TArray<INT> Nodes;		// The FBspNode indices on which this decal lies.

	// Functions.
	friend FArchive& operator<< (FArchive &Ar, FDecal &Decal)
	{
		guard(FDecal<<);
		if( !Ar.IsLoading() && !Ar.IsSaving() )
			Ar << *(UObject**)&Decal.Actor;
		return Ar;
		unguard;
	}
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
