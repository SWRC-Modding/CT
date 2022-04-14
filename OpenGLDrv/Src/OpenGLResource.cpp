#include "OpenGLResource.h"

#include "../Inc/OpenGLRenderDevice.h"
#include "../Inc/opengl.h"

FOpenGLResource::FOpenGLResource(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : RenDev(InRenDev),
                                                                                   CacheId(InCacheId),
                                                                                   Revision(-1),
                                                                                   HashIndex(INDEX_NONE),
                                                                                   HashNext(NULL){
	RenDev->AddResource(this);
}

FOpenGLResource::~FOpenGLResource(){
	RenDev->RemoveResource(this);
}

// FOpenGLShader

FOpenGLShader::FOpenGLShader(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : FOpenGLResource(InRenDev, InCacheId),
                                                                                               Program(GL_NONE),
                                                                                               IsErrorShader(0){}

FOpenGLShader::~FOpenGLShader(){
	if(Program)
		RenDev->glDeleteProgram(Program);
}

void FOpenGLShader::Cache(FShaderGLSL* Shader){
	FString ShaderCode = Shader->GetShaderCode();

	RenDev->ExpandShaderMacros(&ShaderCode);

	GLuint VertexShader = CompileShader(GL_VERTEX_SHADER, ShaderCode, Shader->GetName() + SHADER_FILE_EXTENSION + " - vertex shader");
	GLuint FragmentShader = CompileShader(GL_FRAGMENT_SHADER, ShaderCode, Shader->GetName() + SHADER_FILE_EXTENSION + " - fragment shader");

	// Set revision even if compilation is unsuccessful to avoid recompiling the invalid shader each time it is set
	Revision = Shader->GetRevision();

	if(!VertexShader || !FragmentShader){
		if(VertexShader)
			RenDev->glDeleteShader(VertexShader);

		if(FragmentShader)
			RenDev->glDeleteShader(FragmentShader);

		if(Program){
			RenDev->glDeleteProgram(Program);
			Program = GL_NONE;
		}

		return;
	}

	GLuint NewProgram = RenDev->glCreateProgram();

	RenDev->glAttachShader(NewProgram, VertexShader);
	RenDev->glAttachShader(NewProgram, FragmentShader);
	RenDev->glLinkProgram(NewProgram);
	RenDev->glDetachShader(NewProgram, VertexShader);
	RenDev->glDetachShader(NewProgram, FragmentShader);
	RenDev->glDeleteShader(VertexShader);
	RenDev->glDeleteShader(FragmentShader);

	GLint Status;

	RenDev->glGetProgramiv(NewProgram, GL_LINK_STATUS, &Status);

	if(Status){
		if(Program)
			RenDev->glDeleteProgram(Program);

		Program = NewProgram;
		IsErrorShader = 0;
	}else{
		GLchar Buffer[512];

		RenDev->glGetProgramInfoLog(NewProgram, ARRAY_COUNT(Buffer), NULL, Buffer);
		debugf("Shader program linking failed for %s: %s", Shader->GetName(), Buffer);
		RenDev->glDeleteProgram(NewProgram);
	}
}

void FOpenGLShader::Bind(){
	if(!Program){
		INT ErrorRevision = Revision;

		Cache(&UOpenGLRenderDevice::ErrorShader);
		IsErrorShader = 1;
		Revision = ErrorRevision;
	}

	checkSlow(Program);
	RenDev->glUseProgram(Program);
}

GLuint FOpenGLShader::CompileShader(GLenum Type, const FString& ShaderCode, const FString& ShaderName){
	GLuint Handle = RenDev->glCreateShader(Type);
	const TCHAR* ShaderVars = NULL;

	if(Type == GL_VERTEX_SHADER)
		ShaderVars = *RenDev->VertexShaderVarsText;
	else if(Type == GL_FRAGMENT_SHADER)
		ShaderVars = *RenDev->FragmentShaderVarsText;
	else
		appErrorf("Unsupported shader type (%i)", Type);

	const TCHAR* ShaderText[] = {
		ShaderVars,
		"#line 1\n",
		*ShaderCode
	};

	RenDev->glShaderSource(Handle, ARRAY_COUNT(ShaderText), ShaderText, NULL);
	RenDev->glCompileShader(Handle);

	GLint Status;

	RenDev->glGetShaderiv(Handle, GL_COMPILE_STATUS, &Status);

	if(!Status){
		GLchar Buffer[512];

		RenDev->glGetShaderInfoLog(Handle, ARRAY_COUNT(Buffer), NULL, Buffer);
		debugf("Shader compilation failed for %s: %s", *ShaderName, Buffer);
		RenDev->glDeleteShader(Handle);
		Handle = GL_NONE;
	}

	return Handle;
}

// FOpenGLIndexBuffer

#define INITIAL_DYNAMIC_INDEX_BUFFER_SIZE 32768

FOpenGLIndexBuffer::FOpenGLIndexBuffer(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, bool InIsDynamic) : FOpenGLResource(InRenDev, InCacheId),
                                                                                                           EBO(GL_NONE),
                                                                                                           IndexSize(0),
                                                                                                           BufferSize(0),
                                                                                                           IsDynamic(InIsDynamic){}

FOpenGLIndexBuffer::~FOpenGLIndexBuffer(){
	if(EBO){
		RenDev->glDeleteBuffers(1, &EBO);
		EBO = GL_NONE;
	}
}

void FOpenGLIndexBuffer::Cache(FIndexBuffer* IndexBuffer, INT DynamicBufferSize){
	GLuint OldEBO = EBO;
	RenDev->glCreateBuffers(1, &EBO);

	INT IndexBufferSize = IndexBuffer->GetSize();
	void* Data = RenDev->GetScratchBuffer(IndexBufferSize);

	IndexBuffer->GetContents(Data);

	if(IsDynamic){
		BufferSize = Max(IndexBufferSize, DynamicBufferSize);

		if(DynamicBufferSize > 0)
			debugf("Allocating %ikb byte dynamic index buffer", BufferSize / 1024);

		RenDev->glNamedBufferStorage(EBO, BufferSize, NULL, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
		RenDev->glNamedBufferSubData(EBO, 0, IndexBufferSize, Data);
	}else{
		BufferSize = IndexBufferSize;
		RenDev->glNamedBufferStorage(EBO, IndexBufferSize, Data, 0);
	}

	Revision = IndexBuffer->GetRevision();
	IndexSize = IndexBuffer->GetIndexSize();
	Tail = IndexBufferSize;

	if(OldEBO)
		RenDev->glDeleteBuffers(1, &OldEBO);
}

INT FOpenGLIndexBuffer::AddIndices(FIndexBuffer* IndexBuffer){
	checkSlow(IsDynamic);

	INT AdditionalSize = IndexBuffer->GetSize();

	if(EBO == GL_NONE || AdditionalSize > BufferSize){
		Cache(IndexBuffer, Max(AdditionalSize * 2, INITIAL_DYNAMIC_INDEX_BUFFER_SIZE));

		return 0;
	}

	IndexSize = IndexBuffer->GetIndexSize();

	INT IndexBufferOffset = Align(Tail, IndexSize);
	GLbitfield MapFlags = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;

	if(IndexBufferOffset + AdditionalSize > BufferSize){
		IndexBufferOffset = 0;
		MapFlags |= GL_MAP_INVALIDATE_BUFFER_BIT;
	}

	void* Data = RenDev->glMapNamedBufferRange(EBO, IndexBufferOffset, AdditionalSize, MapFlags);

	IndexBuffer->GetContents(Data);
	RenDev->glUnmapNamedBuffer(EBO);

	Tail = IndexBufferOffset + AdditionalSize;

	return IndexBufferOffset / IndexSize;
}

// FOpenGLVertexStream

#define INITIAL_DYNAMIC_VERTEX_BUFFER_SIZE 131072

FOpenGLVertexStream::FOpenGLVertexStream(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, bool InIsDynamic) : FOpenGLResource(InRenDev, InCacheId),
                                                                                                             VBO(GL_NONE),
                                                                                                             Stride(0),
                                                                                                             BufferSize(0),
                                                                                                             IsDynamic(InIsDynamic){}

FOpenGLVertexStream::~FOpenGLVertexStream(){
	if(VBO){
		RenDev->glDeleteBuffers(1, &VBO);
		VBO = GL_NONE;
	}
}

void FOpenGLVertexStream::Cache(FVertexStream* VertexStream, INT DynamicBufferSize){
	GLuint OldVBO = VBO;
	RenDev->glCreateBuffers(1, &VBO);

	INT StreamSize = VertexStream->GetSize();
	void* Data = NULL;

	VertexStream->GetRawStreamData(&Data, 0);

	if(!Data){
		Data = RenDev->GetScratchBuffer(StreamSize);
		VertexStream->GetStreamData(Data);
	}

	IsDynamic |= VertexStream->HintDynamic() != 0;

	if(IsDynamic){
		BufferSize = Max(StreamSize, DynamicBufferSize);

		if(DynamicBufferSize > 0)
			debugf("Allocating %ikb dynamic vertex buffer", BufferSize / 1024);

		RenDev->glNamedBufferStorage(VBO, BufferSize, NULL, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
		RenDev->glNamedBufferSubData(VBO, 0, StreamSize, Data);
	}else{
		BufferSize = StreamSize;
		RenDev->glNamedBufferStorage(VBO, BufferSize, Data, 0);
	}

	Revision = VertexStream->GetRevision();
	Stride = VertexStream->GetStride();
	Tail = StreamSize;

	if(OldVBO)
		RenDev->glDeleteBuffers(1, &OldVBO);
}

INT FOpenGLVertexStream::AddVertices(FVertexStream* VertexStream){
	checkSlow(IsDynamic);

	INT AdditionalSize = VertexStream->GetSize();

	if(VBO == GL_NONE || AdditionalSize > BufferSize){
		Cache(VertexStream, Max(AdditionalSize * 2, INITIAL_DYNAMIC_VERTEX_BUFFER_SIZE));

		return 0;
	}

	Stride = VertexStream->GetStride();

	INT VertexBufferOffset = ((Tail + Stride - 1) / Stride) * Stride;
	GLbitfield MapFlags = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;

	if(VertexBufferOffset + AdditionalSize > BufferSize){
		VertexBufferOffset = 0;
		MapFlags |= GL_MAP_INVALIDATE_BUFFER_BIT;
	}

	void* Data = RenDev->glMapNamedBufferRange(VBO, VertexBufferOffset, AdditionalSize, MapFlags);

	VertexStream->GetStreamData(Data);
	RenDev->glUnmapNamedBuffer(VBO);

	Tail = VertexBufferOffset + AdditionalSize;

	return VertexBufferOffset / Stride;
}

// FOpenGLTexture

FSolidColorTexture FOpenGLTexture::ErrorTexture(FColor(255, 0, 255));

FOpenGLTexture::FOpenGLTexture(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : FOpenGLResource(InRenDev, InCacheId),
                                                                                 Width(0),
                                                                                 Height(0),
                                                                                 TextureHandle(GL_NONE),
                                                                                 FBO(GL_NONE),
                                                                                 DepthStencilAttachment(GL_NONE),
                                                                                 IsCubemap(false),
                                                                                 HasSharedDepthStencil(false){}

FOpenGLTexture::~FOpenGLTexture(){
	Free();
}

void FOpenGLTexture::Cache(FBaseTexture* BaseTexture, bool bOwnDepthBuffer){
	Free();

	FRenderTarget* RenderTarget = BaseTexture->GetRenderTargetInterface();
	FCubemap* Cubemap = BaseTexture->GetCubemapInterface();
	FCompositeTexture* CompositeTexture = BaseTexture->GetCompositeTextureInterface();
	FTexture* Texture = BaseTexture->GetTextureInterface();
	ETextureFormat SrcFormat = BaseTexture->GetFormat();
	ETextureFormat DestFormat = IsDXTC(SrcFormat) ? SrcFormat : TEXF_RGBA8;

	Width = BaseTexture->GetWidth();
	Height = BaseTexture->GetHeight();
	IsCubemap = false;

	if(RenderTarget){
		if(Width == 0 || Height == 0)
			return;

		// NOTE: bOwnDepthBuffer is also used here to determine whether to use the same format for the target as the backbuffer.
		//       While not technically correct, it works the way it is supposed to.

		if(Width != RenDev->Backbuffer.GetWidth() || Height != RenDev->Backbuffer.GetHeight())
			bOwnDepthBuffer = true;

		if(bOwnDepthBuffer){
			bool IsMipTarget = false;

			for(INT i = 0; i < UFrameFX::MaxMips; ++i){
				if(RenderTarget == UFrameFX::MipTargets[i]){
					IsMipTarget = true;
					break;
				}
			}

			if(IsMipTarget){
				DepthStencilAttachment = GL_NONE; // The FrameFX mip targets don't need a depth or stencil buffer
			}else{
				RenDev->glCreateRenderbuffers(1, &DepthStencilAttachment);
				RenDev->glNamedRenderbufferStorage(DepthStencilAttachment, GL_DEPTH24_STENCIL8, Width, Height);
				HasSharedDepthStencil = false;
			}
		}else{
			checkSlow(RenDev->BackbufferDepthStencil);
			DepthStencilAttachment = RenDev->BackbufferDepthStencil;
			HasSharedDepthStencil = true;
		}

		GLenum Format;

		if(bOwnDepthBuffer)
			Format = RenDev->Use16bit ? GL_RGB565 : GL_RGB8;
		else
			Format = RenDev->Use16bit ? GL_RGBA4 : GL_RGBA8;

		RenDev->glCreateFramebuffers(1, &FBO);
		RenDev->glCreateTextures(GL_TEXTURE_2D, 1, &TextureHandle);
		RenDev->glTextureStorage2D(TextureHandle, 1, Format, Width, Height);
		RenDev->glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT0, TextureHandle, 0);

		if(DepthStencilAttachment)
			RenDev->glNamedFramebufferRenderbuffer(FBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, DepthStencilAttachment);

		checkSlow(RenDev->glCheckNamedFramebufferStatus(FBO, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}else if(Cubemap){
		IsCubemap = true;

		for(INT FaceIndex = 0; FaceIndex < 6; ++FaceIndex){
			FTexture* CubemapFace = Cubemap->GetFace(FaceIndex);

			check(CubemapFace);

			INT MipIndex = CubemapFace->GetFirstMip();
			INT MipWidth = Width >> MipIndex;
			INT MipHeight = Height >> MipIndex;
			void* Data = ConvertTextureData(CubemapFace, DestFormat, MipWidth, MipHeight, MipIndex);

			UploadTextureData(GL_TEXTURE_CUBE_MAP, DestFormat, Data, MipWidth, MipHeight, CubemapFace->GetNumMips() - MipIndex, FaceIndex);

			for(INT i = 0; i < CubemapFace->GetNumMips(); ++i)
				CubemapFace->UnloadRawTextureData(i);
		}

		RenDev->glGenerateTextureMipmap(TextureHandle);
	}else if(CompositeTexture){
		check(DestFormat == SrcFormat);

		INT NumChildren = CompositeTexture->GetNumChildren();
		INT Pitch = GetBytesPerPixel(DestFormat, Width);
		void* Data = RenDev->GetScratchBuffer(GetBytesPerPixel(DestFormat, Width * Height));

		for(INT ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex){
			INT ChildX = 0;
			INT ChildY = 0;
			FTexture* Child = CompositeTexture->GetChild(ChildIndex, &ChildX, &ChildY);

			Child->GetTextureData(0,
			                      CalculateTexelPointer(static_cast<BYTE*>(Data), DestFormat, Pitch, ChildX, ChildY),
			                      Pitch,
			                      DestFormat,
			                      0,
			                      1);
		}

		UploadTextureData(GL_TEXTURE_2D, DestFormat, Data, Width, Height, 1);
	}else if(Texture){
		INT MipIndex = Texture->GetFirstMip();
		INT MipWidth = Width >> MipIndex;
		INT MipHeight = Height >> MipIndex;
		void* Data = ConvertTextureData(Texture, DestFormat, MipWidth, MipHeight, MipIndex);

		UploadTextureData(GL_TEXTURE_2D, DestFormat, Data, MipWidth, MipHeight, Texture->GetNumMips() - MipIndex);
		RenDev->glGenerateTextureMipmap(TextureHandle);

		for(INT i = 0; i < Texture->GetNumMips(); ++i)
			Texture->UnloadRawTextureData(i);
	}

	Revision = BaseTexture->GetRevision();
}

void FOpenGLTexture::Free(){
	Width = 0;
	Height = 0;

	if(TextureHandle){
		RenDev->glDeleteTextures(1, &TextureHandle);
		TextureHandle = GL_NONE;
	}

	if(FBO){
		RenDev->glDeleteFramebuffers(1, &FBO);
		FBO = GL_NONE;
	}

	if(DepthStencilAttachment && !HasSharedDepthStencil){
		RenDev->glDeleteRenderbuffers(1, &DepthStencilAttachment);
		DepthStencilAttachment = GL_NONE;
	}
}

void FOpenGLTexture::BindTexture(GLuint TextureUnit){
	RenDev->glBindTextureUnit(TextureUnit, TextureHandle);
}

void FOpenGLTexture::BindRenderTarget(){
	RenDev->glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void FOpenGLTexture::UploadTextureData(GLenum Target, ETextureFormat Format, void* Data, INT Width, INT Height, INT NumMips, INT CubemapFace){
	if(!Data || Width == 0 || Height == 0){
		Format = TEXF_RGBA8;
		Data = ErrorTexture.GetRawTextureData(0);
		Width = ErrorTexture.GetWidth();
		Height = ErrorTexture.GetHeight();
		NumMips = 1;
	}

	if(IsDXTC(Format)){
		GLenum GLFormat;

		if(Format == TEXF_DXT1)
			GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		else if(Format == TEXF_DXT3)
			GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		else
			GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

		if(Target == GL_TEXTURE_CUBE_MAP){
			checkSlow(CubemapFace >= 0 && CubemapFace < 6);

			if(!TextureHandle)
				RenDev->glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &TextureHandle);

			Target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + CubemapFace;
		}else{
			checkSlow(!TextureHandle);
			RenDev->glCreateTextures(GL_TEXTURE_2D, 1, &TextureHandle);
		}

		RenDev->glCompressedTextureImage2DEXT(TextureHandle, Target, 0, GLFormat, Width, Height, 0, GetBytesPerPixel(Format, Width * Height), Data);
	}else{
		GLenum GLFormat = GL_NONE;
		GLenum GLType = GL_NONE;

		if(Format == TEXF_RGBA8){
			GLFormat = GL_BGRA;
			GLType = GL_UNSIGNED_BYTE;
		}else{
			appErrorf("Invalid texture format '%i'", Format);
		}

		if(Target == GL_TEXTURE_2D){
			checkSlow(!TextureHandle);
			RenDev->glCreateTextures(GL_TEXTURE_2D, 1, &TextureHandle);
			RenDev->glTextureStorage2D(TextureHandle, NumMips, RenDev->Use16bitTextures ? GL_RGBA4 : GL_RGBA8, Width, Height);
			RenDev->glTextureSubImage2D(TextureHandle, 0, 0, 0, Width, Height, GLFormat, GLType, Data);
		}else if(Target == GL_TEXTURE_CUBE_MAP){
			if(!TextureHandle){
				RenDev->glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &TextureHandle);
				RenDev->glTextureStorage2D(TextureHandle, NumMips, RenDev->Use16bitTextures ? GL_RGBA4 : GL_RGBA8, Width, Height);
			}

			RenDev->glTextureSubImage3D(TextureHandle, 0, 0, 0, CubemapFace, Width, Height, 1, GLFormat, GLType, Data);
		}else{
			appErrorf("Invalid texture target '%x'", Target);
		}
	}
}

void* FOpenGLTexture::ConvertTextureData(FTexture* Texture, ETextureFormat DestFormat, INT Width, INT Height, INT MipIndex){
	void* Result;
	ETextureFormat SrcFormat = Texture->GetFormat();
	INT NumPixels = Width * Height;
	INT SrcBufferSize = GetBytesPerPixel(SrcFormat, NumPixels);
	INT DestBufferSize = GetBytesPerPixel(DestFormat, NumPixels);
	void* TextureData = Texture->GetRawTextureData(MipIndex);

	if(TextureData){
		Result = RenDev->GetScratchBuffer(DestBufferSize);
	}else{
		TextureData = RenDev->GetScratchBuffer(SrcBufferSize + DestBufferSize);
		Result = static_cast<BYTE*>(TextureData) + SrcBufferSize;
		Texture->GetTextureData(MipIndex, TextureData, 0, SrcFormat);
	}

	if(SrcFormat == DestFormat)
		return TextureData;

	if(DestFormat == TEXF_RGBA8){
		if(SrcFormat == TEXF_P8){
			UTexture* Tex = Texture->GetUTexture();
			check(Tex && Tex->Palette);
			const TArray<FColor>& Palette = Tex->Palette->Colors;

			for(INT i = 0; i < NumPixels; ++i)
				static_cast<FColor*>(Result)[i] = Palette[static_cast<BYTE*>(TextureData)[i]];
		}else if(SrcFormat == TEXF_RGB8){
			for(INT i = 0; i < NumPixels; ++i){
				BYTE* RGB = static_cast<BYTE*>(TextureData) + i * 3;
				static_cast<FColor*>(Result)[i] = FColor(RGB[0], RGB[1], RGB[2]);
			}
		}else if(SrcFormat == TEXF_L8){
			for(INT i = 0; i < NumPixels; ++i){
				BYTE Value = static_cast<BYTE*>(TextureData)[i];
				static_cast<FColor*>(Result)[i] = FColor(Value, Value, Value);
			}
		}else if(SrcFormat == TEXF_G16){
			for(INT i = 0; i < NumPixels; ++i){
				BYTE Intensity = static_cast<_WORD*>(TextureData)[i] >> 8;
				static_cast<FColor*>(Result)[i] = FColor(Intensity, Intensity, Intensity);
			}
		}else if(SrcFormat == TEXF_V8U8){
			ConvertV8U8ToBGRA8(Result, TextureData, Width, Height);
		}else if(SrcFormat == TEXF_L6V5U5){
			ConvertL6V5U5ToBGRA8(Result, TextureData, Width, Height);
		}else if(SrcFormat == TEXF_X8L8V8U8){
			ConvertX8L8V8U8ToBGRA8(Result, TextureData, Width, Height);
		}else{
			Result = NULL;
		}
	}else{
		Result = NULL;
	}

	return Result;
}
