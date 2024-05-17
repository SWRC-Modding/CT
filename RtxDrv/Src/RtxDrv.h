#ifndef RTXDRV_NATIVE_DEFS
#define RTXDRV_NATIVE_DEFS

#include "../../D3DDrv/Inc/D3DDrv.h"

#if SUPPORTS_PRAGMA_PACK
#pragma pack (push,4)
#endif

#ifndef RTXDRV_API
#define RTXDRV_API DLL_IMPORT
#endif

class FRtxRenderInterface : public FRenderInterface{
public:
	FRenderInterface* Impl;

	virtual void PushState(DWORD Flags = 0){ Impl->PushState(Flags); }
	virtual void PopState(DWORD Flags = 0){ Impl->PopState(Flags); }
	virtual UBOOL SetRenderTarget(FRenderTarget* RenderTarget, bool bOwnDepthBuffer){ return Impl->SetRenderTarget(RenderTarget, bOwnDepthBuffer); }
	virtual UBOOL SetCubeRenderTarget(class FDynamicCubemap* Target, int A, int B){ return Impl->SetCubeRenderTarget(Target, A, B); }
	virtual void SetViewport(INT X, INT Y, INT Width, INT Height){ Impl->SetViewport(X, Y, Width, Height); }
	virtual void Clear(UBOOL UseColor = 1, FColor Color = FColor(0, 0, 0), UBOOL UseDepth = 1, FLOAT Depth = 1.0f, UBOOL UseStencil = 1, DWORD Stencil = 0){ Impl->Clear(UseColor, Color, UseDepth, Depth, UseStencil, Stencil); }
	virtual void PushHit(const BYTE* Data, INT Count){ Impl->PushHit(Data, Count); }
	virtual void PopHit(INT Count, UBOOL Force){ Impl->PopHit(Count, Force); }
	virtual void SetCullMode(ECullMode CullMode){ Impl->SetCullMode(CullMode); }
	virtual void SetAmbientLight(FColor Color){ Impl->SetAmbientLight(Color); }
	virtual void EnableLighting(UBOOL UseDynamic, UBOOL UseStatic = 1, UBOOL Modulate2X = 0, FBaseTexture* Lightmap = NULL, UBOOL LightingOnly = 0, const FSphere& LitSphere = FSphere(FVector(0, 0, 0), 0), int IntValue = 0){ Impl->EnableLighting(UseDynamic, UseStatic, Modulate2X, Lightmap, LightingOnly, LitSphere, IntValue); }
	virtual void SetLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale = 1.0f){ Impl->SetLight(LightIndex, Light, Scale); }
	virtual void SetShaderLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale = 1.0f){ Impl->SetShaderLight(LightIndex, Light, Scale); }
	virtual void SetNPatchTesselation(FLOAT Tesselation){ Impl->SetNPatchTesselation(Tesselation); }
	virtual void SetDistanceFog(UBOOL Enable, FLOAT FogStart, FLOAT FogEnd, FColor Color){ Impl->SetDistanceFog(Enable, FogStart, FogEnd, Color); }
	virtual UBOOL EnableFog(UBOOL Enable){ return Impl->EnableFog(Enable); }
	virtual UBOOL IsFogEnabled(){ return Impl->IsFogEnabled(); }
	virtual void SetGlobalColor(FColor Color){ Impl->SetGlobalColor(Color); }
	virtual void SetTransform(ETransformType Type, const FMatrix& Matrix){ Impl->SetTransform(Type, Matrix); }
	virtual FMatrix GetTransform(ETransformType Type) const { return Impl->GetTransform(Type); }
	virtual void SetMaterial(UMaterial* Material, FString* ErrorString = NULL, UMaterial** ErrorMaterial = NULL, INT* NumPasses = NULL){
		if(Material->IsA<UShader>())
			static_cast<UShader*>(Material)->Bumpmap = NULL;
		else if(Material->IsA<UHardwareShader>() || Material->IsA<UHardwareShaderWrapper>())
			Material = GetDefault<UMaterial>()->DefaultMaterial;
		Impl->SetMaterial(Material, ErrorString, ErrorMaterial, NumPasses);
	}
	virtual UBOOL SetHardwareShaderMaterial(UHardwareShader* Material, FString* ErrorString = NULL, UMaterial** ErrorMaterial = NULL){ return Impl->SetHardwareShaderMaterial(Material, ErrorString, ErrorMaterial); }
	virtual UBOOL ShowAlpha(UMaterial* Material){ return Impl->ShowAlpha(Material); }
	virtual UBOOL IsShadowInterface(){ return Impl->IsShadowInterface(); }
	virtual void SetAntiAliasing(INT Level){ Impl->SetAntiAliasing(Level); }
	virtual void CopyBackBufferToTarget(FAuxRenderTarget* Target){ Impl->CopyBackBufferToTarget(Target); }
	virtual void SetLODDiffuseFade(FLOAT Distance){ Impl->SetLODDiffuseFade(Distance); }
	virtual void SetLODSpecularFade(FLOAT Distance){ Impl->SetLODSpecularFade(Distance); }
	virtual void SetStencilOp(ECompareFunction Test, DWORD Ref, DWORD Mask, EStencilOp FailOp, EStencilOp ZFailOp, EStencilOp PassOp, DWORD WriteMask){ Impl->SetStencilOp(Test, Ref, Mask, FailOp, ZFailOp, PassOp, WriteMask); }
	virtual void EnableStencil(UBOOL Enable){ Impl->EnableStencil(Enable); }
	virtual void EnableDepth(UBOOL Enable){ Impl->EnableDepth(Enable); }
	virtual void SetPrecacheMode(EPrecacheMode PrecacheMode){ Impl->SetPrecacheMode(PrecacheMode); }
	virtual void SetZBias(INT ZBias){ Impl->SetZBias(ZBias); }
	virtual INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams){ return Impl->SetVertexStreams(Shader, Streams, NumStreams); }
	virtual INT SetDynamicStream(EVertexShader Shader, FVertexStream* Stream){ return Impl->SetDynamicStream(Shader, Stream); }
	virtual INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){ return Impl->SetIndexBuffer(IndexBuffer, BaseIndex); }
	virtual INT SetDynamicIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){ return Impl->SetDynamicIndexBuffer(IndexBuffer, BaseIndex); }
	virtual void DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex = INDEX_NONE, INT MaxIndex = INDEX_NONE){ Impl->DrawPrimitive(PrimitiveType, FirstIndex, NumPrimitives, MinIndex, MaxIndex); }
	virtual void PixoSetHint(DWORD Hint){ Impl->PixoSetHint(Hint); }
	virtual void PixoResetHint(DWORD Hint){ Impl->PixoResetHint(Hint); }
	virtual UTexture* PixoCreateTexture(FRenderTarget* RenderTarget, UBOOL CreateMips){ return Impl->PixoCreateTexture(RenderTarget, CreateMips); }
	virtual UBOOL PixoIsVisible(FBox& Box){ return Impl->PixoIsVisible(Box); }
	virtual bool IsVertexBufferBusy(FVertexStream* Stream){ return Impl->IsVertexBufferBusy(Stream); }
	virtual void SetFillMode(EFillMode FillMode){ Impl->SetFillMode(FillMode); }
};

class RTXDRV_API URtxRenderDevice : public UD3DRenderDevice{
	DECLARE_CLASS(URtxRenderDevice,UD3DRenderDevice,CLASS_Config,RtxDrv)
public:

	FRtxRenderInterface RenderInterface;

	virtual FRenderInterface* Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize);
	virtual void Unlock(FRenderInterface* RI);
};

#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif

#endif // RTXDRV_NATIVE_DEFS
