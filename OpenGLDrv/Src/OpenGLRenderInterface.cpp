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
			CurrentState->RenderTarget->Bind();

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

	unguard;
}

UBOOL FOpenGLRenderInterface::SetRenderTarget(FRenderTarget* RenderTarget, bool bFSAA){
	guardFunc;

	bool Updated = false;
	QWORD CacheId = RenderTarget->GetCacheId();
	INT Revision = RenderTarget->GetRevision();
	FOpenGLRenderTarget* NewRenderTarget = static_cast<FOpenGLRenderTarget*>(RenDev->GetCachedResource(CacheId));

	if(!NewRenderTarget)
		NewRenderTarget = new FOpenGLRenderTarget(RenDev, CacheId);

	if(NewRenderTarget->Revision != Revision){
		Updated = true;
		NewRenderTarget->Cache(RenderTarget);
	}

	if(CurrentState->RenderTarget != NewRenderTarget || Updated)
		NewRenderTarget->Bind();

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
	static INT Count = 0;

	if(Count == 0){
		ModifierInfo.ZWrite = true;
		ModifierInfo.ZTest = true;
		ModifierInfo.AlphaTest = true;
		ModifierInfo.TwoSided = false;
		ModifierInfo.Matrix = FMatrix::Identity;
		ModifierInfo.Blending = FB_Overwrite;
	}

	if(!Material)
		Material = static_cast<UMaterial*>(UMaterial::StaticClass()->GetDefaultObject());

	if(CheckMaterial<UShader>(Material, &ModifierInfo)){
		++Count;
		SetMaterial(static_cast<UShader*>(Material)->Diffuse);
		--Count;
	}else if(CheckMaterial<UCombiner>(Material, &ModifierInfo)){
		++Count;
		SetMaterial(static_cast<UCombiner*>(Material)->Material1);
		--Count;
	}else if(CheckMaterial<UConstantMaterial>(Material, &ModifierInfo)){
		++Count;
		SetMaterial(static_cast<UMaterial*>(UMaterial::StaticClass()->GetDefaultObject()));
		--Count;
	}else if(CheckMaterial<UBitmapMaterial>(Material, &ModifierInfo)){
		UBitmapMaterial* Bitmap = static_cast<UBitmapMaterial*>(Material);
		FBaseTexture* BaseTex = Bitmap->GetRenderInterface();
		FTexture* Tex = BaseTex ? BaseTex->GetTextureInterface() : NULL;

		FRenderTarget* RenderTarget = BaseTex ? BaseTex->GetRenderTargetInterface() : NULL;

		if(RenderTarget){
			FOpenGLRenderTarget* GLRenderTarget = static_cast<FOpenGLRenderTarget*>(RenDev->GetCachedResource(RenderTarget->GetCacheId()));

			checkSlow(GLRenderTarget);

			glBindTextureUnit(0, GLRenderTarget->ColorAttachment);

			return;
		}

		if(!Tex){
			++Count;
			SetMaterial(static_cast<UMaterial*>(UMaterial::StaticClass()->GetDefaultObject()));
			--Count;

			return;
		}

		UTexture* Texture = Cast<UTexture>(Bitmap);

		if(Texture){
			if(Texture->bAlphaTexture || Texture->bMasked)
				ModifierInfo.Blending = FB_AlphaBlend;

			if(Texture->bTwoSided)
				ModifierInfo.TwoSided = true;
		}

		QWORD CacheId = Tex->GetCacheId();
		FOpenGLTexture* GLTexture = static_cast<FOpenGLTexture*>(RenDev->GetCachedResource(CacheId));

		if(!GLTexture)
			GLTexture = new FOpenGLTexture(RenDev, CacheId);

		if(GLTexture->Revision != Tex->GetRevision())
			GLTexture->Cache(Tex);

		GLTexture->Bind(0);
	}else if(CheckMaterial<UTerrainMaterial>(Material, &ModifierInfo)){
		++Count;
		SetMaterial(static_cast<UTerrainMaterial*>(Material)->Layers[0].Texture);
		--Count;
	}else if(CheckMaterial<UParticleMaterial>(Material, &ModifierInfo)){
		ModifierInfo.Blending = FB_AlphaBlend;
		++Count;
		SetMaterial(static_cast<UParticleMaterial*>(Material)->BitmapMaterial);
		--Count;
	}else if(CheckMaterial<UProjectorMultiMaterial>(Material, &ModifierInfo)){
		++Count;
		SetMaterial(static_cast<UProjectorMultiMaterial*>(Material)->BaseMaterial);
		--Count;
	}else if(CheckMaterial<UProjectorMaterial>(Material, &ModifierInfo)){
		++Count;
		SetMaterial(static_cast<UProjectorMaterial*>(Material)->Projected);
		--Count;
	}else if(CheckMaterial<UHardwareShaderWrapper>(Material, &ModifierInfo)){
		++Count;
		SetMaterial(static_cast<UHardwareShaderWrapper*>(Material)->ShaderImplementation);
		--Count;
	}else if(CheckMaterial<UHardwareShader>(Material, &ModifierInfo)){
		++Count;
		SetMaterial(static_cast<UHardwareShader*>(Material)->Textures[0]);
		--Count;
	}
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
	CurrentState->bStencilTest = Enable;
	glStencilMask(Enable ? 0xFF : 0x00);
}

void FOpenGLRenderInterface::EnableZWrite(UBOOL Enable){
	CurrentState->bZWrite = Enable;
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

	SetMaterialBlending(&ModifierInfo);

	GLenum Mode = 0;
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
	CurrentState->bZTest = Enable;
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

void FOpenGLRenderInterface::SetFramebufferBlending(EFrameBufferBlending Blending){
	CurrentState->FramebufferBlending = Blending;

	if(Blending == FB_AlphaBlend)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else if(Blending == FB_Invisible)
		glBlendFunc(GL_ZERO, GL_ONE);
	else
		glBlendFunc(GL_ONE, GL_ZERO);
}

void FOpenGLRenderInterface::SetupPerFrameShaderConstants(){
	FLOAT Time = appFmod(static_cast<FLOAT>(appSeconds()), 120.0f);

	CurrentState->Uniforms.Time = Time;
	CurrentState->Uniforms.SinTime = appSin(Time);
	CurrentState->Uniforms.CosTime = appCos(Time);
	CurrentState->Uniforms.TanTime = appTan(Time);
}
