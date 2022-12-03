#include "../../Core/Inc/Core.h"
#include "../../Core/Inc/FOutputDeviceFile.h"
#include "../../Core/Inc/FOutputDeviceWindowsError.h"
#include "../../Core/Inc/FFeedbackContextCmd.h"
#include "../../Core/Inc/FConfigCacheIni.h"
#include "../../Engine/Inc/Engine.h"
#include "../../Window/Inc/Window.h"
#include "../../GameSpyMgr/Inc/GameSpyMgr.h"

#pragma comment(lib, "Winmm.lib") // timeBeginPeriod, timeEndPeriod

static struct FExecHook : public FExec, FNotifyHook{
	WConfigProperties* Preferences;

	FExecHook() : Preferences(NULL){}

	virtual void NotifyDestroy(void* Src){
		if(Src == Preferences){
			Preferences = NULL;
		}else{
			// Set Actor bSelected to false if properties window is closed

			TCHAR SrcWindowClassName[128];
			TCHAR ObjPropWindowClassName[128];

			static_cast<WWindow*>(Src)->GetWindowClassName(SrcWindowClassName);
			MakeWindowClassName(ObjPropWindowClassName, "WObjectProperties");

			if(appStricmp(SrcWindowClassName, ObjPropWindowClassName) == 0){
				WObjectProperties* P = static_cast<WObjectProperties*>(Src);

				for(INT i = 0; i < P->Root._Objects.Num(); ++i){
					AActor* A = Cast<AActor>(P->Root._Objects[i]);

					if(A)
						A->bSelected = 0;
				}
			}
		}
	}

	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar){
		guard(FExecHook::Exec);

		HWND ParentWindow = NULL;

		if(GEngine->Client && GEngine->Client->Viewports.Num() > 0)
			ParentWindow = static_cast<HWND>(GEngine->Client->Viewports[0]->GetWindow());
		else
			ParentWindow = GLogWindow->hWnd;

		if(ParseCommand(&Cmd, "SHOWLOG")){
			GEngine->Client->Viewports[0]->Exec("ENDFULLSCREEN", Ar);
			GLogWindow->Show(1);
			SetFocus(*GLogWindow);
			GLogWindow->Display.ScrollCaret();

			return 1;
		}else if(ParseCommand(&Cmd, "HIDELOG")){
			GLogWindow->Show(0);

			return 1;
		}else if(ParseCommand(&Cmd, "EDITOBJ")){
			UClass*          Class;
			FName            ObjectName;
			UObject*         Outer;
			TArray<UObject*> Objects;

			if(ParseObject<UClass>(Cmd, "Class=", Class, ANY_PACKAGE)){
				for(FObjectIterator It; It; ++It){
					if(It->IsA(Class))
						Objects.AddItem(*It);
				}
			}else if(ParseObject<UObject>(Cmd, "Outer=", Outer, ANY_PACKAGE)){
				for(FObjectIterator It; It; ++It){
					if(It->IsIn(Outer))
						Objects.AddItem(*It);
				}
			}else{
				if(!Parse(Cmd, "Name=", ObjectName))
					ObjectName = FName(Cmd, FNAME_Find);

				if(ObjectName != NAME_None){
					for(FObjectIterator It; It; ++It){
						if(It->GetFName() == ObjectName)
							Objects.AddItem(*It);
					}
				}
			}

			UBOOL ShowAll = 0;
			ParseUBOOL(Cmd, "All=", ShowAll);

			if(Objects.Num() > 0){
				GEngine->Client->Viewports[0]->Exec("ENDFULLSCREEN", Ar);

				if(ShowAll){
					for(INT i = 0; i < Objects.Num(); ++i){
						WObjectProperties* P = new WObjectProperties("EditObj", 0, "", NULL, 1);
						P->SetNotifyHook(this);
						P->OpenWindow(ParentWindow);
						P->Root.SetObjects(&Objects[i], 1);
						P->Show(1);
					}
				}else{
					static INT Count = 0; // Used to cycle through objects when reentering the same command

					WObjectProperties* P = new WObjectProperties("EditObj", 0, "", NULL, 1);
					P->SetNotifyHook(this);
					P->OpenWindow(ParentWindow);
					P->Root.SetObjects(&Objects[Count++ % Objects.Num()], 1);
					P->Show(1);
				}
			}else{
				Ar.Logf("No objects found");
			}

			return 1;
		}else if(ParseCommand(&Cmd, "EDITACTOR")){
			UClass* Class;
			AActor* Found = NULL;
			FName ActorName;
			APlayerController* Player = GIsClient ? GEngine->Client->Viewports[0]->Actor : NULL;
			ULevel* Level = NULL;

			if(GEngine->IsA<UGameEngine>())
				Level = static_cast<UGameEngine*>(GEngine)->GLevel;

			if(Level){
				if(ParseObject<UClass>(Cmd, "Class=", Class, ANY_PACKAGE)){
					FLOAT MinDist = 999999.0f;

					foreach(AllActors, AActor, It, Level){
						FLOAT Dist = Player ? FDist(It->Location, Player->Pawn ? Player->Pawn->Location : Player->Location) : 0.0f;

						if(!It->bDeleteMe && It->IsA(Class) && Dist < MinDist){
							MinDist = Dist;
							Found   = *It;
						}
					}
				}else if(Parse(Cmd, "Name=", ActorName)){
					foreach(AllActors, AActor, It, Level){
						if(!It->bDeleteMe && It->GetFName() == ActorName){
							Found = *It;

							break;
						}
					}
				}else if(Player){
					Found = Player->Target;
				}

				if(Found){
					GEngine->Client->Viewports[0]->Exec("ENDFULLSCREEN", Ar);
					WObjectProperties* P = new WObjectProperties("EditActor", 0, "", NULL, 1);
					P->SetNotifyHook(this);
					P->OpenWindow(ParentWindow);
					P->Root.SetObjects((UObject**)&Found, 1);
					P->Show(1);

					foreach(AllActors, AActor, It, Level)
						It->bSelected = 0;

					Found->bSelected = 1;
				}else{
					Ar.Logf("Target not found");
				}
			}else{
				Ar.Log("No Level Loaded");
			}

			return 1;
		}else if(ParseCommand(&Cmd, "PREFERENCES")){
			GEngine->Client->Viewports[0]->Exec("ENDFULLSCREEN", Ar);

			if(!Preferences){
				Preferences = new WConfigProperties("Preferences", LocalizeGeneral("AdvancedOptionsTitle", "Window"));
				Preferences->SetNotifyHook(this);
				Preferences->OpenWindow(ParentWindow);
				Preferences->ForceRefresh();
			}

			Preferences->Show(1);
			SetFocus(Preferences->hWnd);

			return 1;
		}else if(ParseCommand(&Cmd, "GETRENDEV")){
			if(GEngine && GEngine->GRenDev)
				Ar.Log(GEngine->GRenDev->GetClass()->GetPathName());
			else
				Ar.Logf("No render device in use");

			return 1;
		}else if(!GIsEditor && GIsClient && ParseCommand(&Cmd, "USERENDEV")){
			FString RenderDeviceClass = Cmd;

			if(RenderDeviceClass == "D3D")
				RenderDeviceClass = "D3DDrv.D3DRenderDevice";
			else if(RenderDeviceClass == "OpenGL")
				RenderDeviceClass = "OpenGLDrv.OpenGLRenderDevice";
			else if(RenderDeviceClass == "Mod")
				RenderDeviceClass = "Mod.ModRenderDevice";

			UClass* Class = LoadClass<URenderDevice>(NULL, *RenderDeviceClass, NULL, LOAD_NoWarn | LOAD_Quiet, NULL);

			if(Class){
				if(GEngine->GRenDev && GEngine->GRenDev->GetClass() != Class){
					// Apparently the FStatRecord destructor does not clean up properly and there is still a reference to the old stat record stored in the linked list of child records.
					// So we need to find the records belonging to the old render device and clean them up.
					// This is only done once the render device has successfully been changed but the pointer to the record needs to be acquired here.
					// TODO: Also do the same for OpenGL once/if FStatRecord is used there
					FStatRecord* OldStatRecord = FStatRecord::Main().FindStat("D3D", true);

					// UViewport::TryRenderDevice only works if GEngine->GRenDev is not the same as the current render device
					GEngine->GRenDev = ConstructObject<URenderDevice>(Class);
					GEngine->GRenDev->Init();

					// We need to reset all FCanvasUtils since they still reference the old render interface
					foreachobj(UCanvas, It){
						if(It->pCanvasUtil){
							delete It->pCanvasUtil;
							It->pCanvasUtil = NULL;
						}
					}

					UViewport* Viewport = GEngine->Client->Viewports[0];
					Viewport->TryRenderDevice(*RenderDeviceClass, Viewport->SizeX, Viewport->SizeY, Viewport->IsFullscreen());

					if(GEngine->GRenDev && GEngine->GRenDev->GetClass() == Class){
						GConfig->SetString("Engine.Engine", "RenderDevice", *RenderDeviceClass);

						if(OldStatRecord)
							RemoveStatRecordFromChildList(*OldStatRecord, FStatRecord::Main());
					}else{
						Ar.Logf("Failed to set render device with class %s", *RenderDeviceClass);
					}
				}
			}else{
				Ar.Logf("Unable to find render device class '%s'", *RenderDeviceClass);
			}

			return 1;
		}

		return 0;

		unguard;
	}
} LauncherExecHook;

