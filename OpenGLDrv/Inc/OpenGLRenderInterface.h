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
#define MAX_LIGHTS 8
#define MAX_VERTEX_SHADER_CONSTANTS 96
#define MAX_PIXEL_SHADER_CONSTANTS 8

struct FStreamDeclaration{
	FVertexComponent Components[MAX_VERTEX_COMPONENTS];
	INT              NumComponents;

	void Init(FVertexStream* VertexStream){ NumComponents = VertexStream->GetComponents(Components); }
};

enum EColorArg{
	CA_Diffuse,
	CA_Constant,
	CA_Previous,
	CA_Temp1,
	CA_Temp2,
	CA_Texture0,
	CA_Texture1,
	CA_Texture2,
	CA_Texture3,
	CA_Texture4,
	CA_Texture5,
	CA_Texture6,
	CA_Texture7
};

enum EColorArgModifier{
	CAM_CubeMap = 1 << 30,
	CAM_Invert  = 1 << 31
};

enum EColorOpModifier{
	COPM_SaveTemp1 = 1 << 30,
	COPM_SaveTemp2 = 1 << 31
};

enum EColorOp{
	COP_Arg1,
	COP_Arg2,
	COP_Modulate,
	COP_Modulate2X,
	COP_Modulate4X,
	COP_Add,
	COP_Subtract,
	COP_AlphaBlend,
	COP_AddAlphaModulate
};

enum EAlphaOp{
	AOP_Arg1,
	AOP_Arg2,
	AOP_Modulate,
	AOP_Add,
	AOP_Blend
};

enum EAlphaOpModifier{
	AOPM_LightInfluence = 1 << 31
};

enum EShaderUniforms{
	SU_NumStages            = 0,  // int
	SU_TexCoordCount        = 1,  // int
	SU_StageTexCoordSources = 2,  // int[8]
	SU_StageTexMatrices     = 10, // mat4[8]
	SU_StageColorArgs       = 18, // int[16]
	SU_StageColorOps        = 34, // int[8]
	SU_StageAlphaArgs       = 42, // int[16]
	SU_StageAlphaOps        = 58, // int[8]
	SU_ConstantColor        = 66, // vec4
	SU_LightingEnabled      = 67, // bool
	SU_LightFactor          = 68  // float
};

enum EHardwareShaderUniforms{
	HSU_VSConstants = 0,
	HSU_PSConstants = MAX_VERTEX_SHADER_CONSTANTS
};

#define GLSL_STRUCT(x) GLSL_struct x // Workaround to get struct name to show up in C++ but not GLSL
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
	UNIFORM_BLOCK_MEMBER(STRUCT(TextureInfo){ \
		UNIFORM_STRUCT_MEMBER(bool, IsCubemap) \
		UNIFORM_STRUCT_MEMBER(bool, IsBumpmap) \
	}, TextureInfo[MAX_TEXTURES]) \
	UNIFORM_BLOCK_MEMBER(STRUCT(Light){ \
		UNIFORM_STRUCT_MEMBER(vec4, Color) \
		UNIFORM_STRUCT_MEMBER(vec3, Position) \
		UNIFORM_STRUCT_MEMBER(vec3, Direction) \
		UNIFORM_STRUCT_MEMBER(float, Radius) \
		UNIFORM_STRUCT_MEMBER(float, InvRadius) \
		UNIFORM_STRUCT_MEMBER(float, CosCone) \
		UNIFORM_STRUCT_MEMBER(int, Type) \
	}, Lights[MAX_LIGHTS]) \
	UNIFORM_BLOCK_MEMBER(int, NumLights)

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

struct FOpenGLRenderState{
	ECullMode             CullMode;
	EFillMode             FillMode;

	bool                  bZWrite;
	bool                  bZTest;
	bool                  bStencilTest;

	ECompareFunction      StencilCompare;
	DWORD                 StencilRef;
	DWORD                 StencilMask;
	EStencilOp            StencilFailOp;
	EStencilOp            StencilZFailOp;
	EStencilOp            StencilPassOp;
	DWORD                 StencilWriteMask;

	INT                   ZBias;

	INT                   ViewportX;
	INT                   ViewportY;
	INT                   ViewportWidth;
	INT                   ViewportHeight;

