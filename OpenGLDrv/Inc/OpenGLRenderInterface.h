#pragma once

#include "OpenGLDrv.h"

class UOpenGLRenderDevice;
class FOpenGLRenderTarget;
class FOpenGLIndexBuffer;
class FOpenGLVertexStream;

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
	UNIFORM_BLOCK_MEMBER(mat4, Transform)

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

		UBOOL                 bStencilTest;
		UBOOL                 bZWrite;

		INT                   IndexBufferBaseIndex;
		FOpenGLIndexBuffer*   IndexBuffer;

		unsigned int          VAO;
		INT                   NumVertexStreams;
		FOpenGLVertexStream*  VertexStreams[MAX_VERTEX_STREAMS];
	};

	UOpenGLRenderDevice*      RenDev;

	FOpenGLSavedState         SavedStates[MAX_STATESTACKDEPTH];
	FOpenGLSavedState*        CurrentState;
	FOpenGLSavedState*        PoppedState;

	UBOOL                     NeedUniformUpdate;
	unsigned int              GlobalUBO;

	FOpenGLIndexBuffer*       DynamicIndexBuffer16;
	FOpenGLIndexBuffer*       DynamicIndexBuffer32;

	FStreamDeclaration        VertexStreamDeclarations[MAX_VERTEX_STREAMS];
	FOpenGLVertexStream*      DynamicVertexStream;
	TMap<DWORD, unsigned int> VAOsByDeclId;

	FOpenGLRenderInterface(UOpenGLRenderDevice* InRenDev);

	void FlushResources();
	void UpdateShaderUniforms();

	// Overrides
	virtual void PushState(int);
	virtual void PopState(int);
	virtual UBOOL SetRenderTarget(FRenderTarget* RenderTarget, bool bFSAA);
	virtual void SetViewport(INT X, INT Y, INT Width, INT Height);
	virtual void Clear(UBOOL UseColor, FColor Color, UBOOL UseDepth, FLOAT Depth, UBOOL UseStencil, DWORD Stencil);
	virtual void PushHit(const BYTE* Data, INT Count){ PRINT_FUNC; }
	virtual void PopHit(INT Count, UBOOL Force){ PRINT_FUNC; }
	virtual void SetCullMode(ECullMode CullMode);
	virtual void SetAmbientLight(FColor Color){ PRINT_FUNC; }
	virtual void EnableLighting(UBOOL UseDynamic, UBOOL UseStatic, UBOOL Modulate2X, FBaseTexture* UseLightmap, UBOOL LightingOnly, const FSphere& LitSphere, int){ PRINT_FUNC; }
	virtual void SetLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale){ PRINT_FUNC; }
	virtual void SetNPatchTesselation(FLOAT Tesselation){ PRINT_FUNC; }
	virtual void SetDistanceFog(UBOOL Enable, FLOAT FogStart, FLOAT FogEnd, FColor Color){ PRINT_FUNC; }
	virtual void SetGlobalColor(FColor Color){ PRINT_FUNC; }
	virtual void SetTransform(ETransformType Type, const FMatrix& Matrix);
	virtual FMatrix GetTransform(ETransformType Type) const;
	virtual void SetMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial, INT* NumPasses){ PRINT_FUNC; }
	virtual void SetStencilOp(ECompareFunction Test, DWORD Ref, DWORD Mask, EStencilOp FailOp, EStencilOp ZFailOp, EStencilOp PassOp, DWORD WriteMask){ PRINT_FUNC; }
	virtual void EnableStencilTest(UBOOL Enable);
	virtual void EnableZWrite(UBOOL Enable);
	virtual void SetPrecacheMode(EPrecacheMode PrecacheMode){ PRINT_FUNC; }
	virtual void SetZBias(INT ZBias){ PRINT_FUNC; }
	virtual INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams);
	virtual INT SetDynamicStream(EVertexShader Shader, FVertexStream* Stream);
	virtual INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex);
	virtual INT SetDynamicIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex);
	virtual void DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex, INT MaxIndex);

private:
	INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex, bool IsDynamic);
	INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams, bool IsDynamic);
};
