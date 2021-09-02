#include "../Inc/OpenGLRenderDevice.h"

UHardwareShaderMacros* UOpenGLRenderDevice::HardwareShaderMacros = NULL;
TMap<FString, FString> UOpenGLRenderDevice::ShaderMacros;
TArray<FString>        UOpenGLRenderDevice::ExpandedMacros;

static void SkipWhitespaceSingleLine(const TCHAR** Text){
	while(**Text && appIsSpace(**Text) && **Text != '\n')
		++*Text;
}

// If Out is specified, comments and newlines will be written to it
static void SkipWhitespaceAndComments(const TCHAR** Text, FString* Out = NULL){
	for(;;){
		while(appIsSpace(**Text)){
			if(Out && **Text == '\n'){
				if(*(*Text + 1) == '\r' && *(*Text + 1) == '\n')
					*Text += 2;
				else
					++*Text;

				*Out += "\n";
			}else{
				++*Text;
			}

			// Remove trailing whitespace at end of text
			if(Out && **Text == '\0'){
				TArray<TCHAR>& CharArray = Out->GetCharArray();

				for(INT i = CharArray.Num() - 2; i >= 0; --i){
					if(!appIsSpace(CharArray[i])){
						CharArray[i + 1] = '\0';
						CharArray.Set(i + 2);
						break;
					}
				}
			}
		}

		if(**Text == ';' || (**Text == '/' && *(*Text + 1) == '/')){
			const TCHAR* Start = *Text;

			while(**Text && **Text != '\n')
				++*Text;

			if(Out){
				if(Out->Len() == 0 || (*Out)[Out->Len() - 1] == '\n')
					*Out += "\t";
				else
					*Out += " ";

				if(*Start == ';'){
					++Start;
					SkipWhitespaceSingleLine(&Start);
					*Out += "// ";
				}

				const TCHAR* End = *Text;

				while(appIsSpace(End[-1])) // Remove trailing whitespace on comments
					--End;

				*Out += FStringTemp(static_cast<INT>(End - Start), Start);
			}
		}else if(**Text == '/' && *(*Text + 1) == '*'){
			const TCHAR* Start = *Text;

			*Text += 2;

			while(**Text){
				if(**Text == '*' && *(*Text + 1) == '/'){
					*Text += 2;

					break;
				}

				++*Text;
			}

			if(Out)
				*Out += FStringTemp(static_cast<INT>(*Text - Start), Start);
		}else if(!appIsSpace(**Text)){
			break;
		}
	}
}

void UOpenGLRenderDevice::SetHardwareShaderMacros(UHardwareShaderMacros* Macros){
	checkSlow(Macros);
	debugf("Parsing shader macros from '%s'", Macros->GetPathName());

	HardwareShaderMacros = Macros;

	const TCHAR* Pos = *Macros->Macros;

	// Extract leading comments into special macro so that they are included in the generated file
	{
		FStringTemp Comments("", true);
		SkipWhitespaceAndComments(&Pos, &Comments);

		if(Comments.Len() > 0){
			FString* PrevComments = ShaderMacros.Find("__COMMENTS__");

			if(PrevComments){
				if(Comments != *PrevComments) // Append new comments if they are not identical to the previous ones
					*PrevComments += "\n" + Comments;
			}else{
				ShaderMacros["__COMMENTS__"] = Comments;
			}
		}
	}

	while(*Pos){
		if(*Pos == ';' || *Pos == '/'){
			SkipWhitespaceAndComments(&Pos);
		}else if(*Pos == '{'){
			const TCHAR* First = Pos + 1;

			while(*Pos && *Pos != '}')
				++Pos;

			if(*Pos){
				FStringTemp Name(static_cast<INT>(Pos - First), First);

				++Pos;
				First = Pos;

				while(*Pos && *Pos != '{')
					++Pos;

				FStringTemp MacroText(static_cast<INT>(Pos - First), First);
				FString MacroGLSL;
				bool UsesFog = false;

				if(ConvertD3DAssemblyToGLSL(*MacroText, &MacroGLSL, &UsesFog)){
					if(UsesFog)
						MacroGLSL += "\n#define MACRO_FOG\n";

					ShaderMacros[*Name] = MacroGLSL;
				}else{
					debugf(NAME_Error, "Failed to convert hardware shader macro '%s' to GLSL", *Name);
				}
			}
		}else{
			++Pos;
		}
	}
}

void UOpenGLRenderDevice::ExpandShaderMacros(FString* Text){
	const TCHAR* Pos = **Text;

	while(*Pos){
		if(*Pos == '$'){
			const TCHAR* First = Pos;

			++Pos;

			while(appIsAlnum(*Pos) || *Pos == '_')
				++Pos;

			FStringTemp Name(static_cast<INT>(Pos - First) - 1, First + 1);
			FString* MacroTextPtr = ShaderMacros.Find(*Name);
			FString MacroText;

			if(MacroTextPtr)
				MacroText = **MacroTextPtr;
			else
				debugf(NAME_Error, "Unknown shader macro '%s'", *Name);

			FString Tmp;

			for(INT i = 0; i < ExpandedMacros.Num(); ++i){
				if(ExpandedMacros[i] == Name){
					for(INT j = 0; j < ExpandedMacros.Num(); ++j)
						Tmp += ExpandedMacros[j] + "->";

					Tmp += Name;

					debugf(NAME_Error, "Recursive shader macro '%s'", *Tmp);

					return;
				}
			}

			// Expand macros within macro
			Tmp = MacroText;
			ExpandedMacros.AddItem(Name);
			ExpandShaderMacros(&Tmp);
			ExpandedMacros.Pop();

			INT MacroOffset = static_cast<INT>(First - **Text);

			*Text = FStringTemp(MacroOffset, **Text) + Tmp + Pos;
			Pos = **Text + MacroOffset + Tmp.Len();
		}else{
			++Pos;
		}
	}
}

