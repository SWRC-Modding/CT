#pragma once

#include "../Inc/OpenGLDrv.h"
#include "../Inc/Shader.h"
#include "../Inc/opengl.h"

inline INT GetResourceHashIndex(QWORD CacheId){
	return ((DWORD)CacheId & 0xfff) ^ (((DWORD)CacheId & 0xff00) >> 4) + (((DWORD)CacheId & 0xf0000) >> 16);
}

class UOpenGLRenderDevice;

enum EOpenGLResourceFlags{
	OGLRF_NotInHash = 0x1
};

class FOpenGLResource{
public:
	UOpenGLRenderDevice* RenDev;
	FOpenGLResource*     HashNext;
	QWORD                CacheId;
	INT                  Revision;
	INT                  HashIndex;
	DWORD                Flags;

	FOpenGLResource(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, DWORD InFlags = 0);
	virtual ~FOpenGLResource();
};

// FOpenGLShader

class FOpenGLShader : public FOpenGLResource{
public:
	FOpenGLShader(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, DWORD InFlags = 0);
	virtual ~FOpenGLShader();

	void Cache(FShaderGLSL* Shader);
	void Bind();

	GLuint Program;
	UBOOL  IsErrorShader;

private:
	GLuint CompileShader(GLenum Type, const FString& ShaderCode, const FString& ShaderName);
};

// FOpenGLIndexBuffer

class FOpenGLIndexBuffer : public FOpenGLResource{
public:
	FOpenGLIndexBuffer(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, DWORD InFlags = 0, bool InIsDynamic = false);
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
	FOpenGLVertexStream(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, DWORD InFlags = 0, bool InIsDynamic = false);
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
	FOpenGLTexture(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, DWORD InFlags = 0);
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
	static FSolidColorTexture ErrorTexture;

	void UploadTextureData(GLenum Target, ETextureFormat Format, void* Data, INT Width, INT Height, INT NumMips, INT CubemapFace = -1);
	void* ConvertTextureData(FTexture* Texture, ETextureFormat DestFormat, INT Width, INT Height, INT MipIndex);
};
