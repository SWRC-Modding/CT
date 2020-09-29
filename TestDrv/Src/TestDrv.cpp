#include "../Inc/TestDrv.h"
#include "../../Mod/Inc/Mod.h"

IMPLEMENT_PACKAGE(TestDrv)

#pragma comment(lib, "lib\\minifb.lib")

#include "MiniFB/MiniFB.h"

/*
 * TestRenderInterface
 */

/*
Functions needed for rendering the game (editor probably needs more):
	- SetViewport
	- vtpad1
	- vtpad2
	- SetDynamicStream
	- SetAntiAliasing
	- SetRenderTarget
	- SetDistanceFog
	- Clear
	- SetDynamicIndexBuffer
	- SetLODDiffuseFade
	- SetLODSpecularFade
	- IsShadowInterface
	- SetLight
	- SetTransform
	- SetVertexStreams
	- SetIndexBuffer
	- SetMaterial
	- DrawPrimitive
	- PopState
	- PushState
	- SetCullMode
	- EnableLighting
	- SetAmbientLight
*/

class FTestRenderInterface : public FRenderInterface{
public:
	FRenderInterface* Impl;
	mfb_window* Window;
	INT Width;
	INT Height;
	TArray<FColor> ColorBuffer;

	FTestRenderInterface(FRenderInterface* InImpl) : Impl(InImpl){
		Window = mfb_open("Rendering Test", 800, 600);
		SetRes(800, 600);
	}

	~FTestRenderInterface(){
		mfb_close(Window);
	}

	void SetRes(INT InWidth, INT InHeight){
		Width = InWidth;
		Height = InHeight;
		ColorBuffer.Set(Width * Height);
	}

	void Present(){
		mfb_update_ex(Window, ColorBuffer.GetData(), Width, Height);
		//mfb_wait_sync(Window);
	}

	// Overrides

