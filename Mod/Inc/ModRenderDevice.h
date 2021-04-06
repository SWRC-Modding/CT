#include "Mod.h"

#include "../../D3DDrv/Inc/D3DDrv.h"

class UModRenderDevice;

struct FHitProxyInfo{
	INT       ParentIndex; // INDEX_NONE if no parent
	UBOOL     IsPreferred; // If there is more than one possible selection, prefer this one (Used with stuff that is difficult to select like the gizmo axes)

	FHitProxyInfo(INT InParentIndex, UBOOL Preferred) : ParentIndex(InParentIndex),
	                                                    IsPreferred(Preferred){}
};

/*
 * ModRenderInterface
 */
class FModRenderInterface : public FRenderInterface{
public:
	UModRenderDevice* RenDev;
	FRenderInterface* Impl;
	TArray<INT>       HitStack;   // Indicies into AllHitData of currently pushed hit proxies
	TArray<BYTE>      AllHitData; // Contains FHitProxyInfo followed by HHitProxy for each hit proxy that was pushed during the current frame
	BYTE*             HitData;
	INT*              HitSize;
	UBitmapMaterial*  CurrentTexture;

	FModRenderInterface(UModRenderDevice* InRenDev);

	bool ProcessHitColor(FColor HitColor, INT* OutIndex);
	void ProcessHit(INT HitProxyIndex);

	virtual void PushState(int a){ Impl->PushState(a); }
	virtual void PopState(int a){ Impl->PopState(a); }
	virtual UBOOL SetRenderTarget(FRenderTarget* RenderTarget, bool a){ return 0; }
	virtual void SetViewport(INT X, INT Y, INT Width, INT Height){ Impl->SetViewport(X, Y, Width, Height); }
	virtual void Clear(UBOOL UseColor, FColor Color, UBOOL UseDepth, FLOAT Depth, UBOOL UseStencil, DWORD Stencil){ Impl->Clear(UseColor, FColor(0x00000000), UseDepth, Depth, UseStencil, Stencil); }
	virtual void PushHit(const BYTE* Data, INT Count);
	virtual void PopHit(INT Count, UBOOL Force);
	virtual void SetCullMode(ECullMode CullMode){ Impl->SetCullMode(CullMode); }
	virtual void SetAmbientLight(FColor Color){}
	virtual void EnableLighting(UBOOL UseDynamic, UBOOL UseStatic, UBOOL Modulate2X, FBaseTexture* UseLightmap, UBOOL LightingOnly, const FSphere& LitSphere, int a){}
	virtual void SetLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale){}
	virtual void SetNPatchTesselation(FLOAT Tesselation){ Impl->SetNPatchTesselation(Tesselation); }
	virtual void SetDistanceFog(UBOOL Enable, FLOAT FogStart, FLOAT FogEnd, FColor Color){}
	virtual void SetGlobalColor(FColor Color){}
	virtual void SetTransform(ETransformType Type, const FMatrix& Matrix){ Impl->SetTransform(Type, Matrix); }
	virtual FMatrix GetTransform(ETransformType Type) const{ return Impl->GetTransform(Type); }
	virtual void SetMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial, INT* NumPasses);
	virtual UBOOL SetHardwareShaderMaterial(UHardwareShader* Material, FString* ErrorString, UMaterial** ErrorMaterial);
	virtual void SetStencilOp(ECompareFunction Test, DWORD Ref, DWORD Mask, EStencilOp FailOp, EStencilOp ZFailOp, EStencilOp PassOp, DWORD WriteMask){}
	virtual void EnableStencil(UBOOL Enable){}
	virtual void EnableDepth(UBOOL Enable){ Impl->EnableDepth(Enable); }
	virtual void SetPrecacheMode(EPrecacheMode PrecacheMode){ Impl->SetPrecacheMode(PrecacheMode); }
	virtual void SetZBias(INT ZBias){ Impl->SetZBias(ZBias); }
	virtual INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams){ return Impl->SetVertexStreams(Shader, Streams, NumStreams); }
	virtual INT SetDynamicStream(EVertexShader Shader, FVertexStream* Stream){ return Impl->SetDynamicStream(Shader, Stream); }
	virtual INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){ return Impl->SetIndexBuffer(IndexBuffer, BaseIndex); }
	virtual INT SetDynamicIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){ return Impl->SetDynamicIndexBuffer(IndexBuffer, BaseIndex); }
	virtual void DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex, INT MaxIndex);
	virtual void SetFillMode(EFillMode FillMode){ Impl->SetFillMode(FillMode); }

	// Additional virtual functions from FD3DRenderInterface must be implemented to avoid crashes.
	virtual int d3d1(int a, int b){ return 0; }
	virtual int d3d2(int a){ return 0; }
	virtual int d3d3(int a){ return 0; }
};

/*
 * ModRenderDevice.
 * - Fixes bumpmapping crashes by converting the bumpmaps to a compatible format
 * - Adds alternative selection mechanism for much better performance in the Editor
 */
class MOD_API UModRenderDevice : public UD3DRenderDevice{
	DECLARE_CLASS(UModRenderDevice, UD3DRenderDevice, 0, Mod)
public:
	static UHardwareShader* SolidSelectionShader;
	static UHardwareShader* AlphaSelectionShader;

	UViewport*          LockedViewport;
	FModRenderInterface RenderInterface;
	UBOOL               bEnableSelectionFix;
	UBOOL               bDebugSelectionBuffer; // Shows the selection buffer for five seconds after a click

	// Saved color values from UEngine
	FColor C_ActorArrow;

	UModRenderDevice() : RenderInterface(this){}
	void StaticConstructor(){ bEnableSelectionFix = 1; }

	virtual void Serialize(FArchive& Ar){
		Super::Serialize(Ar);
		Ar << SolidSelectionShader << AlphaSelectionShader;
	}

	virtual UBOOL Init();
	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar);
	virtual FRenderInterface* Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize);
	virtual void Unlock(FRenderInterface* RI);
};
