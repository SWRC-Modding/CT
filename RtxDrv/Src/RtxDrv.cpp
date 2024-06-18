#include "RtxDrv.h"

IMPLEMENT_PACKAGE(RtxDrv)
IMPLEMENT_CLASS(URtxRenderDevice)

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
			MaterialIdsByPath.AddItem(TestDraw(Id, *It.Key()));
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

void URtxRenderDevice::Flush(UViewport* Viewport)
{
	ClearMaterialFlags();
	Super::Flush(Viewport);
}

FRenderInterface* URtxRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize)
{
	RenderInterface.Impl = Super::Lock(Viewport, HitData, HitSize);
	return RenderInterface.Impl ? &RenderInterface : NULL;
}

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

		for(TArray<TestDraw>::TIterator It(MaterialIdsByPath); It; ++It)
		{
			FConfigString& Value = (*Section)[It->Path];
			Value = appItoa(It->Id);
			Value.Dirty = true;
		}
	}

	Super::Unlock(static_cast<FRtxRenderInterface*>(RI)->Impl);
}

void FRtxRenderInterface::SetTransform(ETransformType Type, const FMatrix& Matrix)
{
	Impl->SetTransform(Type, Matrix);
}

void FRtxRenderInterface::SetMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial, INT* NumPasses)
{
	guardFunc;
	CurrentMaterial = Material;

	UMaterial* ActualMaterial = Material;

	while(ActualMaterial && ActualMaterial->IsA<UModifier>())
		ActualMaterial = static_cast<UModifier*>(ActualMaterial)->Material;

	if(ActualMaterial && ActualMaterial->IsA<UParticleMaterial>())
		ActualMaterial = static_cast<UParticleMaterial*>(ActualMaterial)->BitmapMaterial;

	CurrentActualMaterial = ActualMaterial;

	INT Mask = (reinterpret_cast<INT*>(ActualMaterial)[24] & 0x3) >> 2;

	if(ActualMaterial && (Mask & 0x1) == 0)
	{
		Mask |= 0x1;

		if(ActualMaterial->GetFName() != NAME_InGameTempName)
		{
			const TCHAR* Path = ActualMaterial->GetPathName();
			for(INT i = 0; i < RenDev->MaterialIdsByPath.Num(); ++i)
			{
				if(RenDev->MaterialIdsByPath[i].Path == Path)
				{
					Mask |= (i + 1) << 1;
					break;
				}
			}
		}

		reinterpret_cast<INT*>(ActualMaterial)[24] = (reinterpret_cast<INT*>(ActualMaterial)[24] & 0x3) | (Mask << 2);
	}

	DrawParticleTriangles = ActualMaterial && (reinterpret_cast<INT*>(ActualMaterial)[24] >> 3) != 0;

	Impl->SetMaterial(Material, ErrorString, ErrorMaterial, NumPasses);
	unguardf(("%s", Material->GetPathName()))
}

void FRtxRenderInterface::DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex, INT MaxIndex)
{
	Impl->DrawPrimitive(PrimitiveType, FirstIndex, NumPrimitives, MinIndex, MaxIndex);

	if(DrawParticleTriangles)
	{
		DECLARE_STATIC_UOBJECT(UConstantColorMaterial, TestConstantColorMaterial, {});
		DECLARE_STATIC_UOBJECT(UFinalBlend, TestFinalBlend, {
			TestFinalBlend->Material = TestConstantColorMaterial;
		});

		PushState();

		Impl->SetCullMode(CM_None);
		Impl->SetMaterial(TestFinalBlend);
		FVertexStream* TestStreamPtr = &RenDev->MaterialIdsByPath[(reinterpret_cast<INT*>(CurrentActualMaterial)[24] >> 3) - 1].Stream;
		Impl->SetVertexStreams(VS_FixedFunction, &TestStreamPtr, 1);
		Impl->SetIndexBuffer(NULL, 0);
		Impl->DrawPrimitive(PT_TriangleList, 0, 1);
		Impl->SetMaterial(CurrentMaterial);

		PopState();
	}
}

FRenderCaps* URtxRenderDevice::GetRenderCaps()
{
	/* return Super::GetRenderCaps(); */
	static FRenderCaps RenderCaps(1, 14, 1);

	return &RenderCaps;
}

void URtxRenderDevice::ClearMaterialFlags()
{
	foreachobj(UMaterial, Material)
	{
		reinterpret_cast<INT*>(*Material)[24] = (reinterpret_cast<INT*>(*Material)[24] & 0x3);
	}
}
