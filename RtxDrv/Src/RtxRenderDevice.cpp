#include "RtxDrvPrivate.h"
#include "RtxRenderDevice.h"

IMPLEMENT_CLASS(URtxRenderDevice)

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

	FConfigSection* Section = GConfig->GetSectionPrivate("RtxMaterialIds", 0, 1, StaticConfigName());

	if(Section)
	{
		for(FConfigSection::TIterator It(*Section); It; ++It)
		{
			const INT Id = appAtoi(*It.Value());
			RenderInterface.MaterialIdsByPath.AddItem(FRtxRenderInterface::MaterialId(Id, *It.Key()));
		}
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

UBOOL GFirstClear = 0;
FRenderInterface* URtxRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize)
{
	if(Viewport->Actor)
	{
		Viewport->Actor->bVisor = 0;
		Viewport->Actor->VisorModeDefault = 1;
	}

	GFirstClear = 0;
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
	static bool Exited = false;

	if(!Exited && GIsRequestingExit)
	{
		Exited = true;
		FConfigSection* Section = GConfig->GetSectionPrivate("RtxMaterialIds", 1, 0, StaticConfigName());
		check(Section);

		for(TArray<FRtxRenderInterface::MaterialId>::TIterator It(RenderInterface.MaterialIdsByPath); It; ++It)
		{
			FConfigString& Value = (*Section)[It->Path];
			Value = appItoa(It->Id);
			Value.Dirty = true;
		}
	}

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
		s.radius = 5.0f;
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
