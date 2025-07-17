#include "RtxRenderDevice.h"
#include "RtxDrvPrivate.h"
#include "Window.h"
#include "Editor.h"
#include "CodeInjection.h"

IMPLEMENT_CLASS(URtxRenderDevice)

static URtxRenderDevice* GRtxRenDev = NULL;

static ULevel* GetLevel()
{
	if(GEngine->IsA<UGameEngine>())
		return static_cast<UGameEngine*>(GEngine)->GLevel;

	if(GEngine->IsA<UEditorEngine>())
		return static_cast<UEditorEngine*>(GEngine)->Level;

	return NULL;
}

typedef FConvexVolume&(__fastcall*FPlayerSceneNodeGetViewFrustum)(FLevelSceneNode* Self, DWORD Edx, FConvexVolume& Result);

static FPlayerSceneNodeGetViewFrustum OriginalFPlayerSceneNodeGetViewFrustum = NULL;

static FConvexVolume& __fastcall FPlayerSceneNodeGetViewFrustumOverride(FPlayerSceneNode* Self, DWORD Edx, FConvexVolume& Result)
{
	OriginalFPlayerSceneNodeGetViewFrustum(Self, Edx, Result);

	if(GRtxRenDev && GRtxRenDev->GetRtxInterface()->bDisableFrustumCulling)
	{
		for(INT i = 0; i < 5; i++)
			Result.BoundingPlanes[i].W += 10000.0f;
	}

	return Result;
}

// static INT GetMaterialFlags(UMaterial* Material)
// {
// 	return reinterpret_cast<INT*>(Material)[24] >> 2;
// }

// static INT SetMaterialFlags(UMaterial* Material, INT Flags)
// {
// 	reinterpret_cast<INT*>(Material)[24] = (Flags << 2) | (reinterpret_cast<INT*>(Material)[24] & 0x3);
// }

void ClearMaterialFlags()
{
	foreachobj(UMaterial, Material)
		reinterpret_cast<INT*>(*Material)[24] = (reinterpret_cast<INT*>(*Material)[24] & 0x3);
}

void URtxRenderDevice::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	if(Ar.IsGarbageCollecting())
		Ar << Rtx;
}

UBOOL URtxRenderDevice::Exec(const TCHAR* Cmd, FOutputDevice& Ar)
{
	if(ParseCommand(&Cmd, "RTX"))
	{
		if(ParseCommand(&Cmd, "TOGGLEANCHOR"))
		{
			Rtx->bDrawAnchorTriangle = !Rtx->bDrawAnchorTriangle;
			return 1;
		}
		else if(ParseCommand(&Cmd, "PREFERENCES"))
		{
			GEngine->Client->GetLastCurrent()->EndFullscreen();
			WObjectProperties* P = new WObjectProperties("EditObj", 0, "", NULL, 1);
			P->OpenWindow(GLogWindow->hWnd);
			UObject* Obj = Rtx;
			P->Root.SetObjects(&Obj, 1);
			P->Show(1);
			return 1;
		}
		else if(ParseCommand(&Cmd, "SET"))
		{
			FString Key = ParseToken(Cmd, 0);
			FString Value = ParseToken(Cmd, 0);

			if(Key.Len() == 0)
				Ar.Log("Missing variable name");
			else if(Value.Len() == 0)
				Ar.Log("Missing value");
			else
				Rtx->SetConfigVariable(*Key, *Value);

			return 1;
		}
		else if(ParseCommand(&Cmd, "CREATELIGHT"))
		{
			APlayerController* PC    = GEngine->Client->Viewports[0]->Actor;
			URtxLight*         Light = Rtx->CreateLight(true);

			if(PC->Pawn)
				Light->Position = PC->Pawn->Location;
			else
				Light->Position = PC->Location;

			if(!ParseCommand(&Cmd, "!"))
			{
				GEngine->Client->GetLastCurrent()->EndFullscreen();
				WObjectProperties* P = new WObjectProperties("EditObj", 0, "", NULL, 1);
				P->OpenWindow(GLogWindow->hWnd);
				UObject* Obj = Light;
				P->Root.SetObjects(&Obj, 1);
				P->Show(1);
			}

			return 1;
		}
		else if(ParseCommand(&Cmd, "SAVECONFIG"))
		{
			Rtx->SaveConfig();

			for(INT i = 0; i < Rtx->Components.Num(); ++i)
			{
				if(Rtx->Components[i])
				{
					Rtx->Components[i]->SaveConfig();
					Rtx->Components[i]->OnSaveConfig();
				}
			}

			return 1;
		}
	}

	return Super::Exec(Cmd, Ar);
}

void URtxRenderDevice::StaticConstructor()
{
	UseStencil             = 0;
	CanDoDistortionEffects = 0;
}

