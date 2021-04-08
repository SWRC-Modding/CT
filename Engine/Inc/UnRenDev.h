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
	PT_PointList     = 1,
	PT_LineList      = 2,
	PT_TriangleList  = 5,
	PT_TriangleStrip = 6,
	PT_TriangleFan   = 7
};

// Vertex shaders for SetVertexStreams.
enum EVertexShader{
	VS_FixedFunction,
	VS_HardwareShaderDefined
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

class FRenderInterface{
public:
	virtual void PushState(INT Flags = 0) = 0;
	virtual void PopState(INT Flags = 0) = 0;
	virtual UBOOL SetRenderTarget(FRenderTarget* RenderTarget, bool MatchBackbuffer) = 0;
	virtual UBOOL SetCubeRenderTarget(class FDynamicCubemap*, int, int){ return 0; }
	virtual void SetViewport(INT X, INT Y, INT Width, INT Height) = 0;
	virtual void Clear(UBOOL UseColor = 1, FColor Color = FColor(0, 0, 0), UBOOL UseDepth = 1, FLOAT Depth = 1.0f, UBOOL UseStencil = 1, DWORD Stencil = 0) = 0;
	virtual void PushHit(const BYTE* Data, INT Count) = 0;
	virtual void PopHit(INT Count, UBOOL Force) = 0;
	virtual void SetCullMode(ECullMode CullMode) = 0;
	virtual void SetAmbientLight(FColor Color) = 0;
	virtual void EnableLighting(UBOOL UseDynamic, UBOOL UseStatic = 1, UBOOL Modulate2X = 0, FBaseTexture* Lightmap = NULL, UBOOL LightingOnly = 0, const FSphere& LitSphere = FSphere(FVector(0, 0, 0), 0), int = 0) = 0;
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
	virtual void SetAntiAliasing(INT Level){}
	virtual void CopyBackBufferToTarget(FAuxRenderTarget*){}
	virtual void SetLODDiffuseFade(FLOAT Distance){}
	virtual void SetLODSpecularFade(FLOAT Distance){}
	virtual void SetStencilOp(ECompareFunction Test, DWORD Ref, DWORD Mask, EStencilOp FailOp, EStencilOp ZFailOp, EStencilOp PassOp, DWORD WriteMask) = 0;
	virtual void EnableStencil(UBOOL Enable) = 0;
	virtual void EnableDepth(UBOOL Enable) = 0;
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
	// Occlusion query functions. (Only implemented for XBox
	virtual UBOOL BeginOcclusionQuery(){ return 1; }
	virtual UBOOL EndOcclusionQuery(DWORD QueryNum){ return 1; }
	virtual UBOOL GetOcclusionQueryResults(DWORD QueryNum, DWORD& dwPixels){ return 1; }
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

//
// A low-level 3D rendering device.
//
class ENGINE_API URenderDevice : public USubsystem{
	DECLARE_ABSTRACT_CLASS(URenderDevice,USubsystem,CLASS_Config,Engine)
public:
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
	BITFIELD CanDoDistortionEffects;
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
	virtual UBOOL ResourceCached(QWORD CacheId){ return 0; }
	virtual FMemCount ResourceMem(FRenderResource*, UObject*){ return FMemCount(); }
	virtual FMemCount ResourceMemTotal(){ return FMemCount(); }
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
	virtual FMovie* GetNewMovie(ECodecType Codec, FString Filename, UBOOL UseSound, INT FrameRate, int){ return NULL; }
	virtual int GetStateCaching(){ return 0; }
	virtual int SetStateCaching(int){ return 0; }
	virtual int RefreshStates(){}
	virtual INT DoesSupportFSAA(INT Level){ return 0; }
	virtual void TakeScreenshot(const TCHAR* Name, UViewport* Viewport, INT Width, INT Height);
	virtual UBOOL SupportsTextureFormat(ETextureFormat) = 0;
};

#endif
/*------------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------------*/
