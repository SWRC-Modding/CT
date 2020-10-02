#pragma once

#include "../../Mod/Inc/Mod.h"

#ifndef TESTDRV_API
#define TESTDRV_API DLL_IMPORT
LINK_LIB(TestDrv)
#endif

struct mfb_window;
class UTestRenderDevice;

class FTestRenderInterface : public FRenderInterface{
public:
	UTestRenderDevice* RenDev;
	FRenderInterface* Impl;
	mfb_window* Window;
	INT Width;
	INT Height;
	TArray<FColor> ColorBuffer;

	struct State{
		FMatrix Matrices[3];

		State(){
			for(int i = 0; i < ARRAY_COUNT(Matrices); ++i)
				Matrices[i] = FMatrix::Identity;
		}
	};

	TArray<State> States;

	FTestRenderInterface(UTestRenderDevice* InRenDev);
	~FTestRenderInterface();

	void SetRes(INT InWidth, INT InHeight);
	void Present();

	// Overrides

	virtual void PushState(int a);
	virtual void PopState(int a);
	virtual UBOOL SetRenderTarget(FRenderTarget* RenderTarget, bool a);
	virtual UBOOL SetCubeRenderTarget(class FDynamicCubemap* RenderTarget, int a, int b);
	virtual void SetViewport(INT X, INT Y, INT Width, INT Height);
	virtual void Clear(UBOOL UseColor = 1, FColor Color = FColor(0, 0, 0), UBOOL UseDepth = 1,  FLOAT Depth = 1.0f, UBOOL UseStencil = 1, DWORD Stencil = 0);
	virtual void PushHit(const BYTE* Data, INT Count);
	virtual void PopHit(INT Count, UBOOL Force);
	virtual void SetCullMode(ECullMode CullMode);
	virtual void SetAmbientLight(FColor Color);
	virtual void EnableLighting(UBOOL UseDynamic, UBOOL UseStatic = 1, UBOOL Modulate2X = 0, FBaseTexture* UseLightmap = NULL, UBOOL LightingOnly = 0, const FSphere& LitSphere = FSphere(FVector(0, 0, 0), 0), int testint = 0);
	virtual void SetLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale = 1.0f);
	virtual void SetShaderLight(int a, FDynamicLight* b, float c);
	virtual void SetNPatchTesselation(FLOAT Tesselation);
	virtual void SetDistanceFog(UBOOL Enable, FLOAT FogStart, FLOAT FogEnd, FColor Color);
	virtual UBOOL EnableFog(UBOOL Enable);
	virtual UBOOL IsFogEnabled();
	virtual void SetGlobalColor(FColor Color);
	virtual void SetTransform(ETransformType Type, const FMatrix& Matrix);
	virtual FMatrix GetTransform(ETransformType Type) const;
	virtual void SetMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial, INT* NumPasses);
	virtual UBOOL SetHardwareShaderMaterial(UHardwareShader* a, FString* b, UMaterial** c);
	virtual UBOOL ShowAlpha(UMaterial* a);
	virtual UBOOL IsShadowInterface();
	virtual void SetAntiAliasing(int a);
	virtual void CopyBackBufferToTarget(FAuxRenderTarget* a);
	virtual void SetLODDiffuseFade(float a);
	virtual void SetLODSpecularFade(float a);
	virtual void SetStencilOp(ECompareFunction Test, DWORD Ref, DWORD Mask, EStencilOp FailOp, EStencilOp ZFailOp, EStencilOp PassOp, DWORD WriteMask);
	virtual void vtpad1(int);
	virtual void vtpad2(int);
	virtual void SetPrecacheMode(EPrecacheMode PrecacheMode);
	virtual void SetZBias(INT ZBias);
	virtual INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams);
	virtual INT SetDynamicStream(EVertexShader Shader, FVertexStream* Stream);
	virtual INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex);
	virtual INT SetDynamicIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex);
	virtual void DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex = INDEX_NONE, INT MaxIndex = INDEX_NONE);
	virtual void SetFillMode(EFillMode FillMode);
	virtual int vtpad3();
	virtual int vtpad4();
	virtual int vtpad5();
	virtual int d3d1(int a, int b);
	virtual int d3d2(int a);
	virtual int d3d3(int a);
};

class TESTDRV_API UTestRenderDevice : public UModRenderDevice{
	DECLARE_CLASS(UTestRenderDevice, UModRenderDevice, 0, TestDrv);
public:
	FTestRenderInterface RenderInterface;

	UTestRenderDevice() : RenderInterface(this){}

	virtual UBOOL Init();
	virtual void Exit(UViewport* Viewport);
	virtual FRenderInterface* Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize);
	virtual void Unlock(FRenderInterface* RI);
	virtual UBOOL SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes, UBOOL bSaveSize);
	virtual void Present(UViewport* Viewport);
};
