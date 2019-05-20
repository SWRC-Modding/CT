/*=============================================================================
	UnModel.h: Unreal UModel definition.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	UModel.
-----------------------------------------------------------------------------*/

//
// Model objects are used for brushes and for the level itself.
//
enum {MAX_NODES  = 65536};
enum {MAX_POINTS = 128000};
class ENGINE_API UModel : public UPrimitive
{
#ifndef NODECALS
	DECLARE_CLASS(UModel,UPrimitive,0,Engine)
#else
	DECLARE_CLASS(UModel,UPrimitive,CLASS_RuntimeStatic,Engine)
#endif /*NODECALS*/

	// Arrays and subobjects.
	UPolys*					Polys;
	TTransArray<FBspNode>	Nodes;
	TTransArray<FVert>      Verts;
	TTransArray<FVector>	Vectors;
	TTransArray<FVector>	Points;
	TTransArray<FBspSurf>	Surfs;
	TArray<FLightMapIndex>	LightMap;
	TArray<BYTE>			LightBits;
	TArray<FBox>			Bounds;
	TArray<INT>				LeafHulls;
	TArray<FLeaf>			Leaves;
	TArray<AActor*>			Lights;

	// Other variables.
	UBOOL					RootOutside;
	UBOOL					Linked;
	INT						MoverLink;
	INT						NumSharedSides;
	INT						NumZones;
	FZoneProperties			Zones[FBspNode::MAX_ZONES];

	// Constructors.
	UModel() : RootOutside(1),
			   Surfs(this),
			   Vectors(this),
			   Points(this),
			   Verts(this),
			   Nodes(this){
		EmptyModel( 1, 0 );
	}
	UModel( ABrush* Owner, UBOOL InRootOutside=1 );

	// UObject interface.
	void Serialize( FArchive& Ar );
	void PostLoad();

	// UPrimitive interface.
	UBOOL PointCheck
	(
		FCheckResult	&Result,
		AActor			*Owner,
		FVector			Location,
		FVector			Extent,
		DWORD           ExtraNodeFlags
	);
	UBOOL LineCheck
	(
		FCheckResult	&Result,
		AActor			*Owner,
		FVector			End,
		FVector			Start,
		FVector			Extent,
		DWORD           ExtraNodeFlags
	);
	FBox GetCollisionBoundingBox( const AActor *Owner ) const;
	FBox GetRenderBoundingBox( const AActor* Owner, UBOOL Exact );

	// UModel interface.
	void Modify( UBOOL DoTransArrays=0 );
	void BuildBound();
	void Transform( ABrush* Owner );
	void EmptyModel( INT EmptySurfInfo, INT EmptyPolys );
	void ShrinkModel();
	UBOOL PotentiallyVisible( INT iLeaf1, INT iLeaf2 );
	BYTE FastLineCheck( FVector End, FVector Start );

	// UModel transactions.
	void ModifySelectedSurfs( UBOOL UpdateMaster );
	void ModifyAllSurfs( UBOOL UpdateMaster );
	void ModifySurf( INT Index, UBOOL UpdateMaster );

	// UModel collision functions.
	typedef void (*PLANE_FILTER_CALLBACK )(UModel *Model, INT iNode, int Param);
	typedef void (*SPHERE_FILTER_CALLBACK)(UModel *Model, INT iNode, int IsBack, int Outside, int Param);
	// Signature changed as it wouldn't link to Engine.dll
	//FPointRegion PointRegion( AZoneInfo* Zone, FVector Location ) const;
	FPointRegion __thiscall PointRegion(class AZoneInfo* Zone, class FVector const & Location)const;
	FLOAT FindNearestVertex
	(
		const FVector	&SourcePoint,
		FVector			&DestPoint,
		FLOAT			MinRadius,
		INT				&pVertex
	) const;
	void PrecomputeSphereFilter
	(
		const FPlane	&Sphere
	);
	FLightMapIndex* GetLightMapIndex( INT iSurf )
	{
		guard(UModel::GetLightMapIndex);
		if( iSurf == INDEX_NONE )
			return NULL;

		FBspSurf& Surf = Surfs[iSurf];

		if( Surf.iLightMap==INDEX_NONE || !LightMap.Num() )
			return NULL;

		return &LightMap[Surf.iLightMap];
		unguard;
	}
};

