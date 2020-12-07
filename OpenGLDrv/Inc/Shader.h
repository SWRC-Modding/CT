#pragma once

#include "../../Engine/Inc/Engine.h"

#define FRAGMENT_SHADER_EXTENSION ".fsh"
#define VERTEX_SHADER_EXTENSION ".vsh"

class FShaderGLSL : public FRenderResource{
public:
	FShaderGLSL() : Name("__unnamed_internal__", true),
	                VertexShaderMain("void main(void){ vs_main(); }\n\n", true),
	                FragmentShaderMain("void main(void){ fs_main(); }\n\n", true){
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
	const TCHAR* GetVertexShaderMain() const{ return *VertexShaderMain; }
	const TCHAR* GetFragmentShaderMain() const{ return *FragmentShaderMain; }

private:
	FString Name; // For error reporting
	FString VertexShaderText;
	FString FragmentShaderText;
	FString VertexShaderMain;
	FString FragmentShaderMain;
};

class FShaderGLSLFromD3D : public FShaderGLSL{
public:
	FShaderGLSLFromD3D* HashNext;

	void SetUHardwareShader(UHardwareShader* InHardwareShader){
		HardwareShader = InHardwareShader;
		appErrorf("D3D Shader conversion not yet implemented!");
	}

private:
	UHardwareShader* HardwareShader;
};
