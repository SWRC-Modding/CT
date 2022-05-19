#include "OpenGLRenderInterface.h"
#include "OpenGLRenderDevice.h"
#include "OpenGLResource.h"

UMaterial* FOpenGLRenderInterface::RemoveModifiers(UModifier* Modifier, FModifierInfo* ModifierInfo){
	UModifier* FirstModifier = Modifier;
	UMaterial* Material = NULL;

	while(Modifier){
		Material = Modifier->Material;
		Modifier = Cast<UModifier>(Material);
	}

	// Collect modifiers

	if(Material && ModifierInfo){
		Modifier = FirstModifier;

		// Apply modifiers
		while(Modifier != Material){
			if(Modifier->IsA<UTexModifier>()){
				UTexModifier* TexModifier = static_cast<UTexModifier*>(Modifier);

				if(TexModifier->TexCoordSource != TCS_NoChange){
					ModifierInfo->TexCoordSrc = static_cast<ETexCoordSrc>(TexModifier->TexCoordSource);
					ModifierInfo->TexCoordCount = static_cast<ETexCoordCount>(TexModifier->TexCoordCount);
				}

				FMatrix* Matrix = TexModifier->GetMatrix(GEngineTime);

				if(Matrix && *Matrix != FMatrix::Identity){
					ModifierInfo->TexMatrix *= *Matrix;
					ModifierInfo->bUseTexMatrix = 1;
				}

				ModifierInfo->TexUClamp = static_cast<ETexClampModeOverride>(TexModifier->UClampMode);
				ModifierInfo->TexVClamp = static_cast<ETexClampModeOverride>(TexModifier->VClampMode);
			}else if(Modifier->IsA<UFinalBlend>()){
				UFinalBlend* FinalBlend = static_cast<UFinalBlend*>(Modifier);

				if(!ModifyFramebufferBlending){
					ModifyFramebufferBlending = true;
					SetFramebufferBlending(static_cast<EFrameBufferBlending>(FinalBlend->FrameBufferBlending));
					CurrentState->bZTest = FinalBlend->ZTest != 0;
					CurrentState->bZWrite = FinalBlend->ZWrite != 0;
				}

				if(FinalBlend->TwoSided)
					CurrentState->CullMode = CM_None;

				if(FinalBlend->AlphaTest){
					CurrentState->AlphaRef = FinalBlend->AlphaRef / 255.0f;
					++CurrentState->UniformRevision;
				}
			}else if(Modifier->IsA<UColorModifier>()){
				UColorModifier* ColorModifier = static_cast<UColorModifier*>(Modifier);

				CurrentState->ModifyColor = 1;
				CurrentState->ColorFactor = ColorModifier->Color;
				++CurrentState->UniformRevision;

				if(ColorModifier->RenderTwoSided)
					CurrentState->CullMode = CM_None;

				if(!ModifyFramebufferBlending && ColorModifier->AlphaBlend)
					SetFramebufferBlending(FB_AlphaBlend);
			}

			Modifier = static_cast<UModifier*>(Modifier->Material);
		}
	}

	return Material;
}

