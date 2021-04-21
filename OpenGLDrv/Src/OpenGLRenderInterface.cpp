#include "../Inc/OpenGLRenderInterface.h"

#include "../Inc/OpenGLRenderDevice.h"
#include "OpenGLResource.h"
#include "GL/glew.h"

static GLenum GetStencilFunc(/*ECompareFunction*/BYTE Test){
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

static GLenum GetStencilOp(/*EStencilOp*/BYTE StencilOp){
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

static GLint GetTextureWrapMode(/*ETexClampMode*/BYTE Mode){
	if(Mode == TC_Clamp)
		return GL_CLAMP_TO_EDGE;

	return GL_REPEAT;
}

/*
 * FOpenGLRenderInterface
 */

FOpenGLRenderInterface::FOpenGLRenderInterface(UOpenGLRenderDevice* InRenDev) : RenDev(InRenDev),
                                                                                PrecacheMode(PRECACHE_All),
																				CurrentState(&SavedStates[0]){}

void FOpenGLRenderInterface::Init(INT ViewportWidth, INT ViewportHeight){
	// Setup initial state

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);

	RenderState.ViewportX = 0;
	RenderState.ViewportY = 0;
	RenderState.ViewportWidth = ViewportWidth;
	RenderState.ViewportHeight = ViewportHeight;
	glViewport(0, 0, ViewportWidth, ViewportHeight);

	RenderState.CullMode = CM_CW;
	glCullFace(GL_BACK);

	RenderState.FillMode = FM_Solid;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	RenderState.bZTest = true;
	glDepthFunc(GL_LEQUAL);

	RenderState.bZWrite = true;
	glDepthMask(GL_TRUE);

	bStencilEnabled = false;

	RenderState.bStencilTest = false;
	RenderState.StencilCompare = CF_Always;
	RenderState.StencilRef = 0xF;
	RenderState.StencilMask = 0xFF;
	RenderState.StencilFailOp = SO_Keep;
	RenderState.StencilZFailOp = SO_Keep;
	RenderState.StencilPassOp = SO_Keep;
	RenderState.StencilWriteMask = 0xFF;

	glStencilOp(GetStencilOp(RenderState.StencilFailOp), GetStencilOp(RenderState.StencilZFailOp), GetStencilOp(RenderState.StencilPassOp));
	glStencilFunc(GetStencilFunc(RenderState.StencilCompare), RenderState.StencilRef, RenderState.StencilMask);
	glStencilMask(RenderState.StencilWriteMask);

	glCreateSamplers(MAX_TEXTURES, Samplers);
	glBindSamplers(0, MAX_TEXTURES, Samplers);            // 2D texture samplers
	glBindSamplers(MAX_TEXTURES, MAX_TEXTURES, Samplers); // Cubemap samplers

	SetTextureFilter(RenDev->TextureFilter);

	for(int i = 0; i < MAX_TEXTURES; ++i){
		glSamplerParameteri(Samplers[i], GL_TEXTURE_WRAP_S, GL_REPEAT);
		glSamplerParameteri(Samplers[i], GL_TEXTURE_WRAP_T, GL_REPEAT);
		glSamplerParameteri(Samplers[i], GL_TEXTURE_WRAP_R, GL_REPEAT);
		RenderState.TextureUnits[i].ClampU = TC_Wrap;
		RenderState.TextureUnits[i].ClampV = TC_Wrap;
	}

	*static_cast<FOpenGLRenderState*>(CurrentState) = RenderState;

	// Init uniform default values

	CurrentState->LocalToWorld = FMatrix::Identity;
	CurrentState->WorldToCamera = FMatrix::Identity;
	CurrentState->CameraToScreen = FMatrix::Identity;
	CurrentState->GlobalColor = FPlane(1.0, 1.0, 1.0, 1.0);

	for(INT i = 0; i < MAX_SHADER_STAGES; ++i)
		InitDefaultMaterialStageState(i);

	UsingConstantColor = false;
	NumStages = 0;
	ConstantColor = FPlane(1.0f, 1.0f, 1.0f, 1.0f);
	CurrentState->AlphaRef = -1.0f;

	// Create uniform buffer
	glCreateBuffers(1, &GlobalUBO);
	glNamedBufferStorage(GlobalUBO, sizeof(FOpenGLGlobalUniforms), static_cast<FOpenGLGlobalUniforms*>(CurrentState), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, GlobalUBO); // Binding index 0 is reserved for the global uniform block

	VAOsByDeclId.Empty();
}

void FOpenGLRenderInterface::Exit(){
	glDeleteBuffers(1, &GlobalUBO);
	GlobalUBO = GL_NONE;
	glDeleteSamplers(MAX_TEXTURES, Samplers);
	appMemzero(Samplers, sizeof(Samplers));
}

void FOpenGLRenderInterface::CommitRenderState(){
	if(RenderState.CullMode != CurrentState->CullMode){
		if(CurrentState->CullMode != CM_None){
			GLenum NewCullMode;

			if(CurrentState->CullMode == CM_CCW)
				NewCullMode = GL_FRONT;
			else
				NewCullMode = GL_BACK;

			glEnable(GL_CULL_FACE);
			glCullFace(NewCullMode);
		}else{
			glDisable(GL_CULL_FACE);
		}

		RenderState.CullMode = CurrentState->CullMode;
	}

	if(RenderState.FillMode != CurrentState->FillMode){
		glPolygonMode(GL_FRONT_AND_BACK, CurrentState->FillMode == FM_Wireframe ? GL_LINE : GL_FILL);
		RenderState.FillMode = CurrentState->FillMode;
	}

	if(RenderState.bZTest != CurrentState->bZTest){
		glDepthFunc(CurrentState->bZTest ? GL_LEQUAL : GL_ALWAYS);
		RenderState.bZTest = CurrentState->bZTest;
	}

	if(RenderState.bZWrite != CurrentState->bZWrite){
		glDepthMask(CurrentState->bZWrite ? GL_TRUE : GL_FALSE);
		RenderState.bZWrite = CurrentState->bZWrite;
	}

	if(bStencilEnabled){
		if(RenderState.bStencilTest != CurrentState->bStencilTest){
			CurrentState->StencilCompare = CF_Always;
			CurrentState->StencilFailOp = SO_Keep;
			CurrentState->StencilZFailOp = SO_Keep;
			CurrentState->StencilPassOp = SO_Keep;

			if(CurrentState->bStencilTest)
				CurrentState->StencilWriteMask = 0xFF;
			else
				CurrentState->StencilWriteMask = 0x00;

			RenderState.bStencilTest = CurrentState->bStencilTest;
		}

		if(RenderState.StencilFailOp != CurrentState->StencilFailOp ||
		   RenderState.StencilZFailOp != CurrentState->StencilZFailOp ||
		   RenderState.StencilPassOp != CurrentState->StencilPassOp){
			glStencilOp(GetStencilOp(CurrentState->StencilFailOp), GetStencilOp(CurrentState->StencilZFailOp), GetStencilOp(CurrentState->StencilPassOp));
			RenderState.StencilFailOp = CurrentState->StencilFailOp;
			RenderState.StencilZFailOp = CurrentState->StencilZFailOp;
			RenderState.StencilPassOp = CurrentState->StencilPassOp;
		}

		if(RenderState.StencilCompare != CurrentState->StencilCompare ||
		   RenderState.StencilRef != CurrentState->StencilRef ||
		   RenderState.StencilMask != CurrentState->StencilMask){
			glStencilFunc(GetStencilFunc(CurrentState->StencilCompare), CurrentState->StencilRef, CurrentState->StencilMask);
			RenderState.StencilCompare = CurrentState->StencilCompare;
			RenderState.StencilRef = CurrentState->StencilRef;
			RenderState.StencilMask = CurrentState->StencilMask;
		}

		if(RenderState.StencilWriteMask != CurrentState->StencilWriteMask){
			glStencilMask(CurrentState->StencilWriteMask);
			RenderState.StencilWriteMask = CurrentState->StencilWriteMask;
		}
	}

	if(RenderState.ZBias != CurrentState->ZBias){
		glPolygonOffset(-CurrentState->ZBias, -CurrentState->ZBias);
		RenderState.ZBias = CurrentState->ZBias;
	}

	if(RenderState.ViewportX != CurrentState->ViewportX ||
	   RenderState.ViewportY != CurrentState->ViewportY ||
	   RenderState.ViewportWidth != CurrentState->ViewportWidth ||
	   RenderState.ViewportHeight != CurrentState->ViewportHeight){
		glViewport(CurrentState->ViewportX, CurrentState->ViewportY, CurrentState->ViewportWidth, CurrentState->ViewportHeight);

		RenderState.ViewportX = CurrentState->ViewportX;
		RenderState.ViewportY = CurrentState->ViewportY;
		RenderState.ViewportWidth = CurrentState->ViewportWidth;
		RenderState.ViewportHeight = CurrentState->ViewportHeight;
	}

	if(RenderState.SrcBlend != CurrentState->SrcBlend || RenderState.DstBlend != CurrentState->DstBlend){
		glBlendFunc(CurrentState->SrcBlend, CurrentState->DstBlend);
		RenderState.SrcBlend = CurrentState->SrcBlend;
		RenderState.DstBlend = CurrentState->DstBlend;
	}

	if(RenderState.IndexBuffer != CurrentState->IndexBuffer){
		if(CurrentState->IndexBuffer)
			CurrentState->IndexBuffer->Bind();

		RenderState.IndexBuffer = CurrentState->IndexBuffer;
	}

	for(INT i = 0; i < CurrentState->NumVertexStreams; ++i){
		if(RenderState.VertexStreams[i] != CurrentState->VertexStreams[i]){
			CurrentState->VertexStreams[i]->Bind(i);
			RenderState.VertexStreams[i] = CurrentState->VertexStreams[i];
		}
	}

	for(INT i = 0; i < CurrentState->NumTextures; ++i){
		if(RenderState.TextureUnits[i].Texture != CurrentState->TextureUnits[i].Texture){
			if(CurrentState->TextureUnits[i].Texture) // Texture might not be set if the current material is a hardware shader
				CurrentState->TextureUnits[i].Texture->BindTexture(i + CurrentState->TextureInfo[i].IsCubemap * MAX_TEXTURES);

			RenderState.TextureUnits[i].Texture = CurrentState->TextureUnits[i].Texture;
		}

		if(RenderState.TextureUnits[i].ClampU != CurrentState->TextureUnits[i].ClampU){
			glSamplerParameteri(Samplers[i], GL_TEXTURE_WRAP_S, GetTextureWrapMode(CurrentState->TextureUnits[i].ClampU));
			RenderState.TextureUnits[i].ClampU = CurrentState->TextureUnits[i].ClampU;
		}

		if(RenderState.TextureUnits[i].ClampV != CurrentState->TextureUnits[i].ClampV){
			glSamplerParameteri(Samplers[i], GL_TEXTURE_WRAP_T, GetTextureWrapMode(CurrentState->TextureUnits[i].ClampV));
			RenderState.TextureUnits[i].ClampV = CurrentState->TextureUnits[i].ClampV;
		}
	}
}

void FOpenGLRenderInterface::Locked(UViewport* Viewport){
	checkSlow(CurrentState == &SavedStates[0]);
	checkSlow(!LockedViewport);

	LockedViewport = Viewport;

	// Detect settings changes in UOpenGLRenderDevice and act accordingly

	if(TextureFilter != RenDev->TextureFilter)
		SetTextureFilter(RenDev->TextureFilter);

	if(GLEW_EXT_texture_filter_anisotropic && TextureAnisotropy != RenDev->TextureAnisotropy){
		TextureAnisotropy = RenDev->TextureAnisotropy;

		for(INT i = 0; i < MAX_TEXTURES; ++i)
			glSamplerParameterf(Samplers[i], GL_TEXTURE_MAX_ANISOTROPY, Clamp<FLOAT>(TextureAnisotropy, 1, 16));
	}

	if(bStencilEnabled != !!RenDev->UseStencil){
		if(RenDev->UseStencil){
			glEnable(GL_STENCIL_TEST);
			bStencilEnabled = true;
		}else{
			glDisable(GL_STENCIL_TEST);
			bStencilEnabled = false;
		}
	}

	// Setup per-frame shader constants

	FLOAT Time = appFmod(GEngineTime, 120.0f);

	CurrentState->Time = Time;
	CurrentState->SinTime = appSin(Time);
	CurrentState->CosTime = appCos(Time);
	CurrentState->TanTime = appTan(Time);
}

void FOpenGLRenderInterface::Unlocked(){
	checkSlow(CurrentState == &SavedStates[0]);
	LockedViewport = NULL;
}

void FOpenGLRenderInterface::UpdateGlobalShaderUniforms(){
	// Matrices
	CurrentState->LocalToCamera = CurrentState->LocalToWorld * CurrentState->WorldToCamera;
	CurrentState->LocalToScreen = CurrentState->LocalToCamera * CurrentState->CameraToScreen;
	CurrentState->WorldToLocal = CurrentState->LocalToWorld.Inverse();
	CurrentState->WorldToScreen = CurrentState->WorldToCamera * CurrentState->CameraToScreen;
	CurrentState->CameraToWorld = CurrentState->WorldToCamera.Inverse();

	glNamedBufferSubData(GlobalUBO, 0, sizeof(FOpenGLGlobalUniforms), static_cast<FOpenGLGlobalUniforms*>(CurrentState));

	NeedUniformUpdate = 0;
}

void FOpenGLRenderInterface::SetFramebufferBlending(EFrameBufferBlending Mode){
	switch(Mode){
	case FB_Overwrite:
		CurrentState->SrcBlend = GL_ONE;
		CurrentState->DstBlend = GL_ZERO;
		break;
	case FB_Modulate:
		CurrentState->SrcBlend = GL_DST_COLOR;
		CurrentState->DstBlend = GL_SRC_COLOR;
		break;
	case FB_AlphaBlend:
		CurrentState->SrcBlend = GL_SRC_ALPHA;
		CurrentState->DstBlend = GL_ONE_MINUS_SRC_ALPHA;
		break;
	case FB_AlphaModulate_MightNotFogCorrectly:
		CurrentState->SrcBlend = GL_ONE;
		CurrentState->DstBlend = GL_ONE_MINUS_SRC_ALPHA;
		break;
	case FB_Translucent:
		CurrentState->SrcBlend = GL_ONE;
		CurrentState->DstBlend = GL_ONE;
		break;
	case FB_Darken:
		CurrentState->SrcBlend = GL_ZERO;
		CurrentState->DstBlend = GL_ONE_MINUS_SRC_COLOR;
		break;
	case FB_Brighten:
		CurrentState->SrcBlend = GL_ONE;
		CurrentState->DstBlend = GL_ONE_MINUS_SRC_COLOR;
		break;
	case FB_Invisible:
		CurrentState->SrcBlend = GL_ZERO;
		CurrentState->DstBlend = GL_ONE;
		break;
	case FB_ShadowBlend:
		CurrentState->SrcBlend = GL_SRC_ALPHA;
		CurrentState->DstBlend = GL_ONE;
		break;
	default:
		CurrentState->SrcBlend = GL_ONE;
		CurrentState->DstBlend = GL_ZERO;
	}

}

void FOpenGLRenderInterface::SetTextureFilter(BYTE Filter){
	GLint MinFilter;
	GLint MagFilter;

	switch(Filter){
	case TF_Nearest:
		MinFilter = GL_NEAREST_MIPMAP_NEAREST;
		MagFilter = GL_NEAREST;
		break;
	case TF_Bilinear:
		MinFilter = GL_LINEAR_MIPMAP_NEAREST;
		MagFilter = GL_LINEAR;
		break;
	case TF_Trilinear:
		MinFilter = GL_LINEAR_MIPMAP_LINEAR;
		MagFilter = GL_LINEAR;
		break;
	default:
		return;
	}

	for(INT i = 0; i < MAX_TEXTURES; ++i){
		glSamplerParameteri(Samplers[i], GL_TEXTURE_MIN_FILTER, MinFilter);
		glSamplerParameteri(Samplers[i], GL_TEXTURE_MAG_FILTER, MagFilter);
	}

	TextureFilter = Filter;
}

void FOpenGLRenderInterface::SetShader(FShaderGLSL* NewShader){
	QWORD CacheId = NewShader->GetCacheId();
	FOpenGLShader* Shader = static_cast<FOpenGLShader*>(RenDev->GetCachedResource(CacheId));

	if(!Shader)
		Shader = new FOpenGLShader(RenDev, CacheId);

	if(Shader->Revision != NewShader->GetRevision()){
		glUseProgram(GL_NONE);
		Shader->Cache(NewShader);
		Shader->Bind();
	}else if(Shader != CurrentShader){
		Shader->Bind();
	}

	CurrentShader = Shader;
}

unsigned int FOpenGLRenderInterface::GetVAO(const FStreamDeclaration* Declarations, INT NumStreams){
	// Check if there is an existing VAO for this format by hashing the shader declarations
	GLuint& VAO = VAOsByDeclId[appMemCrc(Declarations, sizeof(FStreamDeclaration) * NumStreams)];

	// Create and setup VAO if none was found matching the vertex format
	if(!VAO){
		glCreateVertexArrays(1, &VAO);

		for(INT StreamIndex = 0; StreamIndex < NumStreams; ++StreamIndex){
			const FStreamDeclaration& Decl = Declarations[StreamIndex];
			GLuint Offset = 0;

			for(INT i = 0; i < Decl.NumComponents; ++i){
				BYTE Function = Decl.Components[i].Function; // EFixedVertexFunction
				BYTE Type     = Decl.Components[i].Type;     // EComponentType

				checkSlow(Function < FVF_MAX);
				checkSlow(Type < CT_MAX);

				switch(Type){
				case CT_Float4:
					glVertexArrayAttribFormat(VAO, Function, 4, GL_FLOAT, GL_FALSE, Offset);
					Offset += sizeof(FLOAT) * 4;
					break;
				case CT_Float3:
					glVertexArrayAttribFormat(VAO, Function, 3, GL_FLOAT, GL_FALSE, Offset);
					Offset += sizeof(FLOAT) * 3;
					break;
				case CT_Float2:
					glVertexArrayAttribFormat(VAO, Function, 2, GL_FLOAT, GL_FALSE, Offset);
					Offset += sizeof(FLOAT) * 2;
					break;
				case CT_Float1:
					glVertexArrayAttribFormat(VAO, Function, 1, GL_FLOAT, GL_FALSE, Offset);
					Offset += sizeof(FLOAT);
					break;
				case CT_Color:
					glVertexArrayAttribFormat(VAO, Function, GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE, Offset);
					Offset += sizeof(FColor);
				}

				glEnableVertexArrayAttrib(VAO, Function);
				glVertexArrayAttribBinding(VAO, Function, StreamIndex);
			}
		}
	}

	return VAO;
}

void FOpenGLRenderInterface::PushState(INT Flags){
	++CurrentState;

	check(CurrentState <= &SavedStates[MAX_STATESTACKDEPTH] && "PushState overflow");
	appMemcpy(CurrentState, CurrentState - 1, sizeof(FOpenGLSavedState));
}

void FOpenGLRenderInterface::PopState(INT Flags){
	FOpenGLSavedState* PoppedState = CurrentState;

	--CurrentState;

	check(CurrentState >= &SavedStates[0] && "PopState underflow");

	if(CurrentState->RenderTarget != PoppedState->RenderTarget || CurrentState->RenderTargetMatchesBackbuffer != PoppedState->RenderTargetMatchesBackbuffer)
		SetGLRenderTarget(CurrentState->RenderTarget, CurrentState->RenderTargetMatchesBackbuffer);

	if(CurrentState->VAO != PoppedState->VAO){
		if(CurrentState->VAO != GL_NONE)
			glBindVertexArray(CurrentState->VAO);
		else
			CurrentState->VAO = PoppedState->VAO;
	}

	NeedUniformUpdate = CurrentState->UniformRevision != PoppedState->UniformRevision;
}

UBOOL FOpenGLRenderInterface::SetRenderTarget(FRenderTarget* RenderTarget, bool MatchBackbuffer){
	guardFunc;

	checkSlow(RenderTarget);

	QWORD CacheId = RenderTarget->GetCacheId();
	FOpenGLTexture* GLRenderTarget = static_cast<FOpenGLTexture*>(RenDev->GetCachedResource(CacheId));

	if(!GLRenderTarget)
		GLRenderTarget = new FOpenGLTexture(RenDev, CacheId);

	bool NeedsUpdate = false;

	if(GLRenderTarget->Revision != RenderTarget->GetRevision()){
		GLRenderTarget->Cache(RenderTarget, MatchBackbuffer);
		NeedsUpdate = true;
	}

	if(NeedsUpdate || GLRenderTarget != CurrentState->RenderTarget || MatchBackbuffer != CurrentState->RenderTargetMatchesBackbuffer)
		SetGLRenderTarget(GLRenderTarget, MatchBackbuffer);

	return 1;

	unguard;
}

void FOpenGLRenderInterface::SetViewport(INT X, INT Y, INT Width, INT Height){
	CurrentState->ViewportX = X;
	CurrentState->ViewportY = Y;
	CurrentState->ViewportWidth = Width;
	CurrentState->ViewportHeight = Height;
}

void FOpenGLRenderInterface::Clear(UBOOL UseColor, FColor Color, UBOOL UseDepth, FLOAT Depth, UBOOL UseStencil, DWORD Stencil){
	GLbitfield Flags = 0;

	if(UseColor){
		glClearColor(Color.R / 255.0f, Color.G / 255.0f, Color.B / 255.0f, Color.A / 255.0f);
		Flags |= GL_COLOR_BUFFER_BIT;
	}

	if(UseDepth){
		// If ZWrite is disabled, we need to enable it or clearing will have no effect
		if(!RenderState.bZWrite){
			glDepthMask(GL_TRUE);
			RenderState.bZWrite = true;
		}

		glClearDepth(Depth);
		Flags |= GL_DEPTH_BUFFER_BIT;
	}

	if(UseStencil && bStencilEnabled){
		// Same thing as with depth
		if((RenderState.StencilWriteMask) != 0xFF){
			glStencilMask(0xFF);
			RenderState.StencilWriteMask = 0xFF;
		}

		glClearStencil(Stencil);
		Flags |= GL_STENCIL_BUFFER_BIT;
	}

	glClear(Flags);
}

void FOpenGLRenderInterface::SetCullMode(ECullMode CullMode){
	CurrentState->CullMode = CullMode;
}

void FOpenGLRenderInterface::SetAmbientLight(FColor Color){
	NeedUniformUpdate = true;
	++CurrentState->UniformRevision;
	CurrentState->AmbientLightColor = Color;
}

void FOpenGLRenderInterface::EnableLighting(UBOOL UseDynamic, UBOOL UseStatic, UBOOL Modulate2X, FBaseTexture* Lightmap, UBOOL LightingOnly, const FSphere& LitSphere, int){
	CurrentState->UseDynamicLighting = UseDynamic != 0;
	CurrentState->UseStaticLighting = UseStatic != 0;
	CurrentState->LightingModulate2X = Modulate2X != 0;
	CurrentState->LitSphere = LitSphere;
	CurrentState->Lightmap = Lightmap;
}

void FOpenGLRenderInterface::SetLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale){
	checkSlow(LightIndex <= MAX_LIGHTS);

	if(Light){
		FOpenGLSavedState::Light* ShaderLight = &CurrentState->Lights[LightIndex];

		if(Light->Actor && Light->Actor->LightEffect == LE_Sunlight){
			ShaderLight->Type = 0; // Directional light
			ShaderLight->Direction = -Light->Direction;
			ShaderLight->Color.X = Light->Color.X * 1.75f * Light->Alpha * Scale;
			ShaderLight->Color.Y = Light->Color.Y * 1.75f * Light->Alpha * Scale;
			ShaderLight->Color.Z = Light->Color.Z * 1.75f * Light->Alpha * Scale;
		}else if((Light->Actor && Light->Actor->LightEffect == LE_QuadraticNonIncidence) || CurrentState->LitSphere.W == -1.0f){
			ShaderLight->Type = 1; // Point light
			ShaderLight->Position = Light->Position;
			ShaderLight->Radius = Light->Radius;
			ShaderLight->InvRadius = 1.0f / Light->Radius;
			ShaderLight->Color.X = Light->Color.X * Light->Alpha * Scale;
			ShaderLight->Color.Y = Light->Color.Y * Light->Alpha * Scale;
			ShaderLight->Color.Z = Light->Color.Z * Light->Alpha * Scale;
		}else{
			ShaderLight->Position = Light->Position;
			ShaderLight->Color.X = Light->Color.X * Light->Alpha * Scale;
			ShaderLight->Color.Y = Light->Color.Y * Light->Alpha * Scale;
			ShaderLight->Color.Z = Light->Color.Z * Light->Alpha * Scale;

			if(Light->Actor && Light->Actor->LightCone != 0){
				ShaderLight->Type = 1; // Spotlight
				ShaderLight->Direction = -Light->Direction;
			}else{
				ShaderLight->Type = 1; // Point light
				ShaderLight->Radius = Light->Radius;
				ShaderLight->InvRadius = 1.0f / Light->Radius;
			}
		}

		ShaderLight->Color.W = 1.0f;

		if(LightIndex >= CurrentState->NumLights)
			CurrentState->NumLights = LightIndex + 1;
	}else{
		if(LightIndex < CurrentState->NumLights)
			CurrentState->NumLights = LightIndex;
	}

	++CurrentState->UniformRevision;
	NeedUniformUpdate = true;
}

