#include "../Inc/OpenGLRenderInterface.h"

#include "../Inc/OpenGLRenderDevice.h"
#include "OpenGLResource.h"
#include "GL/glew.h"

/*
 * FOpenGLRenderInterface
 */

FOpenGLRenderInterface::FOpenGLRenderInterface(UOpenGLRenderDevice* InRenDev) : RenDev(InRenDev),
                                                                                CurrentState(&SavedStates[0]),
                                                                                GlobalUBO(GL_NONE){
	CurrentState->Uniforms.LocalToWorld = FMatrix::Identity;
	CurrentState->Uniforms.WorldToCamera = FMatrix::Identity;
	CurrentState->Uniforms.CameraToScreen = FMatrix::Identity;
}

void FOpenGLRenderInterface::FlushResources(){
	check(CurrentState == &SavedStates[0]);

	if(GlobalUBO){
		glDeleteBuffers(1, &GlobalUBO);
		GlobalUBO = GL_NONE;
	}

	CurrentState->RenderTarget = NULL;
	CurrentState->Shader = NULL;
	CurrentState->IndexBuffer = NULL;
	CurrentState->IndexBufferBaseIndex = 0;
	CurrentState->VAO = GL_NONE;
	CurrentState->NumVertexStreams = 0;
}

void FOpenGLRenderInterface::UpdateShaderUniforms(){
	// This is done here to avoid a possibly unnecessary matrix multiplication each time SetTransform is called
	CurrentState->Uniforms.Transform = CurrentState->Uniforms.LocalToWorld *
	                                   CurrentState->Uniforms.WorldToCamera *
	                                   CurrentState->Uniforms.CameraToScreen;

	if(!GlobalUBO){
		glCreateBuffers(1, &GlobalUBO);
		glNamedBufferStorage(GlobalUBO, sizeof(FOpenGLGlobalUniforms), &CurrentState->Uniforms, GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, GlobalUBO); // Binding index 0 is reserved for the global uniform block
	}else{
		glNamedBufferSubData(GlobalUBO, 0, sizeof(FOpenGLGlobalUniforms), &CurrentState->Uniforms);
	}

	NeedUniformUpdate = 0;
}

void FOpenGLRenderInterface::PushState(INT Flags){
	guardFunc;

	// Restore OpenGL state only if needed rather than doing these checks each time PopState is called
	if(PoppedState){
		if(CurrentState->RenderTarget != PoppedState->RenderTarget)
			CurrentState->RenderTarget->BindRenderTarget();

		if(CurrentState->ViewportX != PoppedState->ViewportX ||
		   CurrentState->ViewportY != PoppedState->ViewportY ||
		   CurrentState->ViewportWidth != PoppedState->ViewportWidth ||
		   CurrentState->ViewportHeight != PoppedState->ViewportHeight){
			SetViewport(CurrentState->ViewportX, CurrentState->ViewportY, CurrentState->ViewportWidth, CurrentState->ViewportHeight);
		}

		if(CurrentState->bStencilTest != PoppedState->bStencilTest)
			EnableStencilTest(CurrentState->bStencilTest);

		if(CurrentState->ZBias != PoppedState->ZBias)
			SetZBias(CurrentState->ZBias);

		if(CurrentState->Shader != PoppedState->Shader)
			CurrentState->Shader->Bind();

		if(CurrentState->VAO != PoppedState->VAO)
			glBindVertexArray(CurrentState->VAO);

		for(INT i = 0; i < PoppedState->NumVertexStreams; ++i)
			glBindVertexBuffer(i, GL_NONE, 0, 0);

		for(INT i = 0; i < CurrentState->NumVertexStreams; ++i)
			CurrentState->VertexStreams[i]->Bind(i);

		if(CurrentState->IndexBuffer != PoppedState->IndexBuffer){
			if(CurrentState->IndexBuffer)
				CurrentState->IndexBuffer->Bind();
			else
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);
		}

		NeedUniformUpdate = CurrentState->UniformRevision != PoppedState->UniformRevision;
		PoppedState = NULL;
	}

	++CurrentState;

	check(CurrentState <= &SavedStates[MAX_STATESTACKDEPTH]);

	appMemcpy(CurrentState, CurrentState - 1, sizeof(FOpenGLSavedState));

	unguard;
}

