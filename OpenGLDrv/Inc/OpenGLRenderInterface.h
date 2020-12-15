#pragma once

#include "OpenGLDrv.h"
#include "Shader.h"

class UOpenGLRenderDevice;
class FOpenGLRenderTarget;
class FOpenGLIndexBuffer;
class FOpenGLVertexStream;
class FOpenGLShader;

// GLSL equivalent types with proper alignment

typedef ALIGN(4) __int32 GLSL_bool;
typedef ALIGN(4) __int32 GLSL_int;
typedef ALIGN(4) float GLSL_float;
typedef ALIGN(8) struct{ GLSL_float X; GLSL_float Y; } GLSL_vec2;
typedef ALIGN(16) FVector GLSL_vec3;
typedef ALIGN(16) FPlane GLSL_vec4;
typedef ALIGN(16) FMatrix GLSL_mat4;

// Macro to synchronize the GLSL uniform block with the C++ struct
#define UNIFORM_BLOCK_CONTENTS \
	UNIFORM_BLOCK_MEMBER(mat4, LocalToWorld) \
	UNIFORM_BLOCK_MEMBER(mat4, WorldToCamera) \
	UNIFORM_BLOCK_MEMBER(mat4, CameraToScreen) \
	UNIFORM_BLOCK_MEMBER(mat4, Transform) \
	UNIFORM_BLOCK_MEMBER(float, Time) \
	UNIFORM_BLOCK_MEMBER(float, CosTime) \
	UNIFORM_BLOCK_MEMBER(float, SinTime) \
	UNIFORM_BLOCK_MEMBER(float, TanTime)

// Global uniforms available in every shader
struct FOpenGLGlobalUniforms{
#define UNIFORM_BLOCK_MEMBER(type, name) GLSL_ ## type name;
	UNIFORM_BLOCK_CONTENTS
#undef UNIFORM_BLOCK_MEMBER
};

#define MAX_STATESTACKDEPTH 16
#define MAX_VERTEX_STREAMS MAX_VERTEX_COMPONENTS

struct FStreamDeclaration{
	FVertexComponent Components[MAX_VERTEX_COMPONENTS];
	INT              NumComponents;

	void Init(FVertexStream* VertexStream){ NumComponents = VertexStream->GetComponents(Components); }
};

struct FModifierInfo{
	bool ZWrite;
	bool ZTest;
	bool AlphaTest;
	bool TwoSided;
	FMatrix Matrix;
	EFrameBufferBlending Blending;
};

/*
 * OpenGL RenderInterface
 */
class FOpenGLRenderInterface : public FRenderInterface{
public:
	struct FOpenGLSavedState{
		INT                   UniformRevision;
		FOpenGLGlobalUniforms Uniforms;

		FOpenGLRenderTarget*  RenderTarget;

		INT                   ViewportX;
		INT                   ViewportY;
		INT                   ViewportWidth;
		INT                   ViewportHeight;

		ECullMode             CullMode;
		EFillMode             FillMode;

		UBOOL                 bStencilTest;
		UBOOL                 bZWrite;
		UBOOL                 bZTest;

		INT                   ZBias;

		FOpenGLShader*        Shader;

		INT                   IndexBufferBaseIndex;
		FOpenGLIndexBuffer*   IndexBuffer;

		unsigned int          VAO;
		INT                   NumVertexStreams;
		FOpenGLVertexStream*  VertexStreams[MAX_VERTEX_STREAMS];

		EFrameBufferBlending  FramebufferBlending;
	};

	void SetMaterialBlending(FModifierInfo* ModifierInfo){
		EnableZWrite(ModifierInfo->ZWrite);
		EnableZTest(ModifierInfo->ZTest);

		if(ModifierInfo->TwoSided)
			SetCullMode(CM_None);

		SetFramebufferBlending(ModifierInfo->Blending);
	}

	FModifierInfo        ModifierInfo;

	UOpenGLRenderDevice* RenDev;

	FOpenGLSavedState    SavedStates[MAX_STATESTACKDEPTH];
	FOpenGLSavedState*   CurrentState;
	FOpenGLSavedState*   PoppedState;

	UBOOL                NeedUniformUpdate;
	unsigned int         GlobalUBO;

	FStreamDeclaration   VertexStreamDeclarations[MAX_VERTEX_STREAMS];

