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
	FRenderInterface.
------------------------------------------------------------------------------------*/

enum EHardwareEmulationMode{
	HEM_None     = 0,
	HEM_GeForce1 = 1,
	HEM_XBox     = 2
};

class ENGINE_API FRenderInterface{
	// TODO: Add virtual functions
};

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
// FRenderCaps - render device capabilities exposed to the engine
//
struct FRenderCaps{
	INT   MaxSimultaneousTerrainLayers;
	INT   PixelShaderVersion;
	UBOOL HardwareTL;

	FRenderCaps() : MaxSimultaneousTerrainLayers(1),
	                PixelShaderVersion(0),
	                HardwareTL(0){}
};

// Codec for movies. Just a placeholder since we don't know the actual enum values!
enum ECodecType{};

//
// A movie that is rendered to a texture or the background.
//
class ENGINE_API FMovie{
public:
	UBOOL Playing;
	int   Padding;

	FMovie(FString Filename, int);

	// Virtual functions
	virtual ~FMovie(){}
	virtual int Play(int) = 0;
	virtual void Pause(int) = 0;
	virtual UBOOL IsPaused() = 0;
	virtual void StopNow() = 0;
	virtual void StopAtEnd() = 0;
	virtual UBOOL IsPlaying(){ return Playing; }
	virtual INT GetWidth() = 0;
	virtual INT GetHeight() = 0;
	virtual void PreRender(void*, int, int) = 0;
	virtual void RenderToRGBAArray(BYTE* Buffer) = 0;
	virtual void RenderToNative(void*, int, int){}
	virtual void RenderToTexture(UTexture* Texture); // Calls RenderToRGBAArray by default
	virtual void Serialize(FArchive& Ar){}
};

//
// A low-level 3D rendering device.
//
class ENGINE_API URenderDevice : public USubsystem{
	DECLARE_ABSTRACT_CLASS(URenderDevice,USubsystem,CLASS_Config,Engine)

	// Variables.
	BYTE     DecompFormat;
	INT      RecommendedLOD;
	INT      TerrainLOD;
	BITFIELD HighDetailActors;
	BITFIELD SuperHighDetailActors;
	BITFIELD DetailTextures;
	BITFIELD PrecacheOnFlip;
	BITFIELD SupportsCubemaps;
	BITFIELD SupportsZBIAS;
	BITFIELD UseCompressedLightmaps;
	BITFIELD UseStencil;
	BITFIELD Use16bit;
	BITFIELD Use16bitTextures;
	BITFIELD IsVoodoo3;
	BITFIELD Is3dfx;
	BITFIELD LowQualityTerrain;
	BITFIELD SkyboxHack;
	BITFIELD Pad1[8];
	DWORD    Pad0[8];

	// Constructors.
	void StaticConstructor();

	// URenderDevice low-level functions that drivers must implement.
	virtual UBOOL Init() = 0;
	virtual UBOOL SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes = 0, UBOOL bSaveSize = true) = 0;
	virtual void Exit(UViewport* Viewport) = 0;
	virtual void Flush(UViewport* Viewport) = 0;
	virtual void FlushResource(QWORD CacheId) = 0;
	virtual UBOOL ResourceCached(QWORD CacheId){}
	virtual struct FMemCount ResourceMem(FRenderResource*, UObject*){}
	virtual struct FMemCount ResourceMemTotal(){}
	virtual void UpdateGamma(UViewport* Viewport) = 0;
	virtual void RestoreGamma() = 0;
	virtual UBOOL VSyncEnabled() = 0;
	virtual void EnableVSync(bool bEnable) = 0;
	virtual FRenderInterface* Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize) = 0;
	virtual void Unlock(FRenderInterface* RI) = 0;
	virtual void Present(UViewport* Viewport) = 0;
	virtual void ReadPixels(UViewport* Viewport, FColor* Pixels) = 0;
	virtual void SetEmulationMode(EHardwareEmulationMode Mode) = 0;
	virtual FRenderCaps* GetRenderCaps() = 0;
	virtual void RenderMovie(UViewport* Viewport){}
	virtual FMovie* GetNewMovie(ECodecType Codec, FString Filename, int, int, int){}
	virtual int GetStateCaching(){}
	virtual int SetStateCaching(int){}
	virtual int RefreshStates(){}
	virtual UBOOL DoesSupportFSAA(int){}
	virtual void TakeScreenshot(const char*, class UViewport*, int, int){}
	virtual UBOOL SupportsTextureFormat(ETextureFormat) = 0;
};

#endif
/*------------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------------*/