void FOpenGLRenderInterface::GetShaderConstants(FSConstantsInfo* Info, FPlane* Constants, INT NumConstants){
	if(CurrentState->MatricesChanged)
		UpdateMatrices();

	FMatrix Matrix;

	for(INT i = 0; i < NumConstants; ++i){
		switch(Info[i].Type){
		case EVC_Unused:
			continue;
		case EVC_MaterialDefined:
			Constants[i] = Info[i].Value;
			continue;
		case EVC_WorldToScreenMatrix:
			Matrix = CurrentState->WorldToScreen.Transpose();
			appMemcpy(&Constants[i], &Matrix, sizeof(FLOAT) * 16);
			i += 3;
			continue;
		case EVC_ObjectToScreenMatrix:
			Matrix = CurrentState->LocalToScreen.Transpose();
			appMemcpy(&Constants[i], &Matrix, sizeof(FLOAT) * 16);
			i += 3;
			continue;
		case EVC_ObjectToWorldMatrix:
			Matrix = CurrentState->LocalToWorld.Transpose();
			appMemcpy(&Constants[i], &Matrix, sizeof(FLOAT) * 16);
			i += 3;
			continue;
		case EVC_CameraToWorldMatrix:
			Matrix = CurrentState->CameraToWorld.Transpose();
			appMemcpy(&Constants[i], &Matrix, sizeof(FLOAT) * 16);
			i += 3;
			continue;
		case EVC_WorldToCameraMatrix:
			Matrix = CurrentState->WorldToCamera.Transpose();
			appMemcpy(&Constants[i], &Matrix, sizeof(FLOAT) * 16);
			i += 3;
			continue;
		case EVC_WorldToObjectMatrix:
			Matrix = CurrentState->WorldToLocal.Transpose();
			appMemcpy(&Constants[i], &Matrix, sizeof(FLOAT) * 16);
			i += 3;
			continue;
		case EVC_Time:
			Constants[i].X = CurrentState->Time;
			Constants[i].Y = CurrentState->Time;
			Constants[i].Z = CurrentState->Time;
			Constants[i].W = CurrentState->Time;
			continue;
		case EVC_CosTime:
			Constants[i].X = CurrentState->CosTime;
			Constants[i].Y = CurrentState->CosTime;
			Constants[i].Z = CurrentState->CosTime;
			Constants[i].W = CurrentState->CosTime;
			continue;
		case EVC_SinTime:
			Constants[i].X = CurrentState->SinTime;
			Constants[i].Y = CurrentState->SinTime;
			Constants[i].Z = CurrentState->SinTime;
			Constants[i].W = CurrentState->SinTime;
			continue;
		case EVC_TanTime:
			Constants[i].X = CurrentState->TanTime;
			Constants[i].Y = CurrentState->TanTime;
			Constants[i].Z = CurrentState->TanTime;
			Constants[i].W = CurrentState->TanTime;
			continue;
		case EVC_EyePosition:
			Constants[i].X = CurrentState->CameraToWorld.M[3][0];
			Constants[i].Y = CurrentState->CameraToWorld.M[3][1];
			Constants[i].Z = CurrentState->CameraToWorld.M[3][2];
			Constants[i].W = CurrentState->CameraToWorld.M[3][3];
			continue;
		case EVC_XYCircle:
			Constants[i].X = CurrentState->CosTime * 500.0f;
			Constants[i].Y = CurrentState->SinTime * 500.0f;
			Constants[i].Z = 0.0f;
			Constants[i].W = 1.0f;
			continue;
		case EVC_LightPos1:
		case EVC_LightColor1:
		case EVC_LightInvRadius1:
		case EVC_LightPos2:
		case EVC_LightColor2:
		case EVC_LightInvRadius2:
		case EVC_LightPos3:
		case EVC_LightColor3:
		case EVC_LightInvRadius3:
		case EVC_LightPos4:
		case EVC_LightColor4:
		case EVC_LightInvRadius4:
		case EVC_SpotlightDirection:
		case EVC_SpotlightCosCone:
			{
				FDynamicLight** ShaderLights = CurrentState->HardwareShaderLights;
				FDynamicLight* Spotlight = ShaderLights[0];
				BYTE ConstantType = Info[i].Type;

				if(ConstantType == EVC_SpotlightDirection){
					if(Spotlight && Spotlight->Actor && Spotlight->Actor->LightCone > 0){
						Constants[i].X = Spotlight->Direction.X;
						Constants[i].Y = Spotlight->Direction.Y;
						Constants[i].Z = Spotlight->Direction.Z;
						Constants[i].W = 1.0f;
					}else{
						Constants[i].X = 1.0f;
						Constants[i].Y = 0.0f;
						Constants[i].Z = 0.0f;
						Constants[i].W = 1.0f;
					}

					continue;
				}else if(ConstantType == EVC_SpotlightCosCone){
					if(Spotlight && Spotlight->Actor && Spotlight->Actor->LightCone > 0){
						FLOAT CosCone = Spotlight->Actor->LightCone * 0.00097659999f;

						Constants[i].X = Square(1.0f - CosCone) * 1.075f;
						Constants[i].Y = Constants[i].X * 0.1;
						Constants[i].Z = 1.0f / Constants[i].Y;
						Constants[i].W = 1.0f;
					}else{
						Constants[i].X = 0.0f;
						Constants[i].Y = 0.0f;
						Constants[i].Z = 0.0f;
						Constants[i].W = 1.0f;
					}

					continue;
				}else{
					FDynamicLight* Light = ShaderLights[(ConstantType - EVC_LightPos1) / 3];
					BYTE LightConstant = (ConstantType - EVC_LightPos1) % 3; // 0 = Position, 1 = Color, 2 = InvRadius

					switch(LightConstant){
					case 0: // Position
						if(Light){
							// Transform light from world to object space
							Constants[i] = CurrentState->WorldToLocal.TransformFPlane(FPlane(Light->Position, 1.0f));
						}else{
							Constants[i].X = 10000000.0f;
							Constants[i].Y = 10000000.0f;
							Constants[i].Z = 10000000.0f;
							Constants[i].W = 0.0f;
						}

						continue;
					case 1: // Color
						if(Light)
							Constants[i] = Light->Color * 2.0f;
						else
							Constants[i] = FPlane(0.0f, 0.0f, 0.0f, 0.0f);

						continue;
					case 2: // InvRadius
						if(Light){
#if 0
							// NOTE: This version of the inv radius constants should be correct as it produces the same values the d3d renderer does.
							//       However, for an unknown reason it causes some objects to be very bright so until the problem is known, there's a
							//       workaround which produces incorrect but acceptable results.
							FLOAT InnerRadius = Light->Radius * (Light->Actor->LightRadiusInner / 256.0f);

							Constants[i].X = 1.0f / (Light->Radius - InnerRadius);
							Constants[i].Y = Constants[i].X;
							Constants[i].Z = InnerRadius;
							Constants[i].W = 1.0f;
#else
							Constants[i].X = 1.0f / Light->Radius;
							Constants[i].Y = Constants[i].X;
							Constants[i].Z = 0.0f;
							Constants[i].W = 1.0f;
#endif
						}else{
							Constants[i].X = 10000000.0f;
							Constants[i].Y = 10000000.0f;
							Constants[i].Z = 10000000.0f;
							Constants[i].W = 0.0f;
						}

						continue;
					}
				}

				appErrorf("Shader light constant problem"); // This should never be reached
			}
		case EVC_AmbientLightColor:
			Constants[i] = CurrentState->AmbientLightColor * 2;
			continue;
		case EVC_Flicker:
			{
				static FLOAT LastEngineTime = GEngineTime;
				static FLOAT Rand1 = 0.5f;
				static FLOAT Rand2 = 0.5f;
				static FLOAT Rand3 = 0.5f;

				if(LastEngineTime != GEngineTime){ // Don't update flicker if the game is paused
					LastEngineTime = GEngineTime;
					Rand1 = appRand() / (float)RAND_MAX;
					Rand2 = appRand() / (float)RAND_MAX;
					Rand3 = appRand() / (float)RAND_MAX;
				}

				FLOAT X    = Info[i].Value.X;
				FLOAT Y    = Info[i].Value.Y;
				FLOAT YInv = 1.0f - Y;

				Constants[i].X = Rand1 <= X ? 1.0f : Y * Rand3 + YInv;
				Constants[i].Y = Rand2 <= X ? 1.0f : Y * Rand1 + YInv;
				Constants[i].Z = Rand3 <= X ? 1.0f : Y * Rand2 + YInv;
				Constants[i].W = 1.0f;
			}
			continue;
		case EVC_DrawScale3D:
			if(GCubemapManager && GCubemapManager->Actor){
				Constants[i].X = GCubemapManager->Actor->DrawScale3D.X;
				Constants[i].Y = GCubemapManager->Actor->DrawScale3D.Y;
				Constants[i].Z = GCubemapManager->Actor->DrawScale3D.Z;
			}else{
				Constants[i].X = CurrentState->LocalToWorld.M[0][0];
				Constants[i].Y = CurrentState->LocalToWorld.M[1][1];
				Constants[i].Z = CurrentState->LocalToWorld.M[2][2];
			}

			Constants[i].W = 1.0f;
			continue;
		case EVC_Fog:
			continue; // TODO: Implement Fog
		case EVC_ObjectToCameraMatrix:
			Matrix = CurrentState->LocalToCamera.Transpose();
			appMemcpy(&Constants[i], &Matrix, sizeof(FLOAT) * 16);
			i += 3;
			continue;
		case EVC_EyePositionObjectSpace:
			Constants[i] = CurrentState->WorldToLocal.TransformFPlane(FPlane(CurrentState->CameraToWorld.M[3][0],
			                                                                 CurrentState->CameraToWorld.M[3][1],
			                                                                 CurrentState->CameraToWorld.M[3][2],
			                                                                 CurrentState->CameraToWorld.M[3][3]));
			continue;
		case EVC_2DRotator:
			Constants[i].X = CurrentState->CosTime;
			Constants[i].Y = -CurrentState->SinTime;
			Constants[i].Z = 0.0f;
			Constants[i].W = 0.0f;
			++i;
			Constants[i].X = CurrentState->SinTime;
			Constants[i].Y = CurrentState->CosTime;
			Constants[i].Z = 0.0f;
			Constants[i].W = 0.0f;
			continue;
		default:
			appErrorf("Invalid shader constant (%i)", Info[i].Type);
		}
	}
}

