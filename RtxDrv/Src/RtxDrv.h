#ifndef RTXDRV_NATIVE_DEFS
#define RTXDRV_NATIVE_DEFS

#include "../../D3DDrv/Inc/D3DDrv.h"

#ifndef RTXDRV_API
#define RTXDRV_API DLL_IMPORT
#endif

class URtxRenderDevice;

class FSingleTriangleVertexStream : public FVertexStream{
public:
	FSingleTriangleVertexStream(FLOAT Width, FLOAT Height) : HalfWidth(Width / 2), HalfHeight(Height / 2)
	{
		CacheId = MakeCacheID(CID_RenderVertices);
	}

	FLOAT HalfWidth;
	FLOAT HalfHeight;

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
		D[0] = FVector(-HalfWidth, -HalfHeight, 0);
		D[1] = FVector(HalfHeight, -HalfHeight, 0);
		D[2] = FVector(0, HalfHeight, 0);
	}
};

class FRtxRenderInterface : public FRenderInterface{
public:
	URtxRenderDevice* RenDev;
	FRenderInterface* Impl;

	UBOOL      DrawParticleTriangles;
	UMaterial* CurrentMaterial;
	UMaterial* CurrentActualMaterial;

	virtual void PushState(DWORD Flags = 0){ Impl->PushState(Flags); }
	virtual void PopState(DWORD Flags = 0){ Impl->PopState(Flags); }
	virtual UBOOL SetRenderTarget(FRenderTarget* RenderTarget, bool bOwnDepthBuffer){
		return Impl->SetRenderTarget(RenderTarget, bOwnDepthBuffer);
	}
	virtual UBOOL SetCubeRenderTarget(class FDynamicCubemap* Target, int A, int B){ return Impl->SetCubeRenderTarget(Target, A, B); }
	virtual void SetViewport(INT X, INT Y, INT Width, INT Height){ Impl->SetViewport(X, Y, Width, Height); }
	virtual void Clear(UBOOL UseColor = 1, FColor Color = FColor(0, 0, 0), UBOOL UseDepth = 1, FLOAT Depth = 1.0f, UBOOL UseStencil = 1, DWORD Stencil = 0){
		Impl->Clear(UseColor, Color, UseDepth, Depth, UseStencil, Stencil);
	}
	virtual void PushHit(const BYTE* Data, INT Count){ Impl->PushHit(Data, Count); }
	virtual void PopHit(INT Count, UBOOL Force){ Impl->PopHit(Count, Force); }
	virtual void SetCullMode(ECullMode CullMode){ Impl->SetCullMode(CullMode); }
	virtual void SetAmbientLight(FColor Color){ Impl->SetAmbientLight(Color); }
	virtual void EnableLighting(UBOOL UseDynamic, UBOOL UseStatic = 1, UBOOL Modulate2X = 0, FBaseTexture* Lightmap = NULL, UBOOL LightingOnly = 0, const FSphere& LitSphere = FSphere(FVector(0, 0, 0), 0), int IntValue = 0){
		Impl->EnableLighting(UseDynamic, UseStatic, Modulate2X, Lightmap, LightingOnly, LitSphere, IntValue);
	}
	virtual void SetLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale = 1.0f){ Impl->SetLight(LightIndex, Light, Scale); }
	virtual void SetShaderLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale = 1.0f){ Impl->SetShaderLight(LightIndex, Light, Scale); }
	virtual void SetNPatchTesselation(FLOAT Tesselation){ Impl->SetNPatchTesselation(Tesselation); }
	virtual void SetDistanceFog(UBOOL Enable, FLOAT FogStart, FLOAT FogEnd, FColor Color){ Impl->SetDistanceFog(Enable, FogStart, FogEnd, Color); }
	virtual UBOOL EnableFog(UBOOL Enable){ return Impl->EnableFog(Enable); }
	virtual UBOOL IsFogEnabled(){ return Impl->IsFogEnabled(); }
	virtual void SetGlobalColor(FColor Color){ Impl->SetGlobalColor(Color); }
	virtual void SetTransform(ETransformType Type, const FMatrix& Matrix);
	virtual FMatrix GetTransform(ETransformType Type) const { return Impl->GetTransform(Type); }
	virtual void SetMaterial(UMaterial* Material, FString* ErrorString = NULL, UMaterial** ErrorMaterial = NULL, INT* NumPasses = NULL);
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
	virtual void DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex = INDEX_NONE, INT MaxIndex = INDEX_NONE);
	virtual void SetFillMode(EFillMode FillMode){ Impl->SetFillMode(FillMode); }
};

class RTXDRV_API URtxRenderDevice : public UD3DRenderDevice{
	DECLARE_CLASS(URtxRenderDevice,UD3DRenderDevice,CLASS_Config,RtxDrv)
	static const TCHAR* StaticConfigName(){ return "RtxDrv"; }
public:
	FRtxRenderInterface RenderInterface;

	virtual UBOOL Init();
	virtual void Flush(UViewport* Viewport);
	virtual FRenderInterface* Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize);
	virtual void Unlock(FRenderInterface* RI);
	virtual FRenderCaps* GetRenderCaps();

	struct TestDraw{
		INT Id;
		FSingleTriangleVertexStream Stream;
		FString Path;

		TestDraw(INT InId = 0, const TCHAR* InPath = NULL)
			: Id(InId),
			  Stream(InId + 32, InId + 32),
			  Path(InPath)
		{
			++Stream.Revision;
		}

		void UpdateId(INT InId)
		{
			Id = InId,
			Stream.HalfWidth = (InId + 32) / 2.0f;
			Stream.HalfHeight = Stream.HalfWidth;
			++Stream.Revision;
		}
	};

	TArray<TestDraw> MaterialIdsByPath;

private:
	void ClearMaterialFlags();
};

#endif // RTXDRV_NATIVE_DEFS
