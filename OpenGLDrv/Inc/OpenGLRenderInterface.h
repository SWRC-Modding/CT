#pragma once

#include "OpenGLDrv.h"
#include "Shader.h"

class UOpenGLRenderDevice;
class FOpenGLTexture;
class FOpenGLIndexBuffer;
class FOpenGLVertexStream;
class FOpenGLShader;

#define MAX_STATESTACKDEPTH 16
#define MAX_VERTEX_STREAMS MAX_VERTEX_COMPONENTS
#define MAX_TEXTURES 8
#define MAX_SHADER_STAGES 8
#define MAX_SHADER_LIGHTS 4
#define MAX_VERTEX_SHADER_CONSTANTS 96
#define MAX_PIXEL_SHADER_CONSTANTS 8

struct FStreamDeclaration{
	FVertexComponent Components[MAX_VERTEX_COMPONENTS];
	INT              NumComponents;

	void Init(FVertexStream* VertexStream){ NumComponents = VertexStream->GetComponents(Components); }
};

enum EHardwareShaderUniforms{
	HSU_VSConstants = 0,
	HSU_PSConstants = MAX_VERTEX_SHADER_CONSTANTS
};

enum EShaderLightType{
	SL_Directional,
	SL_Point,
	SL_Spot
};

// NOTE: There seems to be a compiler bug that produces incorrect alignment if both the struct and variable have the same name
#define GLSL_STRUCT(x) GLSL_struct x
#define STRUCT(x) struct

// Macro to synchronize the GLSL uniform block with the C++ struct
#define UNIFORM_BLOCK_CONTENTS \
	UNIFORM_BLOCK_MEMBER(mat4, LocalToWorld) \
	UNIFORM_BLOCK_MEMBER(mat4, WorldToCamera) \
	UNIFORM_BLOCK_MEMBER(mat4, CameraToScreen) \
	UNIFORM_BLOCK_MEMBER(mat4, LocalToCamera) \
	UNIFORM_BLOCK_MEMBER(mat4, LocalToScreen) \
	UNIFORM_BLOCK_MEMBER(mat4, WorldToLocal) \
	UNIFORM_BLOCK_MEMBER(mat4, WorldToScreen) \
	UNIFORM_BLOCK_MEMBER(mat4, CameraToWorld) \
	UNIFORM_BLOCK_MEMBER(float, AlphaRef) \
	UNIFORM_BLOCK_MEMBER(float, Time) \
	UNIFORM_BLOCK_MEMBER(float, CosTime) \
	UNIFORM_BLOCK_MEMBER(float, SinTime) \
	UNIFORM_BLOCK_MEMBER(float, TanTime) \
	UNIFORM_BLOCK_MEMBER(vec4, GlobalColor) \
	UNIFORM_BLOCK_MEMBER(vec4, AmbientLightColor) \
	UNIFORM_BLOCK_MEMBER(vec4, FogColor) \
	UNIFORM_BLOCK_MEMBER(bool, FogEnabled) \
	UNIFORM_BLOCK_MEMBER(float, FogStart) \
	UNIFORM_BLOCK_MEMBER(float, FogEnd) \
	UNIFORM_BLOCK_MEMBER(STRUCT(TextureInfo){ \
		UNIFORM_STRUCT_MEMBER(bool, IsCubemap) \
		UNIFORM_STRUCT_MEMBER(bool, IsBumpmap) \
		UNIFORM_STRUCT_MEMBER(float, BumpSize) \
	}, TextureInfos[MAX_TEXTURES]) \
	UNIFORM_BLOCK_MEMBER(STRUCT(Light){ \
		UNIFORM_STRUCT_MEMBER(vec4, Color) \
		UNIFORM_STRUCT_MEMBER(vec4, Position) \
		UNIFORM_STRUCT_MEMBER(vec4, Direction) \
		UNIFORM_STRUCT_MEMBER(float, Constant) \
		UNIFORM_STRUCT_MEMBER(float, Linear) \
		UNIFORM_STRUCT_MEMBER(float, Quadratic) \
		UNIFORM_STRUCT_MEMBER(float, Cone) \
		UNIFORM_STRUCT_MEMBER(int, Type) \
	}, Lights[MAX_SHADER_LIGHTS]) \
	UNIFORM_BLOCK_MEMBER(bool, UseDynamicLighting) \
	UNIFORM_BLOCK_MEMBER(bool, UseStaticLighting)