void FOpenGLRenderInterface::SetTexture(FBaseTexture* Texture, INT TextureIndex, FLOAT UVScale, FLOAT BumpSize){
	checkSlow(Texture);
	checkSlow(TextureIndex < MAX_TEXTURES);

	QWORD CacheId = Texture->GetCacheId();
	FOpenGLTexture* GLTexture = static_cast<FOpenGLTexture*>(RenDev->GetCachedResource(CacheId));

	if(!GLTexture)
		GLTexture = new FOpenGLTexture(RenDev, CacheId);

	if(GLTexture->Revision != Texture->GetRevision())
		GLTexture->Cache(Texture);

	FOpenGLTextureUnit& TextureUnit = CurrentState->TextureUnits[TextureIndex];
	FOpenGLGlobalUniforms::TextureInfo& TextureInfo = CurrentState->TextureInfos[TextureIndex];

	TextureUnit.Texture = GLTexture;
	CurrentState->NumTextures = Max(CurrentState->NumTextures, TextureIndex + 1);

	UBOOL WasCubemap = TextureInfo.IsCubemap;

	if(GLTexture->IsCubemap){
		TextureInfo.IsCubemap = 1;
		TextureUnit.ClampU = TC_Clamp;
		TextureUnit.ClampV = TC_Clamp;
	}else{
		TextureInfo.IsCubemap = 0;

		if(GLTexture->FBO == GL_NONE){
			TextureUnit.ClampU = Texture->GetUClamp();
			TextureUnit.ClampV = Texture->GetVClamp();
		}else{
			 // Render targets should use TC_Clamp to avoid artifacts at the edges of the screen
			TextureUnit.ClampU = TC_Clamp;
			TextureUnit.ClampV = TC_Clamp;
		}
	}

	UBOOL WasBumpmap = TextureInfo.IsBumpmap;
	FLOAT OldUVScale = TextureInfo.UVScale;
	FLOAT OldBumpSize = TextureInfo.BumpSize;

	TextureInfo.IsBumpmap = IsBumpmap(Texture->GetFormat());
	TextureInfo.UVScale = UVScale;
	TextureInfo.BumpSize = BumpSize;

	if(WasCubemap != TextureInfo.IsCubemap || WasBumpmap != TextureInfo.IsBumpmap || OldUVScale != UVScale || OldBumpSize != TextureInfo.BumpSize)
		++CurrentState->UniformRevision;
}

void FOpenGLRenderInterface::SetBitmapTexture(UBitmapMaterial* Bitmap, INT TextureIndex, FLOAT UVScale, FLOAT BumpSize){
	FBaseTexture* Texture = Bitmap->Get(LockedViewport->CurrentTime, LockedViewport)->GetRenderInterface();
	SetTexture(Texture, TextureIndex, UVScale, BumpSize);
}

void FOpenGLRenderInterface::SetGeneratedShader(FShaderGenerator& ShaderGenerator){
	FOpenGLShader* Shader = &ShadersById[ShaderGenerator.GetShaderId(CurrentState->UseStaticLighting)];

	if(!Shader->IsValid()){
		Shader->RenDev = RenDev;
		Shader->Compile(ShaderGenerator.GetShaderText(CurrentState->UseStaticLighting));
	}

	if(NumTexMatrices > 0)
		RenDev->glProgramUniformMatrix4fv(Shader->Program, 0, NumTexMatrices, GL_TRUE, reinterpret_cast<GLfloat*>(TexMatrices));

	SetShader(*Shader);
}

bool FOpenGLRenderInterface::SetBitmapMaterial(UBitmapMaterial* Material){
	UBitmapMaterial* Detail = NULL;

	if(Material->IsA<UTexture>()){
		Detail = Cast<UBitmapMaterial>(static_cast<UTexture*>(Material)->Detail);

		// If the material is a simple texture, use it to get the blending options
		if(!ModifyFramebufferBlending){
			UTexture* Texture = static_cast<UTexture*>(Material);

			if(Texture->bMasked){
				ModifyFramebufferBlending = true;
				CurrentState->UniformRevision += CurrentState->AlphaRef != 0.5f;
				CurrentState->AlphaRef = 0.5f;
				SetFramebufferBlending(FB_Overwrite);
			}else if(Texture->bAlphaTexture){
				ModifyFramebufferBlending = true;
				CurrentState->UniformRevision += CurrentState->AlphaRef != 0.0f;
				CurrentState->AlphaRef = 0.0f;
				SetFramebufferBlending(FB_AlphaBlend);
			}

			if(Texture->bTwoSided)
				CurrentState->CullMode = CM_None;
		}
	}

	SetBitmapTexture(Material, 0);

	if(Detail)
		SetBitmapTexture(Detail, 2, static_cast<UTexture*>(Material)->DetailScale);

	FOpenGLShader* Shader;

	if(!CurrentState->Lightmap){
		if(CurrentState->UseStaticLighting){
			if(Detail)
				Shader = &BitmapShaderStaticLightingDetail;
			else
				Shader = &BitmapShaderStaticLighting;
		}else{
			if(Detail)
				Shader = &BitmapShaderDetail;
			else
				Shader = &BitmapShader;
		}
	}else{
		SetTexture(CurrentState->Lightmap, 1);

		if(CurrentState->UseStaticLighting || !CurrentState->LightingModulate2X){
			if(CurrentState->UseStaticLighting){
				if(Detail)
					Shader = &BitmapShaderLightmapStaticLightingDetail;
				else
					Shader = &BitmapShaderLightmapStaticLighting;
			}else{
				if(Detail)
					Shader = &BitmapShaderLightmapDetail;
				else
					Shader = &BitmapShaderLightmap;
			}
		}else{
			if(Detail)
				Shader = &BitmapShaderLightmap2XDetail;
			else
				Shader = &BitmapShaderLightmap2X;
		}

	}

	checkSlow(Shader->IsValid());
	SetShader(*Shader);

	return true;
}