void FOpenGLRenderInterface::PopState(INT Flags){
	guardFunc;

	if(!PoppedState)
		PoppedState = CurrentState;

	--CurrentState;

	check(CurrentState >= &SavedStates[0]);

	CurrentState->NeedFixedFunctionShaderUniformUpdate = CurrentState->UsingFixedFunctionShader;

	unguard;
}

UBOOL FOpenGLRenderInterface::SetRenderTarget(FRenderTarget* RenderTarget, bool bFSAA){
	guardFunc;

	bool Updated = false;
	QWORD CacheId = RenderTarget->GetCacheId();
	INT Revision = RenderTarget->GetRevision();
	FOpenGLTexture* NewRenderTarget = static_cast<FOpenGLTexture*>(RenDev->GetCachedResource(CacheId));

	if(!NewRenderTarget)
		NewRenderTarget = new FOpenGLTexture(RenDev, CacheId);

	if(NewRenderTarget->Revision != Revision){
		Updated = true;
		NewRenderTarget->Cache(RenderTarget);
	}

	if(CurrentState->RenderTarget != NewRenderTarget || Updated)
		NewRenderTarget->BindRenderTarget();

	CurrentState->RenderTarget = NewRenderTarget;

	return 1;

	unguard;
}

void FOpenGLRenderInterface::SetViewport(INT X, INT Y, INT Width, INT Height){
	CurrentState->ViewportX = X;
	CurrentState->ViewportY = Y;
	CurrentState->ViewportWidth = Width;
	CurrentState->ViewportHeight = Height;

	glViewport(X, Y, Width, Height);
}

void FOpenGLRenderInterface::Clear(UBOOL UseColor, FColor Color, UBOOL UseDepth, FLOAT Depth, UBOOL UseStencil, DWORD Stencil){
	GLbitfield Flags = 0;

	if(UseColor){
		glClearColor(Color.R / 255.0f, Color.G / 255.0f, Color.B / 255.0f, Color.A / 255.0f);
		Flags |= GL_COLOR_BUFFER_BIT;
	}

	if(UseDepth){
		glClearDepth(Depth);
		Flags |= GL_DEPTH_BUFFER_BIT;
	}

	if(UseStencil){
		glClearStencil(Stencil);
		Flags |= GL_STENCIL_BUFFER_BIT;
	}

	glClear(Flags);
}

void FOpenGLRenderInterface::SetCullMode(ECullMode CullMode){
	CurrentState->CullMode = CullMode;

	if(CullMode != CM_None){
		GLenum NewCullMode;

		if(CullMode == CM_CCW)
			NewCullMode = GL_FRONT;
		else
			NewCullMode = GL_BACK;

		glEnable(GL_CULL_FACE);
		glCullFace(NewCullMode);
	}else{
		glDisable(GL_CULL_FACE);
	}
}

void FOpenGLRenderInterface::SetAmbientLight(FColor Color){
	NeedUniformUpdate = 1;
	++CurrentState->UniformRevision;
	CurrentState->Uniforms.AmbientLight = GLSL_vec4(Color);
}

void FOpenGLRenderInterface::SetTransform(ETransformType Type, const FMatrix& Matrix){
	checkSlow(Type < 3);

	switch(Type){
	case TT_LocalToWorld:
		CurrentState->Uniforms.LocalToWorld = Matrix;
		break;
	case TT_WorldToCamera:
		CurrentState->Uniforms.WorldToCamera = Matrix;
		break;
	case TT_CameraToScreen:
		CurrentState->Uniforms.CameraToScreen = Matrix;
	}

	++CurrentState->UniformRevision;
	NeedUniformUpdate = 1;
}

FMatrix FOpenGLRenderInterface::GetTransform(ETransformType Type) const{
	checkSlow(Type < 3);

	switch(Type){
	case TT_LocalToWorld:
		return CurrentState->Uniforms.LocalToWorld;
	case TT_WorldToCamera:
		return CurrentState->Uniforms.WorldToCamera;
	case TT_CameraToScreen:
		return CurrentState->Uniforms.CameraToScreen;
	}

	return FMatrix::Identity;
}