	unsigned int          SrcBlend;
	unsigned int          DstBlend;

	FRenderTarget*        RenderTarget;

	FOpenGLIndexBuffer*   IndexBuffer;
	INT                   NumVertexStreams;
	FOpenGLVertexStream*  VertexStreams[MAX_VERTEX_STREAMS];

	INT                   NumTextures;
	FOpenGLTextureUnit    TextureUnits[MAX_TEXTURES];
};

/*
 * OpenGL RenderInterface
 */
class FOpenGLRenderInterface : public FRenderInterface{
public:
	struct FOpenGLSavedState : FOpenGLGlobalUniforms, FOpenGLRenderState{
		INT                   UniformRevision;

		unsigned int          VAO;
		INT                   IndexBufferBaseIndex;

		bool                  RenderTargetMatchesBackbuffer;

		// Light

		bool                  UseDynamicLighting;
		bool                  UseStaticLighting;
		bool                  LightingModulate2X;
		FBaseTexture*         Lightmap;
		FSphere               LitSphere;
	};

	UOpenGLRenderDevice*      RenDev;
	UViewport*                LockedViewport;

	bool                      bStencilEnabled;

	EPrecacheMode             PrecacheMode;

	FOpenGLSavedState         SavedStates[MAX_STATESTACKDEPTH];
	FOpenGLSavedState*        CurrentState;
	FOpenGLRenderState        RenderState;

	FOpenGLShader*            CurrentShader;

	bool                      NeedUniformUpdate;
	unsigned int              GlobalUBO;

	INT                       TextureAnisotropy;
	unsigned int              Samplers[MAX_TEXTURES];

	TMap<DWORD, unsigned int> VAOsByDeclId;

	// Fixed function emulation

	bool                      Unlit;
	bool                      UsingConstantColor;
	bool                      ModifyColor;
	bool                      ModifyFramebufferBlending;
	INT                       NumStages;
	INT                       TexCoordCount;
	INT                       StageTexCoordSources[MAX_SHADER_STAGES];
	FMatrix                   StageTexMatrices[MAX_SHADER_STAGES];
	INT                       StageColorArgs[MAX_SHADER_STAGES][2]; // EColorArg for Arg1 and Arg2
	INT                       StageColorOps[MAX_SHADER_STAGES];     // EColorOp
	INT                       StageAlphaArgs[MAX_SHADER_STAGES][2]; // EColorArg for Arg1 and Arg2
	INT                       StageAlphaOps[MAX_SHADER_STAGES];     // EAlphaOp
	FPlane                    ConstantColor;

	FOpenGLRenderInterface(UOpenGLRenderDevice* InRenDev);

	void Init();
	void Exit();
	void CommitRenderState();
	void Locked(UViewport* Viewport);
	void Unlocked();
	void UpdateGlobalShaderUniforms();
	void SetFramebufferBlending(EFrameBufferBlending Mode);

	// Overrides
	virtual void PushState(INT Flags = 0);
	virtual void PopState(INT Flags = 0);
	virtual UBOOL SetRenderTarget(FRenderTarget* RenderTarget, bool MatchBackbuffer);
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

