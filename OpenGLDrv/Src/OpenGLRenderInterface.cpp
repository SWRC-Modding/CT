#include "../Inc/OpenGLRenderInterface.h"

#include "../Inc/OpenGLRenderDevice.h"
#include "OpenGLResource.h"
#include "GL/glew.h"

/*
 * FOpenGLRenderInterface
 */

FOpenGLRenderInterface::FOpenGLRenderInterface(UOpenGLRenderDevice* InRenDev) : RenDev(InRenDev),
                                                                                CurrentState(&SavedStates[0]),
                                                                                SavedStateIndex(0),
                                                                                GlobalUBO(GL_NONE){}

void FOpenGLRenderInterface::FlushResources(){
	check(SavedStateIndex == 0);

	CurrentState->IndexBuffer = NULL;
	CurrentState->IndexBufferBaseIndex = 0;

	if(DynamicIndexBuffer16){
		delete DynamicIndexBuffer16;
		DynamicIndexBuffer16 = NULL;
	}

	if(DynamicIndexBuffer32){
		delete DynamicIndexBuffer32;
		DynamicIndexBuffer32 = NULL;
	}

	if(DynamicVertexStream){
		delete DynamicVertexStream;
		DynamicVertexStream = NULL;
	}

	for(TMap<DWORD, unsigned int>::TIterator It(VAOsByDeclId); It; ++It){
		if(It.Value())
			glDeleteVertexArrays(1, &It.Value());
	}

	VAOsByDeclId.Empty();
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

void FOpenGLRenderInterface::PushState(int){
	guardFunc;

	++SavedStateIndex;
	++CurrentState;

	check(SavedStateIndex < MAX_STATESTACKDEPTH);

	appMemcpy(CurrentState, &SavedStates[SavedStateIndex - 1], sizeof(FOpenGLSavedState));

	unguard;
}

void FOpenGLRenderInterface::PopState(int){
	guardFunc;

	FOpenGLSavedState* PrevState = CurrentState;

	--SavedStateIndex;
	--CurrentState;

	check(SavedStateIndex >= 0);

	// if(CurrentState->RenderTarget != PrevState->RenderTarget){
	// 	if(CurrentState->RenderTarget)
	// 		CurrentState->RenderTarget->Bind();
	// 	else
	// 		glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	// }

	if(CurrentState->ViewportX != PrevState->ViewportX ||
	   CurrentState->ViewportY != PrevState->ViewportY ||
	   CurrentState->ViewportWidth != PrevState->ViewportWidth ||
	   CurrentState->ViewportHeight != PrevState->ViewportHeight){
		SetViewport(CurrentState->ViewportX, CurrentState->ViewportY, CurrentState->ViewportWidth, CurrentState->ViewportHeight);
	}

	if(CurrentState->CullMode != PrevState->CullMode)
		SetCullMode(CurrentState->CullMode);

	if(CurrentState->bStencilTest != PrevState->bStencilTest)
		EnableStencilTest(CurrentState->bStencilTest);

	if(CurrentState->bZWrite != PrevState->bZWrite)
		EnableZWrite(CurrentState->bZWrite);

	if(CurrentState->IndexBuffer != PrevState->IndexBuffer){
		if(CurrentState->IndexBuffer)
			CurrentState->IndexBuffer->Bind();
		else
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);
	}

	NeedUniformUpdate = CurrentState->UniformRevision != PrevState->UniformRevision;

	unguard;
}

