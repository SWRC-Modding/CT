#include "../../Core/Inc/Core.h"
#include "../../Core/Inc/FOutputDeviceFile.h"
#include "../../Core/Inc/FOutputDeviceWindowsError.h"
#include "../../Core/Inc/FFeedbackContextCmd.h"
#include "../../Core/Inc/FConfigCacheIni.h"
#include "../../Engine/Inc/Engine.h"
#include "../../Window/Inc/Window.h"
#include "../../GameSpyMgr/Inc/GameSpyMgr.h"

FOutputDeviceFile Log;
FOutputDeviceWindowsError Error;
FFeedbackContextCmd Warn; // TODO(Leon): Replace with FFeedbackContextWindows

static struct FExecHook : public FExec{
	FExecHook(){ GExec = this; }

	UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar){
		guard(FExecHook::Exec);

		if(ParseCommand(&Cmd, "SHOWLOG")){
			if(!GLogWindow){
				// Create log window if it does not yet exist
				GLogWindow = new WLog(Log.Filename, Log.LogAr, "GameLog");
				GLogWindow->OpenWindow(1, 0);

				if(!GIsRunning)
					GLogWindow->Log(NAME_Title, LocalizeGeneral("Start", "SWRepublicCommando"));
				else
					GLogWindow->Log(NAME_Title, LocalizeGeneral("Run", "SWRepublicCommando"));

				if(GEngine)
					GLogWindow->SetExec(GEngine);
			}

			GLogWindow->Show(1);
			SetFocus(*GLogWindow);
			GLogWindow->Display.ScrollCaret();

			return 1;
		}else if(ParseCommand(&Cmd, "HIDELOG")){
			if(GLogWindow)
				GLogWindow->Show(0);

			return 1;
		}else if(ParseCommand(&Cmd, "EDITOBJ")){
			UObject* Obj = FindObject<UObject>(ANY_PACKAGE, Cmd);

			if(Obj){
				WObjectProperties* P = new WObjectProperties("EditObj", 0, "", NULL, 1);
				P->OpenWindow((HWND)GEngine->Client->Viewports[0]->GetWindow());
				P->Root.SetObjects(&Obj, 1);
				P->Show(1);
			}else{
				Ar.Logf("Object \"%s\" not found", Cmd);
			}

			return 1;
		}else if(ParseCommand(&Cmd, "EDITACTOR")){
			UClass* Class;
			UObject* Found = NULL;

			if(ParseObject<UClass>(Cmd, "Class=", Class, ANY_PACKAGE)){
				AActor* Player = GEngine->Client->Viewports[0]->Actor;
				FLOAT   MinDist = 999999.0f;

				for(TObjectIterator<AActor> It; It; ++It){
					FLOAT Dist = Player ? FDist(It->Location, Player->Location) : 0.0f;

					if((!Player || It->GetLevel() == Player->GetLevel()) &&
					   !It->bDeleteMe &&
					   It->IsA(Class) &&
					   Dist < MinDist){
						MinDist = Dist;
						Found   = *It;
					}
				}
			}else{
				FName ActorName;

				if(Parse(Cmd, "Name=", ActorName)){
					for(TObjectIterator<AActor> It; It; ++It){
						if(!It->bDeleteMe && It->GetFName() == ActorName){
							Found = *It;

							break;
						}
					}
				}
			}

			if(Found){
				WObjectProperties* P = new WObjectProperties("EditActor", 0, "", NULL, 1);
				P->OpenWindow((HWND)GEngine->Client->Viewports[0]->GetWindow());
				P->Root.SetObjects((UObject**)&Found, 1);
				P->Show(1);
			}else{
				Ar.Logf("Target not found");
			}
		}

		return 0;

		unguard;
	}
} LauncherExecHook;

static void InitEngine(){
	guard(InitEngine);
	check(!GEngine);
	DOUBLE LoadTime = appSeconds();

	// First-run menu.
	INT FirstRun = 0;

	GConfig->GetInt("FirstRun", "FirstRun", FirstRun);

	if(ParseParam(appCmdLine(), "FirstRun"))
		FirstRun = 0;

	// Update first-run.
	if(FirstRun < ENGINE_VERSION)
		FirstRun = ENGINE_VERSION;

	GConfig->SetInt("FirstRun", "FirstRun", FirstRun);

	// Create game engine.
	UClass* EngineClass = LoadClass<UEngine>(NULL, "ini:Engine.Engine.GameEngine", NULL, LOAD_NoFail, NULL);

	GEngine = ConstructObject<UEngine>(EngineClass);

	if(GLogWindow){
		GLogWindow->SetExec(GEngine);
		GLogWindow->Log(NAME_Title, LocalizeGeneral("Run", "SWRepublicCommando"));
	}

	GEngine->Init();
	debugf("Startup time: %f seconds", appSeconds() - LoadTime);
	unguard;
}

