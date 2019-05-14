#include <cstdio>
#include "../../Engine/Inc/Engine.h"

// Variables for ServerCommandlet

FString CurrentCmd; // Contains the next command that is to be passed to UEngine::Exec and is gathered by the input thread
					// the command is then executed by the main thread to avoid issues

/*
 * Allows user input in the console while running a server.
 * This function runs in a separate thread in order to not having
 * to pause the main loop while waiting for input.
 */
DWORD WINAPI UpdateServerConsoleInput(PVOID){
	TCHAR Cmd[1024];

	while(GIsRunning && !GIsRequestingExit){
		if(std::fgets(Cmd, sizeof(Cmd), stdin)){
			Cmd[appStrlen(Cmd) - 1] = '\0'; // Removing newline added by fgets
			CurrentCmd = Cmd; // Updating CurrentCmd so that it can be executed by the main thread
							  // Nothing has been done in terms of thread safety as so far there haven't been any issues...

			// Returning in case user requested exit in order to not get to fgets again
			if(CurrentCmd == "EXIT" || CurrentCmd == "QUIT")
				return 0;
		}
	}

	return 0;
}

// Replacement for UServerCommandlet::Main since the one from Engine.dll crashes because it doesn't assign a value to GEngine
void UServerCommandletMain(){
	FString Language;

	if(GConfig->GetString("Engine.Engine", "Language", Language, "System.ini"))
		UObject::SetLanguage(*Language);

	UClass*	EngineClass = LoadClass<UEngine>(NULL, "ini:Engine.Engine.GameEngine", NULL, LOAD_NoFail, NULL);

	// Literally the only reason for this function to be rewritten
	// The original one doesn't assign a value to GEngine which leads to a gpf...
	GEngine = ConstructObject<UEngine>(EngineClass);

	GEngine->Init();

	// Creating input thread
	CreateThread(NULL, 0, UpdateServerConsoleInput, NULL, 0, NULL);

	DOUBLE OldTime = appSeconds();
	DOUBLE SecondStartTime = OldTime;
	INT TickCount = 0;

	GIsRunning = 1;

	//Main loop
	while(GIsRunning && !GIsRequestingExit){
		DOUBLE NewTime = appSeconds();

		//Executing console commands that are gathered by UpdateServerConsoleInput in a different thread
		if(CurrentCmd.Len() > 0){
			if(CurrentCmd == "CLS") // In case user wants to clear screen. Can be useful for testing.
				system("cls"); // Hate using system but it's ok here
			else if(!GEngine->Exec(*CurrentCmd, *GWarn))
				GWarn->Log(LocalizeError("Exec", "Core"));

			CurrentCmd.Empty();
		}

		// Update the world
		GEngine->Tick(NewTime - OldTime);

		// UEngine::Tick may load a new map and cause the timing to be reset (this is a good thing)
		if(appSeconds() < NewTime)
            SecondStartTime = NewTime = appSeconds();

		OldTime = NewTime;

		++TickCount;

		if(OldTime > SecondStartTime + 1.0){
			GEngine->CurrentTickRate = TickCount / (OldTime - SecondStartTime);
			SecondStartTime = OldTime;
			TickCount = 0;
		}

		// Enforce optional maximum tick rate
		FLOAT MaxTickRate = GEngine->GetMaxTickRate();

		if(MaxTickRate > 0.0f){
			FLOAT Delta = (1.0f / MaxTickRate) - (appSeconds() - OldTime);

			appSleep(Delta > 0.0f ? Delta : 0.0f);
		}
	}

	GIsRunning = 0;
}