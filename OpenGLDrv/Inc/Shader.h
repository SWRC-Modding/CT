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
	            const FString& InVertexShaderText = FString(),
	            const FString& InFragmentShaderText = FString()) : Name(InName),
	                                                               VertexShaderText(InVertexShaderText),
	                                                               FragmentShaderText(InFragmentShaderText){
		CacheId = MakeCacheID(CID_RenderShader);
	}

	void SetVertexShaderText(const FString& InVertexShaderText){
		VertexShaderText = InVertexShaderText;
		++Revision;
	}

	void SetFragmentShaderText(const FString& InFragmentShaderText){
		FragmentShaderText = InFragmentShaderText;
		++Revision;
	}

	const TCHAR* GetName() const{ return *Name; }
	const TCHAR* GetVertexShaderText() const{ return *VertexShaderText; }
	const TCHAR* GetFragmentShaderText() const{ return *FragmentShaderText; }

private:
	FString Name;
	FString VertexShaderText;
	FString FragmentShaderText;
};
