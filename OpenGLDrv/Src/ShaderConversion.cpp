#include "../Inc/OpenGLRenderDevice.h"

UHardwareShaderMacros* UOpenGLRenderDevice::HardwareShaderMacros = NULL;
TMap<FString, FString> UOpenGLRenderDevice::HardwareShaderMacroText;

void UOpenGLRenderDevice::ClearHardwareShaderMacros(){
	HardwareShaderMacros = NULL;
	HardwareShaderMacroText.Empty();
}

void UOpenGLRenderDevice::SetHardwareShaderMacros(UHardwareShaderMacros* Macros){
	HardwareShaderMacros = Macros;

	const TCHAR* Pos = *Macros->Macros;

	if(!Pos)
		return;

	while(*Pos){
		if(*Pos == ';' || *Pos == '/'){
			while(*Pos && *Pos != '\n')
				++Pos;
		}else if(*Pos == '{'){
			const TCHAR* First = Pos;

			while(*Pos && *Pos != '}')
				++Pos;

			if(*Pos){
				FStringTemp Name(static_cast<INT>(Pos - First) + 1, First);

				++Pos;
				First = Pos;

				while(*Pos && *Pos != '{')
					++Pos;

				HardwareShaderMacroText[*Name] = FStringTemp(static_cast<INT>(Pos - First), First);
			}
		}else{
			++Pos;
		}
	}
}

void UOpenGLRenderDevice::ExpandHardwareShaderMacros(FString* ShaderText){
	const TCHAR* Pos = **ShaderText;

	if(!Pos)
		return;

	while(*Pos){
		if(*Pos == ';' || *Pos == '/'){
			while(*Pos && *Pos != '\n')
				++Pos;
		}else if(*Pos == '{'){
			const TCHAR* First = Pos;

			while(*Pos && *Pos != '}')
				++Pos;

			if(*Pos){
				FStringTemp Name(static_cast<INT>(Pos - First) + 1, First);
				FString* MacroText = HardwareShaderMacroText.Find(Name);

				if(MacroText){
					INT Index = static_cast<INT>(First - **ShaderText); // Save index since Pos is invalidated when the string is reallocated

					*ShaderText = FStringTemp(Index, **ShaderText) +
					              "// Macro: " + Name + "\n" +
					              *MacroText + "// End macro\n" +
					              (Pos + 1);
					Pos = &(*ShaderText)[Index];
				}
			}
		}else{
			++Pos;
		}
	}
}

#define SHADER_INSTRUCTIONS \
	INSTRUCTION(add) \
	INSTRUCTION(bem) \
	INSTRUCTION(cmp) \
	INSTRUCTION(cnd) \
	INSTRUCTION(dp3) \
	INSTRUCTION(dp4) \
	INSTRUCTION(dst) \
	INSTRUCTION(exp) \
	INSTRUCTION(frc) \
	INSTRUCTION(lit) \
	INSTRUCTION(log) \
	INSTRUCTION(logp) \
	INSTRUCTION(lrp) \
	INSTRUCTION(m3x2) \
	INSTRUCTION(m3x3) \
	INSTRUCTION(m3x4) \
	INSTRUCTION(m4x3) \
	INSTRUCTION(m4x4) \
	INSTRUCTION(mad) \
	INSTRUCTION(max) \
	INSTRUCTION(min) \
	INSTRUCTION(mov) \
	INSTRUCTION(mul) \
	INSTRUCTION(nop) \
	INSTRUCTION(rcp) \
	INSTRUCTION(rsq) \
	INSTRUCTION(sge) \
	INSTRUCTION(slt) \
	INSTRUCTION(sub) \
	INSTRUCTION(tex) \
	INSTRUCTION(texbem) \
	INSTRUCTION(texbeml) \
	INSTRUCTION(texcoord) \
	INSTRUCTION(texcrd) \
	INSTRUCTION(texdepth) \
	INSTRUCTION(texdp3) \
	INSTRUCTION(texdp3tex) \
	INSTRUCTION(texkill) \
	INSTRUCTION(texld) \
	INSTRUCTION(texm3x2depth) \
	INSTRUCTION(texm3x2pad) \
	INSTRUCTION(texm3x2tex) \
	INSTRUCTION(texm3x3) \
	INSTRUCTION(texm3x3pad) \
	INSTRUCTION(texm3x3spec) \
	INSTRUCTION(texm3x3tex) \
	INSTRUCTION(texm3x3vspec) \
	INSTRUCTION(texreg2ar) \
	INSTRUCTION(texreg2gb) \
	INSTRUCTION(texreg2rgb)

FStringTemp UOpenGLRenderDevice::GLSLVertexShaderFromD3DVertexShader(UHardwareShader* Shader){
	FString D3DShaderText = Shader->VertexShaderText;
	FString GLSLShaderText;

	ExpandHardwareShaderMacros(&D3DShaderText);

	return GLSLShaderText;
}

FStringTemp UOpenGLRenderDevice::GLSLFragmentShaderFromD3DPixelShader(UHardwareShader* Shader){
	FString D3DShaderText = Shader->PixelShaderText;
	FString GLSLShaderText;

	ExpandHardwareShaderMacros(&D3DShaderText);

	return GLSLShaderText;
}

FStringTemp ConvertD3DAssemblyToGLSL(){
	return "";
}