void FOpenGLRenderInterface::SetMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial, INT* NumPasses){
	guardFunc;

	// Init default material state

	CurrentState->NeedFixedFunctionShaderUniformUpdate = true;
	CurrentState->UsingConstantColor = false;
	CurrentState->NumStages = 1;
	CurrentState->StageTexCoordIndices[0] = 0;
	CurrentState->StageTexMatrices[0] = FMatrix::Identity;
	CurrentState->StageColorArgs[0] = CA_Diffuse;
	CurrentState->StageColorArgs[1] = CA_Diffuse;
	CurrentState->StageColorOps[0] = COP_Arg1;
	CurrentState->StageAlphaArgs[0] = COP_Arg1;
	CurrentState->StageAlphaArgs[1] = COP_Arg1;
	CurrentState->StageAlphaOps[0] = AOP_Arg1;
	CurrentState->ConstantColor = FPlane(1.0f, 1.0f, 1.0f, 1.0f);

	if(!Material || PrecacheMode == PRECACHE_VertexBuffers)
		Material = GetDefault<UMaterial>()->DefaultMaterial;

	if(GIsEditor){
		TArray<UMaterial*> History;

		if(!Material->CheckCircularReferences(History)){
			if(History.Num() >= 0){
				if(ErrorString)
					*ErrorString = FString::Printf("Circular material reference in '%s'", History.Last()->GetName());

				if(ErrorMaterial)
					*ErrorMaterial = History.Last();
			}

			return;
		}
	}

	Material->PreSetMaterial(GEngineTime);

	if(CheckMaterial<UShader>(&Material)){
		SetSimpleMaterial(static_cast<UShader*>(Material)->Diffuse, ErrorString, ErrorMaterial);
	}else if(CheckMaterial<UCombiner>(&Material)){
		SetSimpleMaterial(Material, ErrorString, ErrorMaterial);
	}else if(CheckMaterial<UConstantMaterial>(&Material)){
		SetSimpleMaterial(Material, ErrorString, ErrorMaterial);
	}else if(CheckMaterial<UBitmapMaterial>(&Material)){
		SetSimpleMaterial(Material, ErrorString, ErrorMaterial);
	}else if(CheckMaterial<UTerrainMaterial>(&Material)){

	}else if(CheckMaterial<UParticleMaterial>(&Material)){

	}else if(CheckMaterial<UProjectorMultiMaterial>(&Material)){

	}else if(CheckMaterial<UProjectorMaterial>(&Material)){

	}else if(CheckMaterial<UHardwareShaderWrapper>(&Material)){
		UHardwareShaderWrapper* HardwareShaderWrapper = static_cast<UHardwareShaderWrapper*>(Material);

		HardwareShaderWrapper->SetupShaderWrapper(this);
		SetSimpleMaterial(HardwareShaderWrapper->ShaderImplementation->Textures[0], ErrorString, ErrorMaterial);
	}else if(CheckMaterial<UHardwareShader>(&Material)){
		SetSimpleMaterial(static_cast<UHardwareShader*>(Material)->Textures[0], ErrorString, ErrorMaterial);
	}

	unguard;
}

bool FOpenGLRenderInterface::SetSimpleMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial){
	INT StagesUsed = 0;
	INT TexturesUsed = 0;

	if(!HandleCombinedMaterial(Material, StagesUsed, TexturesUsed, ErrorString, ErrorMaterial))
		return false;

	CurrentState->NumStages = StagesUsed;

	return true;
}

