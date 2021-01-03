#include "../Inc/Shader.h"

UHardwareShaderMacros* FShaderGLSL::HardwareShaderMacros = NULL;
TMap<FString, FString> FShaderGLSL::HardwareShaderMacroText;;

void FShaderGLSL::ClearHardwareShaderMacros(){
	HardwareShaderMacros = NULL;
	HardwareShaderMacroText.Empty();
}

void FShaderGLSL::SetHardwareShaderMacros(UHardwareShaderMacros* Macros){
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

void FShaderGLSL::ExpandHardwareShaderMacros(FString* ShaderText){
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

void FShaderGLSL::SetVertexShaderFromHardwareShader(UHardwareShader* HardwareShader){
	if(!HardwareShaderMacros)
		SetHardwareShaderMacros(CastChecked<UHardwareShaderMacros>(GEngine->HBumpShaderMacros));

	FStringTemp ShaderText = HardwareShader->VertexShaderText;

	ExpandHardwareShaderMacros(&ShaderText);
	SetVertexShaderText(ShaderText);
}

void FShaderGLSL::SetFragmentShaderFromHardwareShader(UHardwareShader* HardwareShader){
	if(!HardwareShaderMacros)
		SetHardwareShaderMacros(CastChecked<UHardwareShaderMacros>(GEngine->HBumpShaderMacros));

	FStringTemp ShaderText = HardwareShader->PixelShaderText;

	ExpandHardwareShaderMacros(&ShaderText);
	SetFragmentShaderText(ShaderText);
}