void FOpenGLRenderInterface::SetDistanceFog(UBOOL Enable, FLOAT FogStart, FLOAT FogEnd, FColor Color){
	// EnableFog(Enable);
	// CurrentState->FogStart = FogStart;
	// CurrentState->FogEnd = FogEnd;
	// CurrentState->FogColor = Color;
	// ++CurrentState->UniformRevision;
	// NeedUniformUpdate = true;
}

UBOOL FOpenGLRenderInterface::EnableFog(UBOOL Enable){
	// CurrentState->FogEnabled = Enable != 0;
	// ++CurrentState->UniformRevision;
	// NeedUniformUpdate = true;

	// return Enable;
	return 0;
}

UBOOL FOpenGLRenderInterface::IsFogEnabled(){
	return 0;//CurrentState->FogEnabled;
}


void FOpenGLRenderInterface::SetGlobalColor(FColor Color){
	++CurrentState->UniformRevision;
	NeedUniformUpdate = true;
	CurrentState->GlobalColor = Color;
}

void FOpenGLRenderInterface::SetTransform(ETransformType Type, const FMatrix& Matrix){
	checkSlow(Type < 3);

	switch(Type){
	case TT_LocalToWorld:
		CurrentState->LocalToWorld = Matrix;
		break;
	case TT_WorldToCamera:
		CurrentState->WorldToCamera = Matrix;
		break;
	case TT_CameraToScreen:
		CurrentState->CameraToScreen = Matrix;
	}

	++CurrentState->UniformRevision;
	NeedUniformUpdate = true;
}

