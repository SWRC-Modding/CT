/*=============================================================================
	UnRenderUtil.h: Rendering utility definitions.
	Copyright 1997-2001 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Andrew Scheidecker
=============================================================================*/

//
//  EClippingFlag
//
enum EClippingFlag
{
	CF_Inside = 0x1,
	CF_Outside = 0x2
};

//
//  FConvexVolume
//
class ENGINE_API FConvexVolume
{
public:

	enum { MAX_VOLUME_PLANES = 32 };

	FPlane	BoundingPlanes[MAX_VOLUME_PLANES];
	INT		NumPlanes;

	// Constructor
	FConvexVolume();

	// SphereCheck - Determines whether a sphere is inside of the volume, outside of the volume, or both.  Returns EClippingFlag.
	BYTE SphereCheck(FSphere Sphere);

	// BoxCheck - Determines whether a box is inside of the volume, outside of the volume, or both.  Returns EClippingFlag.
	BYTE BoxCheck(FVector Origin,FVector Extent);

	// ClipPolygon
	FPoly ClipPolygon(FPoly Polygon);
};

//
//	FLineVertex
//
class ENGINE_API FLineVertex
{
public:

	FVector	Position;
	FColor	Diffuse;

	// Constructors.
	FLineVertex() {}
	FLineVertex(FVector InPosition,FColor InDiffuse)
	{
		Position = InPosition;
		Diffuse = InDiffuse;
	}
};

//
//	FLineBatcher
//
class ENGINE_API FLineBatcher : public FVertexStream
{
public:

	TArray<FLineVertex>		Vertices;
	QWORD					CacheId;
	UBOOL					ZTest;
	FRenderInterface*		RI;
	char Padding[256];

	// Constructor/destructor.

	FLineBatcher(FRenderInterface* InRI, UBOOL InZTest=1);
	~FLineBatcher();

	// Flush - Renders all buffered lines.
	void Flush(DWORD PolyFlags = 0);

	// DrawLine - Buffers a line for rendering.
	void DrawLine(const FVector& P1, const FVector& P2, FColor Color);

	// DrawPoint - Buffers a point for rendering.  Renders the point as a square of connected lines.
	void DrawPoint(class FSceneNode* SceneNode, const FVector& P, FColor Color);

	// DrawBox - Buffers a wireframe box for rendering.
	void DrawBox(const FBox& Box, FColor Color);

	// DrawCircle - Buffers a wireframe circle for rendering.
	void DrawCircle(const FVector& Base, const FVector& X, const FVector& Y, FColor Color, FLOAT Radius, INT NumSides);

	// DrawCylinder - Buffers a wireframe cylinder for rendering.
	void DrawCylinder(FRenderInterface* RI, const FVector& Base, const FVector& X, const FVector& Y, const FVector& Z, FColor Color, FLOAT Radius, FLOAT HalfHeight, INT NumSides);

	// DrawDirectionalArrow
	void DrawDirectionalArrow(const FVector& InLocation, const FRotator& InRotation, FColor InColor, FLOAT InDrawScale = 1.0f);

	// DrawConvexVolume
	void DrawConvexVolume(const FConvexVolume& Volume, FColor Color);

	// FRenderResource interface.
	virtual QWORD GetCacheId();
	virtual INT GetRevision();


	virtual INT GetSize();
	virtual INT GetStride();
	virtual INT GetComponents(FVertexComponent* OutComponents);
	virtual void GetStreamData(void* Dest);
	virtual void GetRawStreamData(void ** Dest, INT FirstVertex );
};

// Slight hack. If we dont have a FRenderInterface handy - use GTempLineBatcher
// All lines/boxes drawn in world space.
class ENGINE_API FTempLineBatcher
{
public:
	// lines
    TArray<FVector> LineStart;
    TArray<FVector> LineEnd;
    TArray<FColor>  LineColor;

	// boxes
    TArray<FBox>  BoxArray;
    TArray<FColor>  BoxColor;

	// Constructor / Destructor

	FTempLineBatcher() {};
	~FTempLineBatcher() {};

	// Interface

	void AddLine(FVector P1,FVector P2,FColor Color)
	{
		LineStart.AddItem(P1);
		LineEnd.AddItem(P2);
		LineColor.AddItem(Color);
	}

	void AddBox(FBox Box,FColor Color)
	{
		BoxArray.AddItem(Box);
		BoxColor.AddItem(Color);
	}

	void Render(FRenderInterface* InRI, UBOOL InZTest=1);
};

//
//	FCanvasVertex
//
class ENGINE_API FCanvasVertex
{
public:

	FVector	Position;
	FColor	Diffuse;
	FLOAT	U,
			V;

	// Constructors.
	FCanvasVertex() {}
	FCanvasVertex(FVector InPosition,FColor InDiffuse,FLOAT InU,FLOAT InV)
	{
		Position = InPosition;
		Diffuse = InDiffuse;
		U = InU;
		V = InV;
	}
};

//
//	FCanvasUtil
//	Utilities for rendering stuff in "canvas" space.
//	Canvas space is in pixel coordinates, 0 through to viewport width/height.
//
class ENGINE_API FCanvasUtil : public FVertexStream
{
public:

	EPrimitiveType			PrimitiveType;
	UMaterial*				Material;
	INT						NumPrimitives;

	FRenderInterface*		RI;
	FMatrix					ScreenToCanvas,
							CanvasToScreen;

	TArray<FCanvasVertex>	Vertices;
	QWORD					CacheId;

	// Constructor/destructor.
	FCanvasUtil(FRenderTarget* RenderTarget,FRenderInterface* InRI);
	~FCanvasUtil();