bool FOpenGLRenderInterface::SetShaderMaterial(UShader* Shader, const FModifierInfo& ModifierInfo){
	try{
		FShaderGenerator ShaderGenerator;

		HandleShaderMaterial(Shader, ShaderGenerator, ModifierInfo);

		if(!ModifyFramebufferBlending){
			CurrentState->bZTest = true;
			CurrentState->bZWrite = true;

			if(Shader->TwoSided)
				CurrentState->CullMode = CM_None;

			if(Shader->Wireframe)
				CurrentState->FillMode = FM_Wireframe;

			switch(Shader->OutputBlending){
			case OB_Normal:
				if(Shader->Opacity){
					CurrentState->AlphaRef = 0.0f;
					++CurrentState->UniformRevision;
					SetFramebufferBlending(FB_AlphaBlend);
					CurrentState->bZWrite = false;
				}

				break;
			case OB_Masked:
				CurrentState->AlphaRef = 0.5f;
				++CurrentState->UniformRevision;
				SetFramebufferBlending(FB_Overwrite);
				break;
			case OB_Modulate:
				SetFramebufferBlending(FB_Modulate);
				break;
			case OB_Translucent:
				SetFramebufferBlending(FB_Translucent);
				break;
			case OB_Invisible:
				SetFramebufferBlending(FB_Invisible);
				break;
			case OB_Brighten:
				SetFramebufferBlending(FB_Brighten);
				break;
			case OB_Darken:
				SetFramebufferBlending(FB_Darken);
			}
		}

		SetGeneratedShader(ShaderGenerator);

		return true;
	}catch(TCHAR* Error){
		debugf("Error setting shader material '%s': %s", Shader->GetPathName(), Error);
		return false;
	}
}

void FOpenGLRenderInterface::HandleSimpleMaterial(UMaterial* Material, FShaderGenerator& ShaderGenerator, const FModifierInfo* InModifierInfo){
	FModifierInfo ModifierInfo = InModifierInfo ? *InModifierInfo : FModifierInfo();

	if(Material->IsA<UModifier>())
		Material = RemoveModifiers(static_cast<UModifier*>(Material), &ModifierInfo);

	if(Material->IsA<UBitmapMaterial>()){
		INT Index = CurrentState->NumTextures++;

		SetBitmapTexture(static_cast<UBitmapMaterial*>(Material), Index);

		SBYTE Matrix = INDEX_NONE;

		if(ModifierInfo.bUseTexMatrix){
			Matrix = static_cast<SBYTE>(NumTexMatrices++);
			TexMatrices[Matrix] = ModifierInfo.TexMatrix;
		}

		EColorArg TextureArg = ShaderGenerator.AddTexture(Index, ModifierInfo.TexCoordSrc, ModifierInfo.TexCoordCount, Matrix);

		ShaderGenerator.AddColorOp(TextureArg, TextureArg, COP_Assign, CC_RGBA, CR_0);

		UTexture* Texture = Cast<UTexture>(Material);

		if(Texture && Texture->Detail && Texture->Detail->IsA<UBitmapMaterial>()){
			Index = CurrentState->NumTextures++;
			SetBitmapTexture(static_cast<UBitmapMaterial*>(Texture->Detail), Index, Texture->DetailScale);
			EColorArg DetailArg = ShaderGenerator.AddTexture(Index, TCS_Stream0);
			ShaderGenerator.AddColorOp(DetailArg, TextureArg, COP_Modulate2X, CC_RGB, CR_0);
		}
	}else if(Material->IsA<UCombiner>()){
		HandleCombinerMaterial(static_cast<UCombiner*>(Material), ShaderGenerator, false);
	}else if(Material->IsA<UConstantMaterial>()){
		CurrentState->GlobalColor = static_cast<UConstantMaterial*>(Material)->GetColor(GEngineTime);
		++CurrentState->UniformRevision;
		ShaderGenerator.AddColorOp(CA_GlobalColor, CA_GlobalColor, COP_Assign, CC_RGB, CR_0);
		ShaderGenerator.AddColorOp(CA_Const1, CA_Const1, COP_Assign, CC_A, CR_0); // Constant color alpha is not used
	}else if(Material->IsA<UVertexColor>()){
		ShaderGenerator.AddColorOp(CA_Diffuse, CA_Specular, COP_Assign, CC_RGBA, CR_0);
	}else{
		appThrowf("Material of type %s is not a simple material", Material->GetClass()->GetPathName());
	}
}

