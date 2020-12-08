#pragma once

#include "../Inc/OpenGLDrv.h"
#include "../Inc/Shader.h"
#include "GL/glew.h"

inline INT GetResourceHashIndex(QWORD CacheId){
	return ((DWORD)CacheId & 0xfff) ^ (((DWORD)CacheId & 0xff00) >> 4) + (((DWORD)CacheId & 0xf0000) >> 16);
}

class UOpenGLRenderDevice;

class FOpenGLResource{
public:
	UOpenGLRenderDevice* RenDev;
	QWORD                CacheId;
	INT                  Revision;
	INT                  HashIndex;
	FOpenGLResource*     HashNext;

	FOpenGLResource(UOpenGLRenderDevice* InRenDev, QWORD InCacheId);
	virtual ~FOpenGLResource();
};

// FOpenGLShader

class FOpenGLShader : public FOpenGLResource{
public:
	FOpenGLShader(UOpenGLRenderDevice* InRenDev, QWORD InCacheId);
	virtual ~FOpenGLShader();

	void Cache(FShaderGLSL* Shader);
	void Bind() const;

	GLuint Program;

private:
	GLuint CompileShader(FShaderGLSL* Shader, GLenum Type);
};

// FOpenGLRenderTarget

class FOpenGLRenderTarget : public FOpenGLResource{
public:
	FOpenGLRenderTarget(UOpenGLRenderDevice* InRenDev, QWORD InCacheId);
	virtual ~FOpenGLRenderTarget();

	void Cache(FRenderTarget* RenderTarget);
	void Free();
	void Bind() const;

	GLuint FBO;
	GLuint ColorAttachment;
	GLuint DepthStencilAttachment;
};

// FOpenGLIndexBuffer

class FOpenGLIndexBuffer : public FOpenGLResource{
public:
	FOpenGLIndexBuffer(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, bool InIsDynamic = false);
	virtual ~FOpenGLIndexBuffer();

	void Cache(FIndexBuffer* IndexBuffer);
	void Free();
	void Bind() const;

	GLuint EBO;
	INT    IndexSize;
	INT    BufferSize;
	bool   IsDynamic;
};

// FOpenGLVertexStream

class FOpenGLVertexStream : public FOpenGLResource{
public:
	FOpenGLVertexStream(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, bool InIsDynamic = false);
	virtual ~FOpenGLVertexStream();

	void Cache(FVertexStream* VertexStream);
	void Free();
	void Bind(unsigned int Index) const;

	GLuint VBO;
	INT    Stride;
	INT    BufferSize;
	bool   IsDynamic;
};
