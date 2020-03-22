#include "../../Core/Inc/Core.h"
#include "../../Core/Inc/FOutputDeviceFile.h"
#include "../../Core/Inc/FOutputDeviceWindowsError.h"
#include "../../Core/Inc/FFeedbackContextCmd.h"
#include "../../Core/Inc/FConfigCacheIni.h"

FOutputDeviceFile Log;
FOutputDeviceWindowsError Error;
FFeedbackContextCmd Warn; // TODO(Leon): Replace with FFeedbackContextWindows

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){
	int ExitCode = EXIT_SUCCESS;

	GIsStarted = 1;

	try{
		GIsGuarded = 1;

		appInit("SWRepublicCommando", lpCmdLine, &Log, &Error, &Warn, FConfigCacheIni::Factory, 1);



		appPreExit();

		GIsGuarded = 0;
	}catch(...){
		GIsGuarded = 0;
		ExitCode = EXIT_FAILURE;
		Error.HandleError();
	}

	appExit();

	return ExitCode;
}
