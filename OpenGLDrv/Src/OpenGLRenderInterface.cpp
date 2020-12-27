#include "../Inc/OpenGLRenderInterface.h"

#include "../Inc/OpenGLRenderDevice.h"
#include "OpenGLResource.h"
#include "GL/glew.h"

/*
 * FOpenGLRenderInterface
 */

FOpenGLRenderInterface::FOpenGLRenderInterface(UOpenGLRenderDevice* InRenDev) : RenDev(InRenDev),
                                                                                CurrentState(&SavedStates[0]),
                                                                                GlobalUBO(GL_NONE),
                                                                                PrecacheMode(PRECACHE_All){}

void FOpenGLRenderInterface::Init(){
	CurrentState->Uniforms.LocalToWorld = FMatrix::Identity;
	CurrentState->Uniforms.WorldToCamera = FMatrix::Identity;
	CurrentState->Uniforms.CameraToScreen = FMatrix::Identity;

	for(INT i = 0; i < MAX_SHADER_STAGES; ++i)
		InitDefaultMaterialStageState(i);

	CurrentState->UsingConstantColor = false;
	CurrentState->NumStages = 0;
	CurrentState->TexCoordCount = 2;
	CurrentState->ConstantColor = FPlane(1.0f, 1.0f, 1.0f, 1.0f);
	CurrentState->AlphaRef = -1.0f;

	// Create uniform buffer

	glCreateBuffers(1, &GlobalUBO);
	glNamedBufferStorage(GlobalUBO, sizeof(FOpenGLGlobalUniforms), &CurrentState->Uniforms, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, GlobalUBO); // Binding index 0 is reserved for the global uniform block

	// Create samplers

	glCreateSamplers(MAX_TEXTURES, Samplers);
	glBindSamplers(0, MAX_TEXTURES, Samplers);

	for(int i = 0; i < MAX_TEXTURES; ++i){
		glSamplerParameteri(Samplers[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glSamplerParameteri(Samplers[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}

void FOpenGLRenderInterface::Exit(){
	glDeleteBuffers(1, &GlobalUBO);
	GlobalUBO = GL_NONE;

	glDeleteSamplers(MAX_TEXTURES, Samplers);
	appMemzero(Samplers, sizeof(Samplers));

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

	glNamedBufferSubData(GlobalUBO, 0, sizeof(FOpenGLGlobalUniforms), &CurrentState->Uniforms);

	NeedUniformUpdate = 0;
}

void FOpenGLRenderInterface::PushState(INT Flags){
	guardFunc;

	RestoreLastState();

	++CurrentState;

	check(CurrentState <= &SavedStates[MAX_STATESTACKDEPTH]);

	appMemcpy(CurrentState, CurrentState - 1, sizeof(FOpenGLSavedState));

	unguard;
}

void FOpenGLRenderInterface::RestoreLastState(){
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

		if(CurrentState->bZTest != PoppedState->bZTest)
			EnableZTest(CurrentState->bZTest);

		if(CurrentState->bZWrite != PoppedState->bZWrite)
			EnableZWrite(CurrentState->bZWrite);

		NeedUniformUpdate = CurrentState->UniformRevision != PoppedState->UniformRevision;
		PoppedState = NULL;
	}
}

void FOpenGLRenderInterface::PopState(INT Flags){
	guardFunc;

	if(!PoppedState)
		PoppedState = CurrentState;

	--CurrentState;

	check(CurrentState >= &SavedStates[0]);

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

void FOpenGLRenderInterface::InitDefaultMaterialStageState(INT StageIndex){
	// Init default material state
	CurrentState->StageTexWrapModes[StageIndex * 2] = -1;
	CurrentState->StageTexWrapModes[StageIndex * 2 + 1] = -1;
	CurrentState->StageTexCoordSources[StageIndex] = 0;
	CurrentState->StageTexMatrices[StageIndex] = FMatrix::Identity;
	CurrentState->StageColorArgs[StageIndex * 2] = CA_Diffuse;
	CurrentState->StageColorArgs[StageIndex * 2 + 1] = CA_Diffuse;
	CurrentState->StageColorOps[StageIndex] = COP_Arg1;
	CurrentState->StageAlphaArgs[StageIndex * 2] = CA_Diffuse;
	CurrentState->StageAlphaArgs[StageIndex * 2 + 1] = CA_Diffuse;
	CurrentState->StageAlphaOps[StageIndex] = AOP_Arg1;
}

static GLint GetTextureWrapMode(INT Mode){
	if(Mode == TC_Clamp)
		return GL_CLAMP_TO_EDGE;

	return GL_REPEAT;
}

void FOpenGLRenderInterface::SetMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial, INT* NumPasses){
	guardFunc;

	RestoreLastState();

	// Use default material if precaching geometry

	if(!Material || PrecacheMode == PRECACHE_VertexBuffers)
		Material = GetDefault<UMaterial>()->DefaultMaterial;

	// Check for circular references

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

	// Restore default material state

	for(INT i = 0; i < CurrentState->NumStages; ++i)
		InitDefaultMaterialStageState(i);

	CurrentState->UsingConstantColor = false;
	CurrentState->NumStages = 0;
	CurrentState->NumTextures = 0;
	CurrentState->TexCoordCount = 2;
	CurrentState->ConstantColor = FPlane(1.0f, 1.0f, 1.0f, 1.0f);
	CurrentState->FramebufferBlending = FB_Overwrite;
	CurrentState->SrcBlend = GL_ONE;
	CurrentState->DstBlend = GL_ZERO;
	CurrentState->AlphaRef = -1.0f;

	// We check later if these properties have changed after setting the material and udate the OpenGL state
	bool ZTest = CurrentState->bZTest;
	bool ZWrite = CurrentState->bZWrite;
	ECullMode CullMode = CurrentState->CullMode;

	Material->PreSetMaterial(GEngineTime);

	bool Result = false;

	if(CheckMaterial<UShader>(&Material, 0, 0)){
		UShader* Shader = static_cast<UShader*>(Material);

		Result = SetSimpleMaterial(Shader->Diffuse, ErrorString, ErrorMaterial);

		if(Shader->Opacity)
			CurrentState->FramebufferBlending = FB_AlphaBlend;

		if(Shader->OutputBlending == OB_Masked)
			CurrentState->AlphaRef = 0.5f;

		if(Shader->TwoSided)
			CurrentState->CullMode = CM_None;
	}else if(CheckMaterial<UCombiner>(&Material, -1)){
		Result = SetSimpleMaterial(Material, ErrorString, ErrorMaterial);
	}else if(CheckMaterial<UConstantMaterial>(&Material, -1)){
		Result = SetSimpleMaterial(Material, ErrorString, ErrorMaterial);
	}else if(CheckMaterial<UBitmapMaterial>(&Material, -1)){
		Result = SetSimpleMaterial(Material, ErrorString, ErrorMaterial);
	}else if(CheckMaterial<UTerrainMaterial>(&Material, 0)){
		Result = SetTerrainMaterial(static_cast<UTerrainMaterial*>(Material), ErrorString, ErrorMaterial);
	}else if(CheckMaterial<UParticleMaterial>(&Material, 0)){
		CurrentState->AlphaRef = 0.0f;
		CurrentState->FramebufferBlending = FB_AlphaBlend;
		Result = SetSimpleMaterial(static_cast<UParticleMaterial*>(Material)->BitmapMaterial, ErrorString, ErrorMaterial);
		CurrentState->bZWrite = false;
	}else if(CheckMaterial<UProjectorMultiMaterial>(&Material, -1)){
		CurrentState->AlphaRef = 0.5f;
		CurrentState->FramebufferBlending = FB_Modulate;
		Result = SetSimpleMaterial(static_cast<UProjectorMultiMaterial*>(Material)->BaseMaterial, ErrorString, ErrorMaterial);
		CurrentState->bZWrite = false;
	}else if(CheckMaterial<UProjectorMaterial>(&Material, -1)){
		CurrentState->AlphaRef = 0.5f;
		CurrentState->FramebufferBlending = FB_Modulate;
		Result = SetSimpleMaterial(static_cast<UProjectorMaterial*>(Material)->Projected, ErrorString, ErrorMaterial);
		CurrentState->bZWrite = false;
	}else if(CheckMaterial<UHardwareShaderWrapper>(&Material, -1)){
		UHardwareShaderWrapper* HardwareShaderWrapper = static_cast<UHardwareShaderWrapper*>(Material);
		UHardwareShader* HardwareShader = HardwareShaderWrapper->ShaderImplementation;

		HardwareShaderWrapper->SetupShaderWrapper(this);
		Result = SetSimpleMaterial(HardwareShader->Textures[0], ErrorString, ErrorMaterial);

		if(HardwareShader->AlphaBlending)
			CurrentState->FramebufferBlending = FB_AlphaBlend;

		if(HardwareShader->AlphaTest)
			CurrentState->AlphaRef = HardwareShader->AlphaRef / 255.0f;
	}else if(CheckMaterial<UHardwareShader>(&Material, -1)){
		UHardwareShader* HardwareShader = static_cast<UHardwareShader*>(Material);

		Result = SetSimpleMaterial(HardwareShader->Textures[0], ErrorString, ErrorMaterial);

		if(HardwareShader->AlphaBlending)
			CurrentState->FramebufferBlending = FB_AlphaBlend;

		if(HardwareShader->AlphaTest)
			CurrentState->AlphaRef = HardwareShader->AlphaRef / 255.0f;
	}

	if(!Result || CurrentState->NumStages <= 0){ // Reset to default state in error case
		InitDefaultMaterialStageState(0);
		CurrentState->NumStages = 1;
	}

	glUniform1i(SU_NumStages, CurrentState->NumStages);
	glUniform1i(SU_TexCoordCount, CurrentState->TexCoordCount);
	glUniform1iv(SU_StageTexCoordSources, ARRAY_COUNT(CurrentState->StageTexCoordSources), CurrentState->StageTexCoordSources);
	glUniformMatrix4fv(SU_StageTexMatrices, ARRAY_COUNT(CurrentState->StageTexMatrices), GL_FALSE, (GLfloat*)CurrentState->StageTexMatrices);
	glUniform1iv(SU_StageColorArgs, ARRAY_COUNT(CurrentState->StageColorArgs), CurrentState->StageColorArgs);
	glUniform1iv(SU_StageColorOps, ARRAY_COUNT(CurrentState->StageColorOps), CurrentState->StageColorOps);
	glUniform1iv(SU_StageAlphaArgs, ARRAY_COUNT(CurrentState->StageAlphaArgs), CurrentState->StageAlphaArgs);
	glUniform1iv(SU_StageAlphaOps, ARRAY_COUNT(CurrentState->StageAlphaOps), CurrentState->StageAlphaOps);
	glUniform4fv(SU_ConstantColor, 1, (GLfloat*)&CurrentState->ConstantColor);
	glUniform1f(SU_AlphaRef, CurrentState->AlphaRef);

	for(INT i = 0; i < CurrentState->NumTextures; ++i){
		glSamplerParameteri(Samplers[i], GL_TEXTURE_WRAP_S, GetTextureWrapMode(CurrentState->StageTexWrapModes[i * 2]));
		glSamplerParameteri(Samplers[i], GL_TEXTURE_WRAP_T, GetTextureWrapMode(CurrentState->StageTexWrapModes[i * 2 + 1]));
	}

	if(ZTest != CurrentState->bZTest)
		EnableZTest(CurrentState->bZTest);

	if(ZWrite != CurrentState->bZWrite)
		EnableZWrite(CurrentState->bZWrite);

	if(CullMode != CurrentState->CullMode)
		SetCullMode(CurrentState->CullMode);

	switch(CurrentState->FramebufferBlending){
	case FB_Overwrite:
		glBlendFunc(GL_ONE, GL_ZERO);
		break;
	case FB_Modulate:
		glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
		break;
	case FB_AlphaBlend:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case FB_AlphaModulate_MightNotFogCorrectly:
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case FB_Translucent:
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
		break;
	case FB_Darken:
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		break;
	case FB_Brighten:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case FB_Invisible:
		glBlendFunc(GL_ZERO, GL_ONE);
		break;
	case FB_ShadowBlend:
		glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
		break;
	case FB_MAX:
		glBlendFunc(CurrentState->SrcBlend, CurrentState->DstBlend);
	}

	unguard;
}

void FOpenGLRenderInterface::SetBitmapTexture(UBitmapMaterial* Bitmap, INT TextureUnit){
	FBaseTexture* Texture = Bitmap->Get(LockedViewport->CurrentTime, LockedViewport)->GetRenderInterface();

	if(!Texture)
		return;

	QWORD CacheId = Texture->GetCacheId();
	FOpenGLTexture* GLTexture = static_cast<FOpenGLTexture*>(RenDev->GetCachedResource(CacheId));

	if(!GLTexture)
		GLTexture = new FOpenGLTexture(RenDev, CacheId);

	if(GLTexture->Revision != Texture->GetRevision())
		GLTexture->Cache(Texture);

	GLTexture->BindTexture(TextureUnit);

	if(CurrentState->StageTexWrapModes[TextureUnit * 2] < 0)
		CurrentState->StageTexWrapModes[TextureUnit * 2] = Bitmap->UClampMode;

	if(CurrentState->StageTexWrapModes[TextureUnit * 2 + 1] < 0)
		CurrentState->StageTexWrapModes[TextureUnit * 2 + 1] = Bitmap->VClampMode;
}

bool FOpenGLRenderInterface::SetSimpleMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial){
	if(!Material)
		return true;

	INT StagesUsed = 0;
	INT TexturesUsed = 0;

	// If the material is a simple texture, use it to get the blending options
	if(Material->IsA<UTexture>()){
		UTexture* Texture = static_cast<UTexture*>(Material);

		if(Texture->bMasked){
			CurrentState->AlphaRef = 0.5f;
			CurrentState->FramebufferBlending = FB_AlphaBlend;
		}else if(Texture->bAlphaTexture){
			CurrentState->AlphaRef = 0.0f;
			CurrentState->FramebufferBlending = FB_AlphaBlend;
		}

		if(Texture->bTwoSided)
			CurrentState->CullMode = CM_None;
	}

	if(!HandleCombinedMaterial(Material, StagesUsed, TexturesUsed, ErrorString, ErrorMaterial))
		return false;

	CurrentState->NumStages = StagesUsed;
	CurrentState->NumTextures = TexturesUsed;

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
	}else if(CheckMaterial<UBitmapMaterial>(&Material, StagesUsed, TexturesUsed)){
		if(StagesUsed >= MAX_SHADER_STAGES || TexturesUsed >= MAX_TEXTURES){
			if(ErrorString)
				*ErrorString = "No stages left for bitmap material";

			if(ErrorMaterial)
				*ErrorMaterial = Material;

			return false;
		}

		SetBitmapTexture(static_cast<UBitmapMaterial*>(Material), TexturesUsed);

		CurrentState->StageColorArgs[StagesUsed * 2] = CA_Texture0 + TexturesUsed;
		CurrentState->StageColorOps[StagesUsed] = COP_Arg1;
		CurrentState->StageAlphaArgs[StagesUsed * 2] = CA_Texture0 + TexturesUsed;
		CurrentState->StageAlphaOps[StagesUsed] = AOP_Arg1;

		++StagesUsed;
		++TexturesUsed;

		return true;
	}else if(CheckMaterial<UCombiner>(&Material, StagesUsed)){
		UCombiner* Combiner = static_cast<UCombiner*>(Material);
		UMaterial* Material1 = Combiner->Material1;
		UMaterial* Material2 = Combiner->Material2;
		bool BitmapMaterial1 = CheckMaterial<UBitmapMaterial>(&Material1, -1);
		bool BitmapMaterial2 = CheckMaterial<UBitmapMaterial>(&Material2, -1);

		if(!Material1){
			if(ErrorString)
				*ErrorString = "Combiner must specify Material1";

			if(ErrorMaterial)
				*ErrorMaterial = Material;

			return false;
		}else if(!BitmapMaterial1 && !BitmapMaterial2){
			if(ErrorString)
				*ErrorString = "Either Material1 or Material2 must be a simple bitmap material";

			if(ErrorMaterial)
				*ErrorMaterial = Material;

			return false;
		}

		// Swap materials if it makes things easier

		bool Swapped;

		if(!BitmapMaterial2 || (BitmapMaterial1 && Combiner->Mask == Combiner->Material2)){
			Material1 = Combiner->Material2;
			Material2 = Combiner->Material1;
			Swapped = true;
		}else{
			Swapped = false;
		}

		// Set Material1

		if(!HandleCombinedMaterial(Material1, StagesUsed, TexturesUsed, ErrorString, ErrorMaterial))
			return false;

		// Set Mask

		UMaterial* Mask = Combiner->Mask;

		if(Mask){
			if(Mask != Material1 && Mask != Material2 && !Mask->IsA<UBitmapMaterial>() && !Mask->IsA<UVertexColor>() && !Mask->IsA<UConstantMaterial>()){
				if(ErrorString)
					*ErrorString = "Combiner Mask must be a bitmap material, vertex color, constant color, Material1 or Material2";

				if(ErrorMaterial)
					*ErrorMaterial = Material;

				return false;
			}

			if(Mask != Material1 && Mask != Material2){
				// If the mask is a simple bitmap material we don't need to waste an entire stage for it, just the texture unit.
				bool SimpleBitmapMask = Mask->IsA<UBitmapMaterial>() != 0;

				if(SimpleBitmapMask || CheckMaterial<UBitmapMaterial>(&Mask, StagesUsed, TexturesUsed)){
					if(TexturesUsed >= MAX_TEXTURES){
						if(ErrorString)
							*ErrorString = "No texture units left for combiner Mask";

						if(ErrorMaterial)
							*ErrorMaterial = Material;

						return false;
					}

					if(SimpleBitmapMask){
						SetBitmapTexture(static_cast<UBitmapMaterial*>(Mask), TexturesUsed);

						CurrentState->StageAlphaArgs[StagesUsed * 2] = CA_Texture0 + TexturesUsed;
						CurrentState->StageAlphaArgs[StagesUsed] = AOP_Arg1;
						++TexturesUsed;
					}else{
						if(!HandleCombinedMaterial(Mask, StagesUsed, TexturesUsed, ErrorString, ErrorMaterial))
							return false;

						CurrentState->StageColorArgs[StagesUsed * 2 - 2] = CA_Previous;
						CurrentState->StageColorOps[StagesUsed - 1] = COP_Arg1;
						CurrentState->StageAlphaArgs[StagesUsed * 2 - 2] = CA_Texture0 + TexturesUsed - 1;
						CurrentState->StageAlphaArgs[StagesUsed - 1] = AOP_Arg1;
					}
				}else if(CheckMaterial<UVertexColor>(&Mask, StagesUsed)){
					CurrentState->StageAlphaArgs[StagesUsed * 2 - 2] = CA_Diffuse;
					CurrentState->StageAlphaOps[StagesUsed - 1] = AOP_Arg1;
				}else if(CheckMaterial<UConstantMaterial>(&Mask, StagesUsed)){
					if(CurrentState->UsingConstantColor){
						if(ErrorString)
							*ErrorString = "Only one ConstantMaterial may be used per material";

						if(ErrorMaterial)
							*ErrorMaterial = Material;

						return false;
					}

					CurrentState->ConstantColor = static_cast<UConstantMaterial*>(Mask)->GetColor(GEngineTime);
					CurrentState->StageAlphaArgs[StagesUsed * 2 - 2] = CA_Constant;
					CurrentState->StageAlphaOps[StagesUsed - 1] = AOP_Arg1;
				}else{
					if(ErrorString)
						*ErrorString = "Combiner Mask must be a bitmap material, vertex color, constant color, Material1 or Material2";

					if(ErrorMaterial)
						*ErrorMaterial = Material;

					return false;
				}
			}
		}

		// Set Material2

		if(Material2){
			if(!HandleCombinedMaterial(Material2, StagesUsed, TexturesUsed, ErrorString, ErrorMaterial))
				return false;

			if(Swapped){
				CurrentState->StageColorArgs[StagesUsed * 2 - 2] = CA_Texture0 + TexturesUsed - 1;
				CurrentState->StageColorArgs[StagesUsed * 2 - 1] = CA_Previous;
			}else{
				CurrentState->StageColorArgs[StagesUsed * 2 - 2] = CA_Previous;
				CurrentState->StageColorArgs[StagesUsed * 2 - 1] = CA_Texture0 + TexturesUsed - 1;
			}

			if(Mask == Material2){
				CurrentState->StageAlphaArgs[StagesUsed * 2 - 2] = CA_Texture0 + TexturesUsed - 1;
				CurrentState->StageAlphaArgs[StagesUsed * 2 - 1] = CA_Previous;
			}else{
				CurrentState->StageAlphaArgs[StagesUsed * 2 - 2] = CA_Previous;
				CurrentState->StageAlphaArgs[StagesUsed * 2 - 1] = CA_Texture0 + TexturesUsed - 1;
			}
		}

		// Apply color operations

		INT StageIndex = StagesUsed - 1;

		switch(Combiner->CombineOperation){
		case CO_Use_Color_From_Material1:
			CurrentState->StageColorOps[StageIndex] = COP_Arg1;
			break;
		case CO_Use_Color_From_Material2:
			CurrentState->StageColorOps[StageIndex] = COP_Arg2;
			break;
		case CO_Multiply:
			CurrentState->StageColorOps[StageIndex] = COP_Modulate;
			break;
		case CO_Add:
			CurrentState->StageColorOps[StageIndex] = COP_Add;
			break;
		case CO_Subtract:
			CurrentState->StageColorOps[StageIndex] = COP_Subtract;
			break;
		case CO_AlphaBlend_With_Mask:
			CurrentState->StageColorOps[StageIndex] = COP_AlphaBlend;
			break;
		case CO_Add_With_Mask_Modulation:
			CurrentState->StageColorOps[StageIndex] = COP_AddAlphaModulate;
			break;
		case CO_Use_Color_From_Mask:
			CurrentState->StageColorOps[StageIndex] = COP_Arg1; // TODO: This is probably not correct
		}

		// Apply alpha operations

		switch(Combiner->AlphaOperation){
		case AO_Use_Mask:
			break;
		case AO_Multiply:
			if(Mask && Mask != Material1 && Mask != Material2){
				if(ErrorString)
					*ErrorString = "Combiner Mask must be Material1, Material2 or None for AO_Multiply";

				if(ErrorMaterial)
					*ErrorMaterial = Material;

				return false;
			}

			CurrentState->StageAlphaOps[StageIndex] = AOP_Modulate;
			break;
		case AO_Add:
			if(Mask && Mask != Material1 && Mask != Material2){
				if(ErrorString)
					*ErrorString = "Combiner Mask must be Material1, Material2 or None for AO_Add";

				if(ErrorMaterial)
					*ErrorMaterial = Material;

				return false;
			}

			CurrentState->StageAlphaOps[StageIndex] = AOP_Add;
			break;
		case AO_Use_Alpha_From_Material1:
			if(Mask && Mask != Material1 && Mask != Material2){
				if(ErrorString)
					*ErrorString = "Combiner Mask must be Material1, Material2 or None for AO_Use_Alpha_From_Material1";

				if(ErrorMaterial)
					*ErrorMaterial = Material;

				return false;
			}

			CurrentState->StageAlphaOps[StageIndex] = AOP_Arg1;
			break;
		case AO_Use_Alpha_From_Material2:
			if(Mask && Mask != Material1 && Mask != Material2){
				if(ErrorString)
					*ErrorString = "Combiner Mask must be Material1, Material2 or None for AO_Use_Alpha_From_Material2";

				if(ErrorMaterial)
					*ErrorMaterial = Material;

				return false;
			}

			CurrentState->StageAlphaOps[StageIndex] = AOP_Arg2;
			break;
		case AO_AlphaBlend_With_Mask:
			CurrentState->StageAlphaOps[StageIndex] = AOP_Blend;
		}

		return true;
	}

	return true;
}

bool FOpenGLRenderInterface::SetTerrainMaterial(UTerrainMaterial* Terrain, FString* ErrorString, UMaterial** ErrorMaterial){
	checkSlow(Terrain->Layers.Num() == 1);

	INT StagesUsed = 0;
	INT TexturesUsed = 0;

	UMaterial* Material = Terrain->Layers[0].Texture;

	if(Material->IsA<UShader>())
		Material = static_cast<UShader*>(Material)->Diffuse;

	if(Terrain->RenderMethod == RM_AlphaMap){
		CurrentState->StageTexMatrices[StagesUsed] = Terrain->Layers[0].TextureMatrix;
		CurrentState->StageTexCoordSources[StagesUsed] = TCS_WorldCoords;
		CurrentState->TexCoordCount = 3;

		if(CheckMaterial<UBitmapMaterial>(&Material, StagesUsed, TexturesUsed)){
			if(Terrain->FirstPass){
				CurrentState->FramebufferBlending = FB_Overwrite;
			}else{
				CurrentState->FramebufferBlending = FB_AlphaBlend;
				CurrentState->AlphaRef = 0.0f;
			}

			// Color texture

			UBitmapMaterial* BitmapMaterial = static_cast<UBitmapMaterial*>(Material);

			CurrentState->StageTexWrapModes[TexturesUsed * 2] = TC_Wrap;
			CurrentState->StageTexWrapModes[TexturesUsed * 2 + 1] = TC_Wrap;

			SetBitmapTexture(BitmapMaterial, TexturesUsed);

			CurrentState->StageColorArgs[StagesUsed * 2] = CA_Texture0 + TexturesUsed;
			CurrentState->StageColorArgs[StagesUsed * 2 + 1] = CA_Diffuse;
			CurrentState->StageColorOps[StagesUsed] = COP_Arg1; //COP_Modulate; TODO: Lighting
			CurrentState->StageAlphaArgs[StagesUsed * 2] = CA_Texture0 + TexturesUsed;
			CurrentState->StageAlphaOps[StagesUsed] = AOP_Arg1;

			++StagesUsed;
			++TexturesUsed;

			// Alpha map

			SetBitmapTexture(Terrain->Layers[0].AlphaWeight, TexturesUsed);

			CurrentState->StageColorArgs[StagesUsed * 2] = CA_Previous;
			CurrentState->StageColorOps[StagesUsed] = COP_Arg1;
			CurrentState->StageAlphaArgs[StagesUsed * 2] = CA_Texture0 + TexturesUsed;
			CurrentState->StageAlphaArgs[StagesUsed * 2 + 1] = CA_Previous;
			CurrentState->StageAlphaOps[StagesUsed] = BitmapMaterial->IsTransparent() ? AOP_Modulate : AOP_Arg1;
			CurrentState->StageTexCoordSources[StagesUsed] = TCS_Stream0;

			++StagesUsed;
			++TexturesUsed;
		}else{
			return false;
		}
	}else{
		if(Terrain->FirstPass){
			CurrentState->FramebufferBlending = FB_Overwrite;
		}else{
			CurrentState->FramebufferBlending = FB_AlphaBlend;
			CurrentState->bZWrite = false;
		}

		// Weight map

		SetBitmapTexture(Terrain->Layers[0].AlphaWeight, TexturesUsed);

		CurrentState->StageTexCoordSources[StagesUsed] = TCS_Stream0;
		CurrentState->StageAlphaArgs[StagesUsed * 2] = CA_Texture0 + TexturesUsed;
		CurrentState->StageAlphaOps[StagesUsed] = AOP_Arg1;

		++StagesUsed;
		++TexturesUsed;

		// Color texture

		CurrentState->StageTexMatrices[StagesUsed] = Terrain->Layers[0].TextureMatrix;
		CurrentState->StageTexCoordSources[StagesUsed] = TCS_WorldCoords;

		if(!CheckMaterial<UBitmapMaterial>(&Material, StagesUsed, TexturesUsed))
			return false;

		SetBitmapTexture(static_cast<UBitmapMaterial*>(Material), TexturesUsed);

		CurrentState->StageColorArgs[StagesUsed * 2] = CA_Texture0 + TexturesUsed;
		CurrentState->StageColorArgs[StagesUsed * 2 + 1] = CA_Diffuse;
		CurrentState->StageColorOps[StagesUsed] = COP_Arg1; //COP_Modulate; TODO: Lighting
		CurrentState->StageAlphaArgs[StagesUsed * 2] = CA_Previous;
		CurrentState->StageAlphaOps[StagesUsed] = AOP_Arg1;

		++StagesUsed;
		++TexturesUsed;
	}

	CurrentState->NumStages = StagesUsed;
	CurrentState->NumTextures = TexturesUsed;

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
}

void FOpenGLRenderInterface::SetupPerFrameShaderConstants(){
	FLOAT Time = appFmod(static_cast<FLOAT>(appSeconds()), 120.0f);

	CurrentState->Uniforms.Time = Time;
	CurrentState->Uniforms.SinTime = appSin(Time);
	CurrentState->Uniforms.CosTime = appCos(Time);
	CurrentState->Uniforms.TanTime = appTan(Time);
}