bool FOpenGLRenderInterface::HandleCombinedMaterial(UMaterial* Material, INT& StagesUsed, INT& TexturesUsed, FString* ErrorString, UMaterial** ErrorMaterial){
	if(!Material)
		return true;

	if(CheckMaterial<UVertexColor>(&Material, StagesUsed)){
		if(StagesUsed >= MAX_SHADER_STAGES){
			if(ErrorString)
				*ErrorString = "No stages left for vertex color";

			if(ErrorMaterial)
				*ErrorMaterial = Material;

			return false;
		}

		CurrentState->StageColorArgs[StagesUsed * 2] = CA_Diffuse;
		CurrentState->StageColorOps[StagesUsed] = COP_Arg1;
		CurrentState->StageAlphaArgs[StagesUsed * 2 + 1] = CA_Previous;
		CurrentState->StageAlphaOps[StagesUsed] = AOP_Arg2;

		++StagesUsed;

		return true;
	}else if(CheckMaterial<UConstantMaterial>(&Material, StagesUsed)){
		if(CurrentState->UsingConstantColor){
			if(ErrorString)
				*ErrorString = "Only one ConstantMaterial may be used per material";

			if(ErrorMaterial)
				*ErrorMaterial = Material;

			return false;
		}

		if(StagesUsed >= MAX_SHADER_STAGES){
			if(ErrorString)
				*ErrorString = "No stages left for constant color";

			if(ErrorMaterial)
				*ErrorMaterial = Material;

			return false;
		}

		CurrentState->ConstantColor = static_cast<UConstantMaterial*>(Material)->GetColor(GEngineTime);
		CurrentState->StageColorArgs[StagesUsed * 2] = CA_Constant;
		CurrentState->StageColorOps[StagesUsed] = COP_Arg1;
		CurrentState->StageAlphaArgs[StagesUsed * 2] = CA_Constant;
		CurrentState->StageAlphaOps[StagesUsed] = AOP_Arg1;

		++StagesUsed;

		return true;
	}else if(CheckMaterial<UBitmapMaterial>(&Material, StagesUsed)){
		if(StagesUsed >= MAX_SHADER_STAGES || TexturesUsed >= MAX_TEXTURES){
			if(ErrorString)
				*ErrorString = "No stages left for bitmap material";

			if(ErrorMaterial)
				*ErrorMaterial = Material;

			return false;
		}

		UBitmapMaterial* Bitmap = static_cast<UBitmapMaterial*>(Material);
		FBaseTexture* Texture = Bitmap->Get(LockedViewport->CurrentTime, LockedViewport)->GetRenderInterface();

		if(!Texture)
			return true;

		QWORD CacheId = Texture->GetCacheId();
		FOpenGLTexture* GLTexture = static_cast<FOpenGLTexture*>(RenDev->GetCachedResource(CacheId));

		if(!GLTexture)
			GLTexture = new FOpenGLTexture(RenDev, CacheId);

		if(GLTexture->Revision != Texture->GetRevision())
			GLTexture->Cache(Texture);

		GLTexture->BindTexture(TexturesUsed);

		CurrentState->StageColorArgs[StagesUsed * 2] = CA_Texture0 + TexturesUsed;
		CurrentState->StageColorOps[StagesUsed] = COP_Arg1;
		CurrentState->StageAlphaArgs[StagesUsed * 2] = CA_Texture0 + TexturesUsed;
		CurrentState->StageAlphaOps[StagesUsed] = AOP_Arg1;

		++StagesUsed;
		++TexturesUsed;

		return true;
	}else if(CheckMaterial<UCombiner>(&Material, StagesUsed)){
		return HandleCombinedMaterial(static_cast<UCombiner*>(Material)->Material1, StagesUsed, TexturesUsed, ErrorString, ErrorMaterial);
	}

	return true;
}

static GLenum GetStencilFunc(ECompareFunction Test){
	switch(Test){
	case CF_Never:
		return GL_NEVER;
	case CF_Less:
		return GL_LESS;
	case CF_Equal:
		return GL_EQUAL;
	case CF_LessEqual:
		return GL_LEQUAL;
	case CF_Greater:
		return GL_GREATER;
	case CF_NotEqual:
		return GL_NOTEQUAL;
	case CF_GreaterEqual:
		return GL_GEQUAL;
	case CF_Always:
		return GL_ALWAYS;
	}

	return GL_NEVER;
};

static GLenum GetStencilOp(EStencilOp StencilOp){
	switch(StencilOp){
	case SO_Keep:
		return GL_KEEP;
	case SO_Zero:
		return GL_ZERO;
	case SO_Replace:
		return GL_REPLACE;
	case SO_IncrementSat:
		return GL_INCR_WRAP;
	case SO_DecrementSat:
		return GL_DECR_WRAP;
	case SO_Invert:
		return GL_INVERT;
	case SO_Increment:
		return GL_INCR;
	case SO_Decrement:
		return GL_DECR;
	}

	return GL_KEEP;
}

void FOpenGLRenderInterface::SetStencilOp(ECompareFunction Test, DWORD Ref, DWORD Mask, EStencilOp FailOp, EStencilOp ZFailOp, EStencilOp PassOp, DWORD WriteMask){
	glStencilOp(GetStencilOp(FailOp), GetStencilOp(ZFailOp), GetStencilOp(PassOp));
	glStencilFunc(GetStencilFunc(Test), Ref, WriteMask);
}

void FOpenGLRenderInterface::EnableStencilTest(UBOOL Enable){
	CurrentState->bStencilTest = Enable != 0;
	glStencilMask(Enable ? 0xFF : 0x00);
}

void FOpenGLRenderInterface::EnableZWrite(UBOOL Enable){
	CurrentState->bZWrite = Enable != 0;
	glDepthMask(Enable ? GL_TRUE : GL_FALSE);
}

