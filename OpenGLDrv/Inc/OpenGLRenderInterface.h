#pragma once

#include "OpenGLDrv.h"

class FOpenGLRenderInterface : public FRenderInterface{
public:
	virtual void PushState(int){ PRINT_FUNC; }
	virtual void PopState(int){ PRINT_FUNC; }
	virtual UBOOL SetRenderTarget(FRenderTarget* RenderTarget, bool){ PRINT_FUNC; return 1; }
	virtual void SetViewport(INT X, INT Y, INT Width, INT Height){ PRINT_FUNC; }
	virtual void Clear(UBOOL UseColor, FColor Color, UBOOL UseDepth, FLOAT Depth, UBOOL UseStencil, DWORD Stencil);
	virtual void PushHit(const BYTE* Data, INT Count){ PRINT_FUNC; }
	virtual void PopHit(INT Count, UBOOL Force){ PRINT_FUNC; }
	virtual void SetCullMode(ECullMode CullMode){ PRINT_FUNC; }
	virtual void SetAmbientLight(FColor Color){ PRINT_FUNC; }
	virtual void EnableLighting(UBOOL UseDynamic, UBOOL UseStatic, UBOOL Modulate2X, FBaseTexture* UseLightmap, UBOOL LightingOnly, const FSphere& LitSphere, int){ PRINT_FUNC; }
	virtual void SetLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale){ PRINT_FUNC; }
	virtual void SetNPatchTesselation(FLOAT Tesselation){ PRINT_FUNC; }
	virtual void SetDistanceFog(UBOOL Enable, FLOAT FogStart, FLOAT FogEnd, FColor Color){ PRINT_FUNC; }
	virtual void SetGlobalColor(FColor Color){ PRINT_FUNC; }
	virtual void SetTransform(ETransformType Type, const FMatrix& Matrix){ PRINT_FUNC; }
	virtual FMatrix GetTransform(ETransformType Type) const{ PRINT_FUNC; return FMatrix::Identity; }
	virtual void SetMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial, INT* NumPasses){ PRINT_FUNC; }
	virtual void SetStencilOp(ECompareFunction Test, DWORD Ref, DWORD Mask, EStencilOp FailOp, EStencilOp ZFailOp, EStencilOp PassOp, DWORD WriteMask){ PRINT_FUNC; }
	virtual void vtpad1(int){ PRINT_FUNC; }
	virtual void vtpad2(int){ PRINT_FUNC; }
	virtual void SetPrecacheMode(EPrecacheMode PrecacheMode){ PRINT_FUNC; }
	virtual void SetZBias(INT ZBias){ PRINT_FUNC; }
	virtual INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams){ PRINT_FUNC; return 0; }
	virtual INT SetDynamicStream(EVertexShader Shader, FVertexStream* Stream){ PRINT_FUNC; return 0; }
	virtual INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){ PRINT_FUNC; return 0; }
	virtual INT SetDynamicIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){ PRINT_FUNC; return 0; }
	virtual void DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex, INT MaxIndex){ PRINT_FUNC; }
};
