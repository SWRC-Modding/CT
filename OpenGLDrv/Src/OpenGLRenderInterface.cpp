#include "../Inc/OpenGLRenderInterface.h"

#include "../Inc/OpenGLRenderDevice.h"
#include "OpenGLResource.h"
#include "GL/glew.h"

/*
 * FOpenGLVertexArrayObject
 */

FOpenGLVertexArrayObject::~FOpenGLVertexArrayObject(){
	checkSlow(VAO);
	glDeleteVertexArrays(1, &VAO);
}

void FOpenGLVertexArrayObject::Init(const FStreamDeclaration* Declarations, INT NumStreams){
	checkSlow(VAO == GL_NONE);

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
				break;
			default:
				appErrorf("Invalid vertex component type: %i", Type);
			}

			glEnableVertexArrayAttrib(VAO, Function);
			glVertexArrayAttribBinding(VAO, Function, StreamIndex);
		}
	}
}

void FOpenGLVertexArrayObject::Bind(){
	checkSlow(VAO);
	glBindVertexArray(VAO);
}

void FOpenGLVertexArrayObject::BindVertexStream(FOpenGLVertexStream* Stream, INT StreamIndex){
	GLuint VBO = Stream->VBO;
	GLuint Stride = Stream->Stride;

	if(VBOs[StreamIndex] != VBO || Strides[StreamIndex] != Stride){
		VBOs[StreamIndex] = VBO;
		Strides[StreamIndex] = Stride;
		glVertexArrayVertexBuffer(VAO, StreamIndex, VBO, 0, Stride);
	}
}

void FOpenGLVertexArrayObject::BindIndexBuffer(FOpenGLIndexBuffer* IndexBuffer){
	if(EBO != IndexBuffer->EBO){
		EBO = IndexBuffer->EBO;
		glVertexArrayElementBuffer(VAO, EBO);
	}
}

/*
 * Helpers
 */

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

static GLenum GetBlendFunc(/*ED3DBLEND*/BYTE D3DBlend){
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
	checkSlow(RenDev->IsCurrent());

	// Setup initial state

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_POINT);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glEnable(GL_POLYGON_OFFSET_FILL);

	// Viewport

	RenderState.ViewportX = 0;
	RenderState.ViewportY = 0;
	RenderState.ViewportWidth = ViewportWidth;
	RenderState.ViewportHeight = ViewportHeight;
	glViewport(0, 0, ViewportWidth, ViewportHeight);

	// Culling

	glEnable(GL_CULL_FACE);
	RenderState.CullMode = CM_CW;
	LastCullMode = GL_BACK;
	glCullFace(LastCullMode);

	// Fill mode

	RenderState.FillMode = FM_Solid;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// ZTest

	RenderState.bZTest = true;
	glDepthFunc(GL_LEQUAL);

	RenderState.bZWrite = true;
	glDepthMask(GL_TRUE);

	// Stencil

	glDisable(GL_STENCIL_TEST);

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

	// Texture samplers

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

	appMemcpy(static_cast<FOpenGLRenderState*>(CurrentState), &RenderState, sizeof(FOpenGLRenderState));

	// Init uniform default values

	CurrentState->LocalToWorld = FMatrix::Identity;
	CurrentState->WorldToCamera = FMatrix::Identity;
	CurrentState->CameraToScreen = FMatrix::Identity;
	CurrentState->GlobalColor = FPlane(1.0, 1.0, 1.0, 1.0);

	CurrentState->AlphaRef = -1.0f;

	// Create uniform buffer
	glCreateBuffers(1, &GlobalUBO);
	glNamedBufferStorage(GlobalUBO, sizeof(FOpenGLGlobalUniforms), static_cast<FOpenGLGlobalUniforms*>(CurrentState), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, GlobalUBO); // Binding index 0 is reserved for the global uniform block
}