	virtual void PushState(int a){
		//GLog->Logf("FRenderInterface::PushState");
		Impl->PushState(a);
	}
	virtual void PopState(int a){
		//GLog->Logf("FRenderInterface::PopState");
		Impl->PopState(a);
	}
	virtual UBOOL SetRenderTarget(FRenderTarget* RenderTarget, bool a){
		//GLog->Logf("FRenderInterface::SetRenderTarget");
		return Impl->SetRenderTarget(RenderTarget, a);
	}
	virtual void SetCubeRenderTarget(class FDynamicCubemap* RenderTarget, int a, int b){
		//GLog->Logf("FRenderInterface::SetCubeRenderTarget");
		Impl->SetCubeRenderTarget(RenderTarget, a, b);
	}
	virtual void SetViewport(INT X, INT Y, INT Width, INT Height){
		//GLog->Logf("FRenderInterface::SetViewport");
		Impl->SetViewport(X, Y, Width, Height);
	}
	virtual void Clear(UBOOL UseColor = 1, FColor Color = FColor(0, 0, 0), UBOOL UseDepth = 1,  FLOAT Depth = 1.0f, UBOOL UseStencil = 1, DWORD Stencil = 0){
		//GLog->Logf("FRenderInterface::Clear");
		Impl->Clear(UseColor, Color, UseDepth, Depth, UseStencil, Stencil);

		if(UseColor)
			appMemset4(ColorBuffer.GetData(), Color.DWColor(), ColorBuffer.Num());
	}
	virtual void PushHit(const BYTE* Data, INT Count){
		//GLog->Logf("FRenderInterface::PushHit");
		Impl->PushHit(Data, Count);
	}
	virtual void PopHit(INT Count, UBOOL Force){
		//GLog->Logf("FRenderInterface::PopHit");
		Impl->PopHit(Count, Force);
	}
	virtual void SetCullMode(ECullMode CullMode){
		//GLog->Logf("FRenderInterface::SetCullMode");
		Impl->SetCullMode(CullMode);
	}
	virtual void SetAmbientLight(FColor Color){
		//GLog->Logf("FRenderInterface::SetAmbientLight");
		Impl->SetAmbientLight(Color);
	}
	virtual void EnableLighting(UBOOL UseDynamic, UBOOL UseStatic = 1, UBOOL Modulate2X = 0, FBaseTexture* UseLightmap = NULL, UBOOL LightingOnly = 0, const FSphere& LitSphere = FSphere(FVector(0, 0, 0), 0), int testint = 0){
		//GLog->Logf("FRenderInterface::EnableLighting");
		Impl->EnableLighting(UseDynamic, UseStatic, Modulate2X, UseLightmap, LightingOnly, LitSphere, testint);
	}
	virtual void SetLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale = 1.0f){
		//GLog->Logf("FRenderInterface::SetLight");
		Impl->SetLight(LightIndex, Light, Scale);
	}
	virtual void SetShaderLight(int a, FDynamicLight* b, float c){
		//GLog->Logf("FRenderInterface::SetShaderLight");
		Impl->SetShaderLight(a, b, c);
	}
	virtual void SetNPatchTesselation(FLOAT Tesselation){
		//GLog->Logf("FRenderInterface::SetNPatchTesselation");
		Impl->SetNPatchTesselation(Tesselation);
	}
	virtual void SetDistanceFog(UBOOL Enable, FLOAT FogStart, FLOAT FogEnd, FColor Color){
		//GLog->Logf("FRenderInterface::SetDistanceFog");
		Impl->SetDistanceFog(Enable, FogStart, FogEnd, Color);
	}
	virtual UBOOL EnableFog(UBOOL Enable){
		//GLog->Logf("FRenderInterface::EnableFog");
		return Impl->EnableFog(Enable);
	}
	virtual UBOOL IsFogEnabled(){
		//GLog->Logf("FRenderInterface::IsFogEnabled");
		return Impl->IsFogEnabled();
	}
	virtual void SetGlobalColor(FColor Color){
		//GLog->Logf("FRenderInterface::SetGlobalColor");
		Impl->SetGlobalColor(Color);
	}
	virtual void SetTransform(ETransformType Type, const FMatrix& Matrix){
		//GLog->Logf("FRenderInterface::SetTransform");
		Impl->SetTransform(Type, Matrix);
	}
	virtual FMatrix GetTransform(ETransformType Type) const{
		//GLog->Logf("FRenderInterface::GetTransform");
		return Impl->GetTransform(Type);
	}
	virtual void SetMaterial(int a, int b, int c, int d){
		//GLog->Logf("FRenderInterface::SetMaterial");
		Impl->SetMaterial(a, b, c, d);
	}
	virtual UBOOL SetHardwareShaderMaterial(UHardwareShader* a, FString* b, UMaterial** c){
		//GLog->Logf("FRenderInterface::SetHardwareShaderMaterial");
		return Impl->SetHardwareShaderMaterial(a, b, c);
	}
	virtual UBOOL ShowAlpha(UMaterial* a){
		//GLog->Logf("FRenderInterface::ShowAlpha");
		return Impl->ShowAlpha(a);
	}
	virtual UBOOL IsShadowInterface(){
		//GLog->Logf("FRenderInterface::IsShadowInterface");
		return Impl->IsShadowInterface();
	}
	virtual void SetAntiAliasing(int a){
		//GLog->Logf("FRenderInterface::SetAntiAliasing");
		Impl->SetAntiAliasing(a);
	}
	virtual void CopyBackBufferToTarget(FAuxRenderTarget* a){
		//GLog->Logf("FRenderInterface::CopyBackBufferToTarget");
		Impl->CopyBackBufferToTarget(a);
	}
	virtual void SetLODDiffuseFade(float a){
		//GLog->Logf("FRenderInterface::SetLODDiffuseFade");
		Impl->SetLODDiffuseFade(a);
	}
	virtual void SetLODSpecularFade(float a){
		//GLog->Logf("FRenderInterface::SetLODSpecularFade");
		Impl->SetLODSpecularFade(a);
	}
	virtual void SetStencilOp(ECompareFunction Test, DWORD Ref, DWORD Mask, EStencilOp FailOp, EStencilOp ZFailOp, EStencilOp PassOp, DWORD WriteMask){
		//GLog->Logf("FRenderInterface::SetStencilOp");
		Impl->SetStencilOp(Test, Ref, Mask, FailOp, ZFailOp, PassOp, WriteMask);
	}
	virtual void vtpad1(){
		//GLog->Logf("FRenderInterface::vtpad1");
		Impl->vtpad1();
	}
	virtual void vtpad2(){
		//GLog->Logf("FRenderInterface::vtpad2");
		Impl->vtpad2();
	}
	virtual void SetPrecacheMode(EPrecacheMode PrecacheMode){
		//GLog->Logf("FRenderInterface::SetPrecacheMode");
		Impl->SetPrecacheMode(PrecacheMode);
	}
	virtual void SetZBias(INT ZBias){
		//GLog->Logf("FRenderInterface::SetZBias");
		Impl->SetZBias(ZBias);
	}
	virtual INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams){
		//GLog->Logf("FRenderInterface::SetVertexStreams");
		return Impl->SetVertexStreams(Shader, Streams, NumStreams);
	}
	virtual INT SetDynamicStream(EVertexShader Shader, FVertexStream* Stream){
		//GLog->Logf("FRenderInterface::SetDynamicStream");
		return Impl->SetDynamicStream(Shader, Stream);
	}
	virtual INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){
		//GLog->Logf("FRenderInterface::SetIndexBuffer");
		return Impl->SetIndexBuffer(IndexBuffer, BaseIndex);
	}
	virtual INT SetDynamicIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){
		//GLog->Logf("FRenderInterface::SetDynamicIndexBuffer");
		return Impl->SetDynamicIndexBuffer(IndexBuffer, BaseIndex);
	}
	virtual void DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex = INDEX_NONE, INT MaxIndex = INDEX_NONE){
		//GLog->Logf("FRenderInterface::DrawPrimitive");
		Impl->DrawPrimitive(PrimitiveType, FirstIndex, NumPrimitives, MinIndex, MaxIndex);
	}
	virtual void SetFillMode(EFillMode FillMode){
		//GLog->Logf("FRenderInterface::SetFillMode");
		Impl->SetFillMode(FillMode);
	}
	virtual int vtpad3(){
		//GLog->Logf("FRenderInterface::vtpad3");
		return Impl->vtpad3();
	}
	virtual int vtpad4(){
		//GLog->Logf("FRenderInterface::vtpad4");
		return Impl->vtpad4();
	}
	virtual int vtpad5(){
		//GLog->Logf("FRenderInterface::vtpad5");
		return Impl->vtpad5();
	}
	virtual int d3d1(int a, int b){
		//GLog->Logf("FRenderInterface::d3d1");
		return Impl->d3d1(a, b);
	}
	virtual int d3d2(int a){
		//GLog->Logf("FRenderInterface::d3d2");
		return Impl->d3d2(a);
	}
	virtual int d3d3(int a){
		//GLog->Logf("FRenderInterface::d3d3");
		return Impl->d3d3(a);
	}
};