UBOOL URtxRenderDevice::Init()
{
	GRtxRenDev = this;

	// Init SWRCFix if it exists. Hacky but RenderDevice is always loaded at startup...
	HMODULE ModDLL = LoadLibraryA("Mod.dll");

	if(ModDLL)
	{
		void(CDECL*InitSWRCFix)(void) = reinterpret_cast<void(CDECL*)(void)>(GetProcAddress(ModDLL, "InitSWRCFix"));

		if(InitSWRCFix)
			InitSWRCFix();
	}

	static bool bCodeInjected = false;
	if(!bCodeInjected)
	{
		OriginalFPlayerSceneNodeGetViewFrustum = reinterpret_cast<FPlayerSceneNodeGetViewFrustum>(
			PatchDllClassVTable("Engine.dll", "FPlayerSceneNode", NULL, 10, FPlayerSceneNodeGetViewFrustumOverride));
		bCodeInjected = true;
	}

	ClearMaterialFlags();
	UClient* Client                                   = GEngine->Client;
	Client->Shadows                                   = 0;
	Client->FrameFXDisabled                           = 1;
	Client->BloomQuality                              = 0;
	Client->BlurEnabled                               = 0;
	Client->BumpmappingQuality                        = 0;
	GetDefault<APlayerController>()->bVisor           = 0;
	GetDefault<APlayerController>()->VisorModeDefault = 1;

	Rtx = new(this) URtx;

	return Super::Init();
}

void URtxRenderDevice::Exit(UViewport* Viewport)
{
	Rtx->Exit();
	delete Rtx;
	Rtx = NULL;
	Super::Exit(Viewport);
	GRtxRenDev = NULL;
}

UBOOL URtxRenderDevice::SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes, UBOOL bSaveSize)
{
	UBOOL Result = Super::SetRes(Viewport, NewX, NewY, Fullscreen, ColorBytes, bSaveSize);

	if(Result)
		Rtx->Init();

	return Result;
}

void URtxRenderDevice::Flush(UViewport* Viewport)
{
	ClearMaterialFlags();
	Super::Flush(Viewport);
}

/*
 * Check for a level change and notify the RTX interface so it can recreate the components.
 * This is the most reliable way to do it that I've been able to find.
 * UGameEngine::GLevel does not always change when restarting the level so the additional check for a non-empty travel URL is also needed.
 */
void URtxRenderDevice::CheckForLevelChange(UViewport* Viewport)
{
	static bool    bPendingLevel = false;
	static ULevel* Level         = NULL;

	if(Viewport && Viewport->TravelURL.Len() > 0)
	{
		bPendingLevel = true;
	}
	else if(bPendingLevel || Level != GetLevel())
	{
		bPendingLevel = false;
		Level = GetLevel();

		AnchorTriangleStream.Update(appStrihash(Level->GetPathName()) / (FLOAT)MAXDWORD);

		if(Rtx->bDisableSkyZones)
		{
			foreach(AllActors, AZoneInfo, Info, Level)
			{
				Info->SkyZone = NULL;
				Info->bUseSkyDome = 0;
			}
		}

		Rtx->LevelChanged(Level);
	}
}

FRenderInterface* URtxRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize)
{
	CheckForLevelChange(Viewport);

	if(Rtx->bCaptureMode)
		Viewport->Precaching = 1;

	DepthCleared = 0;

	D3D = Super::Lock(Viewport, HitData, HitSize);

	if(!D3D)
		return NULL;

	LockedViewport = Viewport;

	if(Viewport->Actor)
	{
		Viewport->Actor->bVisor = 0;
		Viewport->Actor->VisorModeDefault = 1;
	}

	return this;
}

struct FProjectileLight{
	URtxLight* Light;
	UBOOL      bUsed;
};

void URtxRenderDevice::Unlock(FRenderInterface* RI)
{
	typedef TMap<AProjectile*, FProjectileLight> FProjLightMap;
	static FProjLightMap ProjLights;

	foreach(AllActors, AProjectile, Proj, static_cast<UGameEngine*>(GEngine)->GLevel)
	{
		DECLARE_NAME(GrenadeProj);
		if(Proj->IsA(NGrenadeProj))
			continue;

		FProjectileLight* ProjLight = ProjLights.Find(*Proj);

		if(!ProjLight)
			ProjLight = &ProjLights[*Proj];

		if(!ProjLight->Light)
			ProjLight->Light = Rtx->CreateLight();

		URtxLight* Light = ProjLight->Light;
		Light->Type = RTXLIGHT_Sphere;
		Light->Position = Proj->Location;
		Light->Radiance = 10000.0f;
		Light->Color = FGetHSV(Proj->LightHue, Proj->LightSaturation, Proj->LightBrightness);
		Light->Sphere.Radius = 2.5f;
		Light->Update();

		ProjLight->bUsed = 1;
	}

	for(FProjLightMap::TIterator It(ProjLights); It; ++It)
	{
		if(!It->bUsed)
		{
			Rtx->DestroyLight(It.Value().Light);
			ProjLights.Remove(It.Key());
		}
		else
		{
			It->bUsed = 0;
		}
	}

	Rtx->RenderLights();

	if(Rtx->bCaptureMode)
		LockedViewport->Precaching = 0;

	LockedViewport = NULL;

	Super::Unlock(static_cast<URtxRenderDevice*>(RI)->D3D);
}

