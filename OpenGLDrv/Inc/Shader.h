#pragma once

#include "../../Engine/Inc/Engine.h"

class FShaderGLSL : public FRenderResource{
public:
	FShaderGLSL() : VertexShaderMain("void main(void){ vs_main(); }\n\n", true),
	                FragmentShaderMain("void main(void){ fs_main(); }\n\n", true){
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

	void SetVertexShaderMain(const FString& InVertexShaderMain){
		VertexShaderMain = InVertexShaderMain;
		++Revision;
	}

	void SetFragmentShaderMain(const FString& InFragmentShaderMain){
		FragmentShaderMain = InFragmentShaderMain;
		++Revision;
	}

	const TCHAR* GetVertexShaderText() const{ return *VertexShaderText; }
	const TCHAR* GetFragmentShaderText() const{ return *FragmentShaderText; }
	const TCHAR* GetVertexShaderMain() const{ return *VertexShaderMain; }
	const TCHAR* GetFragmentShaderMain() const{ return *FragmentShaderMain; }

private:
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