FMatrix FOpenGLRenderInterface::GetTransform(ETransformType Type) const{
	checkSlow(Type < 3);

	switch(Type){
	case TT_LocalToWorld:
		return CurrentState->LocalToWorld;
	case TT_WorldToCamera:
		return CurrentState->WorldToCamera;
	case TT_CameraToScreen:
		return CurrentState->CameraToScreen;
	}

	return FMatrix::Identity;
}

void FOpenGLRenderInterface::InitDefaultMaterialStageState(INT StageIndex){
	// Init default material state
	StageTexCoordCount[StageIndex] = 2;
	StageTexCoordSources[StageIndex] = 0;
	StageTexMatrices[StageIndex] = FMatrix::Identity;
	StageColorArgs[StageIndex][0] = CA_Diffuse;
	StageColorArgs[StageIndex][1] = CA_Diffuse;
	StageColorOps[StageIndex] = COP_Arg1;
	StageAlphaArgs[StageIndex][0] = CA_Diffuse;
	StageAlphaArgs[StageIndex][1] = CA_Diffuse;
	StageAlphaOps[StageIndex] = AOP_Arg1;
}

void FOpenGLRenderInterface::SetMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial, INT* NumPasses){
	guardFunc;

	// Use default material if precaching geometry

	if(!Material || PrecacheMode == PRECACHE_VertexBuffers)
		Material = GetDefault<UMaterial>()->DefaultMaterial;

	/*
	 * HACK:
	 * FrameFX render targets are offset by -0.5 pixels which is needed for Direct3D but causes a blurry image with OpenGL.
	 * This is fixed by checking whether the current material is one of the FrameFX shaders and removing the offset from the CameraToScreen matrix.
	 */
	UFrameFX* FrameFX = LockedViewport->Actor->FrameFX;
	if((FrameFX && (Material == FrameFX->ShaderDraw ||
	                Material == FrameFX->ShaderGlow ||
	                Material == FrameFX->ShaderBlur ||
	                (FrameFX->VisionMode && Material == FrameFX->VisionMode->VisionShader)))){
		// Remove offset by truncating float to int.
		// This works because the x/y values will only ever be -1 or 1 (plus fractional offset) for fullscreen quads or 0 for FFrameGrid
		CurrentState->CameraToScreen.M[3][0] = (INT)CurrentState->CameraToScreen.M[3][0];
		CurrentState->CameraToScreen.M[3][1] = (INT)CurrentState->CameraToScreen.M[3][1];
	}

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

	if(NumPasses)
		*NumPasses = 1;

	// Restore default material state

	for(INT i = 0; i < NumStages; ++i)
		InitDefaultMaterialStageState(i);

	NeedUniformUpdate = true;
	UsingConstantColor = false;
	ModifyColor = false;
	ModifyFramebufferBlending = false;
	NumStages = 0;
	ConstantColor = FPlane(1.0f, 1.0f, 1.0f, 1.0f);
	Unlit = false;

	CurrentState->bZTest = true;
	CurrentState->bZWrite = true;
	CurrentState->SrcBlend = GL_ONE;
	CurrentState->DstBlend = GL_ZERO;
	CurrentState->AlphaRef = -1.0f;
	CurrentState->NumTextures = 0;

	Material->PreSetMaterial(GEngineTime);

	bool Result = false;
	bool UseFixedFunction = true;

	if(CheckMaterial<UShader>(&Material, 0)){
		Result = SetShaderMaterial(static_cast<UShader*>(Material), ErrorString, ErrorMaterial);
	}else if(CheckMaterial<UCombiner>(&Material, -1)){
		Result = SetSimpleMaterial(Material, ErrorString, ErrorMaterial);
	}else if(CheckMaterial<UConstantMaterial>(&Material, -1)){
		Result = SetSimpleMaterial(Material, ErrorString, ErrorMaterial);
	}else if(CheckMaterial<UBitmapMaterial>(&Material, -1)){
		Result = SetSimpleMaterial(Material, ErrorString, ErrorMaterial);
	}else if(CheckMaterial<UTerrainMaterial>(&Material, 0)){
		Result = SetTerrainMaterial(static_cast<UTerrainMaterial*>(Material), ErrorString, ErrorMaterial);
	}else if(CheckMaterial<UParticleMaterial>(&Material, 0)){
		Result = SetParticleMaterial(static_cast<UParticleMaterial*>(Material), ErrorString, ErrorMaterial);
	}else if(CheckMaterial<UProjectorMultiMaterial>(&Material, 0)){
		UProjectorMultiMaterial* ProjectorMultiMaterial = static_cast<UProjectorMultiMaterial*>(Material);
		CurrentState->AlphaRef = 0.5f;
		SetFramebufferBlending(FB_Modulate);
		Result = SetSimpleMaterial(ProjectorMultiMaterial->BaseMaterial, ErrorString, ErrorMaterial);
		CurrentState->bZWrite = false;
	}else if(CheckMaterial<UProjectorMaterial>(&Material, 0)){
		CurrentState->AlphaRef = 0.5f;
		SetFramebufferBlending(FB_Modulate);
		Result = SetSimpleMaterial(static_cast<UProjectorMaterial*>(Material)->Projected, ErrorString, ErrorMaterial);
		CurrentState->bZWrite = false;
	}else if(CheckMaterial<UHardwareShaderWrapper>(&Material, 0)){
		Result = static_cast<UHardwareShaderWrapper*>(Material)->SetupShaderWrapper(this) != 0;
		UseFixedFunction = !Result;
	}else if(CheckMaterial<UHardwareShader>(&Material, 0)){
		Result = SetHardwareShaderMaterial(static_cast<UHardwareShader*>(Material), ErrorString, ErrorMaterial) != 0;
		UseFixedFunction = !Result;
	}

	if(!Result || NumStages <= 0){ // Reset to default state in error case
		InitDefaultMaterialStageState(0);
		NumStages = 1;
	}

	if(UseFixedFunction){
		SetShader(&RenDev->FixedFunctionShader);
		glUniform1i(SU_NumStages, NumStages);
		glUniform1iv(SU_StageTexCoordCount, ARRAY_COUNT(StageTexCoordCount), StageTexCoordCount);
		glUniform1iv(SU_StageTexCoordSources, ARRAY_COUNT(StageTexCoordSources), StageTexCoordSources);
		glUniformMatrix4fv(SU_StageTexMatrices, ARRAY_COUNT(StageTexMatrices), GL_FALSE, (GLfloat*)StageTexMatrices);
		glUniform1iv(SU_StageColorArgs, ARRAY_COUNT(StageColorArgs), &StageColorArgs[0][0]);
		glUniform1iv(SU_StageColorOps, ARRAY_COUNT(StageColorOps), StageColorOps);
		glUniform1iv(SU_StageAlphaArgs, ARRAY_COUNT(StageAlphaArgs), &StageAlphaArgs[0][0]);
		glUniform1iv(SU_StageAlphaOps, ARRAY_COUNT(StageAlphaOps), StageAlphaOps);
		glUniform4fv(SU_ConstantColor, 1, (GLfloat*)&ConstantColor);
		glUniform1i(SU_LightingEnabled, CurrentState->UseDynamicLighting && !Unlit);
		glUniform1f(SU_LightFactor, CurrentState->LightingModulate2X ? 2.0f : 1.0f);
	}

	unguard;
}

