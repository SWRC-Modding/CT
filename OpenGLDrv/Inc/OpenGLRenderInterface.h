#pragma once

#include "OpenGLDrv.h"

class FOpenGLRenderInterface : public FRenderInterface{
public:
	virtual void PushState(int){}
	virtual void PopState(int){}
	virtual UBOOL SetRenderTarget(FRenderTarget* RenderTarget, bool){ return 1; }
	virtual void SetViewport(INT X, INT Y, INT Width, INT Height){}
	virtual void Clear(UBOOL UseColor, FColor Color, UBOOL UseDepth, FLOAT Depth, UBOOL UseStencil, DWORD Stencil);
	virtual void PushHit(const BYTE* Data, INT Count){}
	virtual void PopHit(INT Count, UBOOL Force){}
	virtual void SetCullMode(ECullMode CullMode){}
	virtual void SetAmbientLight(FColor Color){}
	virtual void EnableLighting(UBOOL UseDynamic, UBOOL UseStatic, UBOOL Modulate2X, FBaseTexture* UseLightmap, UBOOL LightingOnly, const FSphere& LitSphere, int){}
	virtual void SetLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale){}
	virtual void SetNPatchTesselation(FLOAT Tesselation){}
	virtual void SetDistanceFog(UBOOL Enable, FLOAT FogStart, FLOAT FogEnd, FColor Color){}
	virtual void SetGlobalColor(FColor Color){}
	virtual void SetTransform(ETransformType Type, const FMatrix& Matrix){}
	virtual FMatrix GetTransform(ETransformType Type) const{ return FMatrix::Identity; }
	virtual void SetMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial, INT* NumPasses){}
	virtual void SetStencilOp(ECompareFunction Test, DWORD Ref, DWORD Mask, EStencilOp FailOp, EStencilOp ZFailOp, EStencilOp PassOp, DWORD WriteMask){}
	virtual void vtpad1(int){}
	virtual void vtpad2(int){}
	virtual void SetPrecacheMode(EPrecacheMode PrecacheMode){}
	virtual void SetZBias(INT ZBias){}
	virtual INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams){ return 0; }
	virtual INT SetDynamicStream(EVertexShader Shader, FVertexStream* Stream){ return 0; }
	virtual INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){ return 0; }
	virtual INT SetDynamicIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){ return 0; }
	virtual void DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex, INT MaxIndex){}
};