/*
 * TestRenderDevice
 */

UBOOL UTestRenderDevice::Init(){
	GLog->Log(NAME_Init, "Initializing TestRenderDevice");

	return Super::Init();
}

void UTestRenderDevice::Exit(UViewport* Viewport){
	Super::Exit(Viewport);

	delete RenderInterface;
	RenderInterface = NULL;
}

FRenderInterface* UTestRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize){
	guard(leltest)
	FRenderInterface* RI = Super::Lock(Viewport, HitData, HitSize);

	if(!RenderInterface)
		RenderInterface = new FTestRenderInterface(RI);

	return RenderInterface;
	unguard
}

void UTestRenderDevice::Unlock(FRenderInterface* RI){
	Super::Unlock(static_cast<FTestRenderInterface*>(RI)->Impl);
}

UBOOL UTestRenderDevice::SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes, UBOOL bSaveSize){
	if(RenderInterface)
		RenderInterface->SetRes(NewX, NewY);

	return Super::SetRes(Viewport, NewX, NewY, Fullscreen, ColorBytes, bSaveSize);
}

void UTestRenderDevice::Present(UViewport* Viewport){
	if(RenderInterface)
		RenderInterface->Present();

	Super::Present(Viewport);
}

IMPLEMENT_CLASS(UTestRenderDevice)
