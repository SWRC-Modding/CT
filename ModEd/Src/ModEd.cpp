#include "../../Editor/Inc/Editor.h"
#include "../../Window/Inc/Window.h"
#include "../../Core/Inc/FOutputDeviceWindowsError.h"
#include "../../Core/Inc/FOutputDeviceFile.h"

static void(__fastcall*OriginalUUnrealEdEngineTick)(UEditorEngine*, DWORD, FLOAT) = NULL;

static void __fastcall UnrealEdEngineTickOverride(UEditorEngine* Self, DWORD Edx, FLOAT DeltaTime){
	OriginalUUnrealEdEngineTick(Self, Edx, DeltaTime);

	UBOOL LoadMRU = 1;

	if(!GConfig->GetBool("ModEd", "LoadMRU", LoadMRU, "UnrealEd.ini"))
		GConfig->SetBool("ModEd", "LoadMRU", LoadMRU, "UnrealEd.ini");

	if(LoadMRU){
		FString MapPath;

		if(GConfig->GetFString("MRU", "MRUItem0", MapPath, "UnrealEd.ini") && GFileManager->FileSize(*MapPath) > 0){
			/*
			 * HACK:
			 * Opening the map using the command MAP LOAD FILE=... works but it does not set the global map name.
			 * The map will be treated as "Untitled" and you'll always have to manually select the file to save to.
			 * This is circumvented by calling the command on the editor window itself.
			 * Since the window is not exported it has to be accessed by its address and sent the command to open MRU1 with the command id 20001.
			 */
			WWindow* Window = *reinterpret_cast<WWindow**>(0x10FE39D4);
			Window->OnCommand(20001);
		}
	}

	// Restore original tick function now that the initial setup is done.
	PatchVTable(Self, 32, OriginalUUnrealEdEngineTick);
}

// Init SWRCFix if it exists
static void InitSWRCFix(){
	void* ModDLL = appGetDllHandle("Mod.dll");

	if(ModDLL){
		void(CDECL*InitFunc)(void) = static_cast<void(CDECL*)(void)>(appGetDllExport(ModDLL, "InitSWRCFix"));

		if(InitFunc){
			InitFunc();

			// Set render devices for the viewports

			for(INT i = 0; i < 8; ++i)
				GConfig->SetString(*FString::Printf("U2Viewport%i", i), "Device", "Mod.ModRenderDevice", "UnrealEd.ini");

			GConfig->SetString("Texture Browser",         "Device", "Mod.ModRenderDevice", "UnrealEd.ini");
			GConfig->SetString("Static Mesh Browser",     "Device", "Mod.ModRenderDevice", "UnrealEd.ini");
			GConfig->SetString("Particle System Browser", "Device", "Mod.ModRenderDevice", "UnrealEd.ini");
		}
	}
}

/*
 * appInit hook.
 * This requires a modified executable that calls the ModEdInit function instead of appInit.
 */
DLL_EXPORT void ModEdInit(const TCHAR* InPackage, const TCHAR* InCmdLine, FOutputDevice*, FOutputDeviceError*, FFeedbackContext* InWarn, FConfigCache*(*ConfigFactory)(), UBOOL RequireConfig){
	/*
	 * The output devices are dynamically allocated because otherwise weird stuff happens when an error occurs: For some reason the CRT calls the destructor of the file output device.
	 * However, it is then used again to by UObject::ExitProperties and reopened, deleting the file on disk and losing all previous output.
	 * No need to free the allocations since they live as long as the process does.
	 */
	FOutputDeviceError* Error = new FOutputDeviceWindowsError;
	FOutputDeviceFile*  Log   = new FOutputDeviceFile;
	Log->Unbuffered = 1; // Always unbuffered during appInit

	appInit(InPackage, InCmdLine, Log, Error, InWarn, ConfigFactory, RequireConfig);

	if(!GConfig->GetBool("ModEd", "UnbufferedLog", Log->Unbuffered, "UnrealEd.ini")){
		GConfig->SetBool("ModEd", "UnbufferedLog", 0, "UnrealEd.ini");
		Log->Unbuffered = 0;
	}

	InitSWRCFix();

	OriginalUUnrealEdEngineTick = static_cast<void(__fastcall*)(UEditorEngine*, DWORD, FLOAT)>(PatchDllClassVTable(*(FString(appPackage()) + ".exe"), "UUnrealEdEngine", "UObject", 32, UnrealEdEngineTickOverride));
}
