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
					              "// ==================== Macro: " + Name + " ====================\n" +
					              *MacroText + "// ==================== End macro ====================\n" +
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

enum EShaderInstruction{
	INS_INVALID,
#define INSTRUCTION(x) INS_ ## x,
	SHADER_INSTRUCTIONS
#undef INSTRUCTION
	INS_MAX
};

struct FShaderInstructionString{
	const TCHAR* Str;
	INT NumMatchesWithPrev;
};

static INT GetShaderInstructionStringNumMatches(const TCHAR* Str, const TCHAR* Prev){
	INT i = 0;

	while(Str[i] == Prev[i] && Str[i] && Prev[i])
		++i;

	return i;
}

static FShaderInstructionString ShaderInstructionStrings[] = {
	{"", 0},
#define INSTRUCTION(x) {#x, GetShaderInstructionStringNumMatches(#x, ShaderInstructionStrings[INS_ ## x - 1].Str)},
	SHADER_INSTRUCTIONS
#undef INSTRUCTION
};

static EShaderInstruction ParseShaderInstruction(const TCHAR** Text){
	INT NumMatches = 0;

	for(INT i = 1; appIsAlnum(**Text) && i < INS_MAX; ++i){
		if(NumMatches < ShaderInstructionStrings[i].NumMatchesWithPrev)
			continue;
		else if(NumMatches > ShaderInstructionStrings[i].NumMatchesWithPrev)
			break;

		while(*(*Text + NumMatches) == ShaderInstructionStrings[i].Str[NumMatches])
			++NumMatches;

		if(!appIsAlnum(*(*Text + NumMatches)) && ShaderInstructionStrings[i].Str[NumMatches] == '\0'){
			*Text += NumMatches;

			return static_cast<EShaderInstruction>(i);
		}
	}

	return INS_INVALID;
}

static void SkipCommentsAndWhitespace(const TCHAR** Text){
	while(appIsSpace(**Text) || **Text == ';' || **Text == '/'){
		while(appIsSpace(**Text))
			++*Text;

		if(**Text == ';' || (**Text == '/' && *(*Text + 1) == '/')){
			while(**Text && **Text != '\n')
				++*Text;
		}else if(**Text == '/' && *(*Text + 1) == '*'){
			*Text += 2;

			while(**Text){
				if(**Text == '*' && *(*Text + 1) == '/'){
					*Text += 2;

					break;
				}

				++*Text;
			}
		}
	}
}

FStringTemp UOpenGLRenderDevice::GLSLVertexShaderFromD3DVertexShader(UHardwareShader* Shader){
	FString D3DShaderText = Shader->VertexShaderText;

	ExpandHardwareShaderMacros(&D3DShaderText);

	FString GLSLShaderText = CommonShaderHeaderText + VertexShaderVarsText +
		FString::Printf("layout(location = %i) uniform vec4 VSConstants[%i];\n", HSU_VSConstants, MAX_VERTEX_SHADER_CONSTANTS) +
		                "#define c VSConstants\n\n"
		                "void main(void){\n"
		                    "\tvec4 r0;\n"
		                    "\tvec4 r1;\n"
		                    "\tvec4 r2;\n"
		                    "\tvec4 r3;\n"
		                    "\tvec4 r4;\n"
		                    "\tvec4 r5;\n"
		                    "\tvec4 r6;\n"
		                    "\tvec4 r7;\n"
		                    "\tvec4 r8;\n"
		                    "\tvec4 r9;\n"
		                    "\tvec4 r10;\n"
		                    "\tvec4 r11;\n"
		                    "\tvec4 r12;\n" +
		                    ConvertD3DAssemblyToGLSL(*D3DShaderText) +
		                "}\n\n"
		                "#if 0\n\n" +
		                D3DShaderText +
						"\n#endif\n";

	return GLSLShaderText;
}

FStringTemp UOpenGLRenderDevice::GLSLFragmentShaderFromD3DPixelShader(UHardwareShader* Shader){
	FString D3DShaderText = Shader->PixelShaderText;

	ExpandHardwareShaderMacros(&D3DShaderText);

	FStringTemp GLSLShaderText = CommonShaderHeaderText + FragmentShaderVarsText +
		FString::Printf("layout(location = %i) uniform vec4 PSConstants[%i];\n", HSU_PSConstants, MAX_PIXEL_SHADER_CONSTANTS) +
		                "#define c PSConstants\n\n"
		                "void main(void){\n"
		                    "\tvec4 r0;\n"
		                    "\tvec4 r1;\n"
		                    "\tvec4 r2;\n"
		                    "\tvec4 r3;\n"
		                    "\tvec4 r4;\n"
		                    "\tvec4 r5;\n" +
		                    ConvertD3DAssemblyToGLSL(*D3DShaderText) +
		                    "\n\tFragColor = r0;\n"
		                "}\n\n"
		                "#if 0\n\n" +
		                D3DShaderText +
		                "\n#endif\n";

	return GLSLShaderText;
}

FStringTemp UOpenGLRenderDevice::ConvertD3DAssemblyToGLSL(const TCHAR* Text){
	SkipCommentsAndWhitespace(&Text);

	// Skip shader type and version
	if((*Text == 'v' || *Text == 'p') && Text[1] == 's'){
		while(*Text && *Text != '\n')
			++Text;
	}

	return "";
}
