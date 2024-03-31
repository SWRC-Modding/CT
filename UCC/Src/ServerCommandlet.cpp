#include "Engine.h"

// Variables for ServerCommandlet

static const TCHAR* CurrentConsoleCommand;

/*
 * Allows user input in the console while running a server.
 * This function runs in a separate thread in order to not having
 * to pause the main loop while waiting for input.
 */
static DWORD WINAPI UpdateServerConsoleInput(PVOID)
{
	HANDLE InputHandle = CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if(!InputHandle)
		return 1;

	while(GIsRunning && !GIsRequestingExit)
	{
		TCHAR ConsoleCommandBuffer[512];

		if(!CurrentConsoleCommand)
		{
			DWORD InputLen;
			ReadConsoleA(InputHandle, ConsoleCommandBuffer, ARRAY_COUNT(ConsoleCommandBuffer) - 1, &InputLen, NULL);

			if(InputLen > 0)
			{
				TCHAR* Cmd = ConsoleCommandBuffer;

				// Trim spaces from command

				while(InputLen > 0 && appIsSpace(Cmd[InputLen - 1]))
					--InputLen;

				Cmd[InputLen] = '\0';

				while(InputLen > 0 && appIsSpace(*Cmd))
				{
					++Cmd;
					--InputLen;
				}

				if(InputLen > 0)
					CurrentConsoleCommand = Cmd;
			}
		}
		else
		{
			Sleep(100);
		}
	}

	CloseHandle(InputHandle);

	return 0;
}

// Replacement for UServerCommandlet::Main since the one from Engine.dll crashes because it doesn't assign a value to GEngine
INT UServerCommandletMain()
{
	FString Language;

	if(GConfig->GetFString("Engine.Engine", "Language", Language, "System.ini"))
		UObject::SetLanguage(*Language);

	UClass* EngineClass = LoadClass<UEngine>(NULL, "ini:Engine.Engine.GameEngine", NULL, LOAD_NoFail, NULL);

	GEngine = ConstructObject<UEngine>(EngineClass);
	GEngine->Init();

	// Create input thread
	HANDLE InputThread = CreateThread(NULL, 0, UpdateServerConsoleInput, NULL, 0, NULL);

	DOUBLE OldTime = appSeconds();
	DOUBLE SecondStartTime = OldTime;
	INT TickCount = 0;

	GIsRunning = 1;

	// Main loop
	while(GIsRunning && !GIsRequestingExit)
	{
		DOUBLE NewTime = appSeconds();

		if(CurrentConsoleCommand)
		{
			if(appStricmp(CurrentConsoleCommand, "CLS") == 0) // In case user wants to clear screen. Can be useful for testing.
				system("cls"); // Hate using system but it's ok here
			else if(!GEngine->Exec(CurrentConsoleCommand, *GWarn))
				GWarn->Log(LocalizeError("Exec", "Core"));

			CurrentConsoleCommand = NULL;
		}

		// Update the world
		GEngine->Tick(NewTime - OldTime);

		// UEngine::Tick may load a new map and cause the timing to be reset (this is a good thing)
		if(appSeconds() < NewTime)
			SecondStartTime = NewTime = appSeconds();

		OldTime = NewTime;

		++TickCount;

		if(OldTime > SecondStartTime + 1.0)
		{
			GEngine->CurrentTickRate = TickCount / (OldTime - SecondStartTime);
			SecondStartTime = OldTime;
			TickCount = 0;
		}

		// Enforce optional maximum tick rate
		FLOAT MaxTickRate = GEngine->GetMaxTickRate();

		if(MaxTickRate > 0.0f)
		{
			FLOAT Delta = (1.0f / MaxTickRate) - (appSeconds() - OldTime);

			appSleep(Delta > 0.0f ? Delta : 0.0f);
		}
	}

	GIsRunning = 0;

	WaitForSingleObject(InputThread, INFINITE);
	CloseHandle(InputThread);

	return 0;
}