class ENGINE_API UBspNodes : public UObject
{
	DECLARE_CLASS(UBspNodes,UObject,CLASS_RuntimeStatic,Engine)
	TArray<FBspNode> Element;
	INT _NumZones;
	FZoneProperties	_Zones[FBspNode::MAX_ZONES];
	UBspNodes()
	//: Element( this )
	{
		guard(UBspNodes::UBspNodes);
		_NumZones = 0;
		for( INT i=0; i<FBspNode::MAX_ZONES; i++ )
		{
			_Zones[i].ZoneActor    = NULL;
			_Zones[i].Connectivity = ((QWORD)1)<<i;
			_Zones[i].Visibility   = ~(QWORD)0;
		}	
		unguard;
	}
	void Serialize( FArchive& Ar )
	{
		guard(UBspNodes::Serialize);
		Super::Serialize(Ar);
		if( Ar.IsLoading() )
		{
			INT DbNum=Element.Num(), DbMax=DbNum;
			Ar << DbNum << DbMax;
			Element.Empty( DbNum );
			Element.AddZeroed( DbNum );

			for( INT i=0; i<Element.Num(); i++ )
				Ar << Element[i];

			Ar << AR_INDEX(_NumZones);

			for( INT i=0; i<_NumZones; i++ )
				Ar << _Zones[i];
		}
		unguardobj;
	}
};

class ENGINE_API UBspSurfs : public UObject
{
	DECLARE_CLASS(UBspSurfs,UObject,CLASS_RuntimeStatic,Engine)
	TArray<FBspSurf> Element;
	void Serialize( FArchive& Ar )
	{
		guard(FBspSurfs::Serialize);
		Super::Serialize( Ar );
		if( Ar.IsLoading() )
		{
			INT DbNum=0, DbMax=0;
			Ar << DbNum << DbMax;
			Element.Empty( DbNum );
			Element.AddZeroed( DbNum );
			for( INT i=0; i<Element.Num(); i++ )
				Ar << Element[i];
		}
		unguard;
	}
};

class ENGINE_API UVectors : public UObject
{
	DECLARE_CLASS(UVectors,UObject,CLASS_RuntimeStatic,Engine)
	TArray<FVector> Element;
	void Serialize( FArchive& Ar )
	{
		guard(UVectors::Serialize);
		Super::Serialize( Ar );
		if( Ar.IsLoading() )
		{
			INT DbNum=Element.Num(), DbMax=DbNum;
			Ar << DbNum << DbMax;
			Element.Empty( DbNum );
			Element.Add( DbNum );

			for( INT i=0; i<Element.Num(); i++ )
				Ar << Element[i];
		}
		unguard;
	}
};

class ENGINE_API UVerts : public UObject
{
	DECLARE_CLASS(UVerts,UObject,CLASS_RuntimeStatic,Engine)
	TArray<FVert> Element;
	INT NumSharedSides;
	void Serialize( FArchive& Ar )
	{
		guard(UVerts::Serialize);
		Super::Serialize( Ar );
		if( Ar.IsLoading() )
		{
			Element.CountBytes( Ar );
			INT DbNum=Element.Num(), DbMax=DbNum;
			Ar << DbNum << DbMax;
			Element.Empty( DbNum );
			Element.Add( DbNum );
			for( INT i=0; i<Element.Num(); i++ )
				Ar << Element[i];
		}
		Ar << AR_INDEX(NumSharedSides);
		unguardobj;
	}
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
