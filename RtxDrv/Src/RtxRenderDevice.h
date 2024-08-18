#pragma once

#include "../../D3DDrv/Inc/D3DDrv.h"

class URtxInterface;

class FAnchorTriangleVertexStream : public FVertexStream{
public:
	FAnchorTriangleVertexStream()
	{
		CacheId = MakeCacheID(CID_RenderVertices);
		Update(0.0f);
	}

	void Update(FLOAT InZOffset)
	{
		ZOffset = InZOffset;
		++Revision;
	}

	virtual INT GetStride(){ return sizeof(FVector); }
	virtual INT GetSize(){ return sizeof(FVector) * 3; }

	virtual INT GetComponents(FVertexComponent* Components)
	{
		Components->Type     = CT_Float3;
		Components->Function = FVF_Position;
		return 1;
	}

	virtual void GetStreamData(void* Dest)
	{
		FVector* D = static_cast<FVector*>(Dest);
		D[0] = FVector(-1.0f, -1.0f, ZOffset);
		D[1] = FVector(1.0f, -1.0f, ZOffset);
		D[2] = FVector(0, 1.0f, ZOffset);
	}

private:
	FLOAT ZOffset;
};

class URtxRenderDevice : public UD3DRenderDevice, FRenderInterface{
	DECLARE_CLASS(URtxRenderDevice,UD3DRenderDevice,CLASS_Config,RtxDrv)
	static const TCHAR* StaticConfigName(){ return "RtxDrv"; }
public:
	void StaticConstructor();

	virtual void Serialize(FArchive& Ar);
	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar);
	virtual UBOOL Init();
	virtual void Exit(UViewport* Viewport);
	virtual UBOOL SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes = 0, UBOOL bSaveSize = true);
	virtual void Flush(UViewport* Viewport);
	virtual FRenderInterface* Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize);
	virtual void Unlock(FRenderInterface* RI);
	virtual void Present(UViewport* Viewport);

	URtxInterface* GetRtxInterface(){ return Rtx; }

	// FRenderInterface

	virtual void PushState(DWORD Flags = 0){ D3D->PushState(Flags); }
	virtual void PopState(DWORD Flags = 0){ D3D->PopState(Flags); }
	virtual UBOOL SetRenderTarget(FRenderTarget* RenderTarget, bool bOwnDepthBuffer);
	virtual void SetViewport(INT X, INT Y, INT Width, INT Height){ D3D->SetViewport(X, Y, Width, Height); }
	virtual void Clear(UBOOL UseColor = 1, FColor Color = FColor(0, 0, 0), UBOOL UseDepth = 1, FLOAT Depth = 1.0f, UBOOL UseStencil = 1, DWORD Stencil = 0);
	virtual void PushHit(const BYTE* Data, INT Count){ D3D->PushHit(Data, Count); }
	virtual void PopHit(INT Count, UBOOL Force){ D3D->PopHit(Count, Force); }
	virtual void SetCullMode(ECullMode CullMode){ D3D->SetCullMode(CullMode); }
	virtual void SetAmbientLight(FColor Color){}
	virtual void EnableLighting(UBOOL UseDynamic, UBOOL UseStatic, UBOOL Modulate2X, FBaseTexture* Lightmap, UBOOL LightingOnly, const FSphere& LitSphere, int IntValue);
	virtual void SetLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale = 1.0f);
	virtual void SetShaderLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale = 1.0f){}
	virtual void SetNPatchTesselation(FLOAT Tesselation){ D3D->SetNPatchTesselation(Tesselation); }
	virtual void SetDistanceFog(UBOOL Enable, FLOAT FogStart, FLOAT FogEnd, FColor Color){ D3D->SetDistanceFog(0, 0.0f, 0.0f, FColor()); }
	virtual UBOOL EnableFog(UBOOL Enable){ return D3D->EnableFog(0); }
	virtual UBOOL IsFogEnabled(){ return D3D->IsFogEnabled(); }
	virtual void SetGlobalColor(FColor Color){ D3D->SetGlobalColor(Color); }
	virtual void SetTransform(ETransformType Type, const FMatrix& Matrix);
	virtual FMatrix GetTransform(ETransformType Type) const { return D3D->GetTransform(Type); }
	virtual void SetMaterial(UMaterial* Material, FString* ErrorString = NULL, UMaterial** ErrorMaterial = NULL, INT* NumPasses = NULL);
	virtual UBOOL SetHardwareShaderMaterial(UHardwareShader* Material, FString* ErrorString = NULL, UMaterial** ErrorMaterial = NULL){ return D3D->SetHardwareShaderMaterial(Material, ErrorString, ErrorMaterial); }
	virtual UBOOL ShowAlpha(UMaterial* Material){ return D3D->ShowAlpha(Material); }
	virtual UBOOL IsShadowInterface(){ return 0; }
	virtual void SetAntiAliasing(INT Level){}
	virtual void CopyBackBufferToTarget(FAuxRenderTarget* Target){ D3D->CopyBackBufferToTarget(Target); }
	virtual void SetLODDiffuseFade(FLOAT Distance){ D3D->SetLODDiffuseFade(Distance); }
	virtual void SetLODSpecularFade(FLOAT Distance){ D3D->SetLODSpecularFade(Distance); }
	virtual void SetStencilOp(ECompareFunction Test, DWORD Ref, DWORD Mask, EStencilOp FailOp, EStencilOp ZFailOp, EStencilOp PassOp, DWORD WriteMask){}
	virtual void EnableStencil(UBOOL Enable){}
	virtual void EnableDepth(UBOOL Enable){ D3D->EnableDepth(Enable); }
	virtual void SetPrecacheMode(EPrecacheMode){ D3D->SetPrecacheMode(PRECACHE_All); }
	virtual void SetZBias(INT ZBias){ D3D->SetZBias(ZBias); }
	virtual INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams){ return D3D->SetVertexStreams(Shader, Streams, NumStreams); }
	virtual INT SetDynamicStream(EVertexShader Shader, FVertexStream* Stream){ return D3D->SetDynamicStream(Shader, Stream); }
	virtual INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){ return D3D->SetIndexBuffer(IndexBuffer, BaseIndex); }
	virtual INT SetDynamicIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){ return D3D->SetDynamicIndexBuffer(IndexBuffer, BaseIndex); }
	virtual void DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex = INDEX_NONE, INT MaxIndex = INDEX_NONE);
	virtual void SetFillMode(EFillMode FillMode){ D3D->SetFillMode(FillMode); }

private:
	UViewport*                  LockedViewport;
	ULevel*                     CurrentLevel;
	FAnchorTriangleVertexStream AnchorTriangleStream;
	FRenderInterface*           D3D;
	URtxInterface*              Rtx;

	void ClearMaterialFlags();
	void DrawAnchorTriangle();
};
