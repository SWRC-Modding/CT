/*=============================================================================
	UnSkeletalMesh.h: Unreal mesh objects.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	UAnimation: Unreal Animation object
	Objects containing skeletal or heirarchical animation keys.
	(Classic vertex animation is stored inside UMesh object.)

	Copyright 1999,2000 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* ULodMesh, USkeletalMesh subclassing - Erik
=============================================================================*/


/*-----------------------------------------------------------------------------
	USkeletalMesh.
-----------------------------------------------------------------------------*/

class ENGINE_API USkeletalMesh : public ULodMesh{
	DECLARE_CLASS(USkeletalMesh,ULodMesh,CLASS_SafeReplace,Engine)
public:
	 char Padding[384];
};


/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