	FOpenGLRenderInterface(UOpenGLRenderDevice* InRenDev);

	void FlushResources();
	void UpdateShaderUniforms();

	// Overrides
	virtual void PushState(INT Flags = 0);
	virtual void PopState(INT Flags = 0);
	virtual UBOOL SetRenderTarget(FRenderTarget* RenderTarget, bool bFSAA);
	virtual void SetViewport(INT X, INT Y, INT Width, INT Height);
	virtual void Clear(UBOOL UseColor = 1, FColor Color = FColor(0, 0, 0), UBOOL UseDepth = 1, FLOAT Depth = 1.0f, UBOOL UseStencil = 1, DWORD Stencil = 0);
	virtual void PushHit(const BYTE* Data, INT Count){}
	virtual void PopHit(INT Count, UBOOL Force){}
	virtual void SetCullMode(ECullMode CullMode);
	virtual void SetAmbientLight(FColor Color){}
	virtual void EnableLighting(UBOOL UseDynamic, UBOOL UseStatic = 1, UBOOL Modulate2X = 0, FBaseTexture* UseLightmap = NULL, UBOOL LightingOnly = 0, const FSphere& LitSphere = FSphere(FVector(0, 0, 0), 0), int = 0){}
	virtual void SetLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale = 1.0f){}
	virtual void SetNPatchTesselation(FLOAT Tesselation){}
	virtual void SetDistanceFog(UBOOL Enable, FLOAT FogStart, FLOAT FogEnd, FColor Color){}
	virtual void SetGlobalColor(FColor Color){}
	virtual void SetTransform(ETransformType Type, const FMatrix& Matrix);
	virtual FMatrix GetTransform(ETransformType Type) const;
	virtual void SetMaterial(UMaterial* Material, FString* ErrorString = NULL, UMaterial** ErrorMaterial = NULL, INT* NumPasses = NULL);
	virtual void SetStencilOp(ECompareFunction Test, DWORD Ref, DWORD Mask, EStencilOp FailOp, EStencilOp ZFailOp, EStencilOp PassOp, DWORD WriteMask);
	virtual void EnableStencilTest(UBOOL Enable);
	virtual void EnableZWrite(UBOOL Enable);
	virtual void SetPrecacheMode(EPrecacheMode PrecacheMode){}
	virtual void SetZBias(INT ZBias);
	virtual INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams);
	virtual INT SetDynamicStream(EVertexShader Shader, FVertexStream* Stream);
	virtual INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex);
	virtual INT SetDynamicIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex);
	virtual void DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex = INDEX_NONE, INT MaxIndex = INDEX_NONE);
	virtual void SetFillMode(EFillMode FillMode);

	void EnableZTest(UBOOL Enable);
	void SetShader(FShaderGLSL* NewShader);
	void SetFramebufferBlending(EFrameBufferBlending Blending);

	void SetupPerFrameShaderConstants();

private:
	INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex, bool IsDynamic);
	INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams, bool IsDynamic);
};

template<typename T>
bool CheckMaterial(UMaterial*& Material, FModifierInfo* ModifierInfo){
	if(Material->IsA<T>())
		return true;

	UModifier* Modifier = Cast<UModifier>(Material);

	while(Modifier){
		if(Modifier->IsA<UTexModifier>()){
			FMatrix* Matrix = static_cast<UTexModifier*>(Modifier)->GetMatrix(GEngineTime);

			if(Matrix)
				ModifierInfo->Matrix *= *Matrix;
		}else if(Modifier->IsA<UFinalBlend>()){
			UFinalBlend* FinalBlend = static_cast<UFinalBlend*>(Modifier);

			ModifierInfo->ZWrite    |= FinalBlend->ZWrite != 0;
			ModifierInfo->ZTest     |= FinalBlend->ZTest != 0;
			ModifierInfo->AlphaTest |= FinalBlend->AlphaTest != 0;
			ModifierInfo->TwoSided  |= FinalBlend->TwoSided != 0;
			ModifierInfo->Blending   = static_cast<EFrameBufferBlending>(FinalBlend->FrameBufferBlending);
		}else if(Modifier->IsA<UColorModifier>()){

		}else if(Modifier->IsA<UOpacityModifier>()){

		}

		Material = Modifier->Material;
		Modifier = Cast<UModifier>(Material);
	}

	return Cast<T>(Material) != NULL;
}
