/*=============================================================================
	UnRenDev.h: 3D rendering device class.

	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _UNRENDEV_H_
#define _UNRENDEV_H_

/*------------------------------------------------------------------------------------
	URenderDevice.
------------------------------------------------------------------------------------*/

// Primitive types for DrawPrimitive.

enum EPrimitiveType
{
	PT_TriangleList,
	PT_TriangleStrip,
	PT_TriangleFan,
	PT_PointList,
	PT_LineList
};

// Flags for locking a rendering device.
enum ELockRenderFlags
{
	LOCKR_ClearScreen	    = 1,
	LOCKR_LightDiminish     = 2,
};
enum EDescriptionFlags
{
	RDDESCF_Certified       = 1,
	RDDESCF_Incompatible    = 2,
	RDDESCF_LowDetailWorld  = 4,
	RDDESCF_LowDetailSkins  = 8,
	RDDESCF_LowDetailActors = 16,
};

//
// A low-level 3D rendering device.
//
class ENGINE_API URenderDevice : public USubsystem
{
	DECLARE_ABSTRACT_CLASS(URenderDevice,USubsystem,CLASS_Config,Engine)

	char Padding[132];

	/*// Variables.
	BYTE			DecompFormat;
	INT				RecommendedLOD;
	UViewport*		Viewport;
	FString			Description;
	DWORD			DescFlags;
	BITFIELD		SpanBased;
	BITFIELD		FullscreenOnly;
	BITFIELD		SupportsFogMaps;
	BITFIELD		SupportsDistanceFog;
	BITFIELD		VolumetricLighting;
	BITFIELD		ShinySurfaces;
	BITFIELD		Coronas;
	BITFIELD		HighDetailActors;
	BITFIELD		SupportsTC;
	BITFIELD		PrecacheOnFlip;
	BITFIELD		SupportsLazyTextures;
	BITFIELD		PrefersDeferredLoad;
	BITFIELD		DetailTextures;
	BITFIELD		Pad1[8];
	DWORD			Pad0[8];*/

	//Virtual Functions

	virtual int Init() = 0;
	virtual int SetRes(class UViewport*, int, int, int, int, int) = 0;
	virtual void Exit(class UViewport*) = 0;
	virtual void Flush(class UViewport*) = 0;
	virtual void FlushResource(unsigned __int64) = 0;
	virtual int ResourceCached(unsigned __int64){}
	virtual struct FMemCount ResourceMem(class FRenderResource*, class UObject*){}
	virtual struct FMemCount ResourceMemTotal(){}
	virtual void UpdateGamma(class UViewport*) = 0;
	virtual void RestoreGamma() = 0;
	virtual int VSyncEnabled() = 0;
	virtual void EnableVSync(bool) = 0;
	virtual class FRenderInterface* Lock(class UViewport*, unsigned char*, int*) = 0;
	virtual void Unlock(class FRenderInterface*) = 0;
	virtual void Present(class UViewport*) = 0;
	virtual void ReadPixels(class UViewport*, class FColor*) = 0;
	virtual void SetEmulationMode(enum EHardwareEmulationMode) = 0;
	virtual struct FRenderCaps* GetRenderCaps() = 0;
	virtual void RenderMovie(class UViewport *){}
	virtual class FMovie* GetNewMovie(enum ECodecType, class FString, int, int, int){}
	virtual int GetStateCaching(){}
	virtual int SetStateCaching(int){}
	virtual int RefreshStates(){}
	virtual int DoesSupportFSAA(int){}
	virtual void TakeScreenshot(const char*, class UViewport*, int, int){}
	virtual int SupportsTextureFormat(enum ETextureFormat) = 0;
};

#endif
/*------------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------------*/