	// Flush - Renders all buffered primitives.
	void Flush();

	// BeginPrimitive - Prepares to render a primitive with the given attributes.  Automatically flushes if necessary.

	void BeginPrimitive(EPrimitiveType InType,UMaterial* InMaterial);

	// DrawLine - Buffers a line for rendering.
	void DrawLine(FLOAT X1,FLOAT Y1,FLOAT X2,FLOAT Y2,FColor Color);

	// DrawPoint - Buffers a point for rendering.
	void DrawPoint(FLOAT X1,FLOAT Y1,FLOAT X2,FLOAT Y2,FLOAT Z,FColor Color);

	// DrawTile - Buffers a textured tile for rendering.
	void DrawTile(
		FLOAT X1,
		FLOAT Y1,
		FLOAT X2,
		FLOAT Y2,
		FLOAT U1,
		FLOAT V1,
		FLOAT U2,
		FLOAT V2,
		FLOAT Z,
		UMaterial* Material,
		FColor Color
		);

	// DrawString - Buffers a string for rendering.  Returns the width of the string rendered.
	INT DrawString(
		INT StartX,
		INT StartY,
		const TCHAR* Text,
		class UFont* Font,
		FColor Color
		);

	// FRenderResource interface.
	virtual QWORD GetCacheId();
	virtual INT GetRevision();

	// FVertexStream interface.
	virtual INT GetSize();
	virtual INT GetStride();
	virtual INT GetComponents(FVertexComponent* OutComponents);
	virtual void GetStreamData(void* Dest);
	virtual void GetRawStreamData(void ** Dest, INT FirstVertex );
};

//
//	FRawIndexBuffer
//
class ENGINE_API FRawIndexBuffer : public FIndexBuffer
{
public:

	TArray<_WORD>	Indices;
	QWORD			CacheId;
	INT				Revision;

	// Constructor.
	FRawIndexBuffer();

	// Stripify - Converts a triangle list into a triangle strip.
	virtual INT Stripify();

	// CacheOptimize - Orders a triangle list for better vertex cache coherency.
	virtual void CacheOptimize();

	// Serialization.
	ENGINE_API friend FArchive& operator<<(FArchive& Ar,FRawIndexBuffer& I);

	// FRenderResource interface.
	virtual QWORD GetCacheId();
	virtual INT GetRevision();

	// FIndexBuffer interface.
	virtual INT GetSize();
	virtual void GetContents(void* Data);
	virtual INT GetIndexSize();
};


//
//	FRaw32BitIndexBuffer
//
class ENGINE_API FRaw32BitIndexBuffer : public FIndexBuffer
{
public:

	TArray<DWORD>	Indices;
	QWORD			CacheId;
	INT				Revision;

	// Constructor.
	FRaw32BitIndexBuffer();

	// Serialization.
	ENGINE_API friend FArchive& operator<<(FArchive& Ar,FRaw32BitIndexBuffer& I);

	// FRenderResource interface.
	virtual QWORD GetCacheId();
	virtual INT GetRevision();

	// FIndexBuffer interface.
	virtual INT GetSize();
	virtual void GetContents(void* Data);
	virtual INT GetIndexSize();
};

//
//	FRawColorStream
//
class ENGINE_API FRawColorStream : public FVertexStream
{
public:

	TArray<FColor>			Colors;
	QWORD					CacheId;
	INT						Revision;

	// Constructor.
	FRawColorStream();

	// Serializer.
	friend ENGINE_API FArchive& operator<<(FArchive& Ar,FRawColorStream& ColorStream);

	// FRenderResource interface.
	virtual QWORD GetCacheId();
	virtual INT GetRevision();

	// FVertexStream interface.
	virtual INT GetSize();
	virtual INT GetStride();
	virtual INT GetComponents(FVertexComponent* OutComponents);
	virtual void GetStreamData(void* Dest);
	virtual void GetRawStreamData(void ** Dest, INT FirstVertex );
};

//
//  FSolidColorTexture
//

class ENGINE_API FSolidColorTexture : public FTexture
{
public:

	FColor	Color;

	INT		Revision;
	QWORD	CacheId;

	// Constructor.

	FSolidColorTexture(FColor InColor);

	// FRenderResource interface.

	virtual INT GetRevision();
	virtual QWORD GetCacheId();

	// FBaseTexture interface.

	virtual INT GetWidth();
	virtual INT GetHeight();
	virtual ETexClampMode GetUClamp();
	virtual ETexClampMode GetVClamp();
	virtual ETextureFormat GetFormat();
	virtual INT GetNumMips();
	virtual INT GetFirstMip();

	// FTexture interface.

	virtual void GetTextureData(INT MipIndex,void* Dest,INT DestStride,ETextureFormat DestFormat,UBOOL ColoredMips);
	virtual void* GetRawTextureData(INT MipIndex);
	virtual void UnloadRawTextureData( INT MipIndex ) {}

	virtual UTexture* GetUTexture();
};

//
//	FAuxRenderTarget
//

class FAuxRenderTarget : public FRenderTarget
{
public:

	QWORD			CacheId;
	INT				Revision,
					Width,
					Height;
	ETextureFormat	Format;

	// Constructor.

	FAuxRenderTarget(INT InWidth,INT InHeight,ETextureFormat InFormat);

	// FRenderResource interface.

	virtual INT GetRevision();
	virtual QWORD GetCacheId();

	// FBaseTexture interface.

	virtual INT GetWidth();
	virtual INT GetHeight();
	virtual ETexClampMode GetUClamp();
	virtual ETexClampMode GetVClamp();
	virtual ETextureFormat GetFormat();
	virtual INT GetNumMips();
	virtual INT GetFirstMip();
};