static GLenum GetBlendFunc(ED3DBLEND D3DBlend){
	switch(D3DBlend){
	case NOBLEND:
		return GL_ONE;
	case ZERO:
		return GL_ZERO;
	case ONE:
		return GL_ONE;
	case SRCCOLOR:
		return GL_SRC_COLOR;
	case INVSRCCOLOR:
		return GL_ONE_MINUS_SRC_COLOR;
	case SRCALPHA:
		return GL_SRC_ALPHA;
	case INVSRCALPHA:
		return GL_ONE_MINUS_SRC_ALPHA;
	case DESTALPHA:
		return GL_DST_ALPHA;
	case INVDESTALPHA:
		return GL_ONE_MINUS_DST_ALPHA;
	case DESTCOLOR:
		return GL_DST_COLOR;
	case INVDESTCOLOR:
		return GL_ONE_MINUS_DST_COLOR;
	case SRCALPHASAT:
		return GL_SRC_ALPHA_SATURATE;
	case BOTHSRCALPHA:
		return GL_BLEND_SRC_ALPHA;
	case BOTHINVSRCALPHA:
		return GL_ONE_MINUS_SRC_ALPHA;
	}

	return GL_ONE;
}

void FOpenGLRenderInterface::GetShaderConstants(FSConstantsInfo* Info, FPlane* Constants, INT NumConstants){
	FMatrix Matrix;
	for(INT i = 0; i < NumConstants; ++i){
		switch(Info[i].Type){
		case EVC_Unused:
			continue;
		case EVC_MaterialDefined:
			Constants[i] = Info[i].Value;
			continue;
		case EVC_WorldToScreenMatrix:
			Matrix = CurrentState->WorldToScreen.Transpose();
			appMemcpy(&Constants[i], &Matrix, sizeof(FLOAT) * 16);
			i += 3;
			continue;
		case EVC_ObjectToScreenMatrix:
			Matrix = CurrentState->LocalToScreen.Transpose();
			appMemcpy(&Constants[i], &Matrix, sizeof(FLOAT) * 16);
			i += 3;
			continue;
		case EVC_ObjectToWorldMatrix:
			Matrix = CurrentState->LocalToWorld.Transpose();
			appMemcpy(&Constants[i], &Matrix, sizeof(FLOAT) * 16);
			i += 3;
			continue;
		case EVC_CameraToWorldMatrix:
			Matrix = CurrentState->CameraToWorld.Transpose();
			appMemcpy(&Constants[i], &Matrix, sizeof(FLOAT) * 16);
			i += 3;
			continue;
		case EVC_WorldToCameraMatrix:
			Matrix = CurrentState->WorldToCamera.Transpose();
			appMemcpy(&Constants[i], &Matrix, sizeof(FLOAT) * 16);
			i += 3;
			continue;
		case EVC_WorldToObjectMatrix:
			Matrix = CurrentState->WorldToLocal.Transpose();
			appMemcpy(&Constants[i], &Matrix, sizeof(FLOAT) * 16);
			i += 3;
			continue;
		case EVC_Time:
			Constants[i].X = CurrentState->Time;
			Constants[i].Y = CurrentState->Time;
			Constants[i].Z = CurrentState->Time;
			Constants[i].W = CurrentState->Time;
			continue;
		case EVC_CosTime:
			Constants[i].X = CurrentState->CosTime;
			Constants[i].Y = CurrentState->CosTime;
			Constants[i].Z = CurrentState->CosTime;
			Constants[i].W = CurrentState->CosTime;
			continue;
		case EVC_SinTime:
			Constants[i].X = CurrentState->SinTime;
			Constants[i].Y = CurrentState->SinTime;
			Constants[i].Z = CurrentState->SinTime;
			Constants[i].W = CurrentState->SinTime;
			continue;
		case EVC_TanTime:
			Constants[i].X = CurrentState->TanTime;
			Constants[i].Y = CurrentState->TanTime;
			Constants[i].Z = CurrentState->TanTime;
			Constants[i].W = CurrentState->TanTime;
			continue;
		case EVC_EyePosition:
			Constants[i].X = CurrentState->CameraToWorld.M[3][0];
			Constants[i].Y = CurrentState->CameraToWorld.M[3][1];
			Constants[i].Z = CurrentState->CameraToWorld.M[3][2];
			Constants[i].W = CurrentState->CameraToWorld.M[3][3];
			continue;
		case EVC_XYCircle:
			Constants[i].X = appCos(CurrentState->Time) * 500.0f;
			Constants[i].Y = appSin(CurrentState->Time) * 500.0f;
			Constants[i].Z = 0.0f;
			Constants[i].W = 1.0f;
			continue;
		case EVC_LightPos1:
			Constants[i] = CurrentState->Lights[0].Position;
			continue;
		case EVC_LightColor1:
			Constants[i] = CurrentState->Lights[0].Color;
			continue;
		case EVC_LightInvRadius1:
			Constants[i].X = CurrentState->Lights[0].InvRadius;
			Constants[i].Y = Constants[i].X;
			Constants[i].Z = Constants[i].X;
			Constants[i].W = Constants[i].X;
			continue;
		case EVC_LightPos2:
			Constants[i] = CurrentState->Lights[1].Position;
			continue;
		case EVC_LightColor2:
			Constants[i] = CurrentState->Lights[1].Color;
			continue;
		case EVC_LightInvRadius2:
			Constants[i].X = CurrentState->Lights[1].InvRadius;
			Constants[i].Y = Constants[i].X;
			Constants[i].Z = Constants[i].X;
			Constants[i].W = Constants[i].X;
			continue;
		case EVC_LightPos3:
			Constants[i] = CurrentState->Lights[2].Position;
			continue;
		case EVC_LightColor3:
			Constants[i] = CurrentState->Lights[2].Color;
			continue;
		case EVC_LightInvRadius3:
			Constants[i].X = CurrentState->Lights[2].InvRadius;
			Constants[i].Y = Constants[i].X;
			Constants[i].Z = Constants[i].X;
			Constants[i].W = Constants[i].X;
			continue;
		case EVC_LightPos4:
			Constants[i] = CurrentState->Lights[3].Position;
			continue;
		case EVC_LightColor4:
			Constants[i] = CurrentState->Lights[3].Color;
			continue;
		case EVC_LightInvRadius4:
			Constants[i].X = CurrentState->Lights[3].InvRadius;
			Constants[i].Y = Constants[i].X;
			Constants[i].Z = Constants[i].X;
			Constants[i].W = Constants[i].X;
			continue;
		case EVC_AmbientLightColor:
			Constants[i] = CurrentState->AmbientLightColor;
			continue;
		case EVC_Flicker:
			{
				FLOAT X     = Info[i].Value.X;
				FLOAT Y     = Info[i].Value.Y;
				FLOAT YInv  = 1.0f - Y;
				FLOAT Rand1 = appRand() * 0.000030518509f;
				FLOAT Rand2 = appRand() * 0.000030518509f;
				FLOAT Rand3 = appRand() * 0.000030518509f;

				Constants[i].X = Rand1 <= X ? 1.0f : Y * Rand3 + YInv;
				Constants[i].Y = Rand2 <= X ? 1.0f : Y * Rand1 + YInv;
				Constants[i].Z = Rand3 <= X ? 1.0f : Y * Rand2 + YInv;
				Constants[i].W = 1.0f;
			}
			continue;
		case EVC_SpotlightDirection:
			Constants[i] = CurrentState->Lights[0].Direction; // TODO: Implement spotlights
			continue;
		case EVC_SpotlightCosCone:
			Constants[i].X = CurrentState->Lights[0].CosCone; // TODO: Implement spotlights
			Constants[i].Y = CurrentState->Lights[0].CosCone; // TODO: Implement spotlights
			Constants[i].Z = CurrentState->Lights[0].CosCone; // TODO: Implement spotlights
			Constants[i].W = CurrentState->Lights[0].CosCone; // TODO: Implement spotlights
			continue;
		case EVC_DrawScale3D:
			Constants[i].X = CurrentState->LocalToWorld.M[0][0];
			Constants[i].Y = CurrentState->LocalToWorld.M[1][1];
			Constants[i].Z = CurrentState->LocalToWorld.M[2][2];
			Constants[i].W = 1.0f;
			continue;
		case EVC_Fog:
			continue; // TODO: Implement Fog
		case EVC_ObjectToCameraMatrix:
			Matrix = CurrentState->LocalToCamera.Transpose();
			appMemcpy(&Constants[i], &Matrix, sizeof(FLOAT) * 16);
			i += 3;
			continue;
		case EVC_EyePositionObjectSpace:
			Constants[i] = CurrentState->WorldToLocal.TransformFPlane(FPlane(CurrentState->CameraToWorld.M[3][0],
			                                                                          CurrentState->CameraToWorld.M[3][1],
			                                                                          CurrentState->CameraToWorld.M[3][2],
			                                                                          CurrentState->CameraToWorld.M[3][3]));
			continue;
		case EVC_2DRotator:
			Constants[i].X = appCos(CurrentState->Time);
			Constants[i].Y = -appSin(CurrentState->Time);
			Constants[i].Z = 0.0f;
			Constants[i].W = 1.0f;
			continue;
		}
	}
}

