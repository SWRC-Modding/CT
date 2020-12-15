/*=============================================================================
	UnModel.h: Unreal UModel definition.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

//
//	FBspVertex
//

struct ENGINE_API FBspVertex{
	FVector Position;
	FVector Normal;
	FLOAT   U,  V;
	FLOAT   U2, V2;

	// Serializer.

	void Serialize(FArchive& Ar);
};

//
//	FBspVertexStream
//

class FBspVertexStream : public FVertexStream{
public:
	TArray<FBspVertex> Vertices;

	// Constructor.

	FBspVertexStream();

	// FVertexStream interface.

	virtual INT GetSize();
	virtual INT GetStride();
	virtual INT GetComponents(FVertexComponent* OutComponents);
	virtual void GetStreamData(void* Dest);
	virtual void GetRawStreamData(void** Dest,INT FirstVertex);

	// FRenderResource interface.

	virtual QWORD GetCacheId();
	virtual INT GetRevision();

	// Serializer.

	void Serialize(FArchive& Ar);
};

//
//	FLightBitmap
//

class FLightBitmap{
public:
	AActor*      LightActor;
	TArray<BYTE> Bits;
	INT          SizeX, SizeY;
	INT          MinX, MinY;
	INT          MaxX, MaxY;
	INT          OffsetX, OffsetY; // Offset into framebuffer when rendering light visibility: internal use only.
	INT          Stride;

	// Constructors.

	FLightBitmap();
	FLightBitmap(AActor* InActor, INT InMinX, INT InMinY, INT InMaxX, INT InMaxY);

	// Serializer.

	void Serialize(FArchive& Ar);
};

//
//	FLightMap
//

class ENGINE_API FLightMap : public FTexture{
public:
	ULevel*              Level;
	INT                  iTexture, iSurf, iZone;
	INT                  OffsetX, OffsetY;
	INT                  SizeX, SizeY;
	FMatrix              WorldToLightMap;
	FVector              LightMapBase, LightMapX, LightMapY;
	TArray<FLightBitmap> Bitmaps;
	TArray<AActor*>      DynamicLights;

	// Constructors.

	FLightMap(){}
	FLightMap(ULevel* InLevel, INT InSurfaceIndex, INT InZoneIndex);

	// FTexture interface.

	virtual void* GetRawTextureData(INT MipIndex){ return NULL; }
	virtual void UnloadRawTextureData(INT MipIndex){}
	virtual void GetTextureData(INT MipIndex, void* Dest, INT DestStride, ETextureFormat DestFormat, UBOOL ColoredMips, int);
	virtual UTexture* GetUTexture(){ return NULL; }

	// FBaseTexture interface.

	virtual INT GetWidth(){ return SizeX; }
	virtual INT GetHeight(){ return SizeY; }
	virtual INT GetFirstMip(){ return 0; }
	virtual INT GetNumMips(){ return 1; }
	virtual ETextureFormat GetFormat(){ return TEXF_RGBA8; }

	virtual ETexClampMode GetUClamp(){ return TC_Clamp; }
	virtual ETexClampMode GetVClamp(){ return TC_Clamp; }

	// FRenderResource interface.

	virtual QWORD GetCacheId(){ return 0; }
	virtual INT GetRevision(){ return Revision; }

	// Serializer.

	void Serialize(FArchive& Ar);
};

//
//	FStaticLightMapTexture
//

class ENGINE_API FStaticLightMapTexture : public FTexture{
public:
	TLazyArray<BYTE> Data[2]; // Two mip maps.
	BYTE             Format;
	INT              Width, Height;

	// Constructors.

	FStaticLightMapTexture();

	// FTexture interface.

	virtual void* GetRawTextureData(INT MipIndex);
	virtual void UnloadRawTextureData(INT MipIndex);
	virtual void GetTextureData(INT MipIndex,void* Dest, INT DestStride, ETextureFormat DestFormat, UBOOL ColoredMips, int);
	virtual UTexture* GetUTexture(){ return NULL; }

	// FBaseTexture interface.

	virtual INT GetWidth(){ return Width; }
	virtual INT GetHeight(){ return Height; }
	virtual INT GetFirstMip();
	virtual INT GetNumMips(){ return 2; }
	virtual ETextureFormat GetFormat(){ return (ETextureFormat) Format; }
	virtual ETexClampMode GetUClamp(){ return TC_Wrap; }
	virtual ETexClampMode GetVClamp(){ return TC_Wrap; }

	// FRenderResource interface.

	virtual QWORD GetCacheId(){ return CacheId; }
	virtual INT GetRevision(){ return Revision; }

	// Serializer.

	void Serialize(FArchive& Ar);
};

//
//	FLightMapTexture
//

class ENGINE_API FLightMapTexture : public FCompositeTexture{
public:
	ULevel*                Level;
	TArray<INT>            LightMaps;
	FStaticLightMapTexture StaticTexture;

	// Constructors.

	FLightMapTexture() {}
	FLightMapTexture(ULevel* InLevel);

	// FCompositeTexture interface.

	virtual INT GetNumChildren() { return LightMaps.Num(); }
	virtual FTexture* GetChild(INT ChildIndex,INT* OutChildX,INT* OutChildY);

	// FBaseTexture interface.

	virtual INT GetWidth();//{ return LIGHTMAP_TEXTURE_WIDTH; }
	virtual INT GetHeight();//{ return LIGHTMAP_TEXTURE_HEIGHT; }
	virtual INT GetFirstMip() { return 0; }
	virtual INT GetNumMips() { return 1; }
	virtual ETextureFormat GetFormat() { return TEXF_RGBA8; }
	virtual ETexClampMode GetUClamp() { return TC_Wrap; }
	virtual ETexClampMode GetVClamp() { return TC_Wrap; }

	// FRenderResource interface.

	virtual QWORD GetCacheId() { return CacheId; }
	virtual INT GetRevision() { return Revision; }

	// Serializer.

	void Serialize(FArchive& Ar);
};

//
//	FBspSection
//

class ENGINE_API FBspSection{
public:
	FBspVertexStream Vertices;
	INT              NumNodes;

	UMaterial*       Material;
	DWORD            PolyFlags;

	INT              iLightMapTexture;

	// Constructor.

	FBspSection();

	// Serializer.

	void Serialize(FArchive& Ar);
};

/*-----------------------------------------------------------------------------
	UModel.
-----------------------------------------------------------------------------*/