#pragma warning(push)
#pragma warning(disable : 4324) // structure was padded due to __declspec(align())

// Global uniforms available in every shader
struct FOpenGLGlobalUniforms{
#define UNIFORM_BLOCK_MEMBER(type, name) GLSL_ ## type name;
#define UNIFORM_STRUCT_MEMBER(type, name) GLSL_ ## type name;
	UNIFORM_BLOCK_CONTENTS
#undef UNIFORM_STRUCT_MEMBER
#undef UNIFORM_BLOCK_MEMBER
};

#pragma warning(pop)

struct FOpenGLTextureUnit{
	FOpenGLTexture* Texture;
	BYTE            ClampU; // ETexClampMode
	BYTE            ClampV; // ETexClampMode
};

class FOpenGLVertexArrayObject{
public:
	~FOpenGLVertexArrayObject();

	bool IsValid() const{ return VAO != 0; }
	void Init(const FStreamDeclaration* Declarations, INT NumStreams);
	void Bind();
	void BindVertexStream(FOpenGLVertexStream* Stream, INT StreamIndex);
	void BindIndexBuffer(FOpenGLIndexBuffer* IndexBuffer);

private:
	unsigned int VAO;
	unsigned int EBO;
	unsigned int VBOs[MAX_VERTEX_STREAMS];
	unsigned int Strides[MAX_VERTEX_STREAMS];
};

struct FOpenGLRenderState{
	BYTE                      FillMode; // EFillMode

	BYTE                      CullMode; // ECullMode

	bool                      bZWrite;
	bool                      bZTest;
	bool                      bStencilTest;

	BYTE                      StencilCompare; // ECompareFunction
	BYTE                      StencilRef;
	BYTE                      StencilMask;
	BYTE                      StencilFailOp;  // EStencilOp
	BYTE                      StencilZFailOp; // EStencilOp
	BYTE                      StencilPassOp;  // EStencilOp
	BYTE                      StencilWriteMask;

	_WORD                     ZBias;

	_WORD                     ViewportX;
	_WORD                     ViewportY;
	_WORD                     ViewportWidth;
	_WORD                     ViewportHeight;

	unsigned int              SrcBlend; // GLenum
	unsigned int              DstBlend; // GLenum

	FOpenGLVertexArrayObject* VAO;

	FOpenGLIndexBuffer*       IndexBuffer;
	INT                       NumVertexStreams;
	FOpenGLVertexStream*      VertexStreams[MAX_VERTEX_STREAMS];

	INT                       NumTextures;
	FOpenGLTextureUnit        TextureUnits[MAX_TEXTURES];
};

/*
 * OpenGL RenderInterface
 */
class FOpenGLRenderInterface : public FRenderInterface{
public:
	struct FOpenGLSavedState : FOpenGLGlobalUniforms, FOpenGLRenderState{
		INT             UniformRevision;

		INT             IndexBufferBase;
		INT             VertexBufferBase;

		FOpenGLTexture* RenderTarget;
		bool            RenderTargetOwnDepthBuffer;

		// Light

		bool            LightingModulate2X;
		FBaseTexture*   Lightmap;
		FSphere         LitSphere;
		FDynamicLight*  HardwareShaderLights[MAX_SHADER_LIGHTS];
	};

	// Variables

	UOpenGLRenderDevice*                  RenDev;
	UViewport*                            LockedViewport;

	bool                                  bStencilEnabled;

	unsigned int                          LastCullMode; // GLenum

	EPrecacheMode                         PrecacheMode;

	FOpenGLSavedState                     SavedStates[MAX_STATESTACKDEPTH];
	FOpenGLSavedState*                    CurrentState;
	FOpenGLRenderState                    RenderState;

	FOpenGLShader*                        CurrentShader;

	bool                                  MatricesChanged;
	INT                                   LastUniformRevision;
	unsigned int                          GlobalUBO;

	BYTE                                  TextureFilter; // ETextureFilter
	INT                                   TextureAnisotropy;
	unsigned int                          Samplers[MAX_TEXTURES];

	TMap<DWORD, FOpenGLVertexArrayObject> VAOsByDeclId;

	// Functions

