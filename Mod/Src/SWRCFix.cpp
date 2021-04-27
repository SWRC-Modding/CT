#include "../../Engine/Inc/Engine.h"
#include "../Inc/Mod.h"

/*
 * Property window crash fix
 */

bool(__fastcall*OriginalUClassIsDefaultValue)(UObject* Self, DWORD, const FPropertyInstance&) = NULL;

bool __fastcall UClassIsDefaultValueOverride(UObject* Self, DWORD edx, const FPropertyInstance& PropertyInstance){
	for(INT i = 1; i < PropertyInstance.NestedProperties.Num(); ++i){
		if(PropertyInstance.NestedProperties[i].Property->IsA(UStrProperty::StaticClass()))
			return false;
	}

	return OriginalUClassIsDefaultValue(Self, edx, PropertyInstance);
}

/*
 * FPS limit
 */

FLOAT(__fastcall*OriginalUEngineGetMaxTickRate)(UEngine*, DWORD) = NULL;

static FLOAT __fastcall EngineGetMaxTickRateOverride(UEngine* Self, DWORD Edx){
	FLOAT MaxTickRate = OriginalUEngineGetMaxTickRate(Self, Edx);

	// If the engine doesn't set it's own tick rate (i.e. GetMaxTickRate returns 0), we use FpsLimit instead
	return MaxTickRate <= 0.0f ? USWRCFix::Instance->FpsLimit : MaxTickRate;
}

void USWRCFix::Init(){
	guardFunc;
	debugf("Applying fixes");

	/*
	 * Fix 1:
	 * Placing a FluidSurfaceInfo crashes because the constructor for FDynamicActor accesses its 'Skins' array which is empty by default.
	 * To fix this we simply resize the property in the default actor so that it contains an element.
	 */
	AFluidSurfaceInfo::StaticClass()->GetDefaultActor()->Skins.Set(1);

	/*
	 * Fix 2:
	 * Expanding a struct that contains a string property in an object property window causes a crash while the editor is checking for modified values
	 * in order to highlight them.
	 * This is fixed here by simply checking if there is a nested string property and not calling the original code if there is.
	 * As a downside this causes the string property to be drawn as bold text even though it may have not been modified but this shouldn't be a big deal
	 * since that case is rather rare anyway.
	 */
	OriginalUClassIsDefaultValue = static_cast<bool(__fastcall*)(UObject*, DWORD, const FPropertyInstance&)>(PatchVTable(UClass::StaticClass(), 29, UClassIsDefaultValueOverride));

	/*
	 * Fix 3:
	 * VSync doesn't seem to work for most people with the d3d8 renderer. A very high frame rate causes the mouse pointer in the menu to be super
	 * fast and the helmet shake caused by explosions is way stronger as well.
	 * The engine has a mechanism to limit the fps to a specific value that is returned from UGameEngine::GetMaxTickRate.
	 * However, this is always zero, meaning no limit.
	 * This fix patches the vtable of UGameEngine so it returns a custom value specified in the config.
	 */
	OriginalUEngineGetMaxTickRate = static_cast<FLOAT(__fastcall*)(UEngine*, DWORD)>(PatchVTable(GEngine, 49, EngineGetMaxTickRateOverride));

	/*
	 * Fix 4:
	 * Creating a new HardwareShader via the texture browser crashes because the vertex and pixel shader variables are empty by default which causes the compilation to fail.
	 * To fix it, we simply add a dummy shader implementation.
	 */
	UHardwareShader* DefaultHardwareShader = static_cast<UHardwareShader*>(UHardwareShader::StaticClass()->GetDefaultObject());

	DefaultHardwareShader->VertexShaderText = "vs.1.1\n\n"
	                                          "mov oPos, v0\n";
	DefaultHardwareShader->PixelShaderText  = "ps.1.1\n\n"
	                                          "mov r0, c0\n";

	/*
	 * Fix 5:
	 * By default RC only has a fixed set of screen resolutions.
	 * To support any available resolution, EnumDisplaySettings is used and a list of supported resolutions is created and written to the config file.
	 */
	DEVMODE dm = {{0}};

	dm.dmSize = sizeof(dm);

	TArray<DWORD> AvailableResolutions;

	for(int i = 0; EnumDisplaySettings(NULL, i, &dm) != 0; ++i)
		AvailableResolutions.AddUniqueItem(MAKELONG(dm.dmPelsWidth, dm.dmPelsHeight));

	if(AvailableResolutions.Num() > 0){
		Sort(AvailableResolutions.GetData(), AvailableResolutions.Num());

		FString ResolutionList = "(";

		for(int i = 0; i < AvailableResolutions.Num() - 1; ++i)
			ResolutionList += FString::Printf("\"%ix%i\",", LOWORD(AvailableResolutions[i]), HIWORD(AvailableResolutions[i]));

		ResolutionList += FString::Printf("\"%ix%i\")", LOWORD(AvailableResolutions.Last()), HIWORD(AvailableResolutions.Last()));

		GConfig->SetString("CTGraphicsOptionsPCMenu",
		                   "Options[2].Items",
		                   *ResolutionList,
		                   *(FString("XInterfaceCTMenus.") + UObject::GetLanguage()));
	}

	InitScript();
	unguard;
}

USWRCFix* USWRCFix::Instance = NULL;

void CDECL InitSWRCFix(void){
	USWRCFix::Instance = FindObject<USWRCFix>(ANY_PACKAGE, "SWRCFixInstance");

	if(!USWRCFix::Instance){
		// NOTE: Need to use LoadClass here since the class might not be registered yet if Mod.dll was loaded directly with LoadLibrary
		USWRCFix::Instance = ConstructObject<USWRCFix>(LoadClass<USWRCFix>(NULL, "Mod.SWRCFix", NULL, LOAD_NoFail | LOAD_Throw, NULL),
		                                               ANY_PACKAGE,
		                                               FName("SWRCFixInstance"));
		USWRCFix::Instance->AddToRoot(); // This object should never be garbage collected
		USWRCFix::Instance->Init();
	}
}
