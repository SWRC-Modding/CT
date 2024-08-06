#include "RtxDrvPrivate.h"
#include "RtxRenderDevice.h"

IMPLEMENT_CLASS(URtxRenderDevice)

class USolidColorMaterial : public UConstantMaterial{
	DECLARE_CLASS(USolidColorMaterial,UConstantMaterial,0,RtxDrv)
public:
	virtual FColor GetColor(FLOAT TimeSeconds){ return FColor(255,255,0,255); }
};
IMPLEMENT_CLASS(USolidColorMaterial)

UBOOL URtxRenderDevice::Exec(const TCHAR* Cmd, FOutputDevice& Ar)
{
	return Super::Exec(Cmd, Ar);
}

UBOOL URtxRenderDevice::Init()
{
	// Init SWRCFix if it exists. Hacky but RenderDevice is always loaded at startup...
	HMODULE ModDLL = LoadLibraryA("Mod.dll");

	if(ModDLL)
	{
		void(CDECL*InitSWRCFix)(void) = reinterpret_cast<void(CDECL*)(void)>(GetProcAddress(ModDLL, "InitSWRCFix"));

		if(InitSWRCFix)
			InitSWRCFix();
	}

	RenderInterface.RenDev = this;
	RenderInterface.Impl = NULL;

	ClearMaterialFlags();
	UClient* Client = UTexture::__Client;
	Client->Shadows = 0;
	Client->FrameFXDisabled = 1;
	Client->BloomQuality = 0;
	Client->BlurEnabled = 0;
	Client->BumpmappingQuality = 0;
	GetDefault<APlayerController>()->bVisor = 0;
	GetDefault<APlayerController>()->VisorModeDefault = 1;
	UseStencil = 0;

	return Super::Init();
}

UBOOL URtxRenderDevice::SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes, UBOOL bSaveSize)
{
	UBOOL Result = Super::SetRes(Viewport, NewX, NewY, Fullscreen, ColorBytes, bSaveSize);

	if(Result && !BridgeInterface.initialized)
	{
		if(bridgeapi_initialize(&BridgeInterface) != BRIDGEAPI_ERROR_CODE_SUCCESS || !BridgeInterface.initialized)
		{
			appErrorf("Failed to initialize remix bridge API");
			return 0;
		}

		BridgeInterface.RegisterDevice();
	}

	return Result;
}

void URtxRenderDevice::Flush(UViewport* Viewport)
{
	ClearMaterialFlags();
	Super::Flush(Viewport);
}

FRenderInterface* URtxRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize)
{
	LockedViewport = Viewport;

	if(Viewport->Actor)
	{
		Viewport->Actor->bVisor = 0;
		Viewport->Actor->VisorModeDefault = 1;
	}

	RenderInterface.Impl = Super::Lock(Viewport, HitData, HitSize);
	return RenderInterface.Impl ? &RenderInterface : NULL;
}

struct FLightHandle{
	uint64_t Handle;
	UBOOL    bUsed;
};
class RTXDRV_API UConstantColorMaterial : public UConstantMaterial{
public:
	DECLARE_CLASS(UConstantColorMaterial,UConstantMaterial,0,RtxDrv)
	virtual FColor GetColor(FLOAT TimeSeconds){ return FColor(255,255,0); }
};
IMPLEMENT_CLASS(UConstantColorMaterial)