void FOpenGLRenderInterface::SetZBias(INT ZBias){
	glPolygonOffset(-ZBias, -ZBias);
	CurrentState->ZBias = ZBias;
}

INT FOpenGLRenderInterface::SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams, bool IsDynamic){
	checkSlow(!IsDynamic || NumStreams == 1);

	// NOTE: Stream declarations must be completely zeroed to get consistent hash values when looking up the VAO later
	appMemzero(VertexStreamDeclarations, sizeof(VertexStreamDeclarations));

	INT Size = 0;

	for(INT i = 0; i < NumStreams; ++i){
		QWORD CacheId = Streams[i]->GetCacheId();
		FOpenGLVertexStream* Stream;

		if(IsDynamic){
			Stream = RenDev->GetDynamicVertexStream();
		}else{
			Stream = static_cast<FOpenGLVertexStream*>(RenDev->GetCachedResource(CacheId));

			if(!Stream)
				Stream = new FOpenGLVertexStream(RenDev, CacheId);
		}

		if(IsDynamic || Stream->Revision != Streams[i]->GetRevision()){
			Stream->Cache(Streams[i]);
			Size += Stream->BufferSize;
		}

		CurrentState->VertexStreams[i] = Stream;
		VertexStreamDeclarations[i].Init(Streams[i]);
	}

	CurrentState->NumVertexStreams = NumStreams;

	// Look up VAO by format
	GLuint VAO = RenDev->GetVAO(VertexStreamDeclarations, CurrentState->NumVertexStreams);

	if(VAO != CurrentState->VAO){
		glBindVertexArray(VAO);
		CurrentState->VAO = VAO;

		if(CurrentState->IndexBuffer)
			CurrentState->IndexBuffer->Bind();

		NeedUniformUpdate = 1;
	}

	for(INT i = 0; i < CurrentState->NumVertexStreams; ++i)
		CurrentState->VertexStreams[i]->Bind(i);

	return 0;
}

INT FOpenGLRenderInterface::SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams){
	return SetVertexStreams(Shader, Streams, NumStreams, false);
}

INT FOpenGLRenderInterface::SetDynamicStream(EVertexShader Shader, FVertexStream* Stream){
	SetVertexStreams(Shader, &Stream, 1, true);

	return 0;
}

INT FOpenGLRenderInterface::SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex, bool IsDynamic){
	bool RequiresCaching = false;

	if(IndexBuffer){
		FOpenGLIndexBuffer* Buffer;
		INT IndexSize = IndexBuffer->GetIndexSize();

		checkSlow(IndexSize == sizeof(_WORD) || IndexSize == sizeof(DWORD));

		if(IsDynamic){
			Buffer = RenDev->GetDynamicIndexBuffer(IndexSize);
		}else{
			QWORD CacheId = IndexBuffer->GetCacheId();

			Buffer = static_cast<FOpenGLIndexBuffer*>(RenDev->GetCachedResource(CacheId));

			if(!Buffer)
				Buffer = new FOpenGLIndexBuffer(RenDev, CacheId);
		}

		if(IsDynamic || Buffer->Revision != IndexBuffer->GetRevision() || Buffer->IndexSize != IndexSize){
			RequiresCaching = true;
			Buffer->Cache(IndexBuffer);
		}

		CurrentState->IndexBufferBaseIndex = BaseIndex;
		CurrentState->IndexBuffer = Buffer;

		Buffer->Bind();
	}else{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);
		CurrentState->IndexBuffer = NULL;
		CurrentState->IndexBufferBaseIndex = 0;
	}

	return RequiresCaching ? IndexBuffer->GetSize() : 0;
}

INT FOpenGLRenderInterface::SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){
	return SetIndexBuffer(IndexBuffer, BaseIndex, false); // Returns the size of the index buffer but only if it was newly created. Unused at the call site.
}

INT FOpenGLRenderInterface::SetDynamicIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){
	SetIndexBuffer(IndexBuffer, BaseIndex, true);

	return 0; // Returns the new base index of the dynamic buffer which is always 0 here
}