	FOpenGLRenderInterface(UOpenGLRenderDevice* InRenDev);

	void Init(INT ViewportWidth, INT ViewportHeight);
	void Flush();
	void Exit();
	void CommitRenderState();
	void Locked(UViewport* Viewport);
	void Unlocked();
	void UpdateMatrices();
	void SetFramebufferBlending(EFrameBufferBlending Mode);
	void SetTextureFilter(BYTE Filter);
	void SetGLRenderTarget(FOpenGLTexture* GLRenderTarget, bool bOwnDepthBuffer);
	void SetShader(FShaderGLSL* NewShader);

	// Overrides

	virtual void PushState(DWORD Flags = 0);
	virtual void PopState(DWORD Flags = 0);
	virtual UBOOL SetRenderTarget(FRenderTarget* RenderTarget, bool bOwnDepthBuffer);
	virtual void SetViewport(INT X, INT Y, INT Width, INT Height);
	virtual void Clear(UBOOL UseColor = 1, FColor Color = FColor(0, 0, 0), UBOOL UseDepth = 1, FLOAT Depth = 1.0f, UBOOL UseStencil = 1, DWORD Stencil = 0);
	virtual void PushHit(const BYTE* Data, INT Count){}
	virtual void PopHit(INT Count, UBOOL Force){}
	virtual void SetCullMode(ECullMode CullMode);
	virtual void SetAmbientLight(FColor Color);
	virtual void EnableLighting(UBOOL UseDynamic, UBOOL UseStatic = 1, UBOOL Modulate2X = 0, FBaseTexture* Lightmap = NULL, UBOOL LightingOnly = 0, const FSphere& LitSphere = FSphere(FVector(0, 0, 0), 0), int = 0);
	virtual void SetLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale = 1.0f);
	virtual void SetNPatchTesselation(FLOAT Tesselation){}
	virtual void SetDistanceFog(UBOOL Enable, FLOAT FogStart, FLOAT FogEnd, FColor Color);
	virtual UBOOL EnableFog(UBOOL Enable);
	virtual UBOOL IsFogEnabled();
	virtual void SetGlobalColor(FColor Color);
	virtual void SetTransform(ETransformType Type, const FMatrix& Matrix);
	virtual FMatrix GetTransform(ETransformType Type) const;
	virtual void SetMaterial(UMaterial* Material, FString* ErrorString = NULL, UMaterial** ErrorMaterial = NULL, INT* NumPasses = NULL);
	virtual UBOOL SetHardwareShaderMaterial(UHardwareShader* Material, FString* ErrorString = NULL, UMaterial** ErrorMaterial = NULL);
	virtual void CopyBackBufferToTarget(FAuxRenderTarget* Target);
	virtual void SetStencilOp(ECompareFunction Test, DWORD Ref, DWORD Mask, EStencilOp FailOp, EStencilOp ZFailOp, EStencilOp PassOp, DWORD WriteMask);
	virtual void EnableStencil(UBOOL Enable);
	virtual void EnableDepth(UBOOL Enable);
	virtual void SetPrecacheMode(EPrecacheMode InPrecacheMode){ PrecacheMode = InPrecacheMode; }
	virtual void SetZBias(INT ZBias);
	virtual INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams);
	virtual INT SetDynamicStream(EVertexShader Shader, FVertexStream* Stream);
	virtual INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex);
	virtual INT SetDynamicIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex);
	virtual void DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex = INDEX_NONE, INT MaxIndex = INDEX_NONE);
	virtual void SetFillMode(EFillMode FillMode);

private:
	struct FModifierInfo{
		FMatrix               TexMatrix;
		ETexCoordSrc          TexCoordSrc;
		ETexCoordCount        TexCoordCount;
		ETexClampModeOverride TexUClamp;
		ETexClampModeOverride TexVClamp;
	};

	UMaterial* RemoveModifiers(UModifier* Modifier, FModifierInfo* ModifierInfo = NULL);
	void GetShaderConstants(FSConstantsInfo* Info, FPlane* Constants, INT NumConstants);
	void SetTexture(FBaseTexture* Texture, INT TextureIndex, FLOAT BumpSize = 0.0f);
	void SetBitmapTexture(UBitmapMaterial* Bitmap, INT TextureIndex, FLOAT BumpSize = 0.0f);
};