void FOpenGLRenderInterface::HandleCombinerMaterial(UCombiner* Combiner, FShaderGenerator& ShaderGenerator, bool IsRootMaterial){
	UMaterial* Material1 = Combiner->Material1;

	if(!Material1)
		appThrowf("Combiner Material1 is not set for %s", Combiner->GetPathName());

	HandleSimpleMaterial(Material1, ShaderGenerator);

	if(CurrentState->Lightmap && !UsingLightmap)
		UseLightmap(ShaderGenerator);

	EColorRegister Material1Register = ShaderGenerator.PushTempRegister();

	ShaderGenerator.AddColorOp(CA_R0, CA_R0, COP_Assign, CC_RGBA, Material1Register);

	UMaterial* Material2 = Combiner->Material2;
	bool HaveUniqueMaterial2 = false;

	if(Material2 && Material2 != Material1){
		HandleSimpleMaterial(Material2, ShaderGenerator);
		ShaderGenerator.AddColorOp(CA_R0, CA_R0, COP_Assign, CC_RGBA, ShaderGenerator.PushTempRegister());
		HaveUniqueMaterial2 = true;
	}

	UMaterial* Mask = Combiner->Mask;
	EColorArg MaskArg = CA_Specular;

	// Incorrectly configured combiners use Material1 as the mask
	if(!Mask && (Combiner->CombineOperation >= CO_AlphaBlend_With_Mask || Combiner->AlphaOperation == AO_Use_Mask || Combiner->AlphaOperation == AO_AlphaBlend_With_Mask))
		Mask = Material1;

	if(Mask && Mask != Material1 && Mask != Material2 && !Mask->IsA<UVertexColor>()){
		HandleSimpleMaterial(Mask, ShaderGenerator);
		ShaderGenerator.AddColorOp(CA_R0, CA_R0, COP_Assign, CC_RGBA, CR_1);
		MaskArg = CA_R1;
	}

	EColorArg Material2Arg = CA_Diffuse;

	if(HaveUniqueMaterial2)
		Material2Arg = ShaderGenerator.PopTempRegister();

	EColorArg Material1Arg = ShaderGenerator.PopTempRegister();

	if(Material2 == Material1)
		Material2Arg = Material1Arg;

	if(Mask == Material1)
		MaskArg = Material1Arg;
	else if(Mask == Material2)
		MaskArg = Material2Arg;

	if(Mask && IsRootMaterial && !Combiner->LightBothMaterials)
		ShaderGenerator.AddColorOp(MaskArg, MaskArg, COP_Assign, CC_RGBA, CR_LightMixFactor);

	switch(Combiner->CombineOperation){
	case CO_Use_Color_From_Material1:
		ShaderGenerator.AddColorOp(Material1Arg, Material1Arg, COP_Assign, CC_RGB, CR_0);
		break;
	case CO_Use_Color_From_Material2:
		ShaderGenerator.AddColorOp(Material2Arg, Material2Arg, COP_Assign, CC_RGB, CR_0);
		break;
	case CO_Multiply:
		ShaderGenerator.AddColorOp(Material1Arg, Material2Arg, Combiner->Modulate4X ? COP_Modulate4X : Combiner->Modulate2X ? COP_Modulate2X : COP_Modulate, CC_RGB, CR_0);
		break;
	case CO_Add:
		ShaderGenerator.AddColorOp(Material1Arg, Material2Arg, COP_Add, CC_RGB, CR_0);
		break;
	case CO_Subtract:
		ShaderGenerator.AddColorOp(Material1Arg, Material2Arg, COP_Subtract, CC_RGB, CR_0);
		break;
	case CO_AlphaBlend_With_Mask:
		ShaderGenerator.AddColorOp(MaskArg, MaskArg, COP_Assign, CC_A, Material1Register);
		ShaderGenerator.AddColorOp(Material1Arg, Material2Arg, Combiner->InvertMask ? COP_AlphaBlendInverted : COP_AlphaBlend, CC_RGB, CR_0);
		break;
	case CO_Add_With_Mask_Modulation:
		ShaderGenerator.AddColorOp(Material1Arg, Material2Arg, COP_AddAlphaModulate, CC_RGB, CR_0);
		break;
	case CO_Use_Color_From_Mask:
		ShaderGenerator.AddColorOp(MaskArg, MaskArg, COP_Assign, CC_RGB, CR_0);
	}

	switch(Combiner->AlphaOperation){
	case AO_Use_Mask:
		ShaderGenerator.AddColorOp(MaskArg, MaskArg, COP_Assign, CC_A, CR_0);
		break;
	case AO_Multiply:
		ShaderGenerator.AddColorOp(Material1Arg, Material2Arg, COP_Modulate, CC_A, CR_0);
		break;
	case AO_Add:
		ShaderGenerator.AddColorOp(Material1Arg, Material2Arg, COP_Add, CC_A, CR_0);
		break;
	case AO_Use_Alpha_From_Material1:
		ShaderGenerator.AddColorOp(Material1Arg, Material1Arg, COP_Assign, CC_A, CR_0);
		break;
	case AO_Use_Alpha_From_Material2:
		ShaderGenerator.AddColorOp(Material2Arg, Material2Arg, COP_Assign, CC_A, CR_0);
		break;
	case AO_AlphaBlend_With_Mask:
		ShaderGenerator.AddColorOp(MaskArg, MaskArg, COP_Assign, CC_A, Material1Register);
		ShaderGenerator.AddColorOp(Material1Arg, Material2Arg, COP_AlphaBlend, CC_A, CR_0);
	}
}

