#include "OpenGLResource.h"
#include "OpenGLRenderDevice.h"
#include "opengl.h"

FOpenGLResource::FOpenGLResource(UOpenGLRenderDevice* InRenDev, QWORD InCacheId)
	: RenDev(InRenDev)
	, HashNext(NULL)
	, CacheId(InCacheId)
	, Revision(-1)
	, HashIndex(INDEX_NONE)
{
	RenDev->AddResource(this);
}

FOpenGLResource::~FOpenGLResource()
{
	RenDev->RemoveResource(this);
}

// FOpenGLIndexBuffer

#define INITIAL_DYNAMIC_INDEX_BUFFER_SIZE 32768

FOpenGLIndexBuffer::FOpenGLIndexBuffer(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, bool InIsDynamic)
	: FOpenGLResource(InRenDev, InCacheId)
	, EBO(GL_NONE)
	, IndexSize(0)
	, BufferSize(0)
	, IsDynamic(InIsDynamic)
{
}

FOpenGLIndexBuffer::~FOpenGLIndexBuffer()
{
	if(EBO)
	{
		RenDev->glDeleteBuffers(1, &EBO);
		EBO = GL_NONE;
	}
}

void FOpenGLIndexBuffer::Cache(FIndexBuffer* IndexBuffer, INT DynamicBufferSize)
{
	GLuint OldEBO = EBO;
	RenDev->glCreateBuffers(1, &EBO);

	INT IndexBufferSize = IndexBuffer->GetSize();
	void* Data = RenDev->GetScratchBuffer(IndexBufferSize);

	IndexBuffer->GetContents(Data);

	if(IsDynamic)
	{
		BufferSize = Max(IndexBufferSize, DynamicBufferSize);

		if(DynamicBufferSize > 0)
			debugf("Allocating %ikb dynamic index buffer", BufferSize / 1024);

		RenDev->glNamedBufferStorage(EBO, BufferSize, NULL, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
		RenDev->glNamedBufferSubData(EBO, 0, IndexBufferSize, Data);
	}
	else
	{
		BufferSize = IndexBufferSize;
		RenDev->glNamedBufferStorage(EBO, IndexBufferSize, Data, 0);
	}

	Revision = IndexBuffer->GetRevision();
	IndexSize = IndexBuffer->GetIndexSize();
	Tail = IndexBufferSize;

	if(OldEBO)
		RenDev->glDeleteBuffers(1, &OldEBO);
}

INT FOpenGLIndexBuffer::AddIndices(FIndexBuffer* IndexBuffer)
{
	checkSlow(IsDynamic);

	INT AdditionalSize = IndexBuffer->GetSize();

	if(EBO == GL_NONE || AdditionalSize > BufferSize)
	{
		Cache(IndexBuffer, Max(AdditionalSize * 2, INITIAL_DYNAMIC_INDEX_BUFFER_SIZE));

		return 0;
	}

	IndexSize = IndexBuffer->GetIndexSize();

	INT IndexBufferOffset = Align(Tail, IndexSize);
	GLbitfield MapFlags = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;

	if(IndexBufferOffset + AdditionalSize > BufferSize)
	{
		IndexBufferOffset = 0;
		MapFlags |= GL_MAP_INVALIDATE_BUFFER_BIT;
	}
	else
	{
		MapFlags |= GL_MAP_INVALIDATE_RANGE_BIT;
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

FOpenGLVertexStream::~FOpenGLVertexStream()
{
	if(VBO)
	{
		RenDev->glDeleteBuffers(1, &VBO);
		VBO = GL_NONE;
	}
}

void FOpenGLVertexStream::Cache(FVertexStream* VertexStream, INT DynamicBufferSize)
{
	GLuint OldVBO = VBO;
	RenDev->glCreateBuffers(1, &VBO);

	INT StreamSize = VertexStream->GetSize();
	void* Data = NULL;

	VertexStream->GetRawStreamData(&Data, 0);

	if(!Data)
	{
		Data = RenDev->GetScratchBuffer(StreamSize);
		VertexStream->GetStreamData(Data);
	}

	IsDynamic |= VertexStream->HintDynamic() != 0;

	if(IsDynamic)
	{
		BufferSize = Max(StreamSize, DynamicBufferSize);

		if(DynamicBufferSize > 0)
			debugf("Allocating %ikb dynamic vertex buffer", BufferSize / 1024);

		RenDev->glNamedBufferStorage(VBO, BufferSize, NULL, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
		RenDev->glNamedBufferSubData(VBO, 0, StreamSize, Data);
	}
	else
	{
		BufferSize = StreamSize;
		RenDev->glNamedBufferStorage(VBO, BufferSize, Data, 0);
	}

	Revision = VertexStream->GetRevision();
	Stride = VertexStream->GetStride();
	Tail = StreamSize;

	if(OldVBO)
		RenDev->glDeleteBuffers(1, &OldVBO);
}

INT FOpenGLVertexStream::AddVertices(FVertexStream* VertexStream)
{
	checkSlow(IsDynamic);

	INT AdditionalSize = VertexStream->GetSize();

	if(VBO == GL_NONE || AdditionalSize > BufferSize)
	{
		Cache(VertexStream, Max(AdditionalSize * 2, INITIAL_DYNAMIC_VERTEX_BUFFER_SIZE));

		return 0;
	}

	Stride = VertexStream->GetStride();

	INT VertexBufferOffset = ((Tail + Stride - 1) / Stride) * Stride;
	GLbitfield MapFlags = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;

	if(VertexBufferOffset + AdditionalSize > BufferSize)
	{
		VertexBufferOffset = 0;
		MapFlags |= GL_MAP_INVALIDATE_BUFFER_BIT;
	}
	else
	{
		MapFlags |= GL_MAP_INVALIDATE_RANGE_BIT;
	}

	void* Data = RenDev->glMapNamedBufferRange(VBO, VertexBufferOffset, AdditionalSize, MapFlags);

	VertexStream->GetStreamData(Data);
	RenDev->glUnmapNamedBuffer(VBO);

	Tail = VertexBufferOffset + AdditionalSize;

	return VertexBufferOffset / Stride;
}

// FOpenGLTexture

FOpenGLTexture::FOpenGLTexture(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : FOpenGLResource(InRenDev, InCacheId),
                                                                                 Width(0),
                                                                                 Height(0),
                                                                                 TextureHandle(GL_NONE),
                                                                                 FBO(GL_NONE),
                                                                                 DepthStencilAttachment(GL_NONE),
                                                                                 IsCubemap(false),
                                                                                 HasSharedDepthStencil(false){}

FOpenGLTexture::~FOpenGLTexture()
{
	Free();
}

static GLuint GetGLFormat(ETextureFormat Format, UBOOL Use16bitTextures)
{
	switch(Format)
	{
	case TEXF_V8U8:
		return GL_RG8_SNORM;
	case TEXF_L6V5U5:
	case TEXF_X8L8V8U8:
		return GL_RGBA8_SNORM;
	default:
		return Use16bitTextures ? GL_RGBA4 : GL_RGBA8;
	}
}

void FOpenGLTexture::Cache(FBaseTexture* BaseTexture, bool bOwnDepthBuffer)
{
	guardFunc
	Free();

	FRenderTarget*       RenderTarget     = BaseTexture->GetRenderTargetInterface();
	FCubemap*            Cubemap          = BaseTexture->GetCubemapInterface();
	FCompositeTexture*   CompositeTexture = BaseTexture->GetCompositeTextureInterface();
	FTexture*            Texture          = BaseTexture->GetTextureInterface();
	const ETextureFormat SrcFormat        = BaseTexture->GetFormat();
	const ETextureFormat DestFormat       = IsDXTC(SrcFormat) || IsBumpmap(SrcFormat) ? SrcFormat : TEXF_RGBA8;

	Width = BaseTexture->GetWidth();
	Height = BaseTexture->GetHeight();
	IsCubemap = false;

	if(RenderTarget)
	{
		if(Width == 0 || Height == 0)
			return;

		bool IsMipTarget = false;

		for(INT i = 0; i < UFrameFX::MaxMips; ++i)
		{
			if(RenderTarget == UFrameFX::MipTargets[i])
			{
				IsMipTarget = true;
				break;
			}
		}

		if(IsMipTarget)
		{
			DepthStencilAttachment = GL_NONE; // The FrameFX mip targets don't need a depth or stencil buffer
		}
		else if(bOwnDepthBuffer)
		{
			RenDev->glCreateRenderbuffers(1, &DepthStencilAttachment);
			RenDev->glNamedRenderbufferStorage(DepthStencilAttachment, GL_DEPTH24_STENCIL8, Width, Height);
			HasSharedDepthStencil = false;
		}
		else
		{
			checkSlow(RenDev->BackbufferDepthStencil);
			DepthStencilAttachment = RenDev->BackbufferDepthStencil;
			HasSharedDepthStencil = true;
		}

		RenDev->glCreateFramebuffers(1, &FBO);
		RenDev->glCreateTextures(GL_TEXTURE_2D, 1, &TextureHandle);
		RenDev->glTextureStorage2D(TextureHandle, 1, RenDev->Use16bit ? GL_RGBA4 : GL_RGBA8, Width, Height);
		RenDev->glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT0, TextureHandle, 0);
		RenDev->glTextureParameteri(TextureHandle, GL_TEXTURE_MAX_LEVEL, 0);

		if(DepthStencilAttachment)
			RenDev->glNamedFramebufferRenderbuffer(FBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, DepthStencilAttachment);

		checkSlow(RenDev->glCheckNamedFramebufferStatus(FBO, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}
	else if(Cubemap)
	{
		IsCubemap = true;

		INT CubemapNumMips = Cubemap->GetNumMips() - Cubemap->GetFirstMip();

		if(CubemapNumMips > 0)
		{
			RenDev->glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &TextureHandle);

			if(!IsDXTC(DestFormat))
				RenDev->glTextureStorage2D(TextureHandle, CubemapNumMips, GetGLFormat(DestFormat, RenDev->Use16bitTextures), Width, Height);

			INT MaxLevel = -1;

			for(INT FaceIndex = 0; FaceIndex < 6; ++FaceIndex)
			{
				FTexture* CubemapFace = Cubemap->GetFace(FaceIndex);
				check(CubemapFace);

				INT FirstMip = CubemapFace->GetFirstMip();
				INT MipWidth = Width >> FirstMip;
				INT MipHeight = Height >> FirstMip;
				INT NumMips = CubemapFace->GetNumMips() - FirstMip;
				check(NumMips == CubemapNumMips);

				for(INT MipIndex = 0; MipIndex < NumMips; ++MipIndex)
				{
					void* Data = GetTextureData(CubemapFace, MipWidth, MipHeight, MipIndex + FirstMip);
					UploadTextureData(DestFormat, Data, MipWidth, MipHeight, MipIndex, FaceIndex);

					if(RenDev->bUnloadTextureData)
						CubemapFace->UnloadRawTextureData(MipIndex + FirstMip);

					MipWidth >>= 1;
					MipHeight >>= 1;
					MaxLevel += FaceIndex == 0;

					if(MipWidth < 4 || MipHeight < 4)
						break;
				}
			}

			RenDev->glTextureParameteri(TextureHandle, GL_TEXTURE_MAX_LEVEL, MaxLevel);
		}
	}
	else if(CompositeTexture)
	{
		check(DestFormat == SrcFormat);
		checkSlow(CompositeTexture->GetNumMips() == 1);
		checkSlow(Width > 0 && Height > 0);

		INT NumChildren = CompositeTexture->GetNumChildren();
		INT Pitch = GetBytesPerPixel(DestFormat, Width);
		void* Data = RenDev->GetScratchBuffer(GetBytesPerPixel(DestFormat, Width * Height));

		RenDev->glCreateTextures(GL_TEXTURE_2D, 1, &TextureHandle);

		if(!IsDXTC(DestFormat))
			RenDev->glTextureStorage2D(TextureHandle, 1, GetGLFormat(DestFormat, RenDev->Use16bitTextures), Width, Height);

		for(INT ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
		{
			INT ChildX = 0;
			INT ChildY = 0;
			FTexture* Child = CompositeTexture->GetChild(ChildIndex, &ChildX, &ChildY);

			Child->GetTextureData(0,
			                      CalculateTexelPointer(static_cast<BYTE*>(Data), DestFormat, Pitch, ChildX, ChildY),
			                      Pitch,
			                      DestFormat,
			                      0,
			                      1);

			if(RenDev->bUnloadTextureData)
				Child->UnloadRawTextureData(0);
		}

		UploadTextureData(DestFormat, Data, Width, Height, 0);
		RenDev->glTextureParameteri(TextureHandle, GL_TEXTURE_MAX_LEVEL, 0);
	}
	else if(Texture)
	{
		INT FirstMip = Texture->GetFirstMip();
		INT MipWidth = Width >> FirstMip;
		INT MipHeight = Height >> FirstMip;
		INT NumMips = Texture->GetNumMips() - FirstMip;

		if(NumMips > 0)
		{
			checkSlow(Width > 0 && Height > 0);
			RenDev->glCreateTextures(GL_TEXTURE_2D, 1, &TextureHandle);

			if(!IsDXTC(DestFormat))
				RenDev->glTextureStorage2D(TextureHandle, NumMips, GetGLFormat(DestFormat, RenDev->Use16bitTextures), MipWidth, MipHeight);

			INT MaxLevel = -1;

			for(INT MipIndex = 0; MipIndex < NumMips; ++MipIndex)
			{
				void* Data = GetTextureData(Texture, MipWidth, MipHeight, MipIndex + FirstMip);
				UploadTextureData(DestFormat, Data, MipWidth, MipHeight, MipIndex);

				if(RenDev->bUnloadTextureData)
					Texture->UnloadRawTextureData(MipIndex + FirstMip);

				MipWidth >>= 1;
				MipHeight >>= 1;
				++MaxLevel;

				if(MipWidth < 4 || MipHeight < 4)
					break;
			}

			RenDev->glTextureParameteri(TextureHandle, GL_TEXTURE_MAX_LEVEL, MaxLevel);
		}
		else
		{
			checkSlow(Width == 0 && Height == 0);
		}
	}

	Revision = BaseTexture->GetRevision();
	unguard
}

void FOpenGLTexture::Free()
{
	Width = 0;
	Height = 0;

	if(TextureHandle)
	{
		RenDev->glDeleteTextures(1, &TextureHandle);
		TextureHandle = GL_NONE;
	}

	if(FBO)
	{
		RenDev->glDeleteFramebuffers(1, &FBO);
		FBO = GL_NONE;
	}

	if(DepthStencilAttachment && !HasSharedDepthStencil)
	{
		RenDev->glDeleteRenderbuffers(1, &DepthStencilAttachment);
		DepthStencilAttachment = GL_NONE;
	}
}

void FOpenGLTexture::BindTexture(GLuint TextureUnit)
{
	RenDev->glBindTextureUnit(TextureUnit, TextureHandle);
}

void FOpenGLTexture::BindRenderTarget()
{
	RenDev->glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void FOpenGLTexture::UploadTextureData(ETextureFormat Format, void* Data, INT MipWidth, INT MipHeight, INT MipIndex, INT CubemapFace)
{
	guardFunc
	checkSlow(Data);
	checkSlow(MipWidth > 0);
	checkSlow(MipHeight > 0);

	if(IsDXTC(Format))
	{
		GLenum GLFormat;

		if(Format == TEXF_DXT1)
			GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		else if(Format == TEXF_DXT3)
			GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		else
			GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

		GLenum Target = GL_TEXTURE_2D;

		if(CubemapFace >= 0)
		{
			checkSlow(CubemapFace < 6);
			Target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + CubemapFace;
		}

		RenDev->glCompressedTextureImage2DEXT(TextureHandle, Target, MipIndex, GLFormat, MipWidth, MipHeight, 0, GetBytesPerPixel(Format, MipWidth * MipHeight), Data);
	}
	else
	{
		GLenum GLFormat = GL_NONE;
		GLenum GLType = GL_NONE;

		switch(Format)
		{
		case TEXF_RGBA8:
			GLFormat = GL_BGRA;
			GLType = GL_UNSIGNED_BYTE;
			break;
		case TEXF_V8U8:
			GLFormat = GL_RG;
			GLType = GL_BYTE;
			RenDev->glTextureParameteri(TextureHandle, GL_TEXTURE_SWIZZLE_B, GL_ONE);
			break;
		case TEXF_L6V5U5:
		case TEXF_X8L8V8U8:
			GLFormat = GL_RGBA;
			GLType = GL_BYTE;
			break;
		default:
			appErrorf("Unsupported texture format (%i)", Format);
		}

		if(CubemapFace < 0)
			RenDev->glTextureSubImage2D(TextureHandle, MipIndex, 0, 0, MipWidth, MipHeight, GLFormat, GLType, Data);
		else
			RenDev->glTextureSubImage3D(TextureHandle, MipIndex, 0, 0, CubemapFace, MipWidth, MipHeight, 1, GLFormat, GLType, Data);
	}
	unguard
}

void* FOpenGLTexture::GetTextureData(FTexture* Texture, INT MipWidth, INT MipHeight, INT MipIndex)
{
	guardFunc
	const ETextureFormat SrcFormat      = Texture->GetFormat();
	const INT            NumPixels      = MipWidth * MipHeight;
	const INT            SrcBufferSize  = GetBytesPerPixel(SrcFormat, NumPixels);
	const INT            DestBufferSize = MipWidth * MipHeight * 4; // Worst-case scenario: 4 bytes per pixel
	void*                RawTextureData = Texture->GetRawTextureData(MipIndex);
	void*                Result;

	if(RawTextureData)
	{
		Result = RenDev->GetScratchBuffer(DestBufferSize);
	}
	else
	{
		RawTextureData = RenDev->GetScratchBuffer(SrcBufferSize + DestBufferSize);
		Result = static_cast<BYTE*>(RawTextureData) + SrcBufferSize;
		Texture->GetTextureData(MipIndex, RawTextureData, 0, SrcFormat);
	}

	if(IsDXTC(SrcFormat) || SrcFormat == TEXF_RGBA8 || SrcFormat == TEXF_V8U8 || SrcFormat == TEXF_X8L8V8U8)
		return RawTextureData;

	switch(SrcFormat)
	{
	case TEXF_P8:
		{
			UTexture* Tex = Texture->GetUTexture();
			check(Tex && Tex->Palette);
			const TArray<FColor>& Palette = Tex->Palette->Colors;

			for(INT i = 0; i < NumPixels; ++i)
				static_cast<FColor*>(Result)[i] = Palette[static_cast<BYTE*>(RawTextureData)[i]];

			break;
		}
	case TEXF_RGB8:
		{
			for(INT i = 0; i < NumPixels; ++i)
			{
				BYTE* RGB = static_cast<BYTE*>(RawTextureData) + i * 3;
				static_cast<FColor*>(Result)[i] = FColor(RGB[0], RGB[1], RGB[2]);
			}

			break;
		}
	case TEXF_L8:
		{
			for(INT i = 0; i < NumPixels; ++i)
			{
				BYTE Value = static_cast<BYTE*>(RawTextureData)[i];
				static_cast<FColor*>(Result)[i] = FColor(Value, Value, Value);
			}

			break;
		}
	case TEXF_G16:
		{
			for(INT i = 0; i < NumPixels; ++i)
			{
				BYTE Intensity = static_cast<_WORD*>(RawTextureData)[i] >> 8;
				static_cast<FColor*>(Result)[i] = FColor(Intensity, Intensity, Intensity);
			}

			break;
		}
	case TEXF_L6V5U5:
		{
			for(INT i = 0; i < NumPixels; ++i)
			{
				FL6V5U5Pixel    P1 = static_cast<FL6V5U5Pixel*>(RawTextureData)[i];
				FX8L8V8U8Pixel& P2 = static_cast<FX8L8V8U8Pixel*>(Result)[i];

				P2.U = Map5BitSignedTo8BitSigned(P1.U);
				P2.V = Map5BitSignedTo8BitSigned(P1.V);
				P2.L = (SBYTE)(Map6BitUnsignedTo8BitUnsigned(P1.L) - 128);
				P2.X = P2.L;
			}

			break;
		}
	default:
		appErrorf("Unsupported texture format (%i)", SrcFormat);
	}

	return Result;
	unguard
}
