#pragma once

#include "OpenGLDrv.h"

// GLSL equivalent types with proper alignment

typedef ALIGN(4) __int32 GLSL_bool;
typedef ALIGN(4) __int32 GLSL_int;
typedef ALIGN(4) float GLSL_float;
typedef ALIGN(8) struct{ GLSL_float X; GLSL_float Y; } GLSL_vec2;
typedef ALIGN(16) FVector GLSL_vec3;
typedef ALIGN(16) FPlane GLSL_vec4;
typedef ALIGN(16) FMatrix GLSL_mat4;

#define GLSL_struct struct

// GLSL shader resource

class OPENGLDRV_API FShaderGLSL : public FRenderResource{
public:
	FShaderGLSL() : Name("__unnamed_internal__", true){
		CacheId = MakeCacheID(CID_RenderShader);
	}

	void SetName(const FString& InName){
		Name = InName;
	}

	void SetVertexShaderFromHardwareShader(UHardwareShader* HardwareShader);
	void SetFragmentShaderFromHardwareShader(UHardwareShader* HardwareShader);

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

	static void SetHardwareShaderMacros(UHardwareShaderMacros* Macros);
	static void ClearHardwareShaderMacros();
	static void ExpandHardwareShaderMacros(FString* ShaderText);

private:
	FString Name;
	FString VertexShaderText;
	FString FragmentShaderText;

	static UHardwareShaderMacros* HardwareShaderMacros;
	static TMap<FString, FString> HardwareShaderMacroText;

	void ExpandHardwareShaderMacros();
};