void FOpenGLRenderInterface::DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex, INT MaxIndex){
	if(NeedUniformUpdate)
		UpdateShaderUniforms();

	EnableZTest(1);

	if(CurrentState->UsingFixedFunctionShader && CurrentState->NeedFixedFunctionShaderUniformUpdate){
		// Why is this needed???
		for(INT i = 0; i < CurrentState->NumStages; ++i){
			CurrentState->StageTexMatrices[i].M[3][0] = CurrentState->StageTexMatrices[i].M[2][0];
			CurrentState->StageTexMatrices[i].M[3][1] = CurrentState->StageTexMatrices[i].M[2][1];
		}

		glUniform1i(SU_NumStages, CurrentState->NumStages);
		glUniform1iv(SU_StageTexCoordIndices, ARRAY_COUNT(CurrentState->StageTexCoordIndices), CurrentState->StageTexCoordIndices);
		glUniformMatrix4fv(SU_StageTexMatrices, ARRAY_COUNT(CurrentState->StageTexMatrices), GL_FALSE, (GLfloat*)CurrentState->StageTexMatrices);
		glUniform1iv(SU_StageColorArgs, ARRAY_COUNT(CurrentState->StageColorArgs), CurrentState->StageColorArgs);
		glUniform1iv(SU_StageColorOps, ARRAY_COUNT(CurrentState->StageColorOps), CurrentState->StageColorOps);
		glUniform1iv(SU_StageAlphaArgs, ARRAY_COUNT(CurrentState->StageAlphaArgs), CurrentState->StageAlphaArgs);
		glUniform1iv(SU_StageAlphaOps, ARRAY_COUNT(CurrentState->StageAlphaOps), CurrentState->StageAlphaOps);
		glUniform4fv(SU_ConstantColor, 1, (GLfloat*)&CurrentState->ConstantColor);
		CurrentState->NeedFixedFunctionShaderUniformUpdate = false;
	}

	GLenum Mode  = 0;
	INT    Count = NumPrimitives;

	switch(PrimitiveType){
	case PT_PointList:
		Mode = GL_POINTS;
		break;
	case PT_LineList:
		Mode = GL_LINES;
		Count *= 2;
		break;
	case PT_TriangleList:
		Mode = GL_TRIANGLES;
		Count *= 3;
		break;
	case PT_TriangleStrip:
		Mode = GL_TRIANGLE_STRIP;
		Count += 2;
		break;
	case PT_TriangleFan:
		Mode = GL_TRIANGLE_FAN;
		Count += 2;
		break;
	default:
		appErrorf("Unexpected EPrimitiveType (%i)", PrimitiveType);
	};

	if(CurrentState->IndexBuffer){
		INT IndexSize = CurrentState->IndexBuffer->IndexSize;

		glDrawRangeElements(Mode,
		                    MinIndex,
		                    MaxIndex,
		                    Count,
		                    IndexSize == sizeof(DWORD) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT,
		                    reinterpret_cast<void*>((FirstIndex + CurrentState->IndexBufferBaseIndex) * IndexSize));
	}else{
		glDrawArrays(Mode, 0, Count);
	}
}

void FOpenGLRenderInterface::SetFillMode(EFillMode FillMode){
	CurrentState->FillMode = FillMode;

	glPolygonMode(GL_FRONT_AND_BACK, FillMode == FM_Wireframe ? GL_LINE : GL_FILL);
}

void FOpenGLRenderInterface::EnableZTest(UBOOL Enable){
	CurrentState->bZTest = Enable != 0;
	glDepthFunc(Enable ? GL_LEQUAL : GL_ALWAYS);
}

void FOpenGLRenderInterface::SetShader(FShaderGLSL* NewShader){
	QWORD CacheId = NewShader->GetCacheId();
	FOpenGLShader* Shader = static_cast<FOpenGLShader*>(RenDev->GetCachedResource(CacheId));

	if(!Shader)
		Shader = new FOpenGLShader(RenDev, CacheId);

	if(Shader->Revision != NewShader->GetRevision())
		Shader->Cache(NewShader);

	CurrentState->Shader = Shader;
	Shader->Bind();
	CurrentState->UsingFixedFunctionShader = NewShader == &RenDev->FixedFunctionShader;
	CurrentState->NeedFixedFunctionShaderUniformUpdate = true;
}

void FOpenGLRenderInterface::SetupPerFrameShaderConstants(){
	FLOAT Time = appFmod(static_cast<FLOAT>(appSeconds()), 120.0f);

	CurrentState->Uniforms.Time = Time;
	CurrentState->Uniforms.SinTime = appSin(Time);
	CurrentState->Uniforms.CosTime = appCos(Time);
	CurrentState->Uniforms.TanTime = appTan(Time);
}
