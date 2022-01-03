#pragma once

#include "OpenGLDrv.h"

// GLSL equivalent types with proper alignment

typedef ALIGN(4) __int32 GLSL_bool;
typedef ALIGN(4) __int32 GLSL_int;
typedef ALIGN(4) float GLSL_float;
typedef ALIGN(8) struct{ GLSL_float X; GLSL_float Y; } GLSL_vec2;
typedef ALIGN(16) FPlane GLSL_vec4; // No vec3, use vec4 instead!!!
typedef ALIGN(16) FMatrix GLSL_mat4;

#define GLSL_struct struct ALIGN(16)

// GLSL shader resource

class OPENGLDRV_API FShaderGLSL : public FRenderResource{
public:
	FShaderGLSL(const FString& InName,
	            const FString& InShaderCode = FString()) : Name(InName),
	                                                       ShaderCode(InShaderCode){
		CacheId = MakeCacheID(CID_RenderShader);
	}

	void SetShaderCode(const FString& InShaderCode){
		ShaderCode = InShaderCode;
		++Revision;
	}

	const FString& GetName() const{ return Name; }
	const FString& GetShaderCode() const{ return ShaderCode; }

private:
	FString Name;
	FString ShaderCode;
};