void FOpenGLRenderInterface::HandleShaderMaterial(UShader* Shader, FShaderGenerator& ShaderGenerator, const FModifierInfo& ModifierInfo){
	bool HaveDiffuse = Shader->Diffuse != NULL && (!Shader->SelfIllumination || Shader->SelfIlluminationMask);

	if(HaveDiffuse){
		HandleSimpleMaterial(Shader->Diffuse, ShaderGenerator, &ModifierInfo);

		if(CurrentState->Lightmap && !UsingLightmap)
			UseLightmap(ShaderGenerator);

		ShaderGenerator.AddColorOp(CA_R0, CA_R0, COP_Assign, CC_RGBA, ShaderGenerator.PushTempRegister());
	}

	bool HaveSelfIllumination = Shader->SelfIllumination != NULL;

	if(HaveSelfIllumination){
		if(Shader->SelfIlluminationMask){
			bool HaveUniqueSelfIllumination = false;

			if(HaveDiffuse && Shader->SelfIllumination != Shader->Diffuse){
				HandleSimpleMaterial(Shader->SelfIllumination, ShaderGenerator, &ModifierInfo);
				ShaderGenerator.AddColorOp(CA_R0, CA_R0, COP_Assign, CC_RGBA, ShaderGenerator.PushTempRegister());
				HaveUniqueSelfIllumination = true;
			}

			EColorArg SelfIlluminationMaskArg = CA_Diffuse;

			if(Shader->SelfIlluminationMask != Shader->SelfIllumination && Shader->SelfIlluminationMask != Shader->Diffuse){
				HandleSimpleMaterial(Shader->SelfIlluminationMask, ShaderGenerator, &ModifierInfo);
				ShaderGenerator.AddColorOp(CA_R0, CA_R0, COP_Assign, CC_RGBA, CR_1);
				SelfIlluminationMaskArg = CA_R1;
			}

			EColorArg SelfIlluminationArg = CA_Diffuse;

			if(HaveUniqueSelfIllumination)
				SelfIlluminationArg = ShaderGenerator.PopTempRegister();

			EColorArg DiffuseArg = HaveDiffuse ? ShaderGenerator.PopTempRegister() : CA_Diffuse;

			if(Shader->SelfIllumination == Shader->Diffuse)
				SelfIlluminationArg = DiffuseArg;

			if(Shader->SelfIlluminationMask == Shader->SelfIllumination)
				SelfIlluminationMaskArg = SelfIlluminationArg;
			else if(Shader->SelfIlluminationMask == Shader->Diffuse)
				SelfIlluminationMaskArg = DiffuseArg;

			ShaderGenerator.AddColorOp(DiffuseArg, DiffuseArg, COP_Assign, CC_A, ShaderGenerator.PushTempRegister()); // Save diffuse alpha
			ShaderGenerator.AddColorOp(SelfIlluminationArg, SelfIlluminationArg, COP_Assign, CC_RGB, CR_1);

			if(SelfIlluminationMaskArg != CA_R1)
				ShaderGenerator.AddColorOp(SelfIlluminationMaskArg, SelfIlluminationMaskArg, COP_Assign, CC_A, CR_1);

			ShaderGenerator.AddColorOp(DiffuseArg, CA_R1, COP_AlphaBlend, CC_RGB, CR_0);
			ShaderGenerator.AddColorOp(ShaderGenerator.PopTempRegister(), CA_R0, COP_Assign, CC_A, CR_0); // Restore saved alpha
			ShaderGenerator.AddColorOp(CA_R1, CA_R1, COP_Assign, CC_RGBA, CR_LightMixFactor);
		}else{
			HandleSimpleMaterial(Shader->SelfIllumination, ShaderGenerator, &ModifierInfo);
			// TODO: Maybe just disable dynamic lighting here
			ShaderGenerator.AddColorOp(CA_Const1, CA_Const1, COP_Assign, CC_RGBA, CR_LightMixFactor);
		}
	}

	if(!HaveDiffuse && !HaveSelfIllumination)
		ShaderGenerator.AddColorOp(CA_Diffuse, CA_Diffuse, COP_Assign, CC_RGBA, CR_0);

	if(!HaveDiffuse && CurrentState->Lightmap && !UsingLightmap)
		UseLightmap(ShaderGenerator);

	if(Shader->Opacity){
		if((HaveDiffuse && Shader->Opacity != Shader->Diffuse) || (HaveSelfIllumination && Shader->Opacity != Shader->SelfIllumination)){
			ShaderGenerator.AddColorOp(CA_R0, CA_R0, COP_Assign, CC_RGB, ShaderGenerator.PushTempRegister());
			HandleSimpleMaterial(Shader->Opacity, ShaderGenerator, &ModifierInfo);
			ShaderGenerator.AddColorOp(ShaderGenerator.PopTempRegister(), CA_R0, COP_Assign, CC_RGB, CR_0);
		}
	}

	if(Shader->Detail && Shader->Detail->IsA<UBitmapMaterial>()){
		INT Index = CurrentState->NumTextures++;
		SetBitmapTexture(static_cast<UBitmapMaterial*>(Shader->Detail), Index, Shader->DetailScale);
		EColorArg DetailArg = ShaderGenerator.AddTexture(Index, TCS_Stream0);
		ShaderGenerator.AddColorOp(DetailArg, CA_R0, COP_Modulate2X, CC_RGB, CR_0);
	}

	if(GCubemapManager && GCubemapManager->bEnabled){
		if(Shader->Bumpmap){
			checkSlow(Shader->Bumpmap->IsA<UBitmapMaterial>());

			INT BumpmapIndex = CurrentState->NumTextures++;
			INT EnvMapIndex = CurrentState->NumTextures++;

			bool UseMatrix = false;
			FMatrix* Matrix = NULL;
			UMaterial* EnvMap = Shader->DiffuseEnvMap;

			if(!EnvMap)
				EnvMap = GCubemapManager->GetEnvLightmap(static_cast<EBumpMapType>(Shader->BumpMapType), Shader, &Matrix, UseMatrix);

			if(!EnvMap)
				EnvMap = GCubemapManager->StaticDiffuse;

			checkSlow(EnvMap->IsA<UBitmapMaterial>());

			SetBitmapTexture(static_cast<UBitmapMaterial*>(Shader->Bumpmap), BumpmapIndex, Shader->BumpUVScale, Shader->BumpSize);
			SetBitmapTexture(static_cast<UBitmapMaterial*>(EnvMap), EnvMapIndex);
			ShaderGenerator.AddTexture(BumpmapIndex, TCS_Stream0);
			ShaderGenerator.AddTexture(EnvMapIndex, TCS_BumpSphereCameraSpaceNormal, TCN_2DCoords, INDEX_NONE, BumpmapIndex);

			EColorOp ColorOp = Shader->BumpMapType == BMT_Static_Specular || Shader->BumpMapType == BMT_Specular ? COP_Add : COP_Modulate2X;
			ShaderGenerator.AddColorOp(CA_R0, static_cast<EColorArg>(CA_T0 + EnvMapIndex), ColorOp, CC_RGBA, CR_0);
		}
	}
}

void FOpenGLRenderInterface::UseLightmap(FShaderGenerator& ShaderGenerator){
	checkSlow(CurrentState->Lightmap);
	INT Index = CurrentState->NumTextures++;
	SetTexture(CurrentState->Lightmap, Index);
	EColorArg TextureArg = ShaderGenerator.AddTexture(Index, TCS_Stream1);
	ShaderGenerator.AddColorOp(CA_R0, TextureArg, CurrentState->LightingModulate2X ? COP_Modulate2X : COP_Modulate, CC_RGB, CR_0);
	UsingLightmap = true;
}

bool FOpenGLRenderInterface::SetCombinerMaterial(UCombiner* Combiner){
	try{
		FShaderGenerator ShaderGenerator;

		HandleCombinerMaterial(Combiner, ShaderGenerator, true);
		SetGeneratedShader(ShaderGenerator);

		return true;
	}catch(const TCHAR* Error){
		debugf("Error setting combiner material '%s': %s", Combiner->GetPathName(), Error);
		return false;
	}
}

