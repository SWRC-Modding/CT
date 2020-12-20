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

	static INT Count = 0;

	SetShader(&RenDev->FixedFunctionShader);

	if(Count == 0){
		ZWrite = true;
		ZTest = true;
		AlphaTest = false;
		TwoSided = false;
		AlphaRef = 0.0f;
		TexMatrix = FMatrix::Identity;
		FramebufferBlending = FB_Overwrite;
		ModifyFramebufferBlending = false;
	}

	if(!Material)
		Material = static_cast<UMaterial*>(UMaterial::StaticClass()->GetDefaultObject());

	if(CheckMaterial<UShader>(Material)){
		++Count;
		SetMaterial(static_cast<UShader*>(Material)->Diffuse);
		--Count;
	}else if(CheckMaterial<UCombiner>(Material)){
		INT T = 0;
		SetCombinerMaterial(static_cast<UCombiner*>(Material), T, FSU_MaterialColor);
	}else if(CheckMaterial<UBitmapMaterial>(Material) ||
	         CheckMaterial<UVertexColor>(Material) ||
	         CheckMaterial<UConstantMaterial>(Material)){
		INT T = 0;
		SetSimpleMaterial(Material, T, FSU_DiffuseColor);
	}else if(CheckMaterial<UTerrainMaterial>(Material)){
		++Count;
		SetMaterial(static_cast<UTerrainMaterial*>(Material)->Layers[0].Texture);
		--Count;
	}else if(CheckMaterial<UParticleMaterial>(Material)){
		ModifyFramebufferBlending = true;
		AlphaTest = true;
		FramebufferBlending = FB_AlphaBlend;
		++Count;
		SetMaterial(static_cast<UParticleMaterial*>(Material)->BitmapMaterial);
		--Count;
	}else if(CheckMaterial<UProjectorMultiMaterial>(Material)){
		++Count;
		SetMaterial(static_cast<UProjectorMultiMaterial*>(Material)->BaseMaterial);
		--Count;
	}else if(CheckMaterial<UProjectorMaterial>(Material)){
		++Count;
		SetMaterial(static_cast<UProjectorMaterial*>(Material)->Projected);
		--Count;
	}else if(CheckMaterial<UHardwareShaderWrapper>(Material)){
		++Count;
		static_cast<UHardwareShaderWrapper*>(Material)->SetupShaderWrapper(this);
		SetMaterial(static_cast<UHardwareShaderWrapper*>(Material)->ShaderImplementation);
		--Count;
	}else if(CheckMaterial<UHardwareShader>(Material)){
		AlphaTest = static_cast<UHardwareShader*>(Material)->AlphaTest != 0;
		AlphaRef = static_cast<UHardwareShader*>(Material)->AlphaRef;

		if(static_cast<UHardwareShader*>(Material)->AlphaBlending)
			FramebufferBlending = FB_AlphaBlend;

		ModifyFramebufferBlending = true;

		++Count;
		SetMaterial(static_cast<UHardwareShader*>(Material)->Textures[0]);
		--Count;
	}

	unguard;
}

void FOpenGLRenderInterface::SetSimpleMaterial(UMaterial* Material, INT& TextureUnit, EFixedFunctionFragmentShaderSubroutineUniform ShaderSubroutine){
	if(Material->IsA<UVertexColor>()){
		CurrentState->Shader->FragmentShaderSubroutines[ShaderSubroutine] = FSI_VertexColor;
		NeedShaderSubroutineUpdate = 1;
	}else if(Material->IsA<UConstantMaterial>()){
		FPlane Color = static_cast<UConstantMaterial*>(Material)->GetColor(GEngineTime);

		CurrentState->Shader->SetUniformVec4(1, Color);
		CurrentState->Shader->FragmentShaderSubroutines[ShaderSubroutine] = FSI_ConstantColor;
		NeedShaderSubroutineUpdate = 1;
	}else if(Material->IsA<UBitmapMaterial>()){
		check(TextureUnit < MAX_TEXTURES);

		UBitmapMaterial* Bitmap = static_cast<UBitmapMaterial*>(Material);
		FBaseTexture* BaseTex = Bitmap->GetRenderInterface();
		FTexture* Tex = BaseTex ? BaseTex->GetTextureInterface() : NULL;
		FRenderTarget* RenderTarget = BaseTex ? BaseTex->GetRenderTargetInterface() : NULL;

		if(RenderTarget){
			FOpenGLRenderTarget* GLRenderTarget = static_cast<FOpenGLRenderTarget*>(RenDev->GetCachedResource(RenderTarget->GetCacheId()));

			checkSlow(GLRenderTarget);

			glBindTextureUnit(TextureUnit, GLRenderTarget->ColorAttachment);
		}else{
			if(!Tex){
				static FSolidColorTexture DefaultTexture(FColor(255, 0, 255));

				Tex = &DefaultTexture;
			}

			UTexture* Texture = Cast<UTexture>(Bitmap);

			if(Texture && !ModifyFramebufferBlending){
				if(Texture->bMasked){
					ModifyFramebufferBlending = true;
					FramebufferBlending = FB_Overwrite;
					ZTest = true;
					ZWrite = true;
					AlphaTest = true;
					AlphaRef = 0.5f;
				}else if(Texture->bAlphaTexture){
					ModifyFramebufferBlending = true;
					FramebufferBlending = FB_AlphaBlend;
					ZTest = true;
					ZWrite = true;
					AlphaTest = true;
					AlphaRef = 0.0f;
				}

				TwoSided = Texture->bTwoSided != 0;
			}

			QWORD CacheId = Tex->GetCacheId();
			FOpenGLTexture* GLTexture = static_cast<FOpenGLTexture*>(RenDev->GetCachedResource(CacheId));

			if(!GLTexture)
				GLTexture = new FOpenGLTexture(RenDev, CacheId);

			if(GLTexture->Revision != Tex->GetRevision())
				GLTexture->Cache(Tex);

			GLTexture->Bind(TextureUnit);
		}

		CurrentState->Shader->FragmentShaderSubroutines[ShaderSubroutine] = FSI_Texture0Color + TextureUnit;
		NeedShaderSubroutineUpdate = 1;
		++TextureUnit;
	}else{
		appErrorf("Unexpected material type: '%s'", *Material->GetClass()->FriendlyName);
	}
}

