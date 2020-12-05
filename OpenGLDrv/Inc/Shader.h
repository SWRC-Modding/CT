#pragma once

#include "../../Engine/Inc/Engine.h"

class FShaderGLSL : public FRenderResource{
public:
	FShaderGLSL(){ CacheId = MakeCacheID(CID_RenderShader); }

	void SetVertexShaderText(const FString& InVertexShaderText){
		VertexShaderText = InVertexShaderText;
		++Revision;
	}

	void SetFragmentShaderText(const FString& InFragmentShaderText){
		FragmentShaderText = InFragmentShaderText;
		++Revision;
	}

	const TCHAR* GetVertexShaderText() const{ return *VertexShaderText; }
	const TCHAR* GetFragmentShaderText() const{ return *FragmentShaderText; }

private:
	FString VertexShaderText;
	FString FragmentShaderText;
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
