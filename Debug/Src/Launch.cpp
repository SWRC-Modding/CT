#include <Windows.h>

#include "../../Engine/Inc/Engine.h"
#include "../../Core/Inc/FOutputDeviceWindowsError.h"

__declspec(dllexport) void debugMainLoop(){
	static FOutputDeviceWindowsError Error;

	try{
		GIsGuarded = 1;

		DOUBLE OldTime = appSeconds();

		while(GIsRunning && !GIsRequestingExit){
			DOUBLE	CurrentTime = appSeconds();
			FLOAT DeltaTime	= Clamp(static_cast<float>(CurrentTime - OldTime), 0.0001f, 1.0f);

			OldTime = CurrentTime;

			//Update
			GEngine->Tick(DeltaTime);
			GWindowManager->Tick(DeltaTime);

			//Handle all incoming messages
			MSG Msg;

			while(PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)){
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
	}catch(...){
		GIsGuarded = 0;
		appUnwindf("%s", "debugMainLoop");
		GError->HandleError();
	}
}