void URtxRenderDevice::Present(UViewport* Viewport)
{
	Super::Present(Viewport);
}

/*
 * Draw anchor triangle at world center to parent assets to in remix
 */

class USolidColorMaterial : public UConstantMaterial{
	DECLARE_CLASS(USolidColorMaterial,UConstantMaterial,0,RtxDrv)
public:
	FColor Color;
	virtual FColor GetColor(FLOAT TimeSeconds){ return Color; }
};
IMPLEMENT_CLASS(USolidColorMaterial)

void URtxRenderDevice::DrawAnchorTriangle()
{
	if(!Rtx->bDrawAnchorTriangle || !LockedViewport || !LockedViewport->Actor)
		return;

	AActor*  ViewActor      = LockedViewport->Actor;
	FVector  CameraLocation = ViewActor->Location;
	FRotator CameraRotation = ViewActor->Rotation;
	LockedViewport->Actor->PlayerCalcView(ViewActor, CameraLocation, CameraRotation);

	// Initialize the view matrix.
	FMatrix WorldToCamera = FTranslationMatrix(-CameraLocation);

	if(!LockedViewport->IsOrtho())
	{
		WorldToCamera = WorldToCamera * FInverseRotationMatrix(CameraRotation);
		WorldToCamera = WorldToCamera * FMatrix(
			FPlane(0, 0, 1, 0),
			FPlane(LockedViewport->ScaleX, 0, 0, 0),
			FPlane(0, LockedViewport->ScaleY, 0, 0),
			FPlane(0, 0, 0, 1));
	}
	else if(LockedViewport->Actor->RendMap == REN_OrthXY)
	{
		WorldToCamera = WorldToCamera * FMatrix(
			FPlane(LockedViewport->ScaleX, 0, 0, 0),
			FPlane(0, -LockedViewport->ScaleY, 0, 0),
			FPlane(0, 0, -1, 0),
			FPlane(0, 0, -CameraLocation.Z, 1));
	}
	else if(LockedViewport->Actor->RendMap == REN_OrthXZ)
	{
		WorldToCamera = WorldToCamera * FMatrix(
			FPlane(LockedViewport->ScaleX, 0, 0, 0),
			FPlane(0, 0, -1, 0),
			FPlane(0, LockedViewport->ScaleY, 0, 0),
			FPlane(0, 0, -CameraLocation.Y, 1));
	}
	else if(LockedViewport->Actor->RendMap == REN_OrthYZ)
	{
		WorldToCamera = WorldToCamera * FMatrix(
			FPlane(0, 0, 1, 0),
			FPlane(LockedViewport->ScaleX, 0, 0, 0),
			FPlane(0, LockedViewport->ScaleY, 0, 0),
			FPlane(0, 0, CameraLocation.X, 1));
	}

	// Initialize the projection matrix.
	FMatrix CameraToScreen;
	if(LockedViewport->IsOrtho())
	{
		const FLOAT Zoom = LockedViewport->Actor->OrthoZoom / (LockedViewport->SizeX * 15.0f);
		CameraToScreen = FOrthoMatrix(Zoom * LockedViewport->SizeX / 2,Zoom * LockedViewport->SizeY / 2, 0.5f / HALF_WORLD_MAX, HALF_WORLD_MAX);
	}
	else
	{
		const FLOAT FOV = LockedViewport->Actor->FovAngle * PI / 360.0f;
		CameraToScreen = FPerspectiveMatrix(FOV, FOV, 1.0f, (FLOAT)LockedViewport->SizeX / LockedViewport->SizeY, NEAR_CLIPPING_PLANE, FAR_CLIPPING_PLANE);
	}

	// Material
	DECLARE_STATIC_UOBJECT(USolidColorMaterial, Material, {});
	DECLARE_STATIC_UOBJECT(UFinalBlend, FinalBlend,
	{
		FinalBlend->Material = Material;
		FinalBlend->FrameBufferBlending = FB_Overwrite;
		FinalBlend->ZWrite = 1;
	});

	if(Material->Color != Rtx->AnchorTriangleColor)
	{
		Material->Color = Rtx->AnchorTriangleColor;
		FinalBlend->ResetCachedStates();
	}

	PushState();
	SetTransform(TT_LocalToWorld, FMatrix::Identity);
	SetTransform(TT_WorldToCamera, WorldToCamera);
	SetTransform(TT_CameraToScreen, CameraToScreen);
	SetMaterial(FinalBlend);
	SetIndexBuffer(NULL, 0);
	FVertexStream* Stream = &AnchorTriangleStream;
	SetVertexStreams(VS_FixedFunction, &Stream, 1);
	DrawPrimitive(PT_TriangleList, 0, 1);
	PopState();
}

