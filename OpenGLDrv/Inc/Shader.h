#pragma once

#include "../../Engine/Inc/Engine.h"

// GLSL equivalent types with proper alignment

typedef ALIGN(4) __int32 GLSL_bool;
typedef ALIGN(4) __int32 GLSL_int;
typedef ALIGN(4) float GLSL_float;
typedef ALIGN(8) struct{ GLSL_float X; GLSL_float Y; } GLSL_vec2;
typedef ALIGN(16) FVector GLSL_vec3;
typedef ALIGN(16) FPlane GLSL_vec4;
typedef ALIGN(16) FMatrix GLSL_mat4;

// Macro to synchronize the GLSL uniform block with the C++ struct
#define UNIFORM_BLOCK_CONTENTS \
	UNIFORM_BLOCK_MEMBER(mat4, LocalToWorld) \
	UNIFORM_BLOCK_MEMBER(mat4, WorldToCamera) \
	UNIFORM_BLOCK_MEMBER(mat4, CameraToScreen) \
	UNIFORM_BLOCK_MEMBER(mat4, Transform) \
	UNIFORM_BLOCK_MEMBER(float, Time) \
	UNIFORM_BLOCK_MEMBER(float, CosTime) \
	UNIFORM_BLOCK_MEMBER(float, SinTime) \
	UNIFORM_BLOCK_MEMBER(float, TanTime) \
	UNIFORM_BLOCK_MEMBER(vec4, AmbientLight)

// Global uniforms available in every shader
struct FOpenGLGlobalUniforms{
#define UNIFORM_BLOCK_MEMBER(type, name) GLSL_ ## type name;
	UNIFORM_BLOCK_CONTENTS
#undef UNIFORM_BLOCK_MEMBER
};

// GLSL shader resource

class FShaderGLSL : public FRenderResource{
public:
	FShaderGLSL() : Name("__unnamed_internal__", true){
		CacheId = MakeCacheID(CID_RenderShader);
	}

	void SetName(const FString& InName){
		Name = InName;
	}

	void SetVertexShaderText(const FString& InVertexShaderText){
		VertexShaderText = InVertexShaderText;
		++Revision;
	}

	void SetFragmentShaderText(const FString& InFragmentShaderText){
		FragmentShaderText = InFragmentShaderText;
		++Revision;
	}

	void SetVertexShaderMain(const FString& InVertexShaderMain){
		VertexShaderMain = InVertexShaderMain;
		++Revision;
	}

	void SetFragmentShaderMain(const FString& InFragmentShaderMain){
		FragmentShaderMain = InFragmentShaderMain;
		++Revision;
	}

	const TCHAR* GetName() const{ return *Name; }
	const TCHAR* GetVertexShaderText() const{ return *VertexShaderText; }
	const TCHAR* GetFragmentShaderText() const{ return *FragmentShaderText; }

	FStringTemp GetVertexShaderMain() const{
		if(VertexShaderMain.Len() > 0)
			return VertexShaderMain;

		return FStringTemp("void main(void){\n", true) + "\tgl_Position = " + GetVertexShaderEntryPointName() + "();\n}\n";
	}

	FStringTemp GetFragmentShaderMain() const{
		if(FragmentShaderMain.Len() > 0)
			return FragmentShaderMain;

		return FStringTemp("void main(void){\n", true) + "\tFragColor = " + GetFragmentShaderEntryPointName() + "();\n}\n";
	}

	FStringTemp GetVertexShaderEntryPointName() const{ return Name.Locs() + "_vs_main"; }
	FStringTemp GetFragmentShaderEntryPointName() const{ return Name.Locs() + "_fs_main"; }

private:
	FString Name;
	FString VertexShaderText;
	FString FragmentShaderText;
	FString VertexShaderMain;
	FString FragmentShaderMain;
};
