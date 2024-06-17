/*=============================================================================
	UnMesh.h: Unreal mesh objects.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	FMeshVert.
-----------------------------------------------------------------------------*/

// Packed mesh vertex point for skinned meshes.
#define GET_MESHVERT_DWORD(mv) (*(DWORD*)&(mv))
struct FMeshVert
{
	// Variables.
#if __INTEL_BYTE_ORDER__
	INT X:11; INT Y:11; INT Z:10;
#else
	INT Z:10; INT Y:11; INT X:11;
#endif

	// Constructor.
	FMeshVert()
	{}
	FMeshVert( const FVector& In )
	: X(appRound(In.X)), Y(appRound(In.Y)), Z(appRound(In.Z))
	{}

	// Functions.
	FVector Vector() const
	{
		return FVector( X, Y, Z );
	}

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FMeshVert& V )
	{
		return Ar << GET_MESHVERT_DWORD(V);
	}
};

/*-----------------------------------------------------------------------------
	FMeshTri.
-----------------------------------------------------------------------------*/

// Texture coordinates associated with a vertex and one or more mesh triangles.
// All triangles sharing a vertex do not necessarily have the same texture
// coordinates at the vertex.
struct FMeshUV
{
	BYTE U;
	BYTE V;
	friend FArchive &operator<<( FArchive& Ar, FMeshUV& M )
		{return Ar << M.U << M.V;}
};

struct FMeshFloatUV
{
	FLOAT U;
	FLOAT V;
	friend FArchive &operator<<( FArchive& Ar, FMeshFloatUV& M )
		{return Ar << M.U << M.V;}
};

// One triangular polygon in a mesh, which references three vertices,
// and various drawing/texturing information.
struct FMeshTri
{
	_WORD		iVertex[3];		// Vertex indices.
	FMeshUV		Tex[3];			// Texture UV coordinates.
	DWORD		PolyFlags;		// Surface flags.
	INT			TextureIndex;	// Source texture index.
	friend FArchive &operator<<( FArchive& Ar, FMeshTri& T )
	{
		Ar << T.iVertex[0] << T.iVertex[1] << T.iVertex[2];
		Ar << T.Tex[0] << T.Tex[1] << T.Tex[2];
		Ar << T.PolyFlags << T.TextureIndex;
		return Ar;
	}
};

/*-----------------------------------------------------------------------------
	LOD Mesh structs.
-----------------------------------------------------------------------------*/

// Compact structure to send processing info to mesh digestion at import time.
struct ULODProcessInfo
{
	UBOOL LevelOfDetail;
	UBOOL NoUVData;
	UBOOL OldAnimFormat;
	INT   SampleFrame;
	INT   MinVerts;
	INT   Style;
};

// LOD-style Textured vertex struct. references one vertex, and
// contains texture U,V information. 4 bytes.
// One triangular polygon in a mesh, which references three vertices,
// and various drawing/texturing information.
struct FMeshWedge
{
	_WORD		iVertex;		// Vertex index.
	FMeshUV		TexUV;			// Texture UV coordinates. ( 2 bytes)
	friend FArchive &operator<<( FArchive& Ar, FMeshWedge& T )
	{
		Ar << T.iVertex << T.TexUV;
		return Ar;
	}

	FMeshWedge& operator=( const FMeshWedge& Other )
	{
		// Superfast copy by considering it as a DWORD.
		*(DWORD*)this = *(DWORD*)&Other;
		return *this;
	}
};

// Extended wedge - floating-point UV coordinates.
struct FMeshExtWedge
{
	_WORD		    iVertex;		// Vertex index.
	_WORD           Flags;          // Reserved 16 bits of flags
	FMeshFloatUV	TexUV;			// Texture UV coordinates. ( 2 DWORDS)
	friend FArchive &operator<<( FArchive& Ar, FMeshExtWedge& T )
	{
		Ar << T.iVertex << T.Flags << T.TexUV;
		return Ar;
	}
};



// LOD-style triangular polygon in a mesh, which references three textured vertices.  8 bytes.
struct FMeshFace
{
	_WORD		iWedge[3];		// Textured Vertex indices.
	_WORD		MaterialIndex;	// Source Material (= texture plus unique flags) index.

	friend FArchive &operator<<( FArchive& Ar, FMeshFace& F )
	{
		Ar << F.iWedge[0] << F.iWedge[1] << F.iWedge[2];
		Ar << F.MaterialIndex;
		return Ar;
	}

