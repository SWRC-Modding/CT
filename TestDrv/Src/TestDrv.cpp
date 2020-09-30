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

FTestRenderInterface::FTestRenderInterface(UTestRenderDevice* InRenDev) : RenDev(InRenDev){
	Window = mfb_open("Rendering Test", 1024, 768);
	SetRes(1024, 768);
	States.Set(1);
	States.SetNoShrink(true);
}

FTestRenderInterface::~FTestRenderInterface(){
	mfb_close(Window);
}

void FTestRenderInterface::SetRes(INT InWidth, INT InHeight){
	Width = InWidth;
	Height = InHeight;
	ColorBuffer.Set(Width * Height);
}

void FTestRenderInterface::Present(){
	mfb_update_ex(Window, ColorBuffer.GetData(), Width, Height);
	//mfb_wait_sync(Window);
}

// Overrides

void FTestRenderInterface::PushState(int a){
	//GLog->Logf("FRenderInterface::PushState");
	Impl->PushState(a);
	States.AddItem(States.Last());
}
void FTestRenderInterface::PopState(int a){
	//GLog->Logf("FRenderInterface::PopState");
	Impl->PopState(a);
	States.Pop();
}
UBOOL FTestRenderInterface::SetRenderTarget(FRenderTarget* RenderTarget, bool a){
	//GLog->Logf("FRenderInterface::SetRenderTarget");
	return Impl->SetRenderTarget(RenderTarget, a);
}
UBOOL FTestRenderInterface::SetCubeRenderTarget(class FDynamicCubemap* RenderTarget, int a, int b){
	//GLog->Logf("FRenderInterface::SetCubeRenderTarget");
	return Impl->SetCubeRenderTarget(RenderTarget, a, b);
}
void FTestRenderInterface::SetViewport(INT X, INT Y, INT Width, INT Height){
	//GLog->Logf("FRenderInterface::SetViewport");
	Impl->SetViewport(X, Y, Width, Height);
}
void FTestRenderInterface::Clear(UBOOL UseColor, FColor Color, UBOOL UseDepth,  FLOAT Depth, UBOOL UseStencil, DWORD Stencil){
	//GLog->Logf("FRenderInterface::Clear");
	Impl->Clear(UseColor, Color, UseDepth, Depth, UseStencil, Stencil);

	if(UseColor)
		appMemset4(ColorBuffer.GetData(), Color.DWColor(), ColorBuffer.Num());
}
void FTestRenderInterface::PushHit(const BYTE* Data, INT Count){
	//GLog->Logf("FRenderInterface::PushHit");
	Impl->PushHit(Data, Count);
}
void FTestRenderInterface::PopHit(INT Count, UBOOL Force){
	//GLog->Logf("FRenderInterface::PopHit");
	Impl->PopHit(Count, Force);
}
void FTestRenderInterface::SetCullMode(ECullMode CullMode){
	//GLog->Logf("FRenderInterface::SetCullMode");
	Impl->SetCullMode(CullMode);
}
void FTestRenderInterface::SetAmbientLight(FColor Color){
	//GLog->Logf("FRenderInterface::SetAmbientLight");
	Impl->SetAmbientLight(Color);
}
void FTestRenderInterface::EnableLighting(UBOOL UseDynamic, UBOOL UseStatic, UBOOL Modulate2X, FBaseTexture* UseLightmap, UBOOL LightingOnly, const FSphere& LitSphere, int testint){
	//GLog->Logf("FRenderInterface::EnableLighting");
	Impl->EnableLighting(UseDynamic, UseStatic, Modulate2X, UseLightmap, LightingOnly, LitSphere, testint);
}
void FTestRenderInterface::SetLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale){
	//GLog->Logf("FRenderInterface::SetLight");
	Impl->SetLight(LightIndex, Light, Scale);
}
void FTestRenderInterface::SetShaderLight(int a, FDynamicLight* b, float c){
	//GLog->Logf("FRenderInterface::SetShaderLight");
	Impl->SetShaderLight(a, b, c);
}
void FTestRenderInterface::SetNPatchTesselation(FLOAT Tesselation){
	//GLog->Logf("FRenderInterface::SetNPatchTesselation");
	Impl->SetNPatchTesselation(Tesselation);
}
void FTestRenderInterface::SetDistanceFog(UBOOL Enable, FLOAT FogStart, FLOAT FogEnd, FColor Color){
	//GLog->Logf("FRenderInterface::SetDistanceFog");
	Impl->SetDistanceFog(Enable, FogStart, FogEnd, Color);
}
UBOOL FTestRenderInterface::EnableFog(UBOOL Enable){
	//GLog->Logf("FRenderInterface::EnableFog");
	return Impl->EnableFog(Enable);
}
UBOOL FTestRenderInterface::IsFogEnabled(){
	//GLog->Logf("FRenderInterface::IsFogEnabled");
	return Impl->IsFogEnabled();
}
void FTestRenderInterface::SetGlobalColor(FColor Color){
	//GLog->Logf("FRenderInterface::SetGlobalColor");
	Impl->SetGlobalColor(Color);
}
void FTestRenderInterface::SetTransform(ETransformType Type, const FMatrix& Matrix){
	//GLog->Logf("FRenderInterface::SetTransform");
	Impl->SetTransform(Type, Matrix);
	States.Last().Matrices[Type] = Matrix;
}
FMatrix FTestRenderInterface::GetTransform(ETransformType Type) const{
	//GLog->Logf("FRenderInterface::GetTransform");
	return Impl->GetTransform(Type);
}
void FTestRenderInterface::SetMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial, INT* NumPasses){
	//GLog->Logf("FRenderInterface::SetMaterial");
	Impl->SetMaterial(Material, ErrorString, ErrorMaterial, NumPasses);
}
UBOOL FTestRenderInterface::SetHardwareShaderMaterial(UHardwareShader* a, FString* b, UMaterial** c){
	//GLog->Logf("FRenderInterface::SetHardwareShaderMaterial");
	return Impl->SetHardwareShaderMaterial(a, b, c);
}
UBOOL FTestRenderInterface::ShowAlpha(UMaterial* a){
	//GLog->Logf("FRenderInterface::ShowAlpha");
	return Impl->ShowAlpha(a);
}
UBOOL FTestRenderInterface::IsShadowInterface(){
	//GLog->Logf("FRenderInterface::IsShadowInterface");
	return Impl->IsShadowInterface();
}
void FTestRenderInterface::SetAntiAliasing(int a){
	//GLog->Logf("FRenderInterface::SetAntiAliasing");
	Impl->SetAntiAliasing(a);
}
void FTestRenderInterface::CopyBackBufferToTarget(FAuxRenderTarget* a){
	//GLog->Logf("FRenderInterface::CopyBackBufferToTarget");
	Impl->CopyBackBufferToTarget(a);
}
void FTestRenderInterface::SetLODDiffuseFade(float a){
	//GLog->Logf("FRenderInterface::SetLODDiffuseFade");
	Impl->SetLODDiffuseFade(a);
}
void FTestRenderInterface::SetLODSpecularFade(float a){
	//GLog->Logf("FRenderInterface::SetLODSpecularFade");
	Impl->SetLODSpecularFade(a);
}
void FTestRenderInterface::SetStencilOp(ECompareFunction Test, DWORD Ref, DWORD Mask, EStencilOp FailOp, EStencilOp ZFailOp, EStencilOp PassOp, DWORD WriteMask){
	//GLog->Logf("FRenderInterface::SetStencilOp");
	Impl->SetStencilOp(Test, Ref, Mask, FailOp, ZFailOp, PassOp, WriteMask);
}
void FTestRenderInterface::vtpad1(){
	//GLog->Logf("FRenderInterface::vtpad1");
	Impl->vtpad1();
}
void FTestRenderInterface::vtpad2(){
	//GLog->Logf("FRenderInterface::vtpad2");
	Impl->vtpad2();
}
void FTestRenderInterface::SetPrecacheMode(EPrecacheMode PrecacheMode){
	//GLog->Logf("FRenderInterface::SetPrecacheMode");
	Impl->SetPrecacheMode(PrecacheMode);
}
void FTestRenderInterface::SetZBias(INT ZBias){
	//GLog->Logf("FRenderInterface::SetZBias");
	Impl->SetZBias(ZBias);
}
INT FTestRenderInterface::SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams){
	//GLog->Logf("FRenderInterface::SetVertexStreams");
	return Impl->SetVertexStreams(Shader, Streams, NumStreams);
}
INT FTestRenderInterface::SetDynamicStream(EVertexShader Shader, FVertexStream* Stream){
	//GLog->Logf("FRenderInterface::SetDynamicStream");
	return Impl->SetDynamicStream(Shader, Stream);
}
INT FTestRenderInterface::SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){
	//GLog->Logf("FRenderInterface::SetIndexBuffer");
	return Impl->SetIndexBuffer(IndexBuffer, BaseIndex);
}
INT FTestRenderInterface::SetDynamicIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){
	//GLog->Logf("FRenderInterface::SetDynamicIndexBuffer");
	return Impl->SetDynamicIndexBuffer(IndexBuffer, BaseIndex);
}
void FTestRenderInterface::DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex, INT MaxIndex){
	//GLog->Logf("FRenderInterface::DrawPrimitive");
	Impl->DrawPrimitive(PrimitiveType, FirstIndex, NumPrimitives, MinIndex, MaxIndex);

	FMatrix Transform = States.Last().Matrices[0] * States.Last().Matrices[1] * States.Last().Matrices[2];
	FVector Pos = Transform.TransformFVector(FVector(0.0f, 0.0f, 0.0f));

	Pos.X /= Pos.Z;
	Pos.Y /= Pos.Z;

	if(Pos.Z > 0.0f && fabsf(Pos.X) < 1.0f && fabsf(Pos.Y) < 1.0f){
		INT HalfWidth = Width / 2;
		INT HalfHeight = Height / 2;
		INT X = (INT)(Pos.X * HalfWidth + HalfWidth);
		INT Y = (INT)(-Pos.Y * HalfHeight + HalfHeight);

		for(int i = X - 1; i < X + 2; ++i){
			for(int j = Y - 1; j < Y + 2; ++j){
				if(i >= 0 && i < Width && j >= 0 && j < Height)
					ColorBuffer[i + j * Width] = 0xFFFFFFFF;
			}
		}
	}
}
void FTestRenderInterface::SetFillMode(EFillMode FillMode){
	//GLog->Logf("FRenderInterface::SetFillMode");
	Impl->SetFillMode(FillMode);
}
int FTestRenderInterface::vtpad3(){
	//GLog->Logf("FRenderInterface::vtpad3");
	return Impl->vtpad3();
}
int FTestRenderInterface::vtpad4(){
	//GLog->Logf("FRenderInterface::vtpad4");
	return Impl->vtpad4();
}
int FTestRenderInterface::vtpad5(){
	//GLog->Logf("FRenderInterface::vtpad5");
	return Impl->vtpad5();
}
int FTestRenderInterface::d3d1(int a, int b){
	//GLog->Logf("FRenderInterface::d3d1");
	return Impl->d3d1(a, b);
}
int FTestRenderInterface::d3d2(int a){
	//GLog->Logf("FRenderInterface::d3d2");
	return Impl->d3d2(a);
}
int FTestRenderInterface::d3d3(int a){
	//GLog->Logf("FRenderInterface::d3d3");
	return Impl->d3d3(a);
}

/*
 * TestRenderDevice
 */

UBOOL UTestRenderDevice::Init(){
	GLog->Log(NAME_Init, "Initializing TestRenderDevice");

	return Super::Init();
}

void UTestRenderDevice::Exit(UViewport* Viewport){
	Super::Exit(Viewport);
}

FRenderInterface* UTestRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize){
	RenderInterface.Impl = Super::Lock(Viewport, HitData, HitSize);

	return &RenderInterface;
}

void UTestRenderDevice::Unlock(FRenderInterface* RI){
	Super::Unlock(static_cast<FTestRenderInterface*>(RI)->Impl);
}

UBOOL UTestRenderDevice::SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes, UBOOL bSaveSize){
	RenderInterface.SetRes(NewX, NewY);

	return Super::SetRes(Viewport, NewX, NewY, Fullscreen, ColorBytes, bSaveSize);
}

void UTestRenderDevice::Present(UViewport* Viewport){
	RenderInterface.Present();

	Super::Present(Viewport);
}

IMPLEMENT_CLASS(UTestRenderDevice)
