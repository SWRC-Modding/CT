#pragma once

#include "../Inc/OpenGLDrv.h"
#include "GL/glew.h"

inline INT GetResourceHashIndex(QWORD CacheId){
	return ((DWORD)CacheId & 0xfff) ^ (((DWORD)CacheId & 0xff00) >> 4) + (((DWORD)CacheId & 0xf0000) >> 16);
}

class UOpenGLRenderDevice;

class FOpenGLResource{
public:
	UOpenGLRenderDevice* RenDev;
	QWORD                CacheId;
	INT                  HashIndex;
	FOpenGLResource*     HashNext;

	FOpenGLResource(UOpenGLRenderDevice* InRenDev, QWORD InCacheId);
	virtual ~FOpenGLResource();
};

// FOpenGLShader

enum EOpenGLShaderType{
	OST_Vertex,
	OST_Fragment
};

class FOpenGLShader : public FOpenGLResource{
public:
	FOpenGLShader(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, EOpenGLShaderType InType);
	virtual ~FOpenGLShader();

	void Cache(const TCHAR* Source);

	EOpenGLShaderType Type;
	GLuint Handle;
};

// FOpenGLShaderProgram

class FOpenGLShaderProgram : FOpenGLResource{
public:
	FOpenGLShaderProgram(UOpenGLRenderDevice* InRenDev, QWORD InCacheId);
	virtual ~FOpenGLShaderProgram();

	void Cache(FOpenGLShader* NewVertexShader, FOpenGLShader* NewFragmentShader);

	GLuint Handle;
	FOpenGLShader* VertexShader;
	FOpenGLShader* FragmentShader;
};