//
// Model objects are used for brushes and for the level itself.
//
enum {MAX_NODES  = 65536};
enum {MAX_POINTS = 128000};
class ENGINE_API UModel : public UPrimitive{
	DECLARE_CLASS(UModel,UPrimitive,0,Engine)

	// Arrays and subobjects.
	UPolys*                  Polys;
	TTransArray<FBspNode>    Nodes;
	TTransArray<FVert>       Verts;
	TTransArray<FVector>     Vectors;
	TTransArray<FVector>     Points;
	TTransArray<FBspSurf>    Surfs;
	TArray<FBox>             Bounds;
	TArray<INT>              LeafHulls;
	TArray<FLeaf>            Leaves;
	TArray<AActor*>          Lights;

	TArray<FBspSection>      Sections;
	TArray<FLightMapTexture> LightMapTextures;
	TArray<FLightMap>        LightMaps;

	TArray<INT>              DynamicLightMaps;

	// Other variables.
	UBOOL                    RootOutside;
	UBOOL                    Linked;
	INT                      MoverLink;
	INT                      NumSharedSides;
	INT                      NumZones;
	FZoneProperties          Zones[FBspNode::MAX_ZONES];

	// Constructors.
	UModel();
	UModel(ABrush* Owner, UBOOL InRootOutside = 1);

	// UObject interface.
	void Serialize(FArchive& Ar);
	void PostLoad();

	// UPrimitive interface.
	UBOOL PointCheck(FCheckResult& Result, AActor* Owner, FVector Location, FVector Extent, DWORD ExtraNodeFlags);
	UBOOL LineCheck(FCheckResult& Result, AActor* Owner, FVector End, FVector Start, FVector Extent, DWORD ExtraNodeFlags);
	FBox GetCollisionBoundingBox(const AActor *Owner) const;
	FBox GetRenderBoundingBox(const AActor* Owner, UBOOL Exact);

	// UModel interface.
	void Modify(UBOOL DoTransArrays = 0);
	void BuildBound();
	void Transform(ABrush* Owner);
	void EmptyModel(INT EmptySurfInfo, INT EmptyPolys);
	void ShrinkModel();
	UBOOL PotentiallyVisible(INT iLeaf1, INT iLeaf2);
	BYTE FastLineCheck(FVector End, FVector Start);

	// UModel transactions.
	void ModifySelectedSurfs(UBOOL UpdateMaster);
	void ModifyAllSurfs(UBOOL UpdateMaster);
	void ModifySurf(INT Index, UBOOL UpdateMaster);

	// UModel collision functions.
	typedef void (*PLANE_FILTER_CALLBACK )(UModel *Model, INT iNode, int Param);
	typedef void (*SPHERE_FILTER_CALLBACK)(UModel *Model, INT iNode, int IsBack, int Outside, int Param);
	FPointRegion PointRegion(class AZoneInfo* Zone, const FVector& Location) const;
	FLOAT FindNearestVertex(const FVector& SourcePoint, FVector& DestPoint, FLOAT MinRadius, INT& pVertex) const;
	void PrecomputeSphereFilter(const FPlane& Sphere);
	FLightMapIndex* GetLightMapIndex(INT iSurf);
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
