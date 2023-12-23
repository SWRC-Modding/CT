#include "../../Engine/Inc/Engine.h"
#include "../../Window/Inc/Window.h"
#include "../Inc/Mod.h"

USWRCFix* USWRCFix::Instance = NULL;
UBOOL USWRCFix::RenderingReady = 0;

void CDECL InitSWRCFix(void){
	USWRCFix::Instance = FindObject<USWRCFix>(ANY_PACKAGE, "SWRCFixInstance");

	if(!USWRCFix::Instance){
		// NOTE: Need to use LoadClass here since the class might not be registered yet if Mod.dll was loaded directly with LoadLibrary
		USWRCFix::Instance = ConstructObject<USWRCFix>(LoadClass<UObject>(NULL, "Mod.SWRCFix", NULL, LOAD_NoFail | LOAD_Throw, NULL),
		                                               ANY_PACKAGE,
		                                               FName("SWRCFixInstance"));
		USWRCFix::Instance->AddToRoot(); // This object should never be garbage collected
		USWRCFix::Instance->Init();
	}

	USWRCFix::RenderingReady = !GIsEditor;
}

/*
 * Property window crash fix
 */

static bool(__fastcall*OriginalUClassIsDefaultValue)(UObject* Self, DWORD, const FPropertyInstance&) = NULL;

static bool __fastcall UClassIsDefaultValueOverride(UObject* Self, DWORD edx, const FPropertyInstance& PropertyInstance){
	for(INT i = 1; i < PropertyInstance.NestedProperties.Num(); ++i){
		if(PropertyInstance.NestedProperties[i].Property->IsA(UStrProperty::StaticClass()))
			return false;
	}

	return OriginalUClassIsDefaultValue(Self, edx, PropertyInstance);
}

/*
 * FPS limit
 */

static FLOAT(__fastcall*OriginalUEngineGetMaxTickRate)(UEngine*, DWORD) = NULL;

static FLOAT __fastcall EngineGetMaxTickRateOverride(UEngine* Self, DWORD Edx){
	FLOAT MaxTickRate = OriginalUEngineGetMaxTickRate(Self, Edx);

	// If the engine doesn't set it's own tick rate (i.e. GetMaxTickRate returns 0), we use FpsLimit instead
	return MaxTickRate <= 0.0f ? USWRCFix::Instance->FpsLimit : MaxTickRate;
}

/*
 * Hide reticle when zoomed in and update FOV if necessary
 */

static void(__fastcall*OriginalUEngineDraw)(UEngine*, DWORD, UViewport*, UBOOL, BYTE*, INT*) = NULL;
static INT   PrevViewportWidth = 0;
static INT   PrevViewportHeight = 0;
static INT   PrevAutoFOV = 0;
static FLOAT PrevFOV = 0.0f;
static FLOAT PrevHudArmsFOVFactor = 0.0f;

static void __fastcall EngineDrawOverride(UEngine* Self, DWORD Edx, UViewport* Viewport, UBOOL Blit, BYTE* HitData, INT* HitSize){
	// Update field of view
	if(USWRCFix::Instance->AutoFOV && ((Viewport->SizeX != PrevViewportWidth || Viewport->SizeY != PrevViewportHeight) || USWRCFix::Instance->AutoFOV != PrevAutoFOV)){
		PrevViewportWidth = Viewport->SizeX;
		PrevViewportHeight = Viewport->SizeY;
		PrevAutoFOV = 1;

		FLOAT FOVScale = static_cast<FLOAT>(Viewport->SizeX) / Viewport->SizeY * 0.75f; // 0.75 is the default 4:3 aspect ratio
		FLOAT FOV = appCeil(appAtan(appTan(USWRCFix::Instance->GetDefaultFOV() * PI / 360.0f) * FOVScale) * 360.0f / PI);

		USWRCFix::Instance->SetFOV(Viewport->Actor, FOV);
	}else if(USWRCFix::Instance->FOV != PrevFOV || USWRCFix::Instance->HudArmsFOVFactor != PrevHudArmsFOVFactor){
		PrevFOV = USWRCFix::Instance->FOV;
		PrevHudArmsFOVFactor = USWRCFix::Instance->HudArmsFOVFactor;
		USWRCFix::Instance->SetFOV(Viewport->Actor, USWRCFix::Instance->FOV);
		PrevAutoFOV = 0;
	}

	// Hide reticle when zoomed in
	if(Viewport->Actor && Viewport->Actor->Pawn && Viewport->Actor->Pawn->Weapon){
		AWeapon* Weapon = Viewport->Actor->Pawn->Weapon;

		if(!Weapon->bZoomedUsesNoHUDArms){
			if(Weapon->bWeaponZoom)
				Weapon->Reticle = NULL;
			else if(!Weapon->Reticle)
				Weapon->Reticle = static_cast<AWeapon*>(Weapon->GetClass()->GetDefaultActor())->Reticle;
		}
	}

	OriginalUEngineDraw(Self, Edx, Viewport, Blit, HitData, HitSize);
}


/*
 * UUnrealEdEngine exec hook
 */

static void FlushResources(){
	// Flush render resources
	if(GEngine->GRenDev)
		GEngine->GRenDev->Flush(NULL);

	// Unload texture data
	foreachobj(UTexture, It){
		for(INT i = 0; i < It->Mips.Num(); ++i)
			It->Mips[i].DataArray.Unload();
	}
}

static UBOOL(__fastcall*OriginalUUnrealEdEngineExec)(UEngine*, DWORD, const TCHAR*, FOutputDevice&) = NULL;