void FOpenGLRenderInterface::Flush(){
	checkSlow(CurrentState == &SavedStates[0]);

	CurrentState->IndexBuffer = NULL;
	CurrentState->NumVertexStreams = 0;
	appMemzero(CurrentState->VertexStreams, sizeof(CurrentState->VertexStreams));

	for(INT i = 0; i < CurrentState->NumTextures; ++i)
		CurrentState->TextureUnits[i].Texture = NULL;

	CurrentState->NumTextures = 0;
	CurrentShader = NULL;
	glUseProgram(GL_NONE);

	CurrentState->VAO = NULL;
	glBindVertexArray(GL_NONE);
	VAOsByDeclId.Empty();
}

void FOpenGLRenderInterface::Exit(){
	VAOsByDeclId.Empty();
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

			if(RenderState.CullMode == CM_None)
				glEnable(GL_CULL_FACE);

			if(NewCullMode != LastCullMode)
				glCullFace(NewCullMode);

			LastCullMode = NewCullMode;
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
			CurrentState->VAO->BindIndexBuffer(CurrentState->IndexBuffer);

		RenderState.IndexBuffer = CurrentState->IndexBuffer;
	}

	for(INT i = 0; i < CurrentState->NumVertexStreams; ++i){
		if(RenderState.VertexStreams[i] != CurrentState->VertexStreams[i]){
			CurrentState->VAO->BindVertexStream(CurrentState->VertexStreams[i], i);
			RenderState.VertexStreams[i] = CurrentState->VertexStreams[i];
		}
	}

	for(INT i = 0; i < CurrentState->NumTextures; ++i){
		if(RenderState.TextureUnits[i].Texture != CurrentState->TextureUnits[i].Texture){
			if(CurrentState->TextureUnits[i].Texture) // Texture might not be set if the current material is a hardware shader
				CurrentState->TextureUnits[i].Texture->BindTexture(i + CurrentState->TextureInfos[i].IsCubemap * MAX_TEXTURES);

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

	if(RenderState.VAO != CurrentState->VAO){
		checkSlow(CurrentState->VAO)
		CurrentState->VAO->Bind();
		RenderState.VAO = CurrentState->VAO;
	}
}

void FOpenGLRenderInterface::Locked(UViewport* Viewport){
	checkSlow(RenDev->IsCurrent());
	checkSlow(CurrentState == &SavedStates[0]);
	checkSlow(!LockedViewport);

	LockedViewport = Viewport;

	// Detect settings changes in UOpenGLRenderDevice and act accordingly

	if(TextureFilter != RenDev->TextureFilter)
		SetTextureFilter(RenDev->TextureFilter);

	if(TextureAnisotropy != RenDev->TextureAnisotropy){
		TextureAnisotropy = RenDev->TextureAnisotropy;

		if(GLEW_EXT_texture_filter_anisotropic){
			for(INT i = 0; i < MAX_TEXTURES; ++i)
				glSamplerParameterf(Samplers[i], GL_TEXTURE_MAX_ANISOTROPY, Clamp<FLOAT>(TextureAnisotropy, 1, 16));
		}
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

void FOpenGLRenderInterface::PushState(DWORD Flags){
	++CurrentState;

	check(CurrentState <= &SavedStates[MAX_STATESTACKDEPTH] && "PushState overflow");
	appMemcpy(CurrentState, CurrentState - 1, sizeof(FOpenGLSavedState));
}

void FOpenGLRenderInterface::PopState(DWORD Flags){
	FOpenGLSavedState* PoppedState = CurrentState;

	--CurrentState;

	check(CurrentState >= &SavedStates[0] && "PopState underflow");

	if((Flags & DONT_RESTORE_RENDER_TARGET) != 0){
		CurrentState->RenderTarget = PoppedState->RenderTarget;
		CurrentState->RenderTargetOwnDepthBuffer = PoppedState->RenderTargetOwnDepthBuffer;
	}else if((Flags & FORCE_RESTORE_RENDER_TARGET) != 0 ||
	         (CurrentState->RenderTarget != PoppedState->RenderTarget ||
	          CurrentState->RenderTargetOwnDepthBuffer != PoppedState->RenderTargetOwnDepthBuffer)){
		SetGLRenderTarget(CurrentState->RenderTarget, CurrentState->RenderTargetOwnDepthBuffer);
	}

	NeedUniformUpdate = CurrentState->UniformRevision != PoppedState->UniformRevision;
}

UBOOL FOpenGLRenderInterface::SetRenderTarget(FRenderTarget* RenderTarget, bool bOwnDepthBuffer){
	guardFuncSlow;

	checkSlow(RenderTarget);

	QWORD CacheId = RenderTarget->GetCacheId();
	FOpenGLTexture* GLRenderTarget = static_cast<FOpenGLTexture*>(RenDev->GetCachedResource(CacheId));

	if(!GLRenderTarget)
		GLRenderTarget = new FOpenGLTexture(RenDev, CacheId);

	bool NeedsUpdate = false;

	if(GLRenderTarget->Revision != RenderTarget->GetRevision()){
		GLRenderTarget->Cache(RenderTarget, bOwnDepthBuffer);
		NeedsUpdate = true;
	}

	if(NeedsUpdate || GLRenderTarget != CurrentState->RenderTarget || bOwnDepthBuffer != CurrentState->RenderTargetOwnDepthBuffer)
		SetGLRenderTarget(GLRenderTarget, bOwnDepthBuffer);

	return 1;

	unguardSlow;
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
	checkSlow(LightIndex <= MAX_SHADER_LIGHTS);

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

	CurrentState->ShaderLights[LightIndex] = Light;

	++CurrentState->UniformRevision;
	NeedUniformUpdate = true;
}

void FOpenGLRenderInterface::SetDistanceFog(UBOOL Enable, FLOAT FogStart, FLOAT FogEnd, FColor Color){
	CurrentState->FogEnabled = Enable;
	CurrentState->FogStart = FogStart;
	CurrentState->FogEnd = FogEnd;
	CurrentState->FogColor = Color;
	++CurrentState->UniformRevision;
	NeedUniformUpdate = true;
}

UBOOL FOpenGLRenderInterface::EnableFog(UBOOL Enable){
	CurrentState->FogEnabled = Enable != 0;
	++CurrentState->UniformRevision;
	NeedUniformUpdate = true;

	return Enable;
}

UBOOL FOpenGLRenderInterface::IsFogEnabled(){
	return CurrentState->FogEnabled;
}

void FOpenGLRenderInterface::SetGlobalColor(FColor Color){
	++CurrentState->UniformRevision;
	NeedUniformUpdate = true;
	CurrentState->GlobalColor = Color;
}

void FOpenGLRenderInterface::SetTransform(ETransformType Type, const FMatrix& Matrix){
	/*
	 * HACK:
	 * GIsOpenGL needs to be set to 0. However, it is checked for in UCanvas::Update which applies a 0.5 pixel offset for d3d.
	 * Here we check if the given matrix belongs to the viewport's canvas and remove the offset.
	 */
	if(&Matrix == &LockedViewport->Canvas->pCanvasUtil->CanvasToScreen){
		FMatrix& Mat = const_cast<FMatrix&>(Matrix);

		Mat.M[3][0] = static_cast<INT>(Mat.M[3][0]);
		Mat.M[3][1] = static_cast<INT>(Mat.M[3][1]);
	}

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

void FOpenGLRenderInterface::SetMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial, INT* NumPasses){
	guardFuncSlow;

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

	NeedUniformUpdate = true;

	CurrentState->bZTest = true;
	CurrentState->bZWrite = true;
	CurrentState->SrcBlend = GL_ONE;
	CurrentState->DstBlend = GL_ZERO;
	CurrentState->AlphaRef = -1.0f;
	CurrentState->NumTextures = 0;

	Material->PreSetMaterial(GEngineTime);

	UModifier* Modifier = Cast<UModifier>(Material);

	if(Modifier){
		Material = RemoveModifiers(Cast<UModifier>(Material), NULL);

		if(!Material){
			SetShader(&RenDev->ErrorShader);

			if(ErrorString)
				*ErrorString = "Modifier does not have a material";

			if(ErrorMaterial)
				*ErrorMaterial = Modifier;

			return;
		}
	}

	bool Result = false;
	bool IsHardwareShader = false;

	if(Material->IsA<UHardwareShaderWrapper>()){
		Result = static_cast<UHardwareShaderWrapper*>(Material)->SetupShaderWrapper(this) != 0;
		IsHardwareShader = true;
	}else if(Material->IsA<UHardwareShader>()){
		Result = SetHardwareShaderMaterial(static_cast<UHardwareShader*>(Material), ErrorString, ErrorMaterial) != 0;
		IsHardwareShader = true;
	}

	if(!IsHardwareShader || !Result){
		SetShader(&RenDev->FixedFunctionShader);
		CurrentState->FillMode = FM_Wireframe;
		// Diffuse and Specular are expected to be 1.0 by default for fixed function lighting calculations
		glVertexAttrib4f(FVF_Diffuse,   1.0f, 1.0f, 1.0f, 1.0f);
		glVertexAttrib4f(FVF_Specular,  1.0f, 1.0f, 1.0f, 1.0f);
	}else{
		CurrentState->FillMode = FM_Solid;
		// Diffuse and Specular are expected to be zero by default for hardware shaders
		glVertexAttrib4f(FVF_Diffuse,   0.0f, 0.0f, 0.0f, 0.0f);
		glVertexAttrib4f(FVF_Specular,  0.0f, 0.0f, 0.0f, 0.0f);
	}

#if 0
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
#endif

	unguardSlow;
}

UMaterial* FOpenGLRenderInterface::RemoveModifiers(UModifier* Modifier, FModifierInfo* ModifierInfo){
	UModifier* FirstModifier = Modifier;
	UMaterial* Material = NULL;

	while(Modifier){
		Material = Modifier->Material;
		Modifier = Cast<UModifier>(Material);
	}

	// Collect modifiers

	if(Material && ModifierInfo){
		Modifier = FirstModifier;

		// Apply modifiers
		while(Modifier != Material){
			if(Modifier->IsA<UTexModifier>()){
				UTexModifier* TexModifier = static_cast<UTexModifier*>(Modifier);
				FMatrix* Matrix = TexModifier->GetMatrix(GEngineTime);

				if(TexModifier->TexCoordSource != TCS_NoChange){
					ModifierInfo->TexCoordSrc = static_cast<ETexCoordSrc>(TexModifier->TexCoordSource);

					switch(TexModifier->TexCoordSource){
					case TCS_CameraCoords:
						ModifierInfo->TexMatrix *= CurrentState->WorldToCamera.Transpose();
						break;
					case TCS_CubeCameraSpaceReflection:
						{
							FMatrix Tmp = CurrentState->WorldToCamera;
							Tmp.M[3][0] = 0.0f;
							Tmp.M[3][1] = 0.0f;
							Tmp.M[3][2] = 0.0f;
							Tmp.M[3][3] = 1.0f;
							ModifierInfo->TexMatrix *= Tmp;
						}
					}

					ModifierInfo->TexCoordCount = static_cast<ETexCoordCount>(TexModifier->TexCoordCount);
				}

				if(Matrix)
					ModifierInfo->TexMatrix *= *Matrix;

				ModifierInfo->TexUClamp = static_cast<ETexClampModeOverride>(TexModifier->UClampMode);
				ModifierInfo->TexVClamp = static_cast<ETexClampModeOverride>(TexModifier->VClampMode);
			}else if(Modifier->IsA<UFinalBlend>()){
#if 0
				UFinalBlend* FinalBlend = static_cast<UFinalBlend*>(Modifier);

				ModifyFramebufferBlending = true;
				SetFramebufferBlending(static_cast<EFrameBufferBlending>(FinalBlend->FrameBufferBlending));
				CurrentState->bZTest = FinalBlend->ZTest != 0;
				CurrentState->bZWrite = FinalBlend->ZWrite != 0;

				if(FinalBlend->TwoSided)
					CurrentState->CullMode = CM_None;

				if(FinalBlend->AlphaTest)
					CurrentState->AlphaRef = FinalBlend->AlphaRef / 255.0f;
#endif
			}else if(Modifier->IsA<UColorModifier>()){
#if 0
				UColorModifier* ColorModifier = static_cast<UColorModifier*>(Modifier);

				UsingConstantColor = true;
				ModifyColor = true;
				ConstantColor = ColorModifier->Color;

				if(ColorModifier->RenderTwoSided)
					CurrentState->CullMode = CM_None;

				if(!ModifyFramebufferBlending && ColorModifier->AlphaBlend)
					SetFramebufferBlending(FB_AlphaBlend);
#endif
			}

			Modifier = static_cast<UModifier*>(Modifier->Material);
		}
	}

	return Material;
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
				static FLOAT LastEngineTime = GEngineTime;
				static FLOAT Rand1 = 0.5f;
				static FLOAT Rand2 = 0.5f;
				static FLOAT Rand3 = 0.5f;

				if(LastEngineTime != GEngineTime){ // Don't update flicker if the game is paused
					LastEngineTime = GEngineTime;
					Rand1 = appRand() / (float)RAND_MAX;
					Rand2 = appRand() / (float)RAND_MAX;
					Rand3 = appRand() / (float)RAND_MAX;
				}

				FLOAT X    = Info[i].Value.X;
				FLOAT Y    = Info[i].Value.Y;
				FLOAT YInv = 1.0f - Y;

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
			if(GCubemapManager && GCubemapManager->Actor){
				Constants[i].X = GCubemapManager->Actor->DrawScale3D.X;
				Constants[i].Y = GCubemapManager->Actor->DrawScale3D.Y;
				Constants[i].Z = GCubemapManager->Actor->DrawScale3D.Z;
			}else{
				Constants[i].X = CurrentState->LocalToWorld.M[0][0];
				Constants[i].Y = CurrentState->LocalToWorld.M[1][1];
				Constants[i].Z = CurrentState->LocalToWorld.M[2][2];
			}

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
			Constants[i].X = CurrentState->CosTime;
			Constants[i].Y = -CurrentState->SinTime;
			Constants[i].Z = 0.0f;
			Constants[i].W = 0.0f;
			++i;
			Constants[i].X = CurrentState->SinTime;
			Constants[i].Y = CurrentState->CosTime;
			Constants[i].Z = 0.0f;
			Constants[i].W = 0.0f;
			continue;
		}
	}
}

UBOOL FOpenGLRenderInterface::SetHardwareShaderMaterial(UHardwareShader* Material, FString* ErrorString, UMaterial** ErrorMaterial){
	guardFuncSlow;

	SetShader(RenDev->GetShader(Material));

	CurrentState->bZTest = Material->ZTest != 0;
	CurrentState->bZWrite = Material->ZWrite != 0;

	if(Material->AlphaTest)
		CurrentState->AlphaRef = Material->AlphaRef / 255.0f;

	if(Material->AlphaBlending){
		CurrentState->SrcBlend = GetBlendFunc(Material->SrcBlend);
		CurrentState->DstBlend = GetBlendFunc(Material->DestBlend);
	}

	for(INT i = 0; i < MAX_TEXTURES; ++i){
		if(Material->Textures[i]){
			SetBitmapTexture(Material->Textures[i], i);
			CurrentState->TextureInfos[i].BumpSize = Material->BumpSettings[i].BumpSize;
		}

		++CurrentState->NumTextures;
	}

	for(INT i = MAX_TEXTURES - 1; i >= 0 && Material->Textures[i] == NULL; --i)
		--CurrentState->NumTextures;

	UpdateGlobalShaderUniforms();

	static FPlane Constants[MAX_VERTEX_SHADER_CONSTANTS];

	if(!CurrentShader->IsErrorShader){
		GetShaderConstants(Material->VSConstants, Constants, Material->NumVSConstants);
		glProgramUniform4fv(CurrentShader->Program, HSU_VSConstants, Material->NumVSConstants, (GLfloat*)Constants);
		GetShaderConstants(Material->PSConstants, Constants, Material->NumPSConstants);
		glProgramUniform4fv(CurrentShader->Program, HSU_PSConstants, Material->NumPSConstants, (GLfloat*)Constants);
	}

	return 1;

	unguardSlow;
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
		CurrentState->TextureInfos[TextureUnit].IsCubemap = 1;
		CurrentState->TextureUnits[TextureUnit].ClampU = TC_Clamp;
		CurrentState->TextureUnits[TextureUnit].ClampV = TC_Clamp;
	}else{
		CurrentState->TextureInfos[TextureUnit].IsCubemap = 0;

		if(GLTexture->FBO == GL_NONE){
			CurrentState->TextureUnits[TextureUnit].ClampU = Texture->GetUClamp();
			CurrentState->TextureUnits[TextureUnit].ClampV = Texture->GetVClamp();
		}else{
			 // Render targets should use TC_Clamp to avoid artifacts at the edges of the screen
			CurrentState->TextureUnits[TextureUnit].ClampU = TC_Clamp;
			CurrentState->TextureUnits[TextureUnit].ClampV = TC_Clamp;
		}
	}

	CurrentState->TextureInfos[TextureUnit].IsBumpmap = IsBumpmap(Texture->GetFormat());
}