bool FOpenGLRenderInterface::SetParticleMaterial(UParticleMaterial* Material){
	bool UseSpecialBlend = false;

	switch(Material->ParticleBlending){
	case PTDS_Regular:
		SetFramebufferBlending(FB_Overwrite);
		break;
	case PTDS_AlphaBlend:
		SetFramebufferBlending(FB_AlphaBlend);
		break;
	case PTDS_Modulated:
		SetFramebufferBlending(FB_Modulate);
		CurrentState->FogColor = FPlane(0.5f, 0.5f, 0.5f, 0.0f);
		CurrentState->OverrideFogColor = true;
		UseSpecialBlend = true;
		break;
	case PTDS_Translucent:
		SetFramebufferBlending(FB_Translucent);
		CurrentState->FogColor = FPlane(0.0f, 0.0f, 0.0f, 0.0f);
		CurrentState->OverrideFogColor = true;
		break;
	case PTDS_AlphaModulate_MightNotFogCorrectly:
		SetFramebufferBlending(FB_AlphaModulate_MightNotFogCorrectly);
		CurrentState->FogColor = FPlane(0.0f, 0.0f, 0.0f, 0.0f);
		CurrentState->OverrideFogColor = true;
		break;
	case PTDS_Darken:
		SetFramebufferBlending(FB_Darken);
		CurrentState->FogColor = FPlane(0.0f, 0.0f, 0.0f, 0.0f);
		CurrentState->OverrideFogColor = true;
		break;
	case PTDS_Brighten:
		CurrentState->FogColor = FPlane(0.0f, 0.0f, 0.0f, 0.0f);
		CurrentState->OverrideFogColor = true;
		SetFramebufferBlending(FB_Brighten);
	}

	CurrentState->UniformRevision += CurrentState->OverrideFogColor;

	if(Material->RenderTwoSided)
		CurrentState->CullMode = CM_None;

	if(Material->BitmapMaterial)
		SetBitmapTexture(Material->BitmapMaterial, CurrentState->NumTextures++);

	if(Material->AlphaTest){
		CurrentState->AlphaRef = Material->AlphaRef / 255.0f;
		++CurrentState->UniformRevision;
	}

	CurrentState->bZTest = Material->ZTest != 0;
	CurrentState->bZWrite = Material->ZWrite != 0;
	CurrentState->FillMode = Material->Wireframe ? FM_Wireframe : FM_Solid;

	FOpenGLShader* Shader;

	if(!Material->BlendBetweenSubdivisions){
		if(UseSpecialBlend){
			if(Material->UseTFactor)
				Shader = &ParticleShaderSpecialBlendTFactor;
			else
				Shader = &ParticleShaderSpecialBlend;
		}else{
			if(Material->UseTFactor)
				Shader = &ParticleShaderTFactor;
			else
				Shader = &ParticleShader;
		}
	}else{
		Shader = &ParticleShaderBlendSubdivisions;
	}

	/*
	 * Some testing showed that there are apparently no particle materials in the game where NumProjectors > 0.
	 * This is a good thing and means we don't need to implement that here unless one shows up at some point.
	 */
	checkSlow(Material->NumProjectors == 0);
	checkSlow(Shader->IsValid());
	SetShader(*Shader);

	return true;
}

bool FOpenGLRenderInterface::SetProjectorMaterial(UProjectorMaterial* Material){
	return false;
}