	void SetShader(FShaderGLSL* NewShader);
	unsigned int GetVAO(const FStreamDeclaration* Declarations, INT NumStreams);
	void SetupPerFrameShaderConstants();

private:
	INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex, bool IsDynamic);
	INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams, bool IsDynamic);
	void InitDefaultMaterialStageState(INT StageIndex);
	void SetTexture(FBaseTexture* Texture, INT TextureUnit);
	void SetBitmapTexture(UBitmapMaterial* Bitmap, INT TextureUnit);
	void GetShaderConstants(FSConstantsInfo* Info, FPlane* Constants, INT NumConstants);
	bool SetSimpleMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial);
	bool HandleCombinedMaterial(UMaterial* Material, INT& PassesUsed, INT& TexturesUsed, FString* ErrorString, UMaterial** ErrorMaterial);
	bool SetShaderMaterial(UShader* Shader, FString* ErrorString, UMaterial** ErrorMaterial);
	bool SetTerrainMaterial(UTerrainMaterial* Terrain, FString* ErrorString, UMaterial** ErrorMaterial);
	bool SetParticleMaterial(UParticleMaterial* ParticleMaterial, FString* ErrorString, UMaterial** ErrorMaterial);
	void UseDiffuse();
	void UseLightmap(INT StageIndex, INT TextureUnit);

	template<typename T>
	bool CheckMaterial(UMaterial** Material, INT StageIndex, INT TextureIndex = -1){
		UMaterial* RootMaterial = *Material;

		if(RootMaterial->IsA<T>())
			return true;

		// Check for modifier chain pointing to a material of type T

		UModifier* Modifier = Cast<UModifier>(RootMaterial);

		*Material = NULL;

		while(Modifier){
			*Material = Cast<T>(Modifier->Material);
			Modifier = Cast<UModifier>(Modifier->Material);
		}

		if(!*Material){
			*Material = RootMaterial; // Reset to initial

			return false;
		}else if(StageIndex < 0){ // StageIndex < 0 means we only want to check the material's type but not apply the modifiers
			*Material = RootMaterial; // Reset to initial

			return true;
		}else{ // Collect modifiers
			INT*     StageTexCoordSrc = &StageTexCoordSources[StageIndex];
			FMatrix* StageTexMatrix = &StageTexMatrices[StageIndex];

			Modifier = static_cast<UModifier*>(RootMaterial);

			// Apply modifiers
			while(Modifier != *Material){
				if(Modifier->IsA<UTexModifier>()){
					UTexModifier* TexModifier = static_cast<UTexModifier*>(Modifier);
					FMatrix* Matrix = TexModifier->GetMatrix(GEngineTime);

					if(TexModifier->TexCoordSource != TCS_NoChange){
						*StageTexCoordSrc = TexModifier->TexCoordSource;

						switch(TexModifier->TexCoordSource){
						case TCS_CameraCoords:
							*StageTexMatrix *= CurrentState->WorldToCamera.Transpose();
							break;
						case TCS_CubeCameraSpaceReflection:
							{
								FMatrix Tmp = CurrentState->WorldToCamera;
								Tmp.M[3][0] = 0.0f;
								Tmp.M[3][1] = 0.0f;
								Tmp.M[3][2] = 0.0f;
								Tmp.M[3][3] = 1.0f;
								*StageTexMatrix *= Tmp;
							}
						}

						TexCoordCount = TexModifier->TexCoordCount + 2;
					}

					if(Matrix)
						*StageTexMatrix *= *Matrix;

					if(TextureIndex >= 0){
						if(TexModifier->UClampMode != TCO_UseTextureMode)
							CurrentState->TextureUnits[TextureIndex].ClampU = TexModifier->UClampMode - 1;

						if(TexModifier->VClampMode != TCO_UseTextureMode)
							CurrentState->TextureUnits[TextureIndex].ClampV = TexModifier->VClampMode - 1;
					}
				}else if(Modifier->IsA<UFinalBlend>()){
					UFinalBlend* FinalBlend = static_cast<UFinalBlend*>(Modifier);

					ModifyFramebufferBlending = true;
					SetFramebufferBlending(static_cast<EFrameBufferBlending>(FinalBlend->FrameBufferBlending));
					CurrentState->bZTest = FinalBlend->ZTest != 0;
					CurrentState->bZWrite = FinalBlend->ZWrite != 0;

					if(FinalBlend->TwoSided)
						CurrentState->CullMode = CM_None;

					if(FinalBlend->AlphaTest)
						CurrentState->AlphaRef = FinalBlend->AlphaRef / 255.0f;
				}else if(Modifier->IsA<UColorModifier>()){
					UColorModifier* ColorModifier = static_cast<UColorModifier*>(Modifier);

					UsingConstantColor = true;
					ModifyColor = true;
					ConstantColor = ColorModifier->Color;

					if(ColorModifier->RenderTwoSided)
						CurrentState->CullMode = CM_None;

					if(!ModifyFramebufferBlending && ColorModifier->AlphaBlend)
						SetFramebufferBlending(FB_AlphaBlend);
				}

				Modifier = static_cast<UModifier*>(Modifier->Material);
			}

			return true;
		}
	}
};