void FOpenGLRenderInterface::SetBitmapTexture(UBitmapMaterial* Bitmap, INT TextureUnit){
	FBaseTexture* Texture = Bitmap->Get(LockedViewport->CurrentTime, LockedViewport)->GetRenderInterface();
	SetTexture(Texture, TextureUnit);
}

void FOpenGLRenderInterface::SetGLRenderTarget(FOpenGLTexture* GLRenderTarget, bool bOwnDepthBuffer){
	checkSlow(GLRenderTarget);

	if(bOwnDepthBuffer) // If the depth is shared with the backbuffer, we need to flip the image
		glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
	else
		glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);

	GLRenderTarget->BindRenderTarget();
	SetViewport(0, 0, GLRenderTarget->Width, GLRenderTarget->Height);

	CurrentState->RenderTarget = GLRenderTarget;
	CurrentState->RenderTargetOwnDepthBuffer = bOwnDepthBuffer;
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

INT FOpenGLRenderInterface::SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams){
	guardFuncSlow;

	FStreamDeclaration VertexStreamDeclarations[MAX_VERTEX_STREAMS];

	// NOTE: Stream declarations must be completely zeroed to get consistent hash values when looking up the VAO later
	appMemzero(VertexStreamDeclarations, sizeof(VertexStreamDeclarations));

	INT Size = 0;

	for(INT i = 0; i < NumStreams; ++i){
		QWORD CacheId = Streams[i]->GetCacheId();
		FOpenGLVertexStream* Stream;

		Stream = static_cast<FOpenGLVertexStream*>(RenDev->GetCachedResource(CacheId));

		if(!Stream)
			Stream = new FOpenGLVertexStream(RenDev, CacheId);

		if(Stream->Revision != Streams[i]->GetRevision()){
			Stream->Cache(Streams[i]);
			Size += Stream->BufferSize;
		}

		CurrentState->VertexStreams[i] = Stream;
		VertexStreamDeclarations[i].Init(Streams[i]);
	}

	CurrentState->NumVertexStreams = NumStreams;
	CurrentState->VertexBufferBase = 0;

	// Look up VAO by format
	// Check if there is an existing VAO for this format by hashing the shader declarations
	FOpenGLVertexArrayObject& VAO = VAOsByDeclId[appMemhash(VertexStreamDeclarations, sizeof(FStreamDeclaration) * NumStreams)];

	if(!VAO.IsValid())
		VAO.Init(VertexStreamDeclarations, NumStreams);

	if(&VAO != CurrentState->VAO){
		RenderState.IndexBuffer = NULL;
		appMemzero(RenderState.VertexStreams, sizeof(RenderState.VertexStreams));
		CurrentState->VAO = &VAO;
	}

	return Size;

	unguardSlow;
}