	FMeshFace& operator=( const FMeshFace& Other )
	{
		guardSlow(FMeshFace::operator=);
		this->iWedge[0] = Other.iWedge[0];
		this->iWedge[1] = Other.iWedge[1];
		this->iWedge[2] = Other.iWedge[2];
		this->MaterialIndex = Other.MaterialIndex;
		return *this;
		unguardSlow;
	}
};

// LOD-style mesh material.
struct FMeshMaterial
{
	DWORD		PolyFlags;		// Surface flags.
	INT			TextureIndex;	// Source texture index.
	friend FArchive &operator<<( FArchive& Ar, FMeshMaterial& M )
	{
		return Ar << M.PolyFlags << M.TextureIndex;
	}
};



/*-----------------------------------------------------------------------------
	FMeshVertConnect.
-----------------------------------------------------------------------------*/

// Says which triangles a particular mesh vertex is associated with.
// Precomputed so that mesh triangles can be shaded with Gouraud-style
// shared, interpolated normal shading.
struct FMeshVertConnect
{
	INT	NumVertTriangles;
	INT	TriangleListOffset;
	friend FArchive &operator<<( FArchive& Ar, FMeshVertConnect& C )
		{return Ar << C.NumVertTriangles << C.TriangleListOffset;}
};

/*-----------------------------------------------------------------------------
	UMesh.
-----------------------------------------------------------------------------*/

class UMeshInstance;

class ENGINE_API UMesh : public UPrimitive{
	DECLARE_CLASS(UMesh,UPrimitive,0,Engine)
public:
	 UMeshInstance* DefMeshInstance;

	 UMesh();

	 // TODO: Virtual functions
};

/*-----------------------------------------------------------------------------
	UMeshInstance.
-----------------------------------------------------------------------------*/

class ENGINE_API UMeshInstance : public UPrimitive{
	DECLARE_CLASS(UMeshInstance,UPrimitive,0,Engine)
public:
	 UMeshInstance();

	 INT Padding;

	 // TODO: Virtual functions
};

/*-----------------------------------------------------------------------------
	ULodMesh.
-----------------------------------------------------------------------------*/

struct FImpostorProps{
	UMaterial* Material;
	FVector    Scale3D;
	FRotator   RelativeRotation;
	FVector    RelativeLocation;
	FColor     ImpColor;
	INT        ImpSpaceMode;
	INT        ImpDrawMode;
	INT        ImpLightMode;
};

class ENGINE_API ULodMesh : public UMesh{
	DECLARE_CLASS(ULodMesh,UMesh,CLASS_SafeReplace,Engine)
public:
	INT                   InternalVersion;
	// General LOD mesh data
	INT                   ModelVerts;
	TArray<FMeshVert>     Verts;
	TArray<UMaterial*>    Materials;

	// Scaling.
	FVector               Scale; // Mesh scaling.
	FVector               Origin; // Origin in original coordinate system.
	FRotator              RotOrigin; // Amount to rotate when importing (mostly for yawing).

	// LOD-specific objects.
	TArray<_WORD>         FaceLevel;          // Minimum lod-level indicator for each face.
	TArray<FMeshFace>     Faces;              // Faces
	TArray<_WORD>         CollapseWedgeThus;  // Lod-collapse single-linked list for the wedges.
	TArray<FMeshWedge>    Wedges;             // 'Hoppe-style' textured vertices.
	TArray<FMeshMaterial> MeshMaterials;      // Materials

	// Max of x/y/z mesh scale for LOD gauging (works on top of drawscale).
	FLOAT                 MeshScaleMax;

	// Script-settable LOD controlling parameters.
	FLOAT                 LODStrength;    // Scales the (not necessarily linear) falloff of vertices with distance.
	INT                   LODMinVerts;    // Minimum number of vertices with which to draw a model.
	FLOAT                 LODMorph;       // >0.0 = allow morphing ; 0.0-1.0 = range of vertices to morph.
	FLOAT                 LODZDisplace;   // Z displacement for LOD distance-dependency tweaking.
	FLOAT                 LODHysteresis;  // Controls LOD-level change delay and morphing.

	FLOAT                 FadeSpecNear;
	FLOAT                 FadeSpecFar;
	FLOAT                 FadeDiffNear;
	FLOAT                 FadeDiffFar;

	// Impostor support.
	UBOOL                 bImpostorPresent;
	FImpostorProps        ImpostorProps;

	// Hardware specific.
	FLOAT                 SkinTesselationFactor; // Hardware triangle subdivision factor ( <=1.0 means inactive )

	// Multi-purpose content authentication key.
	DWORD                 AuthenticationKey;
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
