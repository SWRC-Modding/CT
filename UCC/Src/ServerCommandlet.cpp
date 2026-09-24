#include "Engine.h"
#include "IpDrv.h"

// Variables for ServerCommandlet

static const TCHAR* GCurrentCmd = NULL;

static const TCHAR* GetCurrentCmd()
{
	return (const TCHAR*)InterlockedCompareExchangePointer((PVOID*)&GCurrentCmd, NULL, NULL);
}

static void SetCurrentCmd(const TCHAR* Cmd)
{
	InterlockedExchangePointer((PVOID*)&GCurrentCmd, (PVOID)Cmd);
}

/*
 * Allows user input in the console while running a server.
 * This function runs in a separate thread in order to not having
 * to pause the main loop while waiting for input.
 */
static DWORD WINAPI UpdateServerConsoleInput(PVOID)
{
	const HANDLE ConsoleInput = CreateFileA("CONIN$", GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);

	if(ConsoleInput == INVALID_HANDLE_VALUE)
		return 1;

	while(GIsRunning && !GIsRequestingExit)
	{
		if(GetCurrentCmd())
		{
			Sleep(100);
			continue;
		}

		TCHAR InputBuffer[512];
		DWORD InputLen;
		ReadConsoleA(ConsoleInput, InputBuffer, ARRAY_COUNT(InputBuffer) - 1, &InputLen, NULL);

		if(InputLen == 0)
			continue;

		TCHAR* Cmd = InputBuffer;

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
			SetCurrentCmd(Cmd);
	}

	CloseHandle(ConsoleInput);

	return 0;
}

static BOOL WINAPI SignalHandler(DWORD)
{
	GIsRequestingExit = 1;

	// Send an input event to unblock the input thread
	const HANDLE ConsoleInput = CreateFileA("CONIN$", GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

	if(ConsoleInput != INVALID_HANDLE_VALUE)
	{
		INPUT_RECORD Record;
		appMemzero(&Record, sizeof(Record));
		Record.EventType                      = KEY_EVENT;
		Record.Event.KeyEvent.bKeyDown        = TRUE;
		Record.Event.KeyEvent.wRepeatCount    = 1;
		Record.Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
		Record.Event.KeyEvent.uChar.AsciiChar = '\r';

		DWORD Written;
		WriteConsoleInputA(ConsoleInput, &Record, 1, &Written);
		CloseHandle(ConsoleInput);
	}

	return TRUE;
}

// Replacement for UServerCommandlet::Main since the one from Engine.dll crashes because it doesn't assign a value to GEngine
INT UServerCommandletMain()
{
	SetConsoleCtrlHandler(SignalHandler, TRUE);
	SetDefaultMasterServerAddress();

	FString Language;

	if(GConfig->GetFString("Engine.Engine", "Language", Language, "System.ini"))
		UObject::SetLanguage(*Language);

	UClass* EngineClass = LoadClass<UEngine>(NULL, "ini:Engine.Engine.GameEngine", NULL, LOAD_NoFail, NULL);

	GEngine = ConstructObject<UEngine>(EngineClass);
	GEngine->Init();
	GIsRunning = 1;

	HANDLE InputThread     = CreateThread(NULL, 0, UpdateServerConsoleInput, NULL, 0, NULL);
	DOUBLE OldTime         = appSeconds();
	DOUBLE SecondStartTime = OldTime;
	DWORD  TickCount       = 0;

	// Main loop
	while(GIsRunning && !GIsRequestingExit)
	{
		DOUBLE       NewTime    = appSeconds();
		const TCHAR* CurrentCmd = GetCurrentCmd();

		if(CurrentCmd)
		{
			if(appStricmp(CurrentCmd, "CLS") == 0)
				system("cls");
			else if(!GEngine->Exec(CurrentCmd, *GWarn))
				GWarn->Log(LocalizeError("Exec", "Core"));

			SetCurrentCmd(NULL);
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
		const FLOAT MaxTickRate = GEngine->GetMaxTickRate();

		if(MaxTickRate > 0.0f)
		{
			const FLOAT Delta = (1.0f / MaxTickRate) - (appSeconds() - OldTime);

			appSleep(Delta > 0.0f ? Delta : 0.0f);
		}
	}

	GIsRunning = 1;

	WaitForSingleObject(InputThread, INFINITE);
	CloseHandle(InputThread);

	return 0;
}