UBOOL FOpenGLRenderInterface::SetHardwareShaderMaterial(UHardwareShader* Material, FString* ErrorString, UMaterial** ErrorMaterial){
	SetShader(RenDev->GetShader(Material));

	CurrentState->bZTest = Material->ZTest != 0;
	CurrentState->bZWrite = Material->ZWrite != 0;

	if(Material->AlphaTest)
		CurrentState->AlphaRef = Material->AlphaRef / 255.0f;

	if(Material->AlphaBlending){
		CurrentState->SrcBlend = GetBlendFunc(static_cast<ED3DBLEND>(Material->SrcBlend));
		CurrentState->DstBlend = GetBlendFunc(static_cast<ED3DBLEND>(Material->DestBlend));
	}

	for(INT i = 0; i < MAX_TEXTURES; ++i){
		if(Material->Textures[i]){
			SetBitmapTexture(Material->Textures[i], i);
			CurrentState->TextureInfo[i].BumpSize = Material->BumpSettings[i].BumpSize;
		}

		++CurrentState->NumTextures;
	}

	for(INT i = MAX_TEXTURES - 1; i >= 0 && Material->Textures[i] == NULL; --i)
		--CurrentState->NumTextures;

	UpdateGlobalShaderUniforms();

	static FPlane Constants[MAX_VERTEX_SHADER_CONSTANTS];

	GetShaderConstants(Material->VSConstants, Constants, Material->NumVSConstants);
	glUniform4fv(HSU_VSConstants, Material->NumVSConstants, (GLfloat*)Constants);
	GetShaderConstants(Material->PSConstants, Constants, Material->NumPSConstants);
	glUniform4fv(HSU_PSConstants, Material->NumPSConstants, (GLfloat*)Constants);

	return 1;
}