void UOpenGLRenderDevice::ParseGLSLMacros(const FString& Text){
	const TCHAR* Pos = *Text;

	while(*Pos){
		if(*Pos == '@'){
			++Pos;

			const TCHAR* First = Pos;

			while(appIsAlnum(*Pos) || *Pos == '_')
				++Pos;

			if(*Pos){
				FStringTemp Name(static_cast<INT>(Pos - First), First);

				First = Pos;

				while(*Pos && *Pos != '@')
					++Pos;

				ShaderMacros[*Name] = FStringTemp(static_cast<INT>(Pos - First), First);
			}
		}else{
			++Pos;
		}
	}
}

static FStringTemp GetShaderHeaderComment(UHardwareShader* Shader, const char* ShaderType){
	return FString::Printf("/*\n"
	                       " * %s - %s\n"
	                       " */\n\n",
						   Shader->GetPathName(),
	                       ShaderType);
}

FStringTemp UOpenGLRenderDevice::GLSLVertexShaderFromD3DVertexShader(UHardwareShader* Shader){
	debugf("Converting D3D shader assembly to GLSL for '%s.VertexShader'", Shader->GetPathName());

	FString D3DShaderText = Shader->VertexShaderText;
	FString VertexAttributes;

	for(INT i = 0; i < Shader->StreamMapping.Num(); ++i){
		switch(Shader->StreamMapping[i]){
		case FVF_Position:
			VertexAttributes += FString::Printf("#define v%i InPosition\n", i);
			break;
		case FVF_Normal:
			VertexAttributes += FString::Printf("#define v%i InNormal\n", i);
			break;
		case FVF_Diffuse:
			VertexAttributes += FString::Printf("#define v%i InDiffuse\n", i);
			break;
		case FVF_Specular:
			VertexAttributes += FString::Printf("#define v%i InSpecular\n", i);
			break;
		case FVF_TexCoord0:
			VertexAttributes += FString::Printf("#define v%i InTexCoord0\n", i);
			break;
		case FVF_TexCoord1:
			VertexAttributes += FString::Printf("#define v%i InTexCoord1\n", i);
			break;
		case FVF_TexCoord2:
			VertexAttributes += FString::Printf("#define v%i InTexCoord2\n", i);
			break;
		case FVF_TexCoord3:
			VertexAttributes += FString::Printf("#define v%i InTexCoord3\n", i);
			break;
		case FVF_TexCoord4:
			VertexAttributes += FString::Printf("#define v%i InTexCoord4\n", i);
			break;
		case FVF_TexCoord5:
			VertexAttributes += FString::Printf("#define v%i InTexCoord5\n", i);
			break;
		case FVF_TexCoord6:
			VertexAttributes += FString::Printf("#define v%i InTexCoord6\n", i);
			break;
		case FVF_TexCoord7:
			VertexAttributes += FString::Printf("#define v%i InTexCoord7\n", i);
			break;
		case FVF_Tangent:
			VertexAttributes += FString::Printf("#define v%i InTangent\n", i);
			break;
		case FVF_Binormal:
			VertexAttributes += FString::Printf("#define v%i InBinormal\n", i);
		}
	}

	if(VertexAttributes.Len() == 0)
		VertexAttributes += "#define v0 Position\n";

	FStringTemp GLSLShaderText = GetShaderHeaderComment(Shader, "vertex shader") +
		FString::Printf("layout(location = %i) uniform vec4 VSConstants[%i];\n\n", HSU_VSConstants, MAX_VERTEX_SHADER_CONSTANTS) +
		                "#define c VSConstants\n" +
		                VertexAttributes +
		                "#define oPos gl_Position\n"
		                "#define oD0 Diffuse\n"
		                "#define oD1 Specular\n"
		                "#define oT0 TexCoord0\n"
		                "#define oT1 TexCoord1\n"
		                "#define oT2 TexCoord2\n"
		                "#define oT3 TexCoord3\n"
		                "#define oT4 TexCoord4\n"
		                "#define oT5 TexCoord5\n"
		                "#define oT6 TexCoord6\n"
		                "#define oT7 TexCoord7\n"
		                "#define oFog Fog\n"
		                "#define oPts gl_PointSize\n\n"
		                "// Temporary registers\n"
		                "vec4 r0;\n"
		                "vec4 r1;\n"
		                "vec4 r2;\n"
		                "vec4 r3;\n"
		                "vec4 r4;\n"
		                "vec4 r5;\n"
		                "vec4 r6;\n"
		                "vec4 r7;\n"
		                "vec4 r8;\n"
		                "vec4 r9;\n"
						"vec4 r10;\n"
		                "vec4 r11;\n\n"
		                "void main(void){\n";

	FString ConvertedShaderText;
	bool UsesFog = false;

	if(!ConvertD3DAssemblyToGLSL(*D3DShaderText, &ConvertedShaderText, &UsesFog))
		appErrorf("Vertex shader conversion failed (%s)", Shader->GetPathName()); // TODO: Fall back to default implementation

	GLSLShaderText += ConvertedShaderText;

	if(UsesFog){
		GLSLShaderText += "\tFog = calculate_fog(oFog);\n"; // The shader assigns a value to the fog register, so we use that
	}else{
		GLSLShaderText += "#ifndef MACRO_FOG\n" // The base shader doesn't write to oFog but a macro might. If it does, MACRO_FOG is defined
		                      "\tFog = calculate_fog(oFog);\n"
	                      "#else\n"
		                      "\tFog = calculate_fog((LocalToCamera * vec4(InPosition.xyz, 1.0)).z);\n"
	                      "#endif\n"
	                      "}\n";
	}

	return GLSLShaderText;
}