void FOpenGLRenderInterface::SetCombinerMaterial(UCombiner* Combiner, INT& TextureUnit, EFixedFunctionFragmentShaderSubroutineUniform ShaderSubroutine){
	UMaterial* Material1 = Combiner->Material1;
	UMaterial* Material2 = Combiner->Material2;
	UMaterial* Mask = Combiner->Mask;

	if(!Material1)
		appErrorf("Material1 not set for combiner '%s'", Combiner->GetFullName());

	if(!Material2)
		Material2 = GetDefault<UMaterial>()->DefaultMaterial;

	if(!Mask)
		Mask = GetDefault<UMaterial>()->DefaultMaterial;

	// Applying Modifiers
	verify(CheckMaterial<URenderedMaterial>(Material1));
	verify(CheckMaterial<URenderedMaterial>(Material2));
	verify(CheckMaterial<URenderedMaterial>(Mask));

	SetSimpleMaterial(Material1, TextureUnit, FSU_CombinerMaterial1);
	SetSimpleMaterial(Material2, TextureUnit, FSU_CombinerMaterial2);
	SetSimpleMaterial(Mask, TextureUnit, FSU_CombinerMaskColor);

	CurrentState->Shader->FragmentShaderSubroutines[FSU_CombinerMask] = Combiner->InvertMask ? FSI_CombinerMaskInverted : FSI_CombinerMaskDefault;
	CurrentState->Shader->FragmentShaderSubroutines[ShaderSubroutine] = FSI_CombinerMain;
	NeedShaderSubroutineUpdate = 1;
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

	SetMaterialBlending();

	if(NeedShaderSubroutineUpdate)
		CurrentState->Shader->UpdateSubroutines();

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

	if(NewShader == &RenDev->FixedFunctionShader){
		// Init default shader subroutines
		CurrentState->Shader->FragmentShaderSubroutines[FSU_Texture0Coord]     = FSI_TexCoord0;
		CurrentState->Shader->FragmentShaderSubroutines[FSU_Texture1Coord]     = FSI_TexCoord0;
		CurrentState->Shader->FragmentShaderSubroutines[FSU_Texture2Coord]     = FSI_TexCoord0;
		CurrentState->Shader->FragmentShaderSubroutines[FSU_Texture3Coord]     = FSI_TexCoord0;
		CurrentState->Shader->FragmentShaderSubroutines[FSU_Texture4Coord]     = FSI_TexCoord0;
		CurrentState->Shader->FragmentShaderSubroutines[FSU_Texture5Coord]     = FSI_TexCoord0;
		CurrentState->Shader->FragmentShaderSubroutines[FSU_Texture6Coord]     = FSI_TexCoord0;
		CurrentState->Shader->FragmentShaderSubroutines[FSU_Texture7Coord]     = FSI_TexCoord0;
		CurrentState->Shader->FragmentShaderSubroutines[FSU_DiffuseColor]      = FSI_ConstantColor;
		CurrentState->Shader->FragmentShaderSubroutines[FSU_CombinerMaterial1] = FSI_ConstantColor;
		CurrentState->Shader->FragmentShaderSubroutines[FSU_CombinerMaterial2] = FSI_ConstantColor;
		CurrentState->Shader->FragmentShaderSubroutines[FSU_CombinerMaskColor] = FSI_ConstantColor;
		CurrentState->Shader->FragmentShaderSubroutines[FSU_CombinerMask]      = FSI_CombinerMaskDefault;
		CurrentState->Shader->FragmentShaderSubroutines[FSU_MaterialColor]     = FSI_DefaultMaterialColor;
		CurrentState->Shader->FragmentShaderSubroutines[FSU_FinalColor]        = FSI_FinalDiffuseColor;
		UsingFixedFunctionShader = 1;
	}else{
		UsingFixedFunctionShader = 0;
	}
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

void FOpenGLRenderInterface::SetMaterialBlending(){
	if(TwoSided)
		SetCullMode(CM_None);

	EnableZWrite(ZWrite);
	EnableZTest(ZTest);
	SetFramebufferBlending(FramebufferBlending);

	if(UsingFixedFunctionShader){
		if(AlphaTest){
			CurrentState->Shader->SetUniformFloat(0, AlphaRef);
			CurrentState->Shader->FragmentShaderSubroutines[FSU_FinalColor] = FSI_FinalDiffuseColorAlphaTest;
		}else{
			CurrentState->Shader->FragmentShaderSubroutines[FSU_FinalColor] = FSI_FinalDiffuseColor;
		}

		NeedShaderSubroutineUpdate = 1;
	}else{
		NeedShaderSubroutineUpdate = 0;
	}
}