void FOpenGLRenderInterface::SetTexture(FBaseTexture* Texture, INT TextureUnit){
	checkSlow(Texture);

	QWORD CacheId = Texture->GetCacheId();
	FOpenGLTexture* GLTexture = static_cast<FOpenGLTexture*>(RenDev->GetCachedResource(CacheId));

	if(!GLTexture)
		GLTexture = new FOpenGLTexture(RenDev, CacheId);

	if(GLTexture->Revision != Texture->GetRevision())
		GLTexture->Cache(Texture);

	CurrentState->TextureUnits[TextureUnit].Texture = GLTexture;

	if(GLTexture->IsCubemap){
		CurrentState->TextureInfo[TextureUnit].IsCubemap = 1;
		CurrentState->TextureUnits[TextureUnit].ClampU = TC_Clamp;
		CurrentState->TextureUnits[TextureUnit].ClampV = TC_Clamp;
	}else{
		CurrentState->TextureInfo[TextureUnit].IsCubemap = 0;

		if(GLTexture->FBO == GL_NONE){
			CurrentState->TextureUnits[TextureUnit].ClampU = Texture->GetUClamp();
			CurrentState->TextureUnits[TextureUnit].ClampV = Texture->GetVClamp();
		}else{
			 // Render targets should use TC_Clamp to avoid artifacts at the edges of the screen
			CurrentState->TextureUnits[TextureUnit].ClampU = TC_Clamp;
			CurrentState->TextureUnits[TextureUnit].ClampV = TC_Clamp;
		}
	}

	CurrentState->TextureInfo[TextureUnit].IsBumpmap = IsBumpmap(Texture->GetFormat());
}

void FOpenGLRenderInterface::SetBitmapTexture(UBitmapMaterial* Bitmap, INT TextureUnit){
	FBaseTexture* Texture = Bitmap->Get(LockedViewport->CurrentTime, LockedViewport)->GetRenderInterface();
	SetTexture(Texture, TextureUnit);
}

bool FOpenGLRenderInterface::SetSimpleMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial){
	if(!Material)
		return true;

	INT StagesUsed = 0;
	INT TexturesUsed = 0;

	// If the material is a simple texture, use it to get the blending options
	if(!ModifyFramebufferBlending && Material->IsA<UTexture>()){
		UTexture* Texture = static_cast<UTexture*>(Material);

		if(Texture->bMasked){
			ModifyFramebufferBlending = true;
			CurrentState->AlphaRef = 0.5f;
			SetFramebufferBlending(FB_Overwrite);
			NeedUniformUpdate = true;
		}else if(Texture->bAlphaTexture){
			ModifyFramebufferBlending = true;
			CurrentState->AlphaRef = 0.0f;
			SetFramebufferBlending(FB_AlphaBlend);
			NeedUniformUpdate = true;
		}

		if(Texture->bTwoSided)
			CurrentState->CullMode = CM_None;
	}

	if(!HandleCombinedMaterial(Material, StagesUsed, TexturesUsed, ErrorString, ErrorMaterial))
		return false;

	if(CurrentState->UseStaticLighting && !CurrentState->UseDynamicLighting)
		UseDiffuse();

	if(CurrentState->Lightmap){
		UseLightmap(StagesUsed, TexturesUsed);
		++StagesUsed;
		++TexturesUsed;
	}

	if(ModifyColor){
		if(StagesUsed < MAX_SHADER_STAGES){
			StageColorArgs[StagesUsed][0] = CA_Previous;
			StageColorArgs[StagesUsed][1] = CA_Constant;
			StageColorOps[StagesUsed] = COP_Modulate;
			StageAlphaArgs[StagesUsed][0] = CA_Previous;
			StageAlphaArgs[StagesUsed][1] = CA_Constant;
			StageAlphaOps[StagesUsed] = AOP_Modulate;
			++StagesUsed;
		}else{
			if(ErrorString)
				*ErrorString = "No stages left for color modifier";

			if(ErrorMaterial)
				*ErrorMaterial = Material;

			return false;
		}
	}

	NumStages = StagesUsed;
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

		StageColorArgs[StagesUsed][0] = CA_Diffuse;
		StageColorOps[StagesUsed] = COP_Arg1;
		StageAlphaArgs[StagesUsed][1] = CA_Previous;
		StageAlphaOps[StagesUsed] = AOP_Arg2;

		++StagesUsed;

		return true;
	}else if(CheckMaterial<UConstantMaterial>(&Material, StagesUsed)){
		if(UsingConstantColor){
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

		ConstantColor = static_cast<UConstantMaterial*>(Material)->GetColor(GEngineTime);
		StageColorArgs[StagesUsed][0] = CA_Constant;
		StageColorOps[StagesUsed] = COP_Arg1;
		StageAlphaArgs[StagesUsed][0] = CA_Constant;
		StageAlphaOps[StagesUsed] = AOP_Arg1;

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

		StageColorArgs[StagesUsed][0] = CA_Texture0 + TexturesUsed;
		StageColorOps[StagesUsed] = COP_Arg1;
		StageAlphaArgs[StagesUsed][0] = CA_Texture0 + TexturesUsed;
		StageAlphaOps[StagesUsed] = AOP_Arg1;

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
				INT MaskModifiers = Combiner->InvertMask ? CAM_Invert : 0;

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

						StageAlphaArgs[StagesUsed][0] = (CA_Texture0 + TexturesUsed) | MaskModifiers;
						StageAlphaOps[StagesUsed] = AOP_Arg1;
						++TexturesUsed;
					}else{
						if(!HandleCombinedMaterial(Mask, StagesUsed, TexturesUsed, ErrorString, ErrorMaterial))
							return false;

						StageColorArgs[StagesUsed - 1][0] = CA_Previous;
						StageColorOps[StagesUsed - 1] = COP_Arg1;
						StageAlphaArgs[StagesUsed - 1][0] = (CA_Texture0 + TexturesUsed - 1) | MaskModifiers;
						StageAlphaOps[StagesUsed - 1] = AOP_Arg1;
					}
				}else if(CheckMaterial<UVertexColor>(&Mask, StagesUsed)){
					StageAlphaArgs[StagesUsed - 1][0] = CA_Diffuse | MaskModifiers;
					StageAlphaOps[StagesUsed - 1] = AOP_Arg1;
				}else if(CheckMaterial<UConstantMaterial>(&Mask, StagesUsed)){
					if(UsingConstantColor){
						if(ErrorString)
							*ErrorString = "Only one ConstantMaterial may be used per material";

						if(ErrorMaterial)
							*ErrorMaterial = Material;

						return false;
					}

					ConstantColor = static_cast<UConstantMaterial*>(Mask)->GetColor(GEngineTime);
					StageAlphaArgs[StagesUsed - 1][0] = CA_Constant;
					StageAlphaOps[StagesUsed - 1] = AOP_Arg1;
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
				StageColorArgs[StagesUsed - 1][0] = CA_Texture0 + TexturesUsed - 1;
				StageColorArgs[StagesUsed - 1][1] = CA_Previous;
			}else{
				StageColorArgs[StagesUsed - 1][0] = CA_Previous;
				StageColorArgs[StagesUsed - 1][1] = CA_Texture0 + TexturesUsed - 1;
			}

			if(Mask == Material2){
				StageAlphaArgs[StagesUsed - 1][0] = CA_Texture0 + TexturesUsed - 1;
				StageAlphaArgs[StagesUsed - 1][1] = CA_Previous;
			}else{
				StageAlphaArgs[StagesUsed - 1][0] = CA_Previous;
				StageAlphaArgs[StagesUsed - 1][1] = CA_Texture0 + TexturesUsed - 1;
			}
		}

		// Apply color operations

		INT StageIndex = StagesUsed - 1;

		switch(Combiner->CombineOperation){
		case CO_Use_Color_From_Material1:
			StageColorOps[StageIndex] = COP_Arg1;
			break;
		case CO_Use_Color_From_Material2:
			StageColorOps[StageIndex] = COP_Arg2;
			break;
		case CO_Multiply:
			StageColorOps[StageIndex] = Combiner->Modulate4X ? COP_Modulate4X : Combiner->Modulate2X ? COP_Modulate2X : COP_Modulate;
			break;
		case CO_Add:
			StageColorOps[StageIndex] = COP_Add;
			break;
		case CO_Subtract:
			StageColorOps[StageIndex] = COP_Subtract;

			if(!Swapped)
				Exchange(StageColorArgs[StageIndex][0], StageColorArgs[StageIndex][1]);

			break;
		case CO_AlphaBlend_With_Mask:
			StageColorOps[StageIndex] = COP_AlphaBlend;
			break;
		case CO_Add_With_Mask_Modulation:
			StageColorOps[StageIndex] = COP_AddAlphaModulate;
			break;
		case CO_Use_Color_From_Mask:
			StageColorOps[StageIndex] = COP_Arg1; // TODO: This is probably not correct
		}

		// Apply alpha operations

		switch(Combiner->AlphaOperation){
		case AO_Use_Mask:
			break;
		case AO_Multiply:
			StageAlphaOps[StageIndex] = AOP_Modulate;
			break;
		case AO_Add:
			StageAlphaOps[StageIndex] = AOP_Add;
			break;
		case AO_Use_Alpha_From_Material1:
			StageAlphaOps[StageIndex] = AOP_Arg1;
			break;
		case AO_Use_Alpha_From_Material2:
			StageAlphaOps[StageIndex] = AOP_Arg2;
			break;
		case AO_AlphaBlend_With_Mask:
			StageAlphaOps[StageIndex] = AOP_Blend;
		}

		if(Combiner->LightBothMaterials){
			if(CurrentState->UseStaticLighting && !CurrentState->UseDynamicLighting){
				checkSlow(StagesUsed < MAX_SHADER_STAGES);
				StageColorArgs[StagesUsed][0] = CA_Previous;
				StageColorArgs[StagesUsed][1] = CA_Diffuse;
				StageColorOps[StagesUsed] = CurrentState->LightingModulate2X ? COP_Modulate2X : COP_Modulate;
				StageAlphaArgs[StagesUsed][0] = CA_Previous;
				StageAlphaArgs[StagesUsed][1] = CA_Diffuse;
				StageAlphaOps[StagesUsed] = AOP_Modulate;
				++StagesUsed;
			}
		}else{
			StageAlphaOps[StageIndex] |= AOPM_LightInfluence;
		}

		return true;
	}

	return true;
}