static bool RenDevSetOnCommandLine = false;

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

	// Detect RenderDevice

	FString RenderDeviceClass;

	if(Parse(appCmdLine(), "RenDev=", RenderDeviceClass)){
		// Allow short form of known render devices.
		if(RenderDeviceClass == "D3D")
			RenderDeviceClass = "D3DDrv.D3DRenderDevice";
		else if(RenderDeviceClass == "OpenGL")
			RenderDeviceClass = "OpenGLDrv.OpenGLRenderDevice";
		else if(RenderDeviceClass == "Mod")
			RenderDeviceClass = "Mod.ModRenderDevice";

		debugf("RenderDevice set on command line: %s", *RenderDeviceClass);
		GConfig->SetString("Engine.Engine", "RenderDevice", *RenderDeviceClass);
		RenDevSetOnCommandLine = true;
	}

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
	DWORD  TickCount = 0;

	LARGE_INTEGER CounterFrequency;
	LARGE_INTEGER Counter;

	QueryPerformanceFrequency(&CounterFrequency);
	QueryPerformanceCounter(&Counter);

	QWORD TicksFor2ms = (CounterFrequency.QuadPart + 499) / 500;
	QWORD PreviousTicks = Counter.QuadPart;
	QWORD RemainingTicks = 0;

	while(GIsRunning && !GIsRequestingExit){
		DOUBLE NewTime  = appSeconds();
		FLOAT DeltaTime = NewTime - OldTime;

		// Handle all incoming messages.
		guard(MessagePump);
		MSG Msg;

		while(PeekMessageA(&Msg,NULL, 0, 0, PM_REMOVE)){
			if(Msg.message == WM_QUIT)
				GIsRequestingExit = 1;

			TranslateMessage(&Msg);
			DispatchMessageA(&Msg);
		}
		unguard;

		// Update the world.
		guard(UpdateWorld);
		GEngine->Tick(DeltaTime);

		if(GWindowManager)
			GWindowManager->Tick(DeltaTime);

		OldTime = NewTime;
		++TickCount;

		if(OldTime > SecondStartTime + 1){
			GEngine->CurrentTickRate = (FLOAT)TickCount / (OldTime - SecondStartTime);
			SecondStartTime = OldTime;
			TickCount = 0;
		}
		unguard;

		// Enforce optional maximum tick rate.
		guard(EnforceTickRate);
		DWORD MaxTickRate = static_cast<DWORD>(GEngine->GetMaxTickRate());

		if(MaxTickRate > 0){
			QWORD Wait = (CounterFrequency.QuadPart + RemainingTicks) / MaxTickRate;
			RemainingTicks = (CounterFrequency.QuadPart + RemainingTicks) % MaxTickRate;

			timeBeginPeriod(1);

			QWORD Delta;

			do{
				QueryPerformanceCounter(&Counter);
				Delta = Counter.QuadPart - PreviousTicks;

				if(Wait - Delta > TicksFor2ms)
					Sleep(1);
			}while(Delta < Wait);

			timeEndPeriod(1);

			PreviousTicks += Wait;
		}else{
			QueryPerformanceCounter(&Counter);

			PreviousTicks = Counter.QuadPart;
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
	FFeedbackContextCmd Warn;

	GIsStarted = 1;

	try{
		GameSpyCDKeyResponseInterface CDKeyInterface;

		GIsGuarded = 1;

		appInit(appPackage(), lpCmdLine, &Log, &Error, &Warn, FConfigCacheIni::Factory, 1);

		GIsClient = ParseParam(appCmdLine(), "SERVER") == 0;
		GIsServer = 1;
		GIsEditor = 0;
		GIsScriptable = 1;
		GLazyLoad = ParseParam(appCmdLine(), "LAZY");
		GUseFrontEnd = ParseParam(appCmdLine(), "NOUI") == 0;

		InitWindowing();

		GLogWindow = new WLog(Log.Filename, Log.LogAr, "GameLog");
		GLogWindow->OpenWindow(ParseParam(appCmdLine(), "LOG"), 0);
		GLogWindow->Log(NAME_Title, LocalizeGeneral("Start", "SWRepublicCommando"));

		GExec = &LauncherExecHook;

		InitEngine();

		// Init SWRCFix if it exists
		void* ModDLL = appGetDllHandle("Mod.dll");

		if(ModDLL){
			void(CDECL*InitSWRCFix)(void) = static_cast<void(CDECL*)(void)>(appGetDllExport(ModDLL, "InitSWRCFix"));

			if(InitSWRCFix){
				InitSWRCFix();

				if(!RenDevSetOnCommandLine && GEngine->GRenDev && appStricmp(GEngine->GRenDev->GetClass()->GetPathName(), "D3DDrv.D3DRenderDevice") == 0){
					GEngine->Client->Viewports[0]->Exec("ENDFULLSCREEN", *GLog); // D3D doesn't like being created while already in fullscreen
					LauncherExecHook.Exec("USERENDEV MOD", *GLog);
				}
			}
		}

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

// Force use of dedicated GPU
extern "C"{
	DLL_EXPORT DWORD NvOptimusEnablement = 1;
	DLL_EXPORT DWORD AmdPowerXpressRequestHighPerformance = 1;
}