//
// Unreal's main message loop.  All windows in Unreal receive messages
// somewhere below this function on the stack.
//
static void MainLoop(){
	guard(MainLoop);
	check(GEngine);

	// Loop while running.
	GIsRunning = 1;
	DOUBLE OldTime = appSeconds();
	DOUBLE SecondStartTime = OldTime;
	INT TickCount = 0;

	while(GIsRunning && !GIsRequestingExit){
		// Update the world.
		guard(UpdateWorld);
		DOUBLE NewTime  = appSeconds();
		FLOAT DeltaTime = NewTime - OldTime;

		GEngine->Tick(DeltaTime);

		if(GWindowManager)
			GWindowManager->Tick(DeltaTime);

		OldTime = NewTime;
		TickCount++;

		if(OldTime > SecondStartTime + 1){
			GEngine->CurrentTickRate = (FLOAT)TickCount / (OldTime - SecondStartTime);
			SecondStartTime = OldTime;
			TickCount = 0;
		}
		unguard;

		// Enforce optional maximum tick rate.
		guard(EnforceTickRate);
		FLOAT MaxTickRate = GEngine->GetMaxTickRate();
		if(MaxTickRate > 0.0f){
			FLOAT Delta = (1.0f / MaxTickRate) - (appSeconds() - OldTime);
			appSleep(Max(0.0f, Delta));
		}
		unguard;

		// Handle all incoming messages.
		guard(MessagePump);
		MSG Msg;

		while(PeekMessageA(&Msg,NULL, 0, 0, PM_REMOVE)){
			if(Msg.message == WM_QUIT)
				GIsRequestingExit = 1;

			guard(TranslateMessage);
			TranslateMessage(&Msg);
			unguardf(("%08X %i", (INT)Msg.hwnd, Msg.message));

			guard(DispatchMessage);
			DispatchMessageA( &Msg );
			unguardf(("%08X %i", (INT)Msg.hwnd, Msg.message));
		}

		unguard;
	}

	GIsRunning = 0;

	unguard;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){
	int ExitCode = EXIT_SUCCESS;

	GIsStarted = 1;

	try{
		GIsGuarded = 1;

		/*
		 * Setting the language based on locale.
		 * Not really necessary since it is set in appInit later anyway, but this would cover the case when the ini entry is missing.
		 * Not even sure if RC supports all of these languages but whatever... That's how it is done in SWRepublicCommando.exe.
		 */
		switch(GetUserDefaultLCID() & 0xFF){
		case LANG_ENGLISH:
			UObject::SetLanguage("int");
			break;
		case LANG_JAPANESE:
			UObject::SetLanguage("jpt");
			break;
		case LANG_GERMAN:
			UObject::SetLanguage("det");
			break;
		case LANG_FRENCH:
			UObject::SetLanguage("frt");
			break;
		case LANG_SPANISH:
			UObject::SetLanguage("est");
			break;
		case LANG_ITALIAN:
			UObject::SetLanguage("itt");
			break;
		case LANG_KOREAN:
			UObject::SetLanguage("krt");
			break;
		case LANG_CHINESE:
			UObject::SetLanguage("cht");
			break;
		case LANG_PORTUGUESE:
			UObject::SetLanguage("prt");
		}

		FString CmdLine = FStringTemp(lpCmdLine) + " -windowed -log";

		appInit(appPackage(), *CmdLine, &Log, &Error, &Warn, FConfigCacheIni::Factory, 1);

		/*
		 * Using Localization from SWRepublicCommando.exe
		 * Without this there would have to be an accompanying (executable name).(lang) file that contains basic localization.
		 * We simply use the existing one from SWRepublicCommando.exe. That way it also doesn't matter if the executable is renamed.
		 */
		{
			FString LocalizationFile = FString(appPackage()) + "." + UObject::GetLanguage();
			FString OriginalLocalizationFile = FString("SWRepublicCommando.") + UObject::GetLanguage();

			*GConfig->GetSectionPrivate("General", 1, 1, *LocalizationFile) =*GConfig->GetSectionPrivate("General", 1, 1, *OriginalLocalizationFile);

			// We only need an in-memory copy of the localization so there's no need to save the files to disk
			GConfig->Detach(*LocalizationFile);
			GConfig->Detach(*OriginalLocalizationFile);
		}

		// Using Mod.ModRenderDevice if it exists.
		{
			FString RenderDeviceClass;

			GConfig->GetString("Engine.Engine", "RenderDevice", RenderDeviceClass, "System.ini");

			if(RenderDeviceClass == "D3DDrv.D3DRenderDevice"){
				UClass* ModRenderDeviceClass = LoadClass<URenderDevice>(NULL, "Mod.ModRenderDevice", NULL, LOAD_NoWarn | LOAD_Quiet, NULL);

				if(ModRenderDeviceClass){
					GLog->Log("Using Mod.ModRenderDevice");
					GConfig->SetString("Engine.Engine", "RenderDevice", "Mod.ModRenderDevice", "System.ini");
				}
			}
		}

		GIsClient = 1;
		GIsServer = 1;
		GIsEditor = 0;
		GIsScriptable = 1;
		GLazyLoad = ParseParam(appCmdLine(), "LAZY");
		GUseFrontEnd = ParseParam(appCmdLine(), "NOUI") == 0;

		InitWindowing();

		GameSpyCDKeyResponseInterface CDKeyInterface;

		if(ParseParam(appCmdLine(), "LOG"))
			LauncherExecHook.Exec("SHOWLOG", Log);

		InitEngine();

		if(GEngine && !GIsRequestingExit)
			MainLoop();

		if(GLogWindow)
			GLogWindow->Log(NAME_Title, LocalizeGeneral("Exit", "SWRepublicCommando"));

		if(GLogWindow){
			if(GLogHook == GLogWindow)
				GLogHook = NULL;

			delete GLogWindow;
		}

		appPreExit();

		GIsGuarded = 0;
	}catch(...){
		GIsGuarded = 0;
		ExitCode = EXIT_FAILURE;
		Error.HandleError();
	}

	appExit();

	GIsStarted = 0;

	return ExitCode;
}