UBOOL FOpenGLRenderInterface::SetRenderTarget(FRenderTarget* RenderTarget, bool){
	guardFunc;

	// bool Updated = false;
	// QWORD CacheId = RenderTarget->GetCacheId();
	// INT Revision = RenderTarget->GetRevision();
	// FOpenGLRenderTarget* NewRenderTarget = static_cast<FOpenGLRenderTarget*>(RenDev->GetCachedResource(CacheId));

	// if(!NewRenderTarget)
	// 	NewRenderTarget = new FOpenGLRenderTarget(RenDev, CacheId);

	// if(NewRenderTarget->Revision != Revision){
	// 	Updated = true;
	// 	NewRenderTarget->Cache(RenderTarget);
	// 	NewRenderTarget->Revision = Revision;
	// }

	// if(CurrentState->RenderTarget != NewRenderTarget || Updated)
	// 	NewRenderTarget->Bind();

	// CurrentState->RenderTarget = NewRenderTarget;

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

void FOpenGLRenderInterface::EnableStencilTest(UBOOL Enable){
	CurrentState->bStencilTest = Enable;

	if(Enable)
		glEnable(GL_STENCIL_TEST);
	else
		glDisable(GL_STENCIL_TEST);
}

void FOpenGLRenderInterface::EnableZWrite(UBOOL Enable){
	CurrentState->bZWrite = Enable;
	glDepthMask(Enable ? GL_TRUE : GL_FALSE);
}

INT FOpenGLRenderInterface::SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams, bool IsDynamic){
	PRINT_FUNC;

	checkSlow(!IsDynamic || NumStreams == 1);

	// NOTE: Stream declarations must be completely zeroed to get consistent hash values when looking up the VAO later
	appMemzero(VertexStreamDeclarations, sizeof(VertexStreamDeclarations));

	// Unbind previous vertex buffers
	for(INT i = 0; i < NumVertexStreams; ++i)
		glBindVertexBuffer(i, GL_NONE, 0, 0);

	NumVertexStreams = NumStreams;

	INT Size = 0;

	for(INT i = 0; i < NumStreams; ++i){
		QWORD CacheId = Streams[i]->GetCacheId();

		FOpenGLVertexStream* Stream;

		if(IsDynamic){
			if(!DynamicVertexStream)
				DynamicVertexStream = new FOpenGLVertexStream(RenDev, MakeCacheID(CID_RenderVertices), true);

			Stream = DynamicVertexStream;
		}else{
			Stream = static_cast<FOpenGLVertexStream*>(RenDev->GetCachedResource(CacheId));

			if(!Stream)
				Stream = new FOpenGLVertexStream(RenDev, CacheId);
		}

		if(IsDynamic || Stream->Revision != Streams[i]->GetRevision()){
			Stream->Cache(Streams[i]);
			Stream->Revision = Streams[i]->GetRevision();
			Size += Stream->DynamicSize;
		}

		VertexStreams[i] = Stream;
		VertexStreamDeclarations[i].Init(Streams[i]);
	}

	// Check if there is an existing VAO for this format by hashing the shader declarations
	GLuint& VAO = VAOsByDeclId[appMemCrc(VertexStreamDeclarations, sizeof(VertexStreamDeclarations))];

	// Create and setup VAO if none was found matching the vertex
	if(!VAO){
		glCreateVertexArrays(1, &VAO);

		for(INT StreamIndex = 0; StreamIndex < NumVertexStreams; ++StreamIndex){
			const FStreamDeclaration& Decl = VertexStreamDeclarations[StreamIndex];
			GLuint Offset = 0;

			glEnableVertexArrayAttrib(VAO, StreamIndex);

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
					glVertexArrayAttribFormat(VAO, Function, 4, GL_UNSIGNED_BYTE, GL_TRUE, Offset);
					Offset += sizeof(FColor);
					break;
				default:
					appErrorf("Unexpected EComponentType (%i)", Type);
				}

				glVertexArrayAttribBinding(VAO, Function, StreamIndex);
			}
		}
	}

	glBindVertexArray(VAO);

	NeedUniformUpdate = 1; // Uniform buffer state is also stored in the VAO so we need to update it

	// The element array buffer state is stored in the vao so we need to rebind it since the newly bound vao might have a different one or none at all
	if(CurrentState->IndexBuffer)
		CurrentState->IndexBuffer->Bind();

	for(INT i = 0; i < NumVertexStreams; ++i)
		VertexStreams[i]->Bind(i);

	RenDev->DefaultShader->Bind();

	return 0;
}

INT FOpenGLRenderInterface::SetVertexStreams(EVertexShader Shader, FVertexStream** Streams, INT NumStreams){
	PRINT_FUNC;
	return SetVertexStreams(Shader, Streams, NumStreams, false);
}

INT FOpenGLRenderInterface::SetDynamicStream(EVertexShader Shader, FVertexStream* Stream){
	PRINT_FUNC;
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
			if(IndexSize == sizeof(DWORD)){
				if(!DynamicIndexBuffer32)
					DynamicIndexBuffer32 = new FOpenGLIndexBuffer(RenDev, MakeCacheID(CID_RenderIndices), true);

				Buffer = DynamicIndexBuffer32;
			}else{
				if(!DynamicIndexBuffer16)
					DynamicIndexBuffer16 = new FOpenGLIndexBuffer(RenDev, MakeCacheID(CID_RenderIndices), true);

				Buffer = DynamicIndexBuffer16;
			}
		}else{
			QWORD CacheId = IndexBuffer->GetCacheId();

			Buffer = static_cast<FOpenGLIndexBuffer*>(RenDev->GetCachedResource(CacheId));

			if(!Buffer)
				Buffer = new FOpenGLIndexBuffer(RenDev, CacheId);
		}

		if(IsDynamic || Buffer->Revision != IndexBuffer->GetRevision() || Buffer->IndexSize != IndexSize){
			RequiresCaching = true;
			Buffer->Cache(IndexBuffer);
			Buffer->Revision = IndexBuffer->GetRevision();
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
	PRINT_FUNC;

	if(NeedUniformUpdate)
		UpdateShaderUniforms();

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
