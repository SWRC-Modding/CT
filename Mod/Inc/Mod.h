#pragma once

#include "../../D3DDrv/Inc/D3DDrv.h"

#ifndef MOD_API
#define MOD_API DLL_IMPORT
LINK_LIB(Mod)
#endif

#include "ModClasses.h"

/*
 * Patches the given vtable with a custom function and returns the old function.
 * NOTE: This changes the vtable for all objects of the same class.
 * Returns the function pointer that was at Index previously. NULL if there was an error.
 */
MOD_API void* PatchVTable(void* Object, INT Index, void* NewFunc);
/*
 * Patches the vtable for a class that is exported from a dll.
 * VTableName is the name of the particular vtable to patch in case of multiple virtual inheritance.
 */
MOD_API void* PatchDllClassVTable(const TCHAR* DllName, const TCHAR* ClassName, const TCHAR* VTableName, INT Index, void* NewFunc);

/*
 * ModRenderInterface
 * - Used by ModRenderDevice to
 */
class FModRenderInterface : public FRenderInterface{
public:
	FRenderInterface* Impl;

	virtual void PushState(int a){ Impl->PushState(a); }
	virtual void PopState(int a){ Impl->PopState(a); }
	virtual UBOOL SetRenderTarget(FRenderTarget* RenderTarget, bool a){ return Impl->SetRenderTarget(RenderTarget, a); }
	virtual UBOOL SetCubeRenderTarget(class FDynamicCubemap* RenderTarget, int a, int b){ return Impl->SetCubeRenderTarget(RenderTarget, a, b); }
	virtual void SetViewport(INT X, INT Y, INT Width, INT Height){ Impl->SetViewport(X, Y, Width, Height); }
	virtual void Clear(UBOOL UseColor, FColor Color, UBOOL UseDepth, FLOAT Depth, UBOOL UseStencil, DWORD Stencil){ Impl->Clear(UseColor, Color, UseDepth, Depth, UseStencil, Stencil); }
	virtual void PushHit(const BYTE* Data, INT Count){ Impl->PushHit(Data, Count); }
	virtual void PopHit(INT Count, UBOOL Force){ Impl->PopHit(Count, Force); }
	virtual void SetCullMode(ECullMode CullMode){ Impl->SetCullMode(CullMode); }
	virtual void SetAmbientLight(FColor Color){ Impl->SetAmbientLight(Color); }
	virtual void EnableLighting(UBOOL UseDynamic, UBOOL UseStatic, UBOOL Modulate2X, FBaseTexture* UseLightmap, UBOOL LightingOnly, const FSphere& LitSphere, int a){ Impl->EnableLighting(UseDynamic, UseStatic, Modulate2X, UseLightmap, LightingOnly, LitSphere, a); }
	virtual void SetLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale){ Impl->SetLight(LightIndex, Light, Scale); }
	virtual void SetShaderLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale){ Impl->SetShaderLight(LightIndex, Light, Scale); }
	virtual void SetNPatchTesselation(FLOAT Tesselation){ Impl->SetNPatchTesselation(Tesselation); }
	virtual void SetDistanceFog(UBOOL Enable, FLOAT FogStart, FLOAT FogEnd, FColor Color){ Impl->SetDistanceFog(Enable, FogStart, FogEnd, Color); }
	virtual UBOOL EnableFog(UBOOL Enable){ return Impl->EnableFog(Enable); }
	virtual UBOOL IsFogEnabled(){ return Impl->IsFogEnabled(); }
	virtual void SetGlobalColor(FColor Color){ Impl->SetGlobalColor(Color); }
	virtual void SetTransform(ETransformType Type, const FMatrix& Matrix){ Impl->SetTransform(Type, Matrix); }
	virtual FMatrix GetTransform(ETransformType Type) const{ return Impl->GetTransform(Type); }
	virtual void SetMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial, INT* NumPasses){ Impl->SetMaterial(Material, ErrorString, ErrorMaterial, NumPasses); }
	virtual UBOOL SetHardwareShaderMaterial(UHardwareShader* Material, FString* ErrorString, UMaterial** ErrorMaterial){ return Impl->SetHardwareShaderMaterial(Material, ErrorString, ErrorMaterial); }
	virtual UBOOL ShowAlpha(UMaterial* a){ return Impl->ShowAlpha(a); }
	virtual UBOOL IsShadowInterface(){ return Impl->IsShadowInterface(); }
	virtual void SetAntiAliasing(int a){ Impl->SetAntiAliasing(a); }
	virtual void CopyBackBufferToTarget(FAuxRenderTarget* Target){ Impl->CopyBackBufferToTarget(Target); }
	virtual void SetLODDiffuseFade(float a){ Impl->SetLODDiffuseFade(a); }
	virtual void SetLODSpecularFade(float a){ Impl->SetLODSpecularFade(a); }
	virtual void SetStencilOp(ECompareFunction Test, DWORD Ref, DWORD Mask, EStencilOp FailOp, EStencilOp ZFailOp, EStencilOp PassOp, DWORD WriteMask){ Impl->SetStencilOp(Test, Ref, Mask, FailOp, ZFailOp, PassOp, WriteMask); }
	virtual void vtpad1(int a){ Impl->vtpad1(a); }
	virtual void vtpad2(int a){ Impl->vtpad2(a); }
	virtual void SetPrecacheMode(EPrecacheMode PrecacheMode){ Impl->SetPrecacheMode(PrecacheMode); }
	virtual void SetZBias(INT ZBias){ Impl->SetZBias(ZBias); }
	virtual INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams){ return Impl->SetVertexStreams(Shader, Streams, NumStreams); }
	virtual INT SetDynamicStream(EVertexShader Shader, FVertexStream* Stream){ return Impl->SetDynamicStream(Shader, Stream); }
	virtual INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){ return Impl->SetIndexBuffer(IndexBuffer, BaseIndex); }
	virtual INT SetDynamicIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){ return Impl->SetDynamicIndexBuffer(IndexBuffer, BaseIndex); }
	virtual void DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex, INT MaxIndex){ Impl->DrawPrimitive(PrimitiveType, FirstIndex, NumPrimitives, MinIndex, MaxIndex); }
	virtual void PixoSetHint(DWORD Hint){ Impl->PixoSetHint(Hint); }
	virtual void PixoResetHint(DWORD Hint){ Impl->PixoResetHint(Hint); }
	virtual UTexture* PixoCreateTexture(FRenderTarget* RenderTarget, UBOOL CreateMips){ return Impl->PixoCreateTexture(RenderTarget, CreateMips); }
	virtual UBOOL PixoIsVisible(FBox& Box){ return Impl->PixoIsVisible(Box); }
	virtual bool IsVertexBufferBusy(FVertexStream* Stream){ return Impl->IsVertexBufferBusy(Stream); }
	virtual void SetFillMode(EFillMode FillMode){ Impl->SetFillMode(FillMode); }
	virtual int vtpad3(){ return Impl->vtpad3(); }
	virtual int vtpad4(){ return Impl->vtpad4(); }
	virtual int vtpad5(){ return Impl->vtpad5(); }
	virtual int d3d1(int a, int b){ return Impl->d3d1(a, b); }
	virtual int d3d2(int a){ return Impl->d3d2(a); }
	virtual int d3d3(int a){ return Impl->d3d3(a); }
};

/*
 * ModRenderDevice.
 * - Fixes bumpmapping crashes by converting the bumpmaps to a compatible format
 */
class MOD_API UModRenderDevice : public UD3DRenderDevice{
	DECLARE_CLASS(UModRenderDevice, UD3DRenderDevice, 0, Mod)
public:
	static UObject* FOVChanger;
	static FLOAT    FpsLimit;

	FModRenderInterface RenderInterface;

	virtual UBOOL Init();
	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar);
	virtual FRenderInterface* Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize);
	virtual void Unlock(FRenderInterface* RI){ Super::Unlock(RI == &RenderInterface ? static_cast<FModRenderInterface*>(RI)->Impl : RI); }
};