void FOpenGLRenderInterface::SetGLRenderTarget(FOpenGLTexture* GLRenderTarget, bool MatchBackbuffer){
	checkSlow(GLRenderTarget);

	if(MatchBackbuffer)
		glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
	else
		glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);

	GLRenderTarget->BindRenderTarget();
	SetViewport(0, 0, GLRenderTarget->Width, GLRenderTarget->Height);

	CurrentState->RenderTarget = GLRenderTarget;
	CurrentState->RenderTargetMatchesBackbuffer = MatchBackbuffer;
}

bool FOpenGLRenderInterface::SetShaderMaterial(UShader* Shader, FString* ErrorString, UMaterial** ErrorMaterial){
	INT StagesUsed = 0;
	INT TexturesUsed = 0;
	bool HaveDiffuse = false;

	if(Shader->Diffuse){
		if(!HandleCombinedMaterial(Shader->Diffuse, StagesUsed, TexturesUsed, ErrorString, ErrorMaterial))
			return false;

		StageColorOps[StagesUsed - 1] |= COPM_SaveTemp1;
		HaveDiffuse = true;
	}

	if(Shader->SelfIllumination){
		if(Shader->SelfIllumination != Shader->Diffuse){
			if(!HandleCombinedMaterial(Shader->SelfIllumination, StagesUsed, TexturesUsed, ErrorString, ErrorMaterial))
				return false;
		}

		StageColorOps[StagesUsed - 1] |= COPM_SaveTemp2;

		if(Shader->SelfIlluminationMask){
			if(Shader->SelfIlluminationMask == Shader->Diffuse){
				StageColorArgs[StagesUsed][0] = CA_Temp1;
				StageColorArgs[StagesUsed][1] = CA_Temp2;
				StageAlphaArgs[StagesUsed][0] = CA_Temp1;
				StageAlphaOps[StagesUsed] = AOP_Arg1 | AOPM_LightInfluence;
				StageColorOps[StagesUsed] = COP_AlphaBlend;
				++StagesUsed;
			}else if(Shader->SelfIlluminationMask == Shader->SelfIllumination){
				StageColorArgs[StagesUsed][0] = HaveDiffuse ? CA_Temp1 : CA_Diffuse;
				StageColorArgs[StagesUsed][1] = CA_Temp2;
				StageAlphaArgs[StagesUsed][0] = CA_Temp2;
				StageAlphaOps[StagesUsed] = AOP_Arg1 | AOPM_LightInfluence;
				StageColorOps[StagesUsed] = COP_AlphaBlend;
				++StagesUsed;
			}else{
				if(!HandleCombinedMaterial(Shader->SelfIlluminationMask, StagesUsed, TexturesUsed, ErrorString, ErrorMaterial))
					return false;

				StageColorArgs[StagesUsed][0] = HaveDiffuse ? CA_Temp1 : CA_Diffuse;
				StageColorArgs[StagesUsed][1] = CA_Temp2;
				StageAlphaArgs[StagesUsed][0] = CA_Previous;
				StageAlphaOps[StagesUsed] = AOP_Arg1 | AOPM_LightInfluence;
				StageColorOps[StagesUsed] = COP_AlphaBlend;
				++StagesUsed;
			}
		}else{
			Unlit = true;
		}
	}

	if(Shader->Opacity){
		INT AlphaArg;

		if(Shader->Opacity == Shader->Diffuse){
			AlphaArg = CA_Temp1;
		}else if(Shader->Opacity == Shader->SelfIllumination){
			AlphaArg = CA_Temp2;
		}else if(Shader->Opacity->IsA<UVertexColor>()){
			AlphaArg = CA_Diffuse;
		}else{
			StageColorOps[StagesUsed - 1] |= COPM_SaveTemp1;

			if(!HandleCombinedMaterial(Shader->Opacity, StagesUsed, TexturesUsed, ErrorString, ErrorMaterial))
				return false;

			AlphaArg = CA_Previous;
		}

		StageColorArgs[StagesUsed][0] = CA_Temp1;
		StageAlphaArgs[StagesUsed][0] = AlphaArg;
		StageAlphaOps[StagesUsed] = AOP_Arg1;
		StageColorOps[StagesUsed] = COP_Arg1;
		++StagesUsed;
	}

	if(!ModifyFramebufferBlending){
		CurrentState->bZTest = true;
		CurrentState->bZWrite = true;

		if(Shader->TwoSided)
			CurrentState->CullMode = CM_None;

		if(Shader->Wireframe)
			CurrentState->FillMode = FM_Wireframe;

		switch(Shader->OutputBlending){
		case OB_Normal:
			if(Shader->Opacity){
				CurrentState->AlphaRef = 0.0f;
				SetFramebufferBlending(FB_AlphaBlend);
				CurrentState->bZWrite = false;
				NeedUniformUpdate = true;
			}

			break;
		case OB_Masked:
			CurrentState->AlphaRef = 0.5f;
			SetFramebufferBlending(FB_Overwrite);
			NeedUniformUpdate = true;
			break;
		case OB_Modulate:
			SetFramebufferBlending(FB_Modulate);
			break;
		case OB_Translucent:
			SetFramebufferBlending(FB_Translucent);
			break;
		case OB_Invisible:
			SetFramebufferBlending(FB_Invisible);
			break;
		case OB_Brighten:
			SetFramebufferBlending(FB_Brighten);
			break;
		case OB_Darken:
			SetFramebufferBlending(FB_Darken);
		}
	}

	NumStages = StagesUsed;
	CurrentState->NumTextures = TexturesUsed;

	return true;
}

bool FOpenGLRenderInterface::SetTerrainMaterial(UTerrainMaterial* Terrain, FString* ErrorString, UMaterial** ErrorMaterial){
	checkSlow(Terrain->Layers.Num() == 1);

	INT StagesUsed = 0;
	INT TexturesUsed = 0;

	UMaterial* Material = Terrain->Layers[0].Texture;

	if(Material->IsA<UShader>())
		Material = static_cast<UShader*>(Material)->Diffuse;

	StageTexMatrices[StagesUsed] = Terrain->Layers[0].TextureMatrix;
	StageTexCoordCount[StagesUsed] = 3;
	StageTexCoordSources[StagesUsed] = TCS_WorldCoords;

	if(CheckMaterial<UBitmapMaterial>(&Material, StagesUsed, TexturesUsed)){
		if(Terrain->FirstPass){
			SetFramebufferBlending(FB_Overwrite);
		}else{
			SetFramebufferBlending(FB_AlphaBlend);
			CurrentState->AlphaRef = 0.0f;
		}

		// Color texture

		UBitmapMaterial* BitmapMaterial = static_cast<UBitmapMaterial*>(Material);

		SetBitmapTexture(BitmapMaterial, TexturesUsed);

		CurrentState->TextureUnits[TexturesUsed].ClampU = TC_Wrap;
		CurrentState->TextureUnits[TexturesUsed].ClampV = TC_Wrap;

		StageColorArgs[StagesUsed][0] = CA_Texture0 + TexturesUsed;
		StageColorArgs[StagesUsed][1] = CA_Diffuse;
		StageColorOps[StagesUsed] = (CurrentState->UseStaticLighting && !CurrentState->UseDynamicLighting) ? COP_Modulate2X : COP_Arg1;
		StageAlphaArgs[StagesUsed][0] = CA_Texture0 + TexturesUsed;
		StageAlphaOps[StagesUsed] = AOP_Arg1;

		++StagesUsed;
		++TexturesUsed;

		// Alpha map

		SetBitmapTexture(Terrain->Layers[0].AlphaWeight, TexturesUsed);

		StageColorArgs[StagesUsed][0] = CA_Previous;
		StageColorOps[StagesUsed] = COP_Arg1;
		StageAlphaArgs[StagesUsed][0] = CA_Texture0 + TexturesUsed;
		StageAlphaArgs[StagesUsed][1] = CA_Previous;
		StageAlphaOps[StagesUsed] = BitmapMaterial->IsTransparent() ? AOP_Modulate : AOP_Arg1;
		StageTexCoordSources[StagesUsed] = TCS_Stream0;

		++StagesUsed;
		++TexturesUsed;
	}else{
		return false;
	}

	NumStages = StagesUsed;
	CurrentState->NumTextures = TexturesUsed;

	return true;
}