INT FOpenGLRenderInterface::SetDynamicStream(EVertexShader Shader, FVertexStream* Stream){
	guardFuncSlow;

	FOpenGLVertexStream* OpenGLStream = RenDev->GetDynamicVertexStream();

	CurrentState->NumVertexStreams = 1;
	CurrentState->VertexStreams[0] = OpenGLStream;

	FStreamDeclaration VertexStreamDeclaration;
	appMemzero(&VertexStreamDeclaration, sizeof(VertexStreamDeclaration));

	VertexStreamDeclaration.Init(Stream);

	// Look up VAO by format
	// Check if there is an existing VAO for this format by hashing the shader declarations
	FOpenGLVertexArrayObject& VAO = VAOsByDeclId[appMemhash(&VertexStreamDeclaration, sizeof(FStreamDeclaration))];

	if(!VAO.IsValid())
		VAO.Init(&VertexStreamDeclaration, 1);

	if(&VAO != CurrentState->VAO){
		RenderState.IndexBuffer = NULL;
		appMemzero(RenderState.VertexStreams, sizeof(RenderState.VertexStreams));
		CurrentState->VAO = &VAO;
	}

	CurrentState->VertexBufferBase = OpenGLStream->AddVertices(Stream);

	return 0;

	unguardSlow;
}

