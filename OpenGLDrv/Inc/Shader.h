#pragma once

#include "../../Engine/Inc/Engine.h"

#define FRAGMENT_SHADER_EXTENSION ".fsh"
#define VERTEX_SHADER_EXTENSION ".vsh"

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