static UBOOL __fastcall UnrealEdEngineExecOverride(UEngine* Self, DWORD Edx, const TCHAR* Cmd, FOutputDevice& Ar){
	const TCHAR* TempCmd = Cmd;

	if(ParseCommand(&Cmd, "FLUSHRESOURCES")){
		FlushResources();

		return 1;
	}else if(ParseCommand(&TempCmd, "CAMERA") && ParseCommand(&TempCmd, "UPDATE")){
		FString TempString;

		if(Parse(TempCmd, "NAME=", TempString) && TempString == "TextureBrowser"){
			if(Parse(TempCmd, "PACKAGE=", TempString) && TempString == ""){
				if(!GConfig->GetFString("Mod.SWRCFix", "InitialTextureBrowserPackage", TempString)){
					TempString = "Engine";
					GConfig->SetString("Mod.SWRCFix", "InitialTextureBrowserPackage", *TempString);
				}

				OriginalUUnrealEdEngineExec(Self, Edx, *("CAMERA UPDATE FLAGS=1073742464 MISC2=0 REN=17 NAME=TextureBrowser PACKAGE=\"" + TempString + "\" GROUP=\"(All)\""), Ar);
				FlushResources();
				USWRCFix::RenderingReady = 1;

				return 1;
			}
		}
	}else if(ParseCommand(&TempCmd, "ENDTASK")){
		// Fixes the progress indicator that won't disappear sometimes in the editor when compiling scripts.
		// No guarantee this doesn't break anything else...
		INT* TaskCount = reinterpret_cast<INT*>(GWarn) + 3;
		while(*TaskCount > 0)
			GWarn->EndSlowTask();

		return 1;
	}

	return OriginalUUnrealEdEngineExec(Self, Edx, Cmd, Ar);
}

/*
 * Fix initialization
 */

void USWRCFix::Init(){
	guardFunc;

	// Common fixes

	debugf("Applying common fixes");

	/*
	 * Fix 1:
	 * Placing a FluidSurfaceInfo crashes because the constructor for FDynamicActor accesses its 'Skins' array which is empty by default.
	 * To fix this we simply resize the property in the default actor so that it contains an element.
	 */
	GetDefault<AFluidSurfaceInfo>()->Skins.Set(1);

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
	 * Creating a new HardwareShader via the texture browser crashes because the vertex and pixel shader variables are empty by default which causes the compilation to fail.
	 * To fix it, we simply add a dummy shader implementation.
	 */
	UHardwareShader* DefaultHardwareShader = GetDefault<UHardwareShader>();
	DefaultHardwareShader->VertexShaderText = "vs.1.1\n\nmov oPos, v0\n";
	DefaultHardwareShader->PixelShaderText  = "ps.1.1\n\nmov r0, c0\n";

	/*
	 * Detect whether the game or editor is running
	 */
	HMODULE ExeModule = GetModuleHandleA(NULL);
	bool IsUnrealEd = ExeModule && GetProcAddress(ExeModule, "autoclassUUnrealEdEngine") != NULL;

	if(!IsUnrealEd){ // Game specific fixes
		debugf("Applying game fixes");
		/*
		 * Fix 4:
		 * VSync doesn't seem to work for most people with the d3d8 renderer. A very high frame rate causes the mouse pointer in the menu to be super
		 * fast and the helmet shake caused by explosions is way stronger as well.
		 * The engine has a mechanism to limit the fps to a specific value that is returned from UGameEngine::GetMaxTickRate.
		 * However, this is always zero, meaning no limit.
		 * This fix patches the vtable of UGameEngine so it returns a custom value specified in the config.
		 */
		OriginalUEngineGetMaxTickRate = static_cast<FLOAT(__fastcall*)(UEngine*, DWORD)>(PatchDllClassVTable("Engine.dll", "UGameEngine", "UObject", 49, EngineGetMaxTickRateOverride));

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

		if(AvailableResolutions.Num() > 1){
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

		/*
		 * Fix 6:
		 * This mods FOV options revealed an issue with how the game draws the weapon's reticles. It basically checks if the current FOV is lower than the default one
		 * and only then draws the reticle. This way it is hidden when zoomed in. However if you set a very high custom FOV, this check will always fail and the reticle is always drawn.
		 * To fix it, we hook the UEngine::Draw function and set the current weapon's reticle property to NULL if zoomed in which causes it to be hidden.
		 * Here we also calculate the current FOV based on the aspect ratio.
		 */
		OriginalUEngineDraw = static_cast<void(__fastcall*)(UEngine*, DWORD, UViewport*, UBOOL, BYTE*, INT*)>(PatchDllClassVTable("Engine.dll", "UGameEngine", "UObject", 41, EngineDrawOverride));
	}else{ // Editor specific fixes
		debugf("Applying editor fixes");

		/*
		 * Fix 7:
		 * The editor loads all textures at startup which can consume a significant amount of memory.
		 * It does so because initially there is no package selected for the texture browser and thus all textures are shown.
		 * This is fixed by overriding the Exec function and checking for the command that intiializes the texture browser and providing a single package to be initially loaded.
		 * Additionally, a new command is added that alluws manually flushing resources if memory usage gets too high.
		 */
		OriginalUUnrealEdEngineExec = static_cast<UBOOL(__fastcall*)(UEngine*, DWORD, const TCHAR*, FOutputDevice&)>(PatchDllClassVTable(*(FString(appPackage()) + ".exe"), "UUnrealEdEngine", "FExec", 0, UnrealEdEngineExecOverride));
	}

	// Initialize the UnrealScript part of the fix
	InitScript();
	unguard;
}