INT FOpenGLRenderInterface::SetIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){
	guardFuncSlow;

	bool RequiresCaching = false;

	if(IndexBuffer){
		FOpenGLIndexBuffer* Buffer;
		INT IndexSize = IndexBuffer->GetIndexSize();

		checkSlow(IndexSize == sizeof(_WORD) || IndexSize == sizeof(DWORD));

		QWORD CacheId = IndexBuffer->GetCacheId();

		Buffer = static_cast<FOpenGLIndexBuffer*>(RenDev->GetCachedResource(CacheId));

		if(!Buffer)
			Buffer = new FOpenGLIndexBuffer(RenDev, CacheId);

		if(Buffer->Revision != IndexBuffer->GetRevision() || Buffer->IndexSize != IndexSize){
			RequiresCaching = true;
			Buffer->Cache(IndexBuffer);
		}

		CurrentState->IndexBufferBase = BaseIndex;
		CurrentState->IndexBuffer = Buffer;
	}else{
		CurrentState->IndexBufferBase = 0;
		CurrentState->IndexBuffer = NULL;
	}

	return RequiresCaching ? IndexBuffer->GetSize() : 0;

	unguardSlow;
}

INT FOpenGLRenderInterface::SetDynamicIndexBuffer(FIndexBuffer* IndexBuffer, INT BaseIndex){
	guardFuncSlow;

	FOpenGLIndexBuffer* Buffer = RenDev->GetDynamicIndexBuffer(IndexBuffer->GetIndexSize());

	CurrentState->IndexBufferBase = BaseIndex;
	CurrentState->IndexBuffer = Buffer;

	return Buffer->AddIndices(IndexBuffer);

	unguardSlow;
}

void FOpenGLRenderInterface::DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex, INT MaxIndex){
	guardFuncSlow;

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
		break;
	default:
		appErrorf("Invalid primitive type for FOpenGLRenderInterface::DrawPrimitive: %i", PrimitiveType);
	};

	if(RenderState.IndexBuffer){
		INT IndexSize = RenderState.IndexBuffer->IndexSize;

		glDrawRangeElementsBaseVertex(Mode,
		                              MinIndex,
		                              MaxIndex,
		                              Count,
		                              IndexSize == sizeof(_WORD) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
		                              reinterpret_cast<void*>((FirstIndex + CurrentState->IndexBufferBase) * IndexSize),
		                              CurrentState->VertexBufferBase);
	}else{
		glDrawArrays(Mode, CurrentState->VertexBufferBase + FirstIndex, Count);
	}

	unguardSlow;
}

void FOpenGLRenderInterface::SetFillMode(EFillMode FillMode){
	CurrentState->FillMode = FillMode;
}
