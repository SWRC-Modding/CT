#include "ShaderGenerator.h"
#include "OpenGLResource.h"
#include "OpenGLRenderDevice.h"

FStringTemp FShaderGenerator::GetShaderText(bool UseStaticLighting){
	BYTE TexCoordsBySrc[TCS_MAX] = {0};
	INT NumTexMatrices = 0;
	BYTE NumTexCoords = 0;
	FString VertexShaderText;
	FString FragmentShaderText;

	for(INT i = 0; i < NumTextures; ++i){
		BYTE TexCoordIndex;
		BYTE TexCoordSrcIndex = Textures[i].TexCoordSrc;
		bool HaveTexCoord = false;

		if(TexCoordsBySrc[TexCoordSrcIndex] && Textures[i].Matrix < 0){
			TexCoordIndex = TexCoordsBySrc[TexCoordSrcIndex] - 1;
			HaveTexCoord = true;
		}else{
			TexCoordIndex = NumTexCoords;

			if(Textures[i].Matrix < 0)
				TexCoordsBySrc[TexCoordSrcIndex] = ++NumTexCoords;
			else
				++NumTexCoords;
		}

		if(!HaveTexCoord){
			VertexShaderText += FString::Printf("\tTexCoord%i = ", TexCoordIndex);

			FString TexCoord;

			switch(Textures[i].TexCoordSrc){
			case TCS_Stream0:
			case TCS_Stream1:
			case TCS_Stream2:
			case TCS_Stream3:
			case TCS_Stream4:
			case TCS_Stream5:
			case TCS_Stream6:
			case TCS_Stream7:
				TexCoord = FString::Printf("InTexCoord%i", Textures[i].TexCoordSrc);
				break;
			case TCS_WorldCoords:
				TexCoord += "vec4(InPosition.xyz, 1.0)";
				break;
			case TCS_CameraCoords:
				TexCoord += "(WorldToCamera * vec4(Position, 1.0))";
				break;
			case TCS_CubeWorldSpaceReflection:
				TexCoord += "vec4(reflect(normalize(Position - CameraToWorld[3].xyz), InNormal.xyz), 0.0)";
				break;
			case TCS_CubeCameraSpaceReflection:
				TexCoord = "(WorldToCamera * vec4(reflect(normalize(Position - CameraToWorld[3].xyz), InNormal.xyz), 0.0))";
				break;
			case TCS_ProjectorCoords:
				TexCoord = "vec4(Normal, 1.0)";
				break;
			case TCS_NoChange:
				appErrorf("TCS_NoChange cannot be used as a texture coordinate source");
				break;
			case TCS_SphereWorldSpaceReflection:
				TexCoord = "(vec4(reflect(normalize(Position - CameraToWorld[3].xyz), InNormal.xyz), 0.0) * vec4(0.5, -0.5, 0.5, 1.0) + 0.5)";
				break;
			case TCS_SphereCameraSpaceReflection:
				TexCoord = "(WorldToCamera * vec4(reflect(normalize(Position - CameraToWorld[3].xyz), InNormal.xyz), 0.0) * vec4(0.5, -0.5, 0.5, 1.0) + 0.5)";
				break;
			case TCS_CubeWorldSpaceNormal:
				TexCoord += "InNormal";
				break;
			case TCS_CubeCameraSpaceNormal:
				TexCoord = "(WorldToCamera * vec4(InNormal.xyz, 0.0))";
				break;
			case TCS_SphereWorldSpaceNormal:
				TexCoord = "(InNormal * vec4(0.5, -0.5, 0.5, 1.0) + 0.5)";
				break;
			case TCS_SphereCameraSpaceNormal:
				TexCoord = "(WorldToCamera * vec4(InNormal.xyz, 0.0) * vec4(0.5, -0.5, 0.5, 1.0) + 0.5)";
				break;
			case TCS_BumpSphereCameraSpaceNormal:
			case TCS_BumpSphereCameraSpaceReflection:
				//TexCoord = "normalize(LocalToWorld * vec4(InNormal.zyz, 0.0) * vec4(-0.5, -0.5, 0.5, 1.0) + vec4(0.5, 0.5, 0.5, 0.0))";
				TexCoord = "normalize(WorldToCamera * vec4(InNormal.xyz, 0.0) * vec4(0.5, -0.5, 0.5, 1.0) + 0.5)";
				break;
			default:
				appErrorf("Invalid texture coordinate source (%i)", Textures[i].TexCoordSrc);
			}

			if(Textures[i].Matrix >= 0){
				switch(Textures[i].TexCoordCount){
				case TCN_2DCoords:
					TexCoord = "vec4(" + TexCoord + ".xy" + ", 1.0, 1.0)";
					break;
				case TCN_3DCoords:
					TexCoord = "vec4(" + TexCoord + ".xyz" + ", 1.0)";
				}

				TexCoord += FString::Printf(" * TexMatrices[%i]", Textures[i].Matrix);
				NumTexMatrices = Max(NumTexMatrices, Textures[i].Matrix + 1);
			}

			VertexShaderText += TexCoord + ";\n";

			if(Textures[i].bTexCoordProjected){
				switch(Textures[i].TexCoordCount){
				case TCN_3DCoords:
					VertexShaderText += FString::Printf("\tTexCoord%i.xy /= TexCoord%i.z;\n", TexCoordIndex, TexCoordIndex);
					break;
				case TCN_4DCoords:
					VertexShaderText += FString::Printf("\tTexCoord%i.xyz /= TexCoord%i.w;\n", TexCoordIndex, TexCoordIndex);
				}
			}
		}

		if(Textures[i].Bumpmap > 0){
			INT BumpIndex = INDEX_NONE;

			for(INT b = 0; b < i; ++b){
				if(Textures[b].Index == Textures[i].Bumpmap){
					BumpIndex = b;
					break;
				}
			}

			INT TexIndex = Textures[i].Index;

			check(BumpIndex >= 0 && BumpIndex < i);

			FragmentShaderText += FString::Printf("\tconst vec4 t%i = sample_texture%i(TexCoord%i + t%i * TextureInfos[%i].BumpSize) * vec4(vec3(saturate(t%i.a * TextureInfos[%i].BumpLumaScale + TextureInfos[%i].BumpLumaOffset)), 1.0);\n", i, TexIndex, TexCoordIndex, BumpIndex, TexIndex, BumpIndex, TexIndex, TexIndex);
		}else{
			FragmentShaderText += FString::Printf("\tconst vec4 t%i = sample_texture%i(TexCoord%i);\n", i, Textures[i].Index, TexCoordIndex);
		}
	}

	for(INT i = 0; i < NumColorOps; ++i){
		const char* SrcSwizzle = "";
		const char* DestSwizzle = "";

		switch(ColorOps[i].Channel){
		case CC_RGB:
			SrcSwizzle = ".rgb";
			DestSwizzle = ".rgb";
			break;
		case CC_A:
			SrcSwizzle = ".a";
			DestSwizzle = ".a";
			break;
		case CC_A_TO_RGB:
			SrcSwizzle = ".a";
			DestSwizzle = ".rgb";
		}

		FragmentShaderText += FString::Printf("\tr%i%s = ", ColorOps[i].Dest, DestSwizzle);

		FString Arg1RGBA = GetArgString(ColorOps[i].Arg1);
		FString Arg1 = Arg1RGBA + SrcSwizzle;
		FString Arg2 = GetArgString(ColorOps[i].Arg2) + SrcSwizzle;

		if(ColorOps[i].Modifier == COPM_Saturate)
			FragmentShaderText += "saturate(";

		if(ColorOps[i].Channel == CC_A_TO_RGB || ColorOps[i].Channel == CC_A_TO_RGBA)
			FragmentShaderText += "(";

		switch(ColorOps[i].Op){
		case COP_Arg1:
			FragmentShaderText += Arg1;
			break;
		case COP_Arg2:
			FragmentShaderText += Arg2;
			break;
		case COP_Add:
			FragmentShaderText += Arg1 + " + " + Arg2;
			break;
		case COP_Subtract:
			FragmentShaderText += Arg2 + " - " + Arg1;
			break;
		case COP_Modulate:
			FragmentShaderText += Arg1 + " * " + Arg2;
			break;
		case COP_Modulate2X:
			FragmentShaderText += Arg1 + " * " + Arg2 + " * 2";
			break;
		case COP_Modulate4X:
			FragmentShaderText += Arg1 + " * " + Arg2 + " * 4";
			break;
		case COP_ModulateAddDest:
			FragmentShaderText += Arg1 + " * " + Arg2 + FString::Printf(" + r%i%s", ColorOps[i].Dest, SrcSwizzle);
			break;
		case COP_AlphaBlend:
			FragmentShaderText += "mix(" + Arg1 + ", " + Arg2 + ", " + Arg1RGBA + ".a)";
			break;
		case COP_AlphaBlendInverted:
			FragmentShaderText += "mix(" + Arg2 + ", " + Arg1 + ", " + Arg1RGBA + ".a)";
			break;
		case COP_AddAlphaModulate:
			FragmentShaderText += Arg1 + " + " + Arg2 + " * " + Arg1RGBA + ".a";
			break;
		case COP_BlendDiffuseAlpha:
			FragmentShaderText += Arg1 + " * Diffuse.a + " + Arg2 + " * (1 - Diffuse.a)";
		}

		if(ColorOps[i].Channel == CC_A_TO_RGB)
			FragmentShaderText += ").aaa";
		else if(ColorOps[i].Channel == CC_A_TO_RGBA)
			FragmentShaderText += ").aaaa";

		if(ColorOps[i].Modifier == COPM_Saturate)
			FragmentShaderText += ")";

		FragmentShaderText += ";\n";
	}

	FStringTemp ShaderText =
		"#ifdef VERTEX_SHADER\n" +
			(NumTexMatrices > 0 ? FString::Printf("layout(location = 0) uniform mat4 TexMatrices[%i];\n\n", NumTexMatrices) : "") +
		"void main(void){\n"
		"\tPosition = (LocalToWorld * vec4(InPosition.xyz, 1.0)).xyz;\n"
		"\tNormal = normalize((LocalToWorld * vec4(InNormal.xyz, 0.0)).xyz);\n"
		"\tDiffuse = InDiffuse;\n"
		"\tSpecular = InSpecular;\n"
		"\tFog = calculate_fog((LocalToCamera * vec4(InPosition.xyz, 1.0)).z);\n"
		"\tgl_Position = LocalToScreen * vec4(InPosition.xyz, 1.0);\n" +
			VertexShaderText +
		"}\n"
		"#elif defined(FRAGMENT_SHADER)\n"
		"vec3 light_color(void){\n"
		"\tvec3 DiffuseLight = " + (UseStaticLighting ? "Diffuse.rgb" : "vec3(0)") + ";\n"
		"\tvec3 NormalizedNormal = normalize(Normal);\n"
		"\tfor(int i = 0; i < 4; ++i){\n"
		"\t\tfloat DiffuseFactor;\n"
		"\t\tif(Lights[i].Type == SL_Directional){\n"
		"\t\t\tDiffuseFactor = saturate(dot(-Lights[i].Direction.xyz, NormalizedNormal));\n"
		"\t\t}else{\n"
		"\t\t\tvec3 Dir = Lights[i].Position.xyz - Position;\n"
		"\t\t\tvec3 NormalizedDir = normalize(Dir);\n"
		"\t\t\tfloat Dist = length(Dir);\n"
		"\t\t\tDiffuseFactor = saturate(dot(NormalizedDir, NormalizedNormal));\n"
		"\t\t\tif(Lights[i].Type == SL_Spot)\n"
		"\t\t\t\tDiffuseFactor *= saturate((dot(NormalizedDir, normalize(-Lights[i].Direction.xyz)) - Lights[i].Cone * 0.9) / 0.03); // Values hand-tuned to match d3d\n"
		"\t\t\t// Attenuation\n"
		"\t\t\tDiffuseFactor *= 1.0 / (Lights[i].Constant + Lights[i].Linear * Dist + Lights[i].Quadratic * (Dist * Dist));\n"
		"\t\t}\n"
		"\t\tDiffuseLight += Lights[i].Color.rgb * DiffuseFactor;\n"
		"\t}\n"
		"\treturn AmbientLightColor.rgb " + (UseStaticLighting ? " * Specular.rgb" : "") + " + DiffuseLight;\n"
		"}\n"
		"void main(void){\n"
		"\tvec4 r0;\n"
		"\tvec4 r1;\n"
		"\tvec4 r2;\n"
		"\tvec4 r3;\n"
		"\tvec4 r4;\n"
		"\tvec4 r5;\n"
		"\tvec4 r6 = vec4(0.0); // EmissionFactor\n" +
			FragmentShaderText +
		"\tr0.rgb *= ColorFactor.rgb;\n"
		"\tif(UseDynamicLighting)\n"
		"\t\tr0.rgb = mix(r0.rgb * light_color() * LightFactor, r0.rgb, r6.rgb);\n" +
		(UseStaticLighting ? "\telse\n\t\tr0 = mix(r0 * Diffuse, r0, r6);\n" : "") +
		"\talpha_test(r0);\n"
		"\tFragColor = FogEnabled ? apply_fog(r0) : r0;\n"
		"}\n"
		"#else\n"
		"#error Unsupported shader type\n"
		"#endif\n";

	return ShaderText;
}

FStringTemp FShaderGenerator::GetArgString(BYTE Arg){
	if(Arg <= CA_T7)
		return FString::Printf("t%i", Arg);

	if(Arg >= CA_T0R && Arg <= CA_T0A)
		return FString::Printf("t0.%c", "rgba"[Arg - CA_T0R]);

	if(Arg >= CA_R0 && Arg <= CA_EmissionFactor)
		return FString::Printf("r%i", Arg - CA_R0);

	// Diffuse is specular and specular is diffuse... TODO: Rename them to something like color1 and color2 so this makes more sense
	if(Arg == CA_Diffuse)
		return "Specular";

	if(Arg == CA_Specular)
		return "Diffuse";

	if(Arg == CA_GlobalColor)
		return "GlobalColor";

	if(Arg == CA_DiffuseAlpha)
		return "vec4(Specular.a)";

	if(Arg == CA_Const1)
		return "vec4(1)";

	checkSlow(!"Unhandled color arg");

	return "Diffuse";
}
