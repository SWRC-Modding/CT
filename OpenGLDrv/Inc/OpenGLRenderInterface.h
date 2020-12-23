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

struct FStreamDeclaration{
	FVertexComponent Components[MAX_VERTEX_COMPONENTS];
	INT              NumComponents;

	void Init(FVertexStream* VertexStream){ NumComponents = VertexStream->GetComponents(Components); }
};

enum EColorArg{
	CA_Previous,
	CA_Diffuse,
	CA_Constant,
	CA_Texture0,
	CA_Texture1,
	CA_Texture2,
	CA_Texture3,
	CA_Texture4,
	CA_Texture5,
	CA_Texture6,
	CA_Texture7
};

enum EColorOp{
	COP_Arg1,
	COP_Arg2,
	COP_Modulate,
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

enum EShaderUniforms{
	SU_NumStages      = 0,  // int
	SU_StageColorArgs = 1,  // int[16]
	SU_StageColorOps  = 17, // int[8]
	SU_StageAlphaArgs = 25, // int[16]
	SU_StageAlphaOps  = 41, // int[8]
	SU_ConstantColor  = 49  // vec4
};

/*
 * OpenGL RenderInterface
 */
class FOpenGLRenderInterface : public FRenderInterface{
public:
	struct FOpenGLSavedState{
		INT                   UniformRevision;
		FOpenGLGlobalUniforms Uniforms;

		FOpenGLTexture*       RenderTarget;

		INT                   ViewportX;
		INT                   ViewportY;
		INT                   ViewportWidth;
		INT                   ViewportHeight;

		ECullMode             CullMode;
		EFillMode             FillMode;

		bool                  bStencilTest;
		bool                  bZWrite;
		bool                  bZTest;

		INT                   ZBias;

		FOpenGLShader*        Shader;

		INT                   IndexBufferBaseIndex;
		FOpenGLIndexBuffer*   IndexBuffer;

		unsigned int          VAO;
		INT                   NumVertexStreams;
		FOpenGLVertexStream*  VertexStreams[MAX_VERTEX_STREAMS];

		// Fixed function emulation

		bool                  UsingFixedFunctionShader;
		bool                  NeedFixedFunctionShaderUniformUpdate;
		bool                  UsingConstantColor;
		INT                   NumStages;
		INT                   StageColorArgs[MAX_SHADER_STAGES * 2]; // EColorArg for Arg1 and Arg2
		INT                   StageColorOps[MAX_SHADER_STAGES];      // EColorOp
		INT                   StageAlphaArgs[MAX_SHADER_STAGES * 2]; // EColorArg for Arg1 and Arg2
		INT                   StageAlphaOps[MAX_SHADER_STAGES];      // EAlphaOp
		FPlane                ConstantColor;
	};

	UOpenGLRenderDevice*      RenDev;
	UViewport*                LockedViewport;

	FOpenGLSavedState         SavedStates[MAX_STATESTACKDEPTH];
	FOpenGLSavedState*        CurrentState;
	FOpenGLSavedState*        PoppedState;

	bool                      NeedUniformUpdate;
	unsigned int              GlobalUBO;

	FStreamDeclaration        VertexStreamDeclarations[MAX_VERTEX_STREAMS];

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
	virtual void SetAmbientLight(FColor Color);
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
	virtual void SetPrecacheMode(EPrecacheMode InPrecacheMode){ PrecacheMode = InPrecacheMode; }
	virtual void SetZBias(INT ZBias);
	virtual INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams);
	virtual INT SetDynamicStream(EVertexShader Shader, FVertexStream* Stream);
	virtual INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex);
	virtual INT SetDynamicIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex);
	virtual void DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex = INDEX_NONE, INT MaxIndex = INDEX_NONE);
	virtual void SetFillMode(EFillMode FillMode);

	void EnableZTest(UBOOL Enable);
	void SetShader(FShaderGLSL* NewShader);
	void SetupPerFrameShaderConstants();

private:
	EPrecacheMode PrecacheMode;

	INT SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex, bool IsDynamic);
	INT SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams, bool IsDynamic);
	bool SetSimpleMaterial(UMaterial* Material, FString* ErrorString = NULL, UMaterial** ErrorMaterial = NULL);
	bool HandleCombinedMaterial(UMaterial* InMaterial, INT& PassesUsed, INT& TexturesUsed, FString* ErrorString = NULL, UMaterial** ErrorMaterial = NULL);

	template<typename T>
	bool CheckMaterial(UMaterial*& Material){
		UMaterial* RootMaterial = Material;

		if(Material->IsA<T>())
			return true;

		Material = NULL;

		// Check for modifier chain pointing to a material of type T

		UModifier* Modifier = Cast<UModifier>(Material);

		while(Modifier){
			Material = Cast<T>(Modifier->Material);
			Modifier = Cast<UModifier>(Modifier->Material);
		}

		if(Material){
			return true;
		}else{
			Material = RootMaterial; // Reset to initial

			return false;
		}
	}
};