/*
 * FRenderInterface
 */

UBOOL URtxRenderDevice::SetRenderTarget(FRenderTarget* RenderTarget, bool bOwnDepthBuffer)
{
	return D3D->SetRenderTarget(RenderTarget, bOwnDepthBuffer);
}

void URtxRenderDevice::Clear(UBOOL UseColor, FColor Color, UBOOL UseDepth, FLOAT Depth, UBOOL UseStencil, DWORD Stencil)
{
	D3D->Clear(UseColor, Color, UseDepth, Depth, UseStencil, Stencil);

	if(UseDepth && !DepthCleared)
	{
		DepthCleared = 1;
		DrawAnchorTriangle();
	}
}

void URtxRenderDevice::EnableLighting(UBOOL UseDynamic, UBOOL UseStatic, UBOOL Modulate2X, FBaseTexture* Lightmap, UBOOL LightingOnly, const FSphere& LitSphere, int IntValue)
{
	if(UseStatic && Lightmap)
		IgnoreDrawCall |= IgnoreDrawCall_Lightmap;
	else
		IgnoreDrawCall &= ~IgnoreDrawCall_Lightmap;

	UseDynamic = 0;
	Lightmap = NULL;
	Modulate2X = 0;
	D3D->EnableLighting(UseDynamic, UseStatic, Modulate2X, Lightmap, LightingOnly, LitSphere, IntValue);
}

void URtxRenderDevice::SetLight(INT LightIndex, FDynamicLight* Light, FLOAT Scale)
{
	if(Rtx->bEnableD3DLights)
		D3D->SetLight(LightIndex, Light, Scale);
}

void URtxRenderDevice::SetTransform(ETransformType Type, const FMatrix& Matrix)
{
	D3D->SetTransform(Type, Matrix);
}

void URtxRenderDevice::SetMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial, INT* NumPasses)
{
	if(Material)
	{
		if(Material->IsA<UProjectorMultiMaterial>())
			IgnoreDrawCall |= IgnoreDrawCall_Material;
		else
			IgnoreDrawCall &= ~IgnoreDrawCall_Material;

		UMaterial** ActualMaterial = &Material;

		while(*ActualMaterial && (*ActualMaterial)->IsA<UModifier>())
			ActualMaterial = &((UModifier*)*ActualMaterial)->Material;

		if(*ActualMaterial)
		{
			if(Rtx->bReplaceHardwareShaderMaterials && (*ActualMaterial)->IsA<UHardwareShaderWrapper>())
			{
				UClass* Class = (*ActualMaterial)->GetClass();

				if(Class->IsChildOf<UHsBumpDiffSpec>())
					*ActualMaterial = static_cast<UHsBumpDiffSpec*>(*ActualMaterial)->DiffuseTexture;
				else if(Class == UHsBumpDiffBlend::StaticClass())
					*ActualMaterial = static_cast<UHsBumpDiffBlend*>(*ActualMaterial)->DiffuseTexture;
				else if(Class == UHsBumpDiffBlendMask::StaticClass())
					*ActualMaterial = static_cast<UHsBumpDiffBlendMask*>(*ActualMaterial)->DiffuseTexture;
				else if(Class == UHsBumpDiffBlendMaskIllum::StaticClass())
					*ActualMaterial = static_cast<UHsBumpDiffBlendMaskIllum*>(*ActualMaterial)->DiffuseTexture;
				else if(Class == UHsFalloff::StaticClass())
					*ActualMaterial = static_cast<UHsFalloff*>(*ActualMaterial)->DiffuseTexture;
				else if(Class == UHsHologram::StaticClass())
					*ActualMaterial = static_cast<UHsHologram*>(*ActualMaterial)->DiffuseTexture;
			}
		}

		if(!*ActualMaterial)
			*ActualMaterial = GetDefault<UMaterial>()->DefaultMaterial;
	}

	D3D->SetMaterial(Material, ErrorString, ErrorMaterial, NumPasses);
}

void URtxRenderDevice::DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex, INT MaxIndex)
{
	if(!IgnoreDrawCall)
		D3D->DrawPrimitive(PrimitiveType, FirstIndex, NumPrimitives, MinIndex, MaxIndex);
}
