#include "../../Core/Inc/Core.h"
#include "../../Core/Inc/FOutputDeviceFile.h"
#include "../../Core/Inc/FOutputDeviceWindowsError.h"
#include "../../Core/Inc/FFeedbackContextCmd.h"
#include "../../Core/Inc/FConfigCacheIni.h"
#include "../../Engine/Inc/Engine.h"
#include "../../Window/Inc/Window.h"
#include "../../GameSpyMgr/Inc/GameSpyMgr.h"

static struct FExecHook : public FExec{
	UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar){
		guard(FExecHook::Exec);

		if(ParseCommand(&Cmd, "SHOWLOG")){
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

			return 1;
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

	GLogWindow->SetExec(GEngine);
	GLogWindow->Log(NAME_Title, LocalizeGeneral("Run", "SWRepublicCommando"));

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

			TranslateMessage(&Msg);
			DispatchMessageA( &Msg );
		}

		unguard;
	}

	GIsRunning = 0;

	unguard;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){
	int ExitCode = EXIT_SUCCESS;
	FOutputDeviceFile Log;
	FOutputDeviceWindowsError Error;
	FFeedbackContextCmd Warn; // TODO(Leon): Replace with FFeedbackContextWindows

	GIsStarted = 1;

	try{
		GameSpyCDKeyResponseInterface CDKeyInterface;

		GIsGuarded = 1;

		appInit(appPackage(), lpCmdLine, &Log, &Error, &Warn, FConfigCacheIni::Factory, 1);

		GIsClient = 1;
		GIsServer = 1;
		GIsEditor = 0;
		GIsScriptable = 1;
		GLazyLoad = ParseParam(appCmdLine(), "LAZY");
		GUseFrontEnd = ParseParam(appCmdLine(), "NOUI") == 0;

		// Using Mod.ModRenderDevice if it exists.
		{
			FString RenderDeviceClass;

			GConfig->GetString("Engine.Engine", "RenderDevice", RenderDeviceClass, "System.ini");

			if(RenderDeviceClass == "D3DDrv.D3DRenderDevice"){ // Only use custom render device if there isn't another one specified
				UClass* ModRenderDeviceClass = LoadClass<URenderDevice>(NULL, "Mod.ModRenderDevice", NULL, LOAD_NoWarn | LOAD_Quiet, NULL);

				if(ModRenderDeviceClass){
					GLog->Log("Using Mod.ModRenderDevice");
					GConfig->SetString("Engine.Engine", "RenderDevice", "Mod.ModRenderDevice", "System.ini");
				}
			}
		}

		InitWindowing();

		GLogWindow = new WLog(Log.Filename, Log.LogAr, "GameLog");
		GLogWindow->OpenWindow(ParseParam(appCmdLine(), "LOG"), 0);
		GLogWindow->Log(NAME_Title, LocalizeGeneral("Start", "SWRepublicCommando"));

		InitEngine();

		GExec = &LauncherExecHook;

		if(GEngine && !GIsRequestingExit)
			MainLoop();

		GLogWindow->Log(NAME_Title, LocalizeGeneral("Exit", "SWRepublicCommando"));

		if(GLogHook == GLogWindow)
			GLogHook = NULL;

		delete GLogWindow;

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
