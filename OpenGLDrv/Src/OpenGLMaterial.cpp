#include "../Inc/OpenGLRenderInterface.h"
#include "../Inc/OpenGLRenderDevice.h"
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
				FMatrix* Matrix = TexModifier->GetMatrix(GEngineTime);

				if(TexModifier->TexCoordSource != TCS_NoChange){
					ModifierInfo->TexCoordSrc = static_cast<ETexCoordSrc>(TexModifier->TexCoordSource);

					switch(TexModifier->TexCoordSource){
					case TCS_CameraCoords:
						ModifierInfo->TexMatrix *= CurrentState->WorldToCamera.Transpose();
						break;
					case TCS_CubeCameraSpaceReflection:
						{
							FMatrix Tmp = CurrentState->WorldToCamera;
							Tmp.M[3][0] = 0.0f;
							Tmp.M[3][1] = 0.0f;
							Tmp.M[3][2] = 0.0f;
							Tmp.M[3][3] = 1.0f;
							ModifierInfo->TexMatrix *= Tmp;
						}
					}

					ModifierInfo->TexCoordCount = static_cast<ETexCoordCount>(TexModifier->TexCoordCount);
				}

				if(Matrix)
					ModifierInfo->TexMatrix *= *Matrix;

				ModifierInfo->TexUClamp = static_cast<ETexClampModeOverride>(TexModifier->UClampMode);
				ModifierInfo->TexVClamp = static_cast<ETexClampModeOverride>(TexModifier->VClampMode);
			}else if(Modifier->IsA<UFinalBlend>()){
#if 0
				UFinalBlend* FinalBlend = static_cast<UFinalBlend*>(Modifier);

				ModifyFramebufferBlending = true;
				SetFramebufferBlending(static_cast<EFrameBufferBlending>(FinalBlend->FrameBufferBlending));
				CurrentState->bZTest = FinalBlend->ZTest != 0;
				CurrentState->bZWrite = FinalBlend->ZWrite != 0;

				if(FinalBlend->TwoSided)
					CurrentState->CullMode = CM_None;

				if(FinalBlend->AlphaTest)
					CurrentState->AlphaRef = FinalBlend->AlphaRef / 255.0f;
#endif
			}else if(Modifier->IsA<UColorModifier>()){
#if 0
				UColorModifier* ColorModifier = static_cast<UColorModifier*>(Modifier);

				UsingConstantColor = true;
				ModifyColor = true;
				ConstantColor = ColorModifier->Color;

				if(ColorModifier->RenderTwoSided)
					CurrentState->CullMode = CM_None;

				if(!ModifyFramebufferBlending && ColorModifier->AlphaBlend)
					SetFramebufferBlending(FB_AlphaBlend);
#endif
			}

			Modifier = static_cast<UModifier*>(Modifier->Material);
		}
	}

	return Material;
}

void FOpenGLRenderInterface::GetShaderConstants(FSConstantsInfo* Info, FPlane* Constants, INT NumConstants){
	if(MatricesChanged)
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
					if(Spotlight && Spotlight->Actor->LightCone > 0){
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
					if(Spotlight && Spotlight->Actor->LightCone > 0){
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

void FOpenGLRenderInterface::SetTexture(FBaseTexture* Texture, INT TextureIndex, FLOAT BumpSize){
	checkSlow(Texture);

	QWORD CacheId = Texture->GetCacheId();
	FOpenGLTexture* GLTexture = static_cast<FOpenGLTexture*>(RenDev->GetCachedResource(CacheId));

	if(!GLTexture)
		GLTexture = new FOpenGLTexture(RenDev, CacheId);

	if(GLTexture->Revision != Texture->GetRevision())
		GLTexture->Cache(Texture);

	FOpenGLTextureUnit& TextureUnit = CurrentState->TextureUnits[TextureIndex];
	FOpenGLGlobalUniforms::TextureInfo& TextureInfo = CurrentState->TextureInfos[TextureIndex];

	TextureUnit.Texture = GLTexture;

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
	FLOAT OldBumpSize = TextureInfo.BumpSize;

	TextureInfo.IsBumpmap = IsBumpmap(Texture->GetFormat());
	TextureInfo.BumpSize = BumpSize;

	if(WasCubemap != TextureInfo.IsCubemap || WasBumpmap != TextureInfo.IsBumpmap || OldBumpSize != TextureInfo.BumpSize)
		++CurrentState->UniformRevision;
}

void FOpenGLRenderInterface::SetBitmapTexture(UBitmapMaterial* Bitmap, INT TextureIndex, FLOAT BumpSize){
	FBaseTexture* Texture = Bitmap->Get(LockedViewport->CurrentTime, LockedViewport)->GetRenderInterface();
	SetTexture(Texture, TextureIndex, BumpSize);
}

