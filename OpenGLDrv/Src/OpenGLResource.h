#pragma once

#include "../../Engine/Inc/Engine.h"
#include "opengl.h"

inline INT GetResourceHashIndex(QWORD CacheId){
	return ((DWORD)CacheId & 0xfff) ^ (((DWORD)CacheId & 0xff00) >> 4) + (((DWORD)CacheId & 0xf0000) >> 16);
}

class UOpenGLRenderDevice;

class FOpenGLResource{
public:
	UOpenGLRenderDevice* RenDev;
	FOpenGLResource*     HashNext;
	QWORD                CacheId;
	INT                  Revision;
	INT                  HashIndex;

	FOpenGLResource(UOpenGLRenderDevice* InRenDev, QWORD InCacheId);
	virtual ~FOpenGLResource();
};

// FOpenGLIndexBuffer

class FOpenGLIndexBuffer : public FOpenGLResource{
public:
	FOpenGLIndexBuffer(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, bool InIsDynamic = false);
	virtual ~FOpenGLIndexBuffer();

	void Cache(FIndexBuffer* IndexBuffer, INT DynamicBufferSize = 0);
	INT AddIndices(FIndexBuffer* IndexBuffer);

	GLuint EBO;
	INT    IndexSize;
	INT    BufferSize;
	INT    Tail;
	bool   IsDynamic;
};

// FOpenGLVertexStream

class FOpenGLVertexStream : public FOpenGLResource{
public:
	FOpenGLVertexStream(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, bool InIsDynamic = false);
	virtual ~FOpenGLVertexStream();

	void Cache(FVertexStream* VertexStream, INT DynamicBufferSize = 0);
	INT AddVertices(FVertexStream* VertexStream);

	GLuint VBO;
	INT    Stride;
	INT    BufferSize;
	INT    Tail;
	bool   IsDynamic;
};

// FOpenGLTexture

class FOpenGLTexture : public FOpenGLResource{
public:
	FOpenGLTexture(UOpenGLRenderDevice* InRenDev, QWORD InCacheId);
	virtual ~FOpenGLTexture();

	void Cache(FBaseTexture* BaseTexture, bool RenderTargetMatchBackbuffer = false);
	void Free();
	void BindTexture(GLuint TextureUnit);
	void BindRenderTarget();

	INT    Width;
	INT    Height;
	GLuint TextureHandle;
	GLuint FBO;
	GLuint DepthStencilAttachment;
	bool   IsCubemap;
	bool   HasSharedDepthStencil;

private:
	void UploadTextureData(ETextureFormat Format, void* Data, INT Width, INT Height, INT MipIndex, INT CubemapFace = -1);
	void* ConvertTextureData(FTexture* Texture, INT Width, INT Height, INT MipIndex, ETextureFormat DestFormat);
};