FStringTemp UOpenGLRenderDevice::GLSLFragmentShaderFromD3DPixelShader(UHardwareShader* Shader){
	debugf("Converting D3D shader assembly to GLSL for '%s.PixelShader'", Shader->GetPathName());

	FString D3DShaderText = Shader->PixelShaderText;

	FStringTemp GLSLShaderText = GetShaderHeaderComment(Shader, "pixel shader") +
		FString::Printf("layout(location = %i) uniform vec4 PSConstants[%i];\n\n", HSU_PSConstants, MAX_PIXEL_SHADER_CONSTANTS) +
		                "#define c PSConstants\n"
		                "#define v0 Diffuse\n"
		                "#define v1 Specular\n\n"
		                "// Temporary registers\n"
		                "vec4 r0;\n"
		                "vec4 r1;\n"
		                "vec4 r2;\n"
		                "vec4 r3;\n"
		                "vec4 r4;\n"
		                "vec4 r5;\n\n"
		                "void main(void){\n";

	FString ConvertedShaderText;

	if(!ConvertD3DAssemblyToGLSL(*D3DShaderText, &ConvertedShaderText, NULL))
		appErrorf("Pixel shader conversion failed (%s)", Shader->GetPathName()); // TODO: Fall back to default implementation

	GLSLShaderText += ConvertedShaderText +
	                      "\n"
	                      "\talpha_test(r0);\n"
	                      "\tFragColor = r0;\n\n"
						  "\tif(FogEnabled)\n"
							"\t\tFragColor = apply_fog(FragColor);\n"
		              "}\n";

	return GLSLShaderText;
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

static INT GetShaderInstructionStringNumMatches(const TCHAR* Str, const TCHAR* Prev){
	INT i = 0;

	while(Str[i] == Prev[i] && Str[i] && Prev[i])
		++i;

	return i;
}

/*
 * Instruction strings are stored with a number indicating the amount of characters from the beginning that are equal to the previous instruction in the list.
 * This is done do simplify and speed up parsing but requires the instructions to be sorted alphabetically.
 */
struct FShaderInstructionString{
	const TCHAR* Str;
	INT NumMatchesWithPrev;
};

static FShaderInstructionString ShaderInstructionStrings[] = {
	{"", 0},
#define INSTRUCTION(x) {#x, GetShaderInstructionStringNumMatches(#x, ShaderInstructionStrings[INS_ ## x - 1].Str)},
	SHADER_INSTRUCTIONS
#undef INSTRUCTION
};

static EShaderInstruction ParseShaderInstructionName(const TCHAR** Text){
	INT StartIndex;

	// Ignore coissued instruction operator
	if(**Text == '+'){
		++*Text;
		SkipWhitespaceSingleLine(Text);
	}

	// Basic binary search for the first character to bring down the number of comparisons.
	if(**Text < ShaderInstructionStrings[INS_MAX / 2].Str[0]){
		StartIndex = INS_MAX / 4;

		if(**Text < ShaderInstructionStrings[StartIndex].Str[0])
			StartIndex = 0;
	}else{
		StartIndex = INS_MAX / 2 + INS_MAX / 4;

		if(**Text < ShaderInstructionStrings[StartIndex].Str[0])
			StartIndex = INS_MAX / 2;
	}

	// Find previous index where NumMatchesWithPrev == 0 which is the first one in a range of instructions starting with the same character.
	while(ShaderInstructionStrings[StartIndex].NumMatchesWithPrev != 0)
		--StartIndex;

	INT NumMatches = 0;

	for(INT i = StartIndex; appIsAlnum(**Text) && i < INS_MAX; ++i){
		if(NumMatches < ShaderInstructionStrings[i].NumMatchesWithPrev)
			continue;

		if(NumMatches > ShaderInstructionStrings[i].NumMatchesWithPrev)
			break; // Error: invalid instruction

		while(*(*Text + NumMatches) == ShaderInstructionStrings[i].Str[NumMatches])
			++NumMatches;

		if(!appIsAlnum(*(*Text + NumMatches)) && ShaderInstructionStrings[i].Str[NumMatches] == '\0'){
			*Text += NumMatches;

			return static_cast<EShaderInstruction>(i);
		}
	}

	return INS_INVALID;
}

enum EShaderExpr{
	EXPR_Float,
	EXPR_Float2,
	EXPR_Float3,
	EXPR_Float4
};

#define MAX_SHADER_EXPR 32 // Maximum length of a shader expression (i.e. an instruction or argument including the read or write mask). Arbitrary value but should be long enough for any possible case...
#define MAX_SHADER_INSTRUCTION_ARGS 4
#define MAX_SHADER_INSTRUCTION_MODIFIERS 4 // No idea what the actual limit is, but this seems sane

struct FShaderInstructionArg{
	EShaderExpr ExprType;
	INT RegisterIndex;
	TCHAR Register;
	TCHAR Swizzle[5];
	bool Invert;
	bool Negate;
	bool ModulateX2;
	bool Bias;
	bool BiasX2;
};

struct FShaderInstructionModifier{
	bool Saturate;  // Whether to clamp the result of the instruction to the 0.0-1.0 range
	TCHAR Operator; // Either '*' or '/'
	INT Operand;    // Constant used with operator
};

struct FShaderInstruction{
	EShaderInstruction Type;
	EShaderExpr ExprType;
	TCHAR Destination[MAX_SHADER_EXPR];
	FShaderInstructionArg Args[4];
	INT NumArgs;
	FShaderInstructionModifier Modifiers[MAX_SHADER_INSTRUCTION_MODIFIERS];
	INT NumModifiers;
};

static bool ParseShaderInstructionArg(const TCHAR** Text, FShaderInstructionArg* Arg){
	appMemzero(Arg, sizeof(FShaderInstructionArg));

	if(**Text == '1'){
		Arg->Invert = true;
		++*Text;

		SkipWhitespaceSingleLine(Text);

		if(**Text != '-')
			return false;

		++*Text;

		SkipWhitespaceSingleLine(Text);
	}else if(**Text == '-'){
		Arg->Negate = true;
		++*Text;

		SkipWhitespaceSingleLine(Text);
	}

	if(!appIsAlpha(**Text))
		return false;

	Arg->Register = **Text;
	++*Text;

	SkipWhitespaceSingleLine(Text);

	if(**Text == '['){
		++*Text;
		SkipWhitespaceSingleLine(Text);
	}

	if(!appIsDigit(**Text))
		return false;

	while(appIsDigit(**Text)){
		Arg->RegisterIndex = Arg->RegisterIndex * 10 + (**Text - '0');
		++*Text;
	}

	SkipWhitespaceSingleLine(Text);

	if(**Text == ']')
		++*Text;

	if(**Text == '_'){
		++*Text;

		if(appStrncmp(*Text, "x2", 2) == 0)
			Arg->ModulateX2 = true;
		else if(appStrncmp(*Text, "bx2", 3) == 0)
			Arg->BiasX2 = true;
		else if(appStrncmp(*Text, "bias", 4) == 0)
			Arg->Bias = true;
		else
			return false;

		while(appIsAlnum(**Text))
			++*Text;
	}

	SkipWhitespaceSingleLine(Text);

	INT MaskLen = 0;

	if(**Text == '.'){
		++*Text;

		SkipWhitespaceSingleLine(Text);

		if(!appIsAlpha(**Text))
			return false;

		const TCHAR* Start = *Text;

		while(appIsAlpha(**Text))
			++*Text;

		MaskLen = static_cast<INT>(*Text - Start);

		if(MaskLen > 4)
			return false;

		appMemcpy(&Arg->Swizzle[0], Start, MaskLen);
	}

	if(MaskLen == 1)
		Arg->ExprType = EXPR_Float;
	else if(MaskLen == 2)
		Arg->ExprType = EXPR_Float2;
	else if(MaskLen == 3)
		Arg->ExprType = EXPR_Float3;
	else if(MaskLen == 4 || MaskLen == 0)
		Arg->ExprType = EXPR_Float4;
	else
		return false;

	return true;
}

static bool ParseShaderInstructionArgs(const TCHAR** Text, FShaderInstruction* Instruction){
	for(INT i = 0; i < MAX_SHADER_INSTRUCTION_ARGS; ++i){
		SkipWhitespaceSingleLine(Text);

		if(**Text == '\n' || **Text == ';' || **Text == '/' || **Text == '\0'){ // End of line or start of comment means we are done parsing the arguments
			Instruction->NumArgs = i;

			return true;
		}

		if(**Text != ',') // Arguments are expected to be comma separated
			return false;

		++*Text;

		SkipWhitespaceSingleLine(Text);

		if(!ParseShaderInstructionArg(Text, &Instruction->Args[i]))
			return false;
	}

	return false;
}

static bool ParseShaderInstruction(const TCHAR** Text, FShaderInstruction* Instruction){
	appMemzero(Instruction, sizeof(FShaderInstruction));

	Instruction->Type = ParseShaderInstructionName(Text);

	if(Instruction->Type == INS_INVALID)
		return false;

	while(**Text == '_'){
		++*Text;

		if(**Text == 'x' || **Text == 'd'){
			TCHAR Op = **Text == 'x' ? '*' : '/';

			++*Text;

			if(!appIsDigit(**Text))
				return false;

			Instruction->Modifiers[Instruction->NumModifiers].Operator = Op;
			Instruction->Modifiers[Instruction->NumModifiers].Operand = **Text - '0';
			++*Text;
		}else if(appStrncmp(*Text, "sat", 3) == 0){
			*Text += 3;
			Instruction->Modifiers[Instruction->NumModifiers].Saturate = true;
		}else{
			return false;
		}

		++Instruction->NumModifiers;
	}

	SkipWhitespaceSingleLine(Text);

	const TCHAR* Start = *Text;

	while(appIsAlnum(**Text))
		++*Text;

	INT Len = static_cast<INT>(*Text - Start);

	if(Len >= MAX_SHADER_EXPR - 1)
		return false;

	appMemcpy(Instruction->Destination, Start, Len);
	SkipWhitespaceSingleLine(Text);

	INT MaskLen = 0;

	if(**Text == '.'){
		++*Text;

		SkipWhitespaceSingleLine(Text);

		if(!appIsAlpha(**Text))
			return false;

		Start = *Text;

		while(appIsAlpha(**Text))
			++*Text;

		MaskLen = static_cast<INT>(*Text - Start);

		if(MaskLen + Len + 1 >= MAX_SHADER_EXPR - 1)
			return false;

		Instruction->Destination[Len] = '.';
		appMemcpy(&Instruction->Destination[Len + 1], Start, MaskLen);
		Len += MaskLen;
	}

	if(MaskLen == 1)
		Instruction->ExprType = EXPR_Float;
	else if(MaskLen == 2)
		Instruction->ExprType = EXPR_Float2;
	else if(MaskLen == 3)
		Instruction->ExprType = EXPR_Float3;
	else if(MaskLen == 4 || MaskLen == 0)
		Instruction->ExprType = EXPR_Float4;
	else
		return false;

	return ParseShaderInstructionArgs(Text, Instruction);
}

static void WriteShaderInstructionArg(const FShaderInstructionArg& Arg, EShaderExpr TargetType, FString* Out){
	FString Tmp;

	if(Arg.Register == 'c')
		Tmp = FString::Printf("c[%i]", Arg.RegisterIndex);
	else
		Tmp = FString::Printf("%c%i", Arg.Register, Arg.RegisterIndex);

	if(Arg.Swizzle[0])
		Tmp += FString::Printf(".%s", Arg.Swizzle);

	if(TargetType != Arg.ExprType){
		switch(Arg.ExprType){
		case EXPR_Float:
			if(TargetType == EXPR_Float2)
				Tmp = "vec2(" + Tmp + ")";
			else if(TargetType == EXPR_Float3)
				Tmp = "vec3(" + Tmp + ")";
			else if(TargetType == EXPR_Float4)
				Tmp = "vec4(" + Tmp + ")";

			break;
		case EXPR_Float2:
			if(TargetType == EXPR_Float)
				Tmp += ".x";
			else if(TargetType == EXPR_Float3)
				Tmp += ".xyy";
			else if(TargetType == EXPR_Float4)
				Tmp += ".xyyy";

			break;
		case EXPR_Float3:
			if(TargetType == EXPR_Float)
				Tmp += ".x";
			else if(TargetType == EXPR_Float2)
				Tmp += ".xy";
			else if(TargetType == EXPR_Float4)
				Tmp += ".xyzz";

			break;
		case EXPR_Float4:
			if(TargetType == EXPR_Float)
				Tmp += ".x";
			else if(TargetType == EXPR_Float2)
				Tmp += ".xy";
			else if(TargetType == EXPR_Float3)
				Tmp += ".xyz";
		}
	}

	if(Arg.Invert)
		*Out += FString::Printf("(1.0 - %s)", *Tmp);
	else if(Arg.Negate)
		*Out += FString::Printf("-%s", *Tmp);
	else if(Arg.ModulateX2)
		*Out += FString::Printf("(%s * 2)", *Tmp);
	else if(Arg.Bias)
		*Out += FString::Printf("(%s - 0.5)", *Tmp);
	else if(Arg.BiasX2)
		*Out += FString::Printf("((%s - 0.5) * 2)", *Tmp);
	else
		*Out += Tmp;
}

static bool WriteShaderInstructionRhs(FString* Out, FShaderInstruction& Instruction, EShaderExpr* ResultExpr){
	FShaderInstructionArg* Args = Instruction.Args;

#define REQUIRE_ARGS(n) \
	if(Instruction.NumArgs != n) \
		return false

	switch(Instruction.Type){
	case INS_add:
		REQUIRE_ARGS(2);
		*ResultExpr = Args[0].ExprType;
		WriteShaderInstructionArg(Args[0], *ResultExpr, Out);
		*Out += " + ";
		WriteShaderInstructionArg(Args[1], *ResultExpr, Out);
		break;
	case INS_bem:
		return false; // TODO: Implement
	case INS_cmp:
		return false; // TODO: Implement
	case INS_cnd:
		REQUIRE_ARGS(3);
		*ResultExpr = Args[1].ExprType;
		WriteShaderInstructionArg(Args[0], EXPR_Float, Out);
		*Out += " > 0.5 ? ";
		WriteShaderInstructionArg(Args[1], *ResultExpr, Out);
		*Out += " : ";
		WriteShaderInstructionArg(Args[2], *ResultExpr, Out);
		break;
	case INS_dp3:
		REQUIRE_ARGS(2);
		*ResultExpr = EXPR_Float;
		*Out += "dot(";
		WriteShaderInstructionArg(Args[0], Args[0].ExprType, Out);

		if(Args[0].ExprType != EXPR_Float3)
			*Out += ".xyz, ";

		WriteShaderInstructionArg(Args[1], Args[1].ExprType, Out);

		if(Args[1].ExprType != EXPR_Float3)
			*Out += ".xyz";

		*Out += ")";
		break;
	case INS_dp4:
		REQUIRE_ARGS(2);
		*ResultExpr = EXPR_Float;
		*Out += "dot(";
		WriteShaderInstructionArg(Args[0], EXPR_Float4, Out);
		*Out += ", ";
		WriteShaderInstructionArg(Args[1], EXPR_Float4, Out);
		*Out += ")";
		break;
	case INS_dst:
		return false; // TODO: Implement
	case INS_exp:
		REQUIRE_ARGS(1);
		*ResultExpr = EXPR_Float;
		*Out += "pow(2.0, ";
		WriteShaderInstructionArg(Args[1], *ResultExpr, Out);
		*Out += ")";
		break;
	case INS_frc:
		REQUIRE_ARGS(1);
		*ResultExpr = Args[0].ExprType;
		WriteShaderInstructionArg(Args[0], *ResultExpr, Out);
		*Out += " - floor(";
		WriteShaderInstructionArg(Args[0], *ResultExpr, Out);
		*Out += ")";
		break;
	case INS_lit:
		return false; // TODO: Implement
	case INS_log:
	case INS_logp:
		REQUIRE_ARGS(1);
		*ResultExpr = EXPR_Float;
		*Out += "log(";
		WriteShaderInstructionArg(Args[0], Args[0].ExprType, Out);
		*Out += ") / log(2.0)";
		break;
	case INS_lrp:
		REQUIRE_ARGS(3);
		*ResultExpr = Min(Args[1].ExprType, Args[2].ExprType);
		*Out += "mix(";
		WriteShaderInstructionArg(Args[2], *ResultExpr, Out);
		*Out += ", ";
		WriteShaderInstructionArg(Args[1], *ResultExpr, Out);
		*Out += ", ";
		WriteShaderInstructionArg(Args[0], Args[0].ExprType, Out);
		*Out += ")";
		break;
	case INS_m3x2:
		return false; // TODO: Implement
	case INS_m3x3:
	case INS_m3x4:
		REQUIRE_ARGS(2);
		*ResultExpr = Instruction.ExprType;

		*Out += FString::Printf("\t%s.x = dot(", Instruction.Destination);
		WriteShaderInstructionArg(Args[0], EXPR_Float3, Out);
		*Out += ", ";
		WriteShaderInstructionArg(Args[1], EXPR_Float3, Out);
		*Out += ");\n";

		*Out += FString::Printf("\t%s.y = dot(", Instruction.Destination);
		WriteShaderInstructionArg(Args[0], EXPR_Float3, Out);
		*Out += ", ";
		++Args[1].RegisterIndex;
		WriteShaderInstructionArg(Args[1], EXPR_Float3, Out);
		*Out += ");\n";

		*Out += FString::Printf("\t%s.z = dot(", Instruction.Destination);
		WriteShaderInstructionArg(Args[0], EXPR_Float3, Out);
		*Out += ", ";
		++Args[1].RegisterIndex;
		WriteShaderInstructionArg(Args[1], EXPR_Float3, Out);
		*Out += ")";
		break;
	case INS_m4x3:
		REQUIRE_ARGS(2);
		*ResultExpr = Instruction.ExprType;

		*Out += FString::Printf("\t%s.x = dot(", Instruction.Destination);
		WriteShaderInstructionArg(Args[0], EXPR_Float3, Out);
		*Out += ", ";
		WriteShaderInstructionArg(Args[1], EXPR_Float3, Out);
		*Out += ");\n";

		*Out += FString::Printf("\t%s.y = dot(", Instruction.Destination);
		WriteShaderInstructionArg(Args[0], EXPR_Float3, Out);
		*Out += ", ";
		++Args[1].RegisterIndex;
		WriteShaderInstructionArg(Args[1], EXPR_Float3, Out);
		*Out += ");\n";

		*Out += FString::Printf("\t%s.z = dot(", Instruction.Destination);
		WriteShaderInstructionArg(Args[0], EXPR_Float3, Out);
		*Out += ", ";
		++Args[1].RegisterIndex;
		WriteShaderInstructionArg(Args[1], EXPR_Float3, Out);
		*Out += ")";
		break;
	case INS_m4x4:
		REQUIRE_ARGS(2);
		*ResultExpr = Instruction.ExprType;

		*Out += FString::Printf("\t%s.x = dot(", Instruction.Destination);
		WriteShaderInstructionArg(Args[0], EXPR_Float4, Out);
		*Out += ", ";
		WriteShaderInstructionArg(Args[1], EXPR_Float4, Out);
		*Out += ");\n";

		*Out += FString::Printf("\t%s.y = dot(", Instruction.Destination);
		WriteShaderInstructionArg(Args[0], EXPR_Float4, Out);
		*Out += ", ";
		++Args[1].RegisterIndex;
		WriteShaderInstructionArg(Args[1], EXPR_Float4, Out);
		*Out += ");\n";

		*Out += FString::Printf("\t%s.z = dot(", Instruction.Destination);
		WriteShaderInstructionArg(Args[0], EXPR_Float4, Out);
		*Out += ", ";
		++Args[1].RegisterIndex;
		WriteShaderInstructionArg(Args[1], EXPR_Float4, Out);
		*Out += ");\n";

		*Out += FString::Printf("\t%s.w = dot(", Instruction.Destination);
		WriteShaderInstructionArg(Args[0], EXPR_Float4, Out);
		*Out += ", ";
		++Args[1].RegisterIndex;
		WriteShaderInstructionArg(Args[1], EXPR_Float4, Out);
		*Out += ")";
		break;
	case INS_mad:
		{
			REQUIRE_ARGS(3);
			EShaderExpr MulExpr = Max(Max(Args[0].ExprType, Args[1].ExprType), Instruction.ExprType);
			*ResultExpr = MulExpr;
			WriteShaderInstructionArg(Args[0], MulExpr, Out);
			*Out += " * ";
			WriteShaderInstructionArg(Args[1], MulExpr, Out);
			*Out += " + ";
			WriteShaderInstructionArg(Args[2], MulExpr, Out);
			break;
		}
	case INS_max:
		REQUIRE_ARGS(2);
		*ResultExpr = Args[0].ExprType;
		*Out += "max(";
		WriteShaderInstructionArg(Args[0], *ResultExpr, Out);
		*Out += ", ";
		WriteShaderInstructionArg(Args[1], *ResultExpr, Out);
		*Out += ")";
		break;
	case INS_min:
		REQUIRE_ARGS(2);
		*ResultExpr = Args[0].ExprType;
		*Out += "min(";
		WriteShaderInstructionArg(Args[0], *ResultExpr, Out);
		*Out += ", ";
		WriteShaderInstructionArg(Args[1], *ResultExpr, Out);
		*Out += ")";
		break;
	case INS_mov:
		REQUIRE_ARGS(1);
		*ResultExpr = Args[0].ExprType;
		WriteShaderInstructionArg(Args[0], *ResultExpr, Out);
		break;
	case INS_mul:
		REQUIRE_ARGS(2);
		*ResultExpr = Max(Args[0].ExprType, Args[1].ExprType);
		WriteShaderInstructionArg(Args[0], *ResultExpr, Out);
		*Out += " * ";
		WriteShaderInstructionArg(Args[1], *ResultExpr, Out);
		break;
	case INS_nop:
		break;
	case INS_rcp:
		REQUIRE_ARGS(1);
		*ResultExpr = Args[0].ExprType;
		*Out += "1 / ";
		WriteShaderInstructionArg(Args[0], *ResultExpr, Out);
		break;
	case INS_rsq:
		REQUIRE_ARGS(1);
		*ResultExpr = Args[0].ExprType;
		*Out += "inversesqrt(";
		WriteShaderInstructionArg(Args[0], *ResultExpr, Out);
		*Out += ")";
		break;
	case INS_sge:
		REQUIRE_ARGS(2);
		*ResultExpr = Args[0].ExprType;
		*Out += "saturate(sign(";
		WriteShaderInstructionArg(Args[0], *ResultExpr, Out);
		*Out += " - ";
		WriteShaderInstructionArg(Args[1], *ResultExpr, Out);
		*Out += ") + 1.0)";
		break;
	case INS_slt:
		REQUIRE_ARGS(2);
		*ResultExpr = Args[0].ExprType;
		*Out += "saturate(sign(";
		WriteShaderInstructionArg(Args[1], *ResultExpr, Out);
		*Out += " - ";
		WriteShaderInstructionArg(Args[0], *ResultExpr, Out);
		*Out += "))";
		break;
	case INS_sub:
		REQUIRE_ARGS(2);
		*ResultExpr = Args[0].ExprType;
		WriteShaderInstructionArg(Args[0], *ResultExpr, Out);
		*Out += " - ";
		WriteShaderInstructionArg(Args[1], *ResultExpr, Out);
		break;
	case INS_tex:
		REQUIRE_ARGS(0);
		*ResultExpr = EXPR_Float4;
		*Out += FString::Printf("sample_texture%c(TexCoord%c)", Instruction.Destination[1], Instruction.Destination[1]);
		break;
	case INS_texbem:
		REQUIRE_ARGS(1);
		*ResultExpr = EXPR_Float4;
		*Out += FString::Printf("sample_texture%c(TexCoord%c + ", Instruction.Destination[1], Instruction.Destination[1]);
		WriteShaderInstructionArg(Args[0], EXPR_Float4, Out);
		*Out += FString::Printf(" * TextureInfos[%c].BumpSize)", Instruction.Destination[1]);
		break;
	case INS_texbeml:
		return false; // TODO: Implement
	case INS_texcoord:
		return false; // TODO: Implement
	case INS_texcrd:
		return false; // TODO: Implement
	case INS_texdepth:
		return false; // TODO: Implement
	case INS_texdp3:
		return false; // TODO: Implement
	case INS_texdp3tex:
		return false; // TODO: Implement
	case INS_texkill:
		return false; // TODO: Implement
	case INS_texld:
		return false; // TODO: Implement
	case INS_texm3x2depth:
		return false; // TODO: Implement
	case INS_texm3x2pad:
		return false; // TODO: Implement
	case INS_texm3x2tex:
		return false; // TODO: Implement
	case INS_texm3x3:
		return false; // TODO: Implement
	case INS_texm3x3pad:
		return false; // TODO: Implement
	case INS_texm3x3spec:
		return false; // TODO: Implement
	case INS_texm3x3tex:
		return false; // TODO: Implement
	case INS_texm3x3vspec:
		return false; // TODO: Implement
	case INS_texreg2ar:
		return false; // TODO: Implement
	case INS_texreg2gb:
		return false; // TODO: Implement
	case INS_texreg2rgb:
		return false; // TODO: Implement
	default:
		return false;
	}

	return true;
}

static bool WriteShaderInstruction(FShaderInstruction& Instruction, FString* Out){
	EShaderExpr ResultExpr = EXPR_Float4;
	FString Rhs;

	if(!WriteShaderInstructionRhs(&Rhs, Instruction, &ResultExpr))
		return false;

	if(Instruction.Type >= INS_tex && Instruction.Type <= INS_texreg2rgb) // Declare variable for texture instruction
		*Out += FString::Printf("\tconst vec4 %s = ", Instruction.Destination);
	else if(Instruction.Type < INS_m3x2 || Instruction.Type > INS_m4x4) // Matrix instructions are special and write the assignment themselves
		*Out += FString::Printf("\t%s = ", Instruction.Destination);

	// Move saturate modifier to the beginning of the list since it needs to be applied first.
	for(INT i = 0; i < Instruction.NumModifiers - 1; ++i){
		if(Instruction.Modifiers[i + 1].Saturate)
			Exchange(Instruction.Modifiers[i], Instruction.Modifiers[i + 1]);
	}

	for(INT i = 0; i < Instruction.NumModifiers; ++i){
		if(Instruction.Modifiers[i].Saturate)
			*Out += "saturate(";
		else
			*Out += "(";
	}

	if(Instruction.ExprType != ResultExpr){
		const TCHAR* WriteMask = appStrstr(Instruction.Destination, ".");

		if(WriteMask && Instruction.ExprType <= ResultExpr){
			*Out += FString::Printf("(%s)%s", *Rhs, WriteMask);
		}else{
			switch(Instruction.ExprType){
			case EXPR_Float:
				*Out += "(" + Rhs + ").x";
				break;
			case EXPR_Float2:
				if(ResultExpr == EXPR_Float)
					*Out += "vec2(" + Rhs + ")";
				else
					*Out += "(" + Rhs + ").xy";

				break;
			case EXPR_Float3:
				if(ResultExpr == EXPR_Float)
					*Out += "vec3(" + Rhs + ")";
				else if(ResultExpr == EXPR_Float2)
					*Out += "(" + Rhs + ").xyy";
				else if(ResultExpr == EXPR_Float4)
					*Out += "(" + Rhs + ").xyz";

				break;
			case EXPR_Float4:
				if(ResultExpr == EXPR_Float)
					*Out += "vec4(" + Rhs + ")";
				else if(ResultExpr == EXPR_Float2)
					*Out += "(" + Rhs + ").xyyy";
				else if(ResultExpr == EXPR_Float3)
					*Out += "(" + Rhs + ").xyzz";
			}
		}
	}else{
		*Out += Rhs;
	}

	for(INT i = 0; i < Instruction.NumModifiers; ++i){
		if(Instruction.Modifiers[i].Saturate)
			*Out += ")";
		else
			*Out += FString::Printf(" %c %i)", Instruction.Modifiers[i].Operator, Instruction.Modifiers[i].Operand);
	}

	*Out += ";";

	return true;
}

bool UOpenGLRenderDevice::ConvertD3DAssemblyToGLSL(const TCHAR* Text, FString* Out, bool* UsesFog){
	SkipWhitespaceAndComments(&Text);

	// Skip shader type and version. We don't care and just assume the highest supported versions are vs.1.1 and ps.1.4
	if((*Text == 'v' || *Text == 'p') && Text[1] == 's'){
		while(*Text && *Text != '\n')
			++Text;
	}

	while(*Text && appIsSpace(*Text)) // Skip initial whitespace until first comment or instruction
		++Text;

	SkipWhitespaceAndComments(&Text, Out);

	FShaderInstruction Instruction;

	while(*Text){
		if(*Text != '{'){
			if(!ParseShaderInstruction(&Text, &Instruction))
				return false;

			if(!WriteShaderInstruction(Instruction, Out))
				return false;

			if(UsesFog){
				if(!*UsesFog && appStrncmp(Instruction.Destination, "oFog", 4) == 0)
					*UsesFog = true;
			}
		}else{
			const TCHAR* First = Text + 1;

			while(*Text && *Text != '}')
				++Text;

			if(*Text){
				*Out += "\t$" + FStringTemp(static_cast<INT>(Text - First), First);
				++Text;
			}
		}

		if(*Text)
			SkipWhitespaceAndComments(&Text, Out);
	}

	if((*Out)[Out->Len() - 1] != '\n')
		*Out += "\n";

	return true;
}
