/*=============================================================================
	UnDynBsp.h: Unreal dynamic Bsp object support
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*---------------------------------------------------------------------------------------
	FMovingBrushTrackerBase virtual base class.
---------------------------------------------------------------------------------------*/

//
// Moving brush tracker.
//
class FMovingBrushTrackerBase
{
public:
	// Constructors/destructors.
	virtual ~FMovingBrushTrackerBase() {};

	// Public operations:
	virtual void Update( AActor* Actor )=0;
	virtual void Flush( AActor* Actor )=0;
	virtual UBOOL SurfIsDynamic( INT iSurf )=0;
	virtual void CountBytes( FArchive& Ar )=0;
};
ENGINE_API FMovingBrushTrackerBase* GNewBrushTracker( ULevel* Level );

/*---------------------------------------------------------------------------------------
	The End.
---------------------------------------------------------------------------------------*/