bool FOpenGLRenderInterface::SetParticleMaterial(UParticleMaterial* ParticleMaterial, FString* ErrorString, UMaterial** ErrorMaterial){
	INT StagesUsed = 0;
	INT TexturesUsed = 0;

	switch(ParticleMaterial->ParticleBlending){
	case PTDS_Regular:
		SetFramebufferBlending(FB_Overwrite);
		break;
	case PTDS_AlphaBlend:
		SetFramebufferBlending(FB_AlphaBlend);
		break;
	case PTDS_Modulated:
		SetFramebufferBlending(FB_Modulate);
		break;
	case PTDS_Translucent:
		SetFramebufferBlending(FB_Translucent);
		break;
	case PTDS_AlphaModulate_MightNotFogCorrectly:
		SetFramebufferBlending(FB_AlphaModulate_MightNotFogCorrectly);
		break;
	case PTDS_Darken:
		SetFramebufferBlending(FB_Darken);
		break;
	case PTDS_Brighten:
		SetFramebufferBlending(FB_Brighten);
	}

	ModifyFramebufferBlending = true;

	if(ParticleMaterial->BitmapMaterial)
		SetBitmapTexture(ParticleMaterial->BitmapMaterial, TexturesUsed);

	if(!ParticleMaterial->BlendBetweenSubdivisions){
		StageColorArgs[StagesUsed][0] = CA_Texture0 + TexturesUsed;
		StageAlphaArgs[StagesUsed][0] = CA_Texture0 + TexturesUsed;

		if(ParticleMaterial->UseTFactor){
			StageColorArgs[StagesUsed][1] = CA_Constant;
			StageAlphaArgs[StagesUsed][1] = CA_Constant;
		}else{
			StageColorArgs[StagesUsed][1] = CA_Diffuse;
			StageAlphaArgs[StagesUsed][1] = CA_Diffuse;
		}

		if(ParticleMaterial->ParticleBlending != PTDS_Modulated){
			StageColorOps[StagesUsed] = COP_Modulate;
			StageAlphaOps[StagesUsed] = AOP_Modulate;
		}else{
			StageColorOps[StagesUsed] = COP_AlphaBlend;
			StageAlphaOps[StagesUsed] = AOP_Blend;
		}

		++TexturesUsed;
		++StagesUsed;
	}else{
		StageColorArgs[StagesUsed][0] = CA_Texture0 + TexturesUsed;
		StageAlphaArgs[StagesUsed][0] = CA_Texture0 + TexturesUsed;
		StageColorOps[StagesUsed] = COP_Arg1;
		StageAlphaOps[StagesUsed] = AOP_Arg1;
		++StagesUsed;
		StageColorArgs[StagesUsed][0] = CA_Texture0 + TexturesUsed;
		StageColorArgs[StagesUsed][1] = CA_Previous;
		StageAlphaArgs[StagesUsed][0] = CA_Texture0 + TexturesUsed;
		StageAlphaArgs[StagesUsed][1] = CA_Previous;
		StageColorOps[StagesUsed] = COP_AlphaBlend;
		StageAlphaOps[StagesUsed] = AOP_Blend;
		StageTexCoordSources[StagesUsed] = TCS_Stream1;
		++StagesUsed;
		StageColorArgs[StagesUsed][0] = CA_Diffuse;
		StageColorArgs[StagesUsed][1] = CA_Previous;
		StageAlphaArgs[StagesUsed][0] = CA_Previous;
		StageColorOps[StagesUsed] = COP_Modulate;
		StageAlphaOps[StagesUsed] = AOP_Arg1;
		++StagesUsed;
		++TexturesUsed;
	}

	if(ParticleMaterial->RenderTwoSided)
		CurrentState->CullMode = CM_None;

	if(ParticleMaterial->AlphaTest)
		CurrentState->AlphaRef = ParticleMaterial->AlphaRef / 255.0f;

	CurrentState->bZTest = ParticleMaterial->ZTest != 0;
	CurrentState->bZWrite = ParticleMaterial->ZWrite != 0;
	CurrentState->FillMode = ParticleMaterial->Wireframe ? FM_Wireframe : FM_Solid;

	NumStages = StagesUsed;
	CurrentState->NumTextures = TexturesUsed;

	return true;
}

void FOpenGLRenderInterface::UseDiffuse(){
	StageColorArgs[0][1] = CA_Diffuse;
	StageColorOps[0] = CurrentState->LightingModulate2X ? COP_Modulate2X : COP_Modulate;
	StageAlphaArgs[0][1] = CA_Diffuse;
	StageAlphaOps[0] = AOP_Modulate;
}

void FOpenGLRenderInterface::UseLightmap(INT StageIndex, INT TextureUnit){
	checkSlow(CurrentState->Lightmap);
	checkSlow(StageIndex < MAX_SHADER_STAGES);
	checkSlow(TextureUnit < MAX_TEXTURES);

	SetTexture(CurrentState->Lightmap, TextureUnit);
	StageTexCoordSources[StageIndex] = TCS_Stream1;
	StageColorArgs[StageIndex][0] = CA_Previous;
	StageColorArgs[StageIndex][1] = CA_Texture0 + TextureUnit;
	StageColorOps[StageIndex] = (!CurrentState->UseStaticLighting && CurrentState->LightingModulate2X) ? COP_Modulate2X : COP_Modulate;
	StageAlphaArgs[StageIndex][0] = CA_Previous;
	StageAlphaOps[StageIndex] = AOP_Arg1;
}

void FOpenGLRenderInterface::CopyBackBufferToTarget(FAuxRenderTarget* Target){
	FOpenGLTexture* GLTarget = static_cast<FOpenGLTexture*>(RenDev->GetCachedResource(Target->GetCacheId()));
	FOpenGLTexture* Backbuffer = static_cast<FOpenGLTexture*>(RenDev->GetCachedResource(RenDev->Backbuffer.GetCacheId()));

	if(!GLTarget || !Backbuffer)
		return;

	GLbitfield Flags = GL_COLOR_BUFFER_BIT;

	if(GLTarget->DepthStencilAttachment && Backbuffer->DepthStencilAttachment)
		Flags |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;

	glBlitNamedFramebuffer(Backbuffer->FBO,
	                       GLTarget->FBO,
	                       0, 0, Backbuffer->Width, Backbuffer->Height,
	                       0, 0, GLTarget->Width, GLTarget->Height,
	                       Flags,
	                       GL_NEAREST);
}

void FOpenGLRenderInterface::SetStencilOp(ECompareFunction Test, DWORD Ref, DWORD Mask, EStencilOp FailOp, EStencilOp ZFailOp, EStencilOp PassOp, DWORD WriteMask){
	CurrentState->StencilCompare = Test;
	CurrentState->StencilRef = static_cast<BYTE>(Ref);
	CurrentState->StencilMask = static_cast<BYTE>(Mask);
	CurrentState->StencilFailOp = FailOp;
	CurrentState->StencilZFailOp = ZFailOp;
	CurrentState->StencilPassOp = PassOp;
	CurrentState->StencilWriteMask = static_cast<BYTE>(WriteMask);
}

void FOpenGLRenderInterface::EnableStencil(UBOOL Enable){
	CurrentState->bStencilTest = Enable != 0;
}

void FOpenGLRenderInterface::EnableDepth(UBOOL Enable){
	if(Enable){
		CurrentState->bZWrite = true;
		CurrentState->bZTest = true;
	}else{
		CurrentState->bZWrite = false;
		CurrentState->bZTest = false;
	}
}

void FOpenGLRenderInterface::SetZBias(INT ZBias){
	CurrentState->ZBias = ZBias;
}

INT FOpenGLRenderInterface::SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams, bool IsDynamic){
	checkSlow(!IsDynamic || NumStreams == 1);

	FStreamDeclaration VertexStreamDeclarations[MAX_VERTEX_STREAMS];

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
	GLuint VAO = GetVAO(VertexStreamDeclarations, NumStreams);

	if(VAO != CurrentState->VAO){
		RenderState.IndexBuffer = NULL;
		appMemzero(RenderState.VertexStreams, sizeof(RenderState.VertexStreams));
		glBindVertexArray(VAO);
		CurrentState->VAO = VAO;
	}

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
	}else{
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
	CommitRenderState();

	if(NeedUniformUpdate)
		UpdateGlobalShaderUniforms();

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
	};

	if(RenderState.IndexBuffer){
		INT IndexSize = RenderState.IndexBuffer->IndexSize;

		glDrawRangeElements(Mode,
		                    MinIndex,
		                    MaxIndex,
		                    Count,
		                    IndexSize == sizeof(_WORD) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
		                    reinterpret_cast<void*>((FirstIndex + CurrentState->IndexBufferBaseIndex) * IndexSize));
	}else{
		glDrawArrays(Mode, 0, Count);
	}
}

void FOpenGLRenderInterface::SetFillMode(EFillMode FillMode){
	CurrentState->FillMode = FillMode;
}
