#pragma once

#include "Core.h"
#include "opengl.h"

class UOpenGLRenderDevice;

// GLSL equivalent types with proper alignment

typedef ALIGN(4) __int32 GLSL_bool;
typedef ALIGN(4) __int32 GLSL_int;
typedef ALIGN(4) float GLSL_float;
typedef ALIGN(8) struct{ GLSL_float X; GLSL_float Y; } GLSL_vec2;
typedef ALIGN(16) FPlane GLSL_vec4; // No vec3, use vec4 instead!!!
typedef ALIGN(16) FMatrix GLSL_mat4;

#define GLSL_struct struct ALIGN(16)

// FOpenGLShader

class FOpenGLShader{
public:
	FOpenGLShader(UOpenGLRenderDevice* InRenDev);
	~FOpenGLShader();

	void Compile(const TCHAR* InShaderCode, const TCHAR* ShaderName); // Shader name is used for error messages only
	void Bind() const;
	void Free();
	bool IsValid() const{ return !!Program; }

	UOpenGLRenderDevice* RenDev;
	GLuint               Program;

private:
	GLuint CompileShader(GLenum Type, const TCHAR* ShaderCode, const TCHAR* ShaderName);
};

class FOpenGLCachedShader : public FOpenGLShader{
public:
	FOpenGLCachedShader(UOpenGLRenderDevice* InRenDev) : FOpenGLShader(InRenDev){}

	INT Index;
};
