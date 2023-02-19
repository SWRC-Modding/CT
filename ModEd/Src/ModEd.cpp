#include "../../Core/Inc/Core.h"
#include "../../Core/Inc/FOutputDeviceWindowsError.h"
#include "../../Core/Inc/FOutputDeviceFile.h"

static FOutputDeviceWindowsError* Error;
static FOutputDeviceFile* Log;

DLL_EXPORT void ModEdInit(const TCHAR* InPackage, const TCHAR* InCmdLine, FOutputDevice* /*InLog*/, FOutputDeviceError* /*InError*/, FFeedbackContext* InWarn, FConfigCache*(*ConfigFactory)(), UBOOL RequireConfig){
	/*
	 * The output devices are dynamically allocated because weird stuff happens when an error occurs: For some reason the CRT calls the destructor of the file output device.
	 * However, it is then used again to by UObject::ExitProperties and reopened, deleting the file on disk and losing all previous output.
	 * No need to free the allocations since they live as long as the process does.
	 */
	Error = new FOutputDeviceWindowsError;
	Log = new FOutputDeviceFile;
	Log->Unbuffered = 1; // Always unbuffered during appInit

	appInit(InPackage, InCmdLine, Log, Error, InWarn, ConfigFactory, RequireConfig);

	if(!GConfig->GetBool("ModEd", "UnbufferedLog", Log->Unbuffered, NULL)){
		GConfig->SetBool("ModEd", "UnbufferedLog", 1, NULL);
		Log->Unbuffered = 0;
	}

	// Init SWRCFix if it exists
	void* ModDLL = appGetDllHandle("Mod.dll");

	if(ModDLL){
		void(CDECL*InitSWRCFix)(void) = static_cast<void(CDECL*)(void)>(appGetDllExport(ModDLL, "InitSWRCFix"));

		if(InitSWRCFix)
			InitSWRCFix();
	}
}