bool FOpenGLRenderInterface::SetTerrainMaterial(UTerrainMaterial* Material){
	checkSlow(Material->Layers.Num() >= 1);

	if(Material->RenderMethod == RM_AlphaMap){
		FTerrainMaterialLayer& Layer = Material->Layers[0];
		UBitmapMaterial* BitmapMaterial = Cast<UBitmapMaterial>(Layer.Texture);

		if(BitmapMaterial){
			if(!Material->FirstPass){
				SetFramebufferBlending(FB_AlphaBlend);
				CurrentState->AlphaRef = 0;
				++CurrentState->UniformRevision;
			}

			SetBitmapTexture(BitmapMaterial, 0);
			SetBitmapTexture(Layer.AlphaWeight, 1);

			CurrentState->NumTextures = 2;

			bool UseLighting = CurrentState->UseStaticLighting || CurrentState->UseDynamicLighting;
			FOpenGLShader* Shader = UseLighting ? &TerrainShaderAlphaMapBitmapLighting : &TerrainShaderAlphaMapBitmap;

			if(!Shader->IsValid()){
				FShaderGenerator ShaderGenerator;

				ShaderGenerator.AddTexture(0, TCS_WorldCoords, TCN_2DCoords, 0);
				ShaderGenerator.AddTexture(1, TCS_Stream0);
				ShaderGenerator.AddColorOp(CA_T0, CA_T0, COP_Assign, CC_RGBA, CR_0);

				if(UseLighting)
					ShaderGenerator.AddColorOp(CA_R0, CA_Diffuse, COP_Modulate2X, CC_RGB, CR_0);

				ShaderGenerator.AddColorOp(CA_T1, CA_T1, COP_Assign, CC_A, CR_0);

				Shader->Compile(ShaderGenerator.GetShaderText(false));
			}

			RenDev->glProgramUniformMatrix4fv(Shader->Program, 0, 1, GL_TRUE, reinterpret_cast<const GLfloat*>(&Layer.TextureMatrix));
			SetShader(*Shader);
		}else{
			UShader* ShaderMaterial = Cast<UShader>(Layer.Texture);

			if(!ShaderMaterial){
				debugf("Terrain material must be either a bitmap or shader");
				return false;
			}

			FShaderGenerator ShaderGenerator;
			FModifierInfo ModifierInfo;
			UMaterial* Opacity = ShaderMaterial->Opacity;

			if(Material->FirstPass)
				ModifierInfo.TexCoordSrc = TCS_WorldCoords;
			else if(Opacity)
				ShaderMaterial->Opacity = Layer.AlphaWeight;

			HandleShaderMaterial(ShaderMaterial, ShaderGenerator, ModifierInfo);
			ShaderMaterial->Opacity = Opacity;
			SetGeneratedShader(ShaderGenerator);
		}
	}else{
		if(!Material->FirstPass){
			SetFramebufferBlending(FB_Translucent);
			CurrentState->bZWrite = false;
			CurrentState->FogColor = FPlane(0.0f, 0.0f, 0.0f, 0.0f);
			CurrentState->OverrideFogColor = true;
		}

		if(Material->RenderMethod == RM_CombinedWeightMap){
			const TArray<FTerrainMaterialLayer>& Layers = Material->Layers;
			checkSlow(Layers.Num() == 3 || Layers.Num() == 4);
			checkSlow(Layers[0].Texture->IsA<UBitmapMaterial>());
			checkSlow(Layers[1].Texture->IsA<UBitmapMaterial>());
			checkSlow(Layers[2].Texture->IsA<UBitmapMaterial>());

			SetBitmapTexture(Layers[0].AlphaWeight, 0);
			SetBitmapTexture(static_cast<UBitmapMaterial*>(Layers[0].Texture), 1);
			SetBitmapTexture(static_cast<UBitmapMaterial*>(Layers[1].Texture), 2);
			SetBitmapTexture(static_cast<UBitmapMaterial*>(Layers[2].Texture), 3);

			TexMatrices[0] = Layers[0].TextureMatrix;
			TexMatrices[1] = Layers[1].TextureMatrix;
			TexMatrices[2] = Layers[2].TextureMatrix;

			// HACK:
			// WTF??? Terrain code *sometimes* produces an incorrect matrix causing the textures to be stretched.
			// M[0][0] == 0.0f is a reliable way to check for that and correct the error.
			// FIXME: Do the other RenderMethod branches also need this? Haven't seen any visual errors so far...
			if(TexMatrices[0].M[0][0] == 0.0f){
				Exchange(TexMatrices[0].M[0][0], TexMatrices[0].M[0][1]);
				Exchange(TexMatrices[0].M[2][0], TexMatrices[0].M[1][1]);
				Exchange(TexMatrices[0].M[2][1], TexMatrices[0].M[2][2]);
			}

			if(TexMatrices[1].M[0][0] == 0.0f){
				Exchange(TexMatrices[1].M[0][0], TexMatrices[1].M[0][2]);
				Exchange(TexMatrices[1].M[0][1], TexMatrices[1].M[1][1]);
				Exchange(TexMatrices[1].M[1][1], TexMatrices[1].M[2][2]);
			}

			bool Layer4 = Layers.Num() > 3;

			if(Layer4){
				SetBitmapTexture(static_cast<UBitmapMaterial*>(Layers[3].Texture), 4);
				TexMatrices[3] = Layers[3].TextureMatrix;

				if(TexMatrices[3].M[0][0] == 0.0f){
					Exchange(TexMatrices[3].M[0][0], TexMatrices[3].M[0][2]);
					Exchange(TexMatrices[3].M[0][1], TexMatrices[3].M[1][1]);
					Exchange(TexMatrices[3].M[1][1], TexMatrices[3].M[2][2]);
				}

				NumTexMatrices = 4;
			}else{
				NumTexMatrices = 3;
			}

			FOpenGLShader* Shader = Layer4 ? &TerrainShaderCombinedWeightMap4 : &TerrainShaderCombinedWeightMap3;

			if(!Shader->IsValid()){
				FShaderGenerator ShaderGenerator;

				ShaderGenerator.AddTexture(0, TCS_Stream0);
				ShaderGenerator.AddTexture(1, TCS_WorldCoords, TCN_2DCoords, 0);
				ShaderGenerator.AddTexture(2, TCS_WorldCoords, TCN_2DCoords, 1);
				ShaderGenerator.AddTexture(3, TCS_WorldCoords, TCN_2DCoords, 2);
				ShaderGenerator.AddColorOp(CA_T1, CA_T0R, COP_Modulate, CC_RGBA, CR_0);
				ShaderGenerator.AddColorOp(CA_T2, CA_T0G, COP_ModulateAddDest, CC_RGBA, CR_0);
				ShaderGenerator.AddColorOp(CA_T3, CA_T0B, COP_ModulateAddDest, CC_RGBA, CR_0);

				if(Layer4){
					ShaderGenerator.AddTexture(4, TCS_WorldCoords, TCN_2DCoords, 3);
					ShaderGenerator.AddColorOp(CA_T4, CA_T0A, COP_ModulateAddDest, CC_RGBA, CR_0);
				}

				ShaderGenerator.AddColorOp(CA_R0, CA_Specular, COP_Modulate, CC_RGBA, CR_0);
				Shader->Compile(ShaderGenerator.GetShaderText(false));
			}

			RenDev->glProgramUniformMatrix4fv(Shader->Program, 0, NumTexMatrices, GL_TRUE, reinterpret_cast<const GLfloat*>(TexMatrices));
			SetShader(*Shader);
		}else{
			FShaderGenerator ShaderGenerator;
			const TArray<FTerrainMaterialLayer>& Layers = Material->Layers;
			checkSlow(Layers[0].Texture->IsA<UBitmapMaterial>());

			SetBitmapTexture(static_cast<UBitmapMaterial*>(Layers[0].Texture), 0);
			SetBitmapTexture(static_cast<UBitmapMaterial*>(Layers[0].AlphaWeight), 1);
			TexMatrices[0] = Layers[0].TextureMatrix;
			NumTexMatrices = 1;

			ShaderGenerator.AddTexture(0, TCS_WorldCoords, TCN_2DCoords, 0);
			ShaderGenerator.AddTexture(1, TCS_Stream0);
			ShaderGenerator.AddColorOp(CA_Specular, CA_T0, COP_Modulate2X, CC_RGBA, CR_0);
			ShaderGenerator.AddColorOp(CA_R0, CA_T1, COP_Modulate, CC_RGBA, Layers.Num() == 1 ? CR_0 : CR_1);

			for(INT i = 1; i < Layers.Num(); ++i){
				checkSlow(Layers[0].Texture->IsA<UBitmapMaterial>());
				INT TextureIndex = i * 2;
				INT AlphaWeightIndex = TextureIndex + 1;

				SetBitmapTexture(static_cast<UBitmapMaterial*>(Layers[0].Texture), TextureIndex);
				SetBitmapTexture(static_cast<UBitmapMaterial*>(Layers[0].AlphaWeight), AlphaWeightIndex);
				TexMatrices[i] = Layers[i].TextureMatrix;
				++NumTexMatrices;

				ShaderGenerator.AddTexture(TextureIndex, TCS_WorldCoords, TCN_2DCoords, i);
				ShaderGenerator.AddTexture(AlphaWeightIndex, TCS_Stream0);
				ShaderGenerator.AddColorOp(static_cast<EColorArg>(CA_T0 + TextureIndex), CA_Specular, COP_Modulate2X, CC_RGBA, CR_0);
				ShaderGenerator.AddColorOp(static_cast<EColorArg>(CA_T0 + AlphaWeightIndex), CA_R0, COP_ModulateAddDest, CC_RGBA, CR_1);

				if(i == Layers.Num() - 1)
					ShaderGenerator.AddColorOp(CA_R1, CA_R1, COP_Assign, CC_RGBA, CR_0);
			}

			SetGeneratedShader(ShaderGenerator);
		}
	}

	return true;
}

bool FOpenGLRenderInterface::SetSimpleMaterial(UMaterial* Material, const FModifierInfo& ModifierInfo){
	try{
		FShaderGenerator ShaderGenerator;

		HandleSimpleMaterial(Material, ShaderGenerator, &ModifierInfo);

		if(CurrentState->Lightmap && !UsingLightmap)
			UseLightmap(ShaderGenerator);

		SetGeneratedShader(ShaderGenerator);

		return true;
	}catch(const TCHAR* Error){
		debugf("Error setting simple material '%s': %s", Material->GetPathName(), Error);
		return false;
	}
}
