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

// Cull modes.
enum ECullMode{
	CM_CW,
	CM_CCW,
	CM_None
};

// Transform types.
enum ETransformType{
	TT_LocalToWorld,
	TT_WorldToCamera,
	TT_CameraToScreen
};

// Primitive types for DrawPrimitive.
enum EPrimitiveType{
	PT_TriangleList,
	PT_TriangleStrip,
	PT_TriangleFan,
	PT_PointList,
	PT_LineList
};

// Vertex shaders for SetVertexStreams.
enum EVertexShader{
	VS_FixedFunction
};

// Stencil buffer operations.
enum EStencilOp{
	SO_Keep = 1,
	SO_Zero,
	SO_Replace,
	SO_IncrementSat,
	SO_DecrementSat,
	SO_Invert,
	SO_Increment,
	SO_Decrement
};

// Comparison functions.
enum ECompareFunction{
	CF_Never = 1,
	CF_Less,
	CF_Equal,
	CF_LessEqual,
	CF_Greater,
	CF_NotEqual,
	CF_GreaterEqual,
	CF_Always
};

enum EHardwareEmulationMode{
	HEM_None,
	HEM_GeForce1,
	HEM_XBox
};

// Precaching options.
enum EPrecacheMode{
	PRECACHE_VertexBuffers,
	PRECACHE_All
};

class ENGINE_API FRenderInterface{
public:
	virtual void PushState(int) = 0;
	virtual void PopState(int) = 0;
	virtual UBOOL SetRenderTarget(FRenderTarget* RenderTarget, bool) = 0;
	virtual UBOOL SetCubeRenderTarget(class FDynamicCubemap*, int, int){}
	virtual void SetViewport(INT X, INT Y, INT Width, INT Height) = 0;
	virtual void Clear(UBOOL UseColor = 1, FColor Color = FColor(0, 0, 0), UBOOL UseDepth = 1, FLOAT Depth = 1.0f, UBOOL UseStencil = 1, DWORD Stencil = 0) = 0;
	virtual void PushHit(const BYTE* Data, INT Count) = 0;
	virtual void PopHit(INT Count, UBOOL Force) = 0;
	virtual void SetCullMode(ECullMode CullMode) = 0;
	virtual void SetAmbientLight(FColor Color) = 0;
	virtual void EnableLighting(UBOOL UseDynamic, UBOOL UseStatic = 1, UBOOL Modulate2X = 0, FBaseTexture* UseLightmap = NULL, UBOOL LightingOnly = 0, const FSphere& LitSphere = FSphere(FVector(0, 0, 0), 0), int = 0) = 0;
	virtual void SetLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale = 1.0f) = 0;
	virtual void SetShaderLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale = 1.0f){}
	virtual void SetNPatchTesselation(FLOAT Tesselation) = 0;
	virtual void SetDistanceFog(UBOOL Enable, FLOAT FogStart, FLOAT FogEnd, FColor Color) = 0;
	virtual UBOOL EnableFog(UBOOL Enable){ return 0; }
	virtual UBOOL IsFogEnabled(){ return 0; }
	virtual void SetGlobalColor(FColor Color) = 0;
	virtual void SetTransform(ETransformType Type, const FMatrix& Matrix){}
	virtual FMatrix GetTransform(ETransformType Type) const{ return FMatrix::Identity; }
	virtual void SetMaterial(UMaterial* Material, FString* ErrorString = NULL, UMaterial** ErrorMaterial = NULL, INT* NumPasses = NULL) = 0;
	virtual UBOOL SetHardwareShaderMaterial(UHardwareShader* Material, FString* ErrorString = NULL, UMaterial** ErrorMaterial = NULL){ return 0; }
	virtual UBOOL ShowAlpha(UMaterial*){ return 0; }
	virtual UBOOL IsShadowInterface(){ return 0; }
	virtual void SetAntiAliasing(int){}
	virtual void CopyBackBufferToTarget(FAuxRenderTarget*){}
	virtual void SetLODDiffuseFade(float){}
	virtual void SetLODSpecularFade(float){}
	virtual void SetStencilOp(ECompareFunction Test, DWORD Ref, DWORD Mask, EStencilOp FailOp, EStencilOp ZFailOp, EStencilOp PassOp, DWORD WriteMask);
	virtual void vtpad1() = 0; // Possibly stencil op related (modifies same memory);
	virtual void vtpad2() = 0; // Possibly stencil op related (modifies same memory);
	virtual void SetPrecacheMode(EPrecacheMode PrecacheMode) = 0;
	virtual void SetZBias(INT ZBias) = 0;
	virtual INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams) = 0;
	virtual INT SetDynamicStream(EVertexShader Shader, FVertexStream* Stream) = 0;
	virtual INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex) = 0;
	virtual INT SetDynamicIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex) = 0;
	virtual void DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex = INDEX_NONE, INT MaxIndex = INDEX_NONE) = 0;
	virtual void PixoSetHint(DWORD Hint){}
	virtual void PixoResetHint(DWORD Hint){}
	virtual UTexture* PixoCreateTexture(FRenderTarget* RenderTarget, UBOOL CreateMips){ return NULL; }
	virtual UBOOL PixoIsVisible(FBox&){ return 1; }
	virtual bool IsVertexBufferBusy(FVertexStream*){ return false; }
	virtual void SetFillMode(EFillMode FillMode){}
	virtual int vtpad3(){ return 1; };
	virtual int vtpad4(){ return 1; };
	virtual int vtpad5(){ return 1; };

	// The following virtual functions belong to FD3DRenderInterface and should be removed again
	virtual int d3d1(int, int) = 0;
	virtual int d3d2(int) = 0;
	virtual int d3d3(int) = 0;
};

/*------------------------------------------------------------------------------------
	URenderDevice.
------------------------------------------------------------------------------------*/

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

// Codec for movies.
enum ECodecType{
	CODEC_None,
	CODEC_RoQ,
	CODEC_Unused, // Treated like CODEC_None by UD3DRenderDevice
	CODEC_AVI
};

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
// A movie using the RoQ format from the Quake engine.
//
class ENGINE_API FRoQMovie : public FMovie{
public:
	INT Padding[8];

	FRoQMovie(FString, int, int);

	// Overrides
	virtual ~FRoQMovie();
	virtual UBOOL Play(int);
	virtual void Pause(int);
	virtual UBOOL IsPaused();
	virtual void StopNow();
	virtual void StopAtEnd();
	virtual INT GetWidth();
	virtual INT GetHeight();
	virtual void PreRender(void*, int, int);
	virtual void RenderToRGBAArray(BYTE*);
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
