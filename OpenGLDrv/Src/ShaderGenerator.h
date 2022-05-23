#pragma once

class FOpenGLShader;
class UOpenGLRenderDevice;

#define MAX_SHADER_TEXTURE_REGISTERS 8
#define MAX_SHADER_COLOR_OPERATIONS 32
#define MAX_TEMP_REGISTERS 4

enum EColorArg{
	CA_T0,
	CA_T1,
	CA_T2,
	CA_T3,
	CA_T4,
	CA_T5,
	CA_T6,
	CA_T7,
	CA_T0R,
	CA_T0G,
	CA_T0B,
	CA_T0A,
	CA_R0,
	CA_R1,
	CA_R2,
	CA_R3,
	CA_R4,
	CA_R5,
	CA_EmissionFactor,
	CA_Diffuse,
	CA_Specular,
	CA_GlobalColor,
	CA_DiffuseAlpha,
	CA_Const1
};

enum EColorOp{
	COP_Arg1,
	COP_Arg2,
	COP_Add,
	COP_Subtract,
	COP_Modulate,
	COP_Modulate2X,
	COP_Modulate4X,
	COP_ModulateAddDest,
	COP_AlphaBlend,
	COP_AlphaBlendInverted,
	COP_AddAlphaModulate,
	COP_BlendDiffuseAlpha
};

enum EColorOpModifier{
	COPM_None,
	COPM_Saturate
};

enum EColorChannel{
	CC_RGBA,
	CC_RGB,
	CC_A,
	CC_A_TO_RGB,
	CC_A_TO_RGBA
};

enum EColorRegister{
	CR_0,
	CR_1,
	// The following registers should not be used directly
	CR_2,
	CR_3,
	CR_4,
	CR_5,
	CR_EmissionFactor
};

class FShaderGenerator{
public:
	FShaderGenerator(){
		Reset();
	}

	void Reset(){
		TempRegister = 0;
		NumTextures = 0;
		NumColorOps = 0;
	}

	EColorArg AddTexture(INT Index, ETexCoordSrc TexCoordSrc, ETexCoordCount TexCoordCount = TCN_2DCoords, SBYTE Matrix = INDEX_NONE, bool bTexCoordProjected = false, INT Bumpmap = INDEX_NONE){
		checkSlow(Index < 8);
		checkSlow(Bumpmap < Index);

		if(NumTextures >= MAX_SHADER_TEXTURE_REGISTERS)
			appThrowf("Exceeded maximum amount of shader texture registers (%i)", MAX_SHADER_TEXTURE_REGISTERS);

		Textures[NumTextures].Index = static_cast<BYTE>(Index);
		Textures[NumTextures].TexCoordSrc = static_cast<BYTE>(TexCoordSrc);
		Textures[NumTextures].TexCoordCount = static_cast<BYTE>(TexCoordCount);
		Textures[NumTextures].Matrix = Matrix;
		Textures[NumTextures].bTexCoordProjected = bTexCoordProjected;
		Textures[NumTextures].Bumpmap = static_cast<SBYTE>(Bumpmap);

		return static_cast<EColorArg>(NumTextures++);
	}

	void AddColorOp(EColorArg Arg1, EColorArg Arg2, EColorOp Op, EColorChannel Channel, EColorRegister Dest, EColorOpModifier Modifier = COPM_None){
		if(NumColorOps >= MAX_SHADER_COLOR_OPERATIONS)
			appThrowf("Exceeded maximum amount of shader color operations (%i)", MAX_SHADER_COLOR_OPERATIONS);

		ColorOps[NumColorOps].Dest = static_cast<BYTE>(Dest);
		ColorOps[NumColorOps].Arg1 = static_cast<BYTE>(Arg1);
		ColorOps[NumColorOps].Arg2 = static_cast<BYTE>(Arg2);
		ColorOps[NumColorOps].Op = static_cast<BYTE>(Op);
		ColorOps[NumColorOps].Channel = static_cast<BYTE>(Channel);
		ColorOps[NumColorOps].Modifier = static_cast<BYTE>(Modifier);
		++NumColorOps;
	}

	EColorRegister PushTempRegister(){
		if(TempRegister >= MAX_TEMP_REGISTERS)
			appThrowf("Used more than available temporary shader registers");

		return static_cast<EColorRegister>(CR_2 + TempRegister++);
	}

	EColorArg PopTempRegister(){
		checkSlow(TempRegister > 0);
		return static_cast<EColorArg>(CA_R2 + --TempRegister);
	}

	DWORD GetShaderId(bool UseStaticLighting) const{
		return appMemhash(ColorOps, NumColorOps * sizeof(ColorOps[0]), appMemhash(Textures, NumTextures * sizeof(Textures[0]), UseStaticLighting));
	}

	FStringTemp GetShaderText(bool UseStaticLighting);

private:
	struct FTextureRegister{
		BYTE  Index;
		BYTE  TexCoordSrc;
		BYTE  TexCoordCount;
		SBYTE Matrix;
		bool  bTexCoordProjected;
		SBYTE Bumpmap;
	};

	struct FColorOp{
		BYTE Dest;
		BYTE Arg1;
		BYTE Arg2;
		BYTE Op;
		BYTE Channel;
		BYTE Modifier;
	};

	INT                  TempRegister;
	INT                  NumTextures;
	INT                  NumColorOps;
	FTextureRegister     Textures[MAX_SHADER_TEXTURE_REGISTERS];
	FColorOp             ColorOps[MAX_SHADER_COLOR_OPERATIONS];

	FStringTemp GetArgString(BYTE Arg);
};