void URtxRenderDevice::Unlock(FRenderInterface* RI)
{
	typedef TMap<AProjectile*, FLightHandle> FHandleMap;
	static FHandleMap HandleMap;

	foreach(AllActors, AProjectile, Proj, static_cast<UGameEngine*>(GEngine)->GLevel)
	{
		FLightHandle* HandlePtr = HandleMap.Find(*Proj);

		if(HandlePtr)
			BridgeInterface.DestroyLight(HandlePtr->Handle);
		else
			HandlePtr = &HandleMap[*Proj];

		x86::remixapi_LightInfo l;
		l.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO;
		l.hash = reinterpret_cast<uint32_t>(*Proj);

		FPlane Col = FGetHSV(Proj->LightHue, Proj->LightSaturation, Proj->LightBrightness) * 10000;

		l.radiance.x = Col.X;
		l.radiance.y = Col.Y;
		l.radiance.z = Col.Z;

		FVector Loc = Proj->Location;
		x86::remixapi_LightInfoSphereEXT s;
		s.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_SPHERE_EXT;
		s.position.x = Loc.X;
		s.position.y = Loc.Y;
		s.position.z = Loc.Z;
		s.radius = 2.5f;
		s.shaping_hasvalue = FALSE;
		s.shaping_value.direction.x = 0.0f;
		s.shaping_value.direction.y = 1.0f;
		s.shaping_value.direction.z = 0.0f;
		s.shaping_value.coneAngleDegrees = 80.0f;
		s.shaping_value.coneSoftness = 1.0f;
		s.shaping_value.focusExponent = 0.0f;

		HandlePtr->bUsed = 1;
		HandlePtr->Handle = BridgeInterface.CreateSphereLight(&l, &s);
		BridgeInterface.DrawLightInstance(HandlePtr->Handle);
	}

	for(FHandleMap::TIterator It(HandleMap); It; ++It)
	{
		if(!It->bUsed)
			HandleMap.Remove(It.Key());
		else
			It->bUsed = 0;
	}

	DECLARE_STATIC_UOBJECT(USolidColorMaterial, Material, {});
	DECLARE_STATIC_UOBJECT(UFinalBlend, FinalBlend,
	{
		FinalBlend->Material = Material;
		FinalBlend->FrameBufferBlending = FB_Overwrite;
		FinalBlend->ColorWriteEnable = 1;
	});

	// Draw anchor triangle at world center to parent stuff to in remix
	if(LockedViewport && LockedViewport->Actor)
	{
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

		RenderInterface.PushState();
		RenderInterface.SetTransform(TT_LocalToWorld, FMatrix::Identity);
		RenderInterface.SetTransform(TT_WorldToCamera, WorldToCamera);
		RenderInterface.SetTransform(TT_CameraToScreen, CameraToScreen);
		RenderInterface.SetMaterial(FinalBlend);
		RenderInterface.SetIndexBuffer(NULL, 0);
		FVertexStream* Stream = &AnchorTriangle;
		RenderInterface.SetVertexStreams(VS_FixedFunction, &Stream, 1);
		RenderInterface.DrawPrimitive(PT_TriangleList, 0, 1);
		RenderInterface.PopState();
	}

	LockedViewport = NULL;

	Super::Unlock(static_cast<FRtxRenderInterface*>(RI)->Impl);
}

void URtxRenderDevice::Present(UViewport* Viewport)
{
	Super::Present(Viewport);
}

FRenderCaps* URtxRenderDevice::GetRenderCaps()
{
	/* return Super::GetRenderCaps(); */
	static FRenderCaps RenderCaps(1, 14, 1);

	return &RenderCaps;
}

UBOOL FRtxRenderInterface::SetRenderTarget(FRenderTarget* RenderTarget, bool bOwnDepthBuffer)
{
	debugf("SETRENDERTARGET: %p", RenderTarget);
	return Impl->SetRenderTarget(RenderTarget, bOwnDepthBuffer);
}

void FRtxRenderInterface::Clear(UBOOL UseColor, FColor Color, UBOOL UseDepth, FLOAT Depth, UBOOL UseStencil, DWORD Stencil)
{
	Impl->Clear(UseColor, Color, UseDepth, Depth, UseStencil, Stencil);
}

void FRtxRenderInterface::SetTransform(ETransformType Type, const FMatrix& Matrix)
{
	Impl->SetTransform(Type, Matrix);
}

void FRtxRenderInterface::SetMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial, INT* NumPasses)
{
	Impl->SetMaterial(Material, ErrorString, ErrorMaterial, NumPasses);
}

void FRtxRenderInterface::DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex, INT MaxIndex)
{
	Impl->DrawPrimitive(PrimitiveType, FirstIndex, NumPrimitives, MinIndex, MaxIndex);
}

void URtxRenderDevice::ClearMaterialFlags()
{
	foreachobj(UMaterial, Material)
		reinterpret_cast<INT*>(*Material)[24] = (reinterpret_cast<INT*>(*Material)[24] & 0x3);
}

void URtx::execTestFunc(FFrame& Stack, void* Result)
{

}
