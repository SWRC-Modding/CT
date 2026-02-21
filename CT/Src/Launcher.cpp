#include "Core.h"
#include "FOutputDeviceFile.h"
#include "FOutputDeviceWindowsError.h"
#include "FFeedbackContextCmd.h"
#include "FConfigCacheIni.h"
#include "Engine.h"
#include "Editor.h"
#include "Window.h"
#include "GameSpyMgr.h"
#include "../Res/resource.h"

#pragma comment(lib, "Winmm.lib") // timeBeginPeriod, timeEndPeriod

// Allow short form of known render devices.
static FStringTemp GetFullRenderDeviceClassName(const FString& RenderDeviceClass)
{
	if(RenderDeviceClass == "D3D")
		return "D3DDrv.D3DRenderDevice";
	else if(RenderDeviceClass == "OpenGL")
		return "OpenGLDrv.OpenGLRenderDevice";
	else if(RenderDeviceClass == "Mod")
		return "Mod.ModRenderDevice";
	else if(RenderDeviceClass == "Rtx")
		return "RtxDrv.RtxRenderDevice";

	return RenderDeviceClass;
}

static void EndFullscreen()
{
	if(GEngine && GEngine->Client && GEngine->Client->Viewports.Num() > 0 && GEngine->Client->Viewports[0])
		GEngine->Client->Viewports[0]->EndFullscreen();
}

static bool SwitchRenderDevice(UClass* Class)
{
	if(!Class || GEngine->GRenDev->GetClass() == Class)
		return false;

	check(Class->IsChildOf(URenderDevice::StaticClass()));

	// Apparently the FStatRecord destructor does not clean up properly and there is still a reference to the old stat record stored in the linked list of child records.
	// So we need to find the records belonging to the old render device and clean them up.
	// This is only done once the render device has successfully been changed but the pointer to the record needs to be acquired here.
	// TODO: Also do the same for OpenGL once/if FStatRecord is used there
	FStatRecord* OldStatRecord = FStatRecord::Main().FindStat("D3D", true);

	// UViewport::TryRenderDevice only works if GEngine->GRenDev is not the same as the current render device
	GEngine->GRenDev = ConstructObject<URenderDevice>(Class);
	GEngine->GRenDev->Init();

	// We need to reset all FCanvasUtils since they still reference the old render interface
	foreachobj(UCanvas, It)
	{
		if(It->pCanvasUtil)
		{
			delete It->pCanvasUtil;
			It->pCanvasUtil = NULL;
		}
	}

	UViewport* Viewport = GEngine->Client->Viewports[0];
	Viewport->TryRenderDevice(Class->GetPathName(), Viewport->SizeX, Viewport->SizeY, Viewport->IsFullscreen());

	if(GEngine->GRenDev && GEngine->GRenDev->GetClass() == Class)
	{
		GConfig->SetString("Engine.Engine", "RenderDevice", Class->GetPathName());

		if(OldStatRecord)
			RemoveStatRecordFromChildList(*OldStatRecord, FStatRecord::Main());

		return true;
	}

	return false;
}

static ULevel* GetLevel()
{
	if(GEngine->IsA<UGameEngine>())
		return static_cast<UGameEngine*>(GEngine)->GLevel;

	if(GEngine->IsA<UEditorEngine>())
		return static_cast<UEditorEngine*>(GEngine)->Level;

	return NULL;
}

static struct FExecHook : public FExec, FNotifyHook{
	WConfigProperties* Preferences;

	FExecHook() : Preferences(NULL){}

	virtual void NotifyDestroy(void* Src)
	{
		if(Src == Preferences)
		{
			Preferences = NULL;
		}
		else
		{
			// Set Actor bSelected to false if properties window is closed

			TCHAR SrcWindowClassName[128];
			TCHAR ObjPropWindowClassName[128];

			static_cast<WWindow*>(Src)->GetWindowClassName(SrcWindowClassName);
			MakeWindowClassName(ObjPropWindowClassName, "WObjectProperties");

			if(appStricmp(SrcWindowClassName, ObjPropWindowClassName) == 0)
			{
				WObjectProperties* P = static_cast<WObjectProperties*>(Src);

				for(INT i = 0; i < P->Root._Objects.Num(); ++i)
				{
					AActor* A = Cast<AActor>(P->Root._Objects[i]);

					if(A)
						A->bSelected = 0;
				}
			}
		}
	}

	AActor* FindActor(const TCHAR* Cmd, FOutputDevice& Ar)
	{
		ULevel* Level = GetLevel();

		if(!Level)
		{
			Ar.Log("No level loaded");
			return NULL;
		}

		UClass* Class = NULL;

		if(appStrfind(Cmd, "Class="))
		{
			if(!ParseObject<UClass>(Cmd, "Class=", Class, ANY_PACKAGE))
			{
				Ar.Log("Invalid class name");
				return NULL;
			}

			if(!Class->IsChildOf(AActor::StaticClass()))
			{
				Ar.Log("Not a subclass of Actor");
				return NULL;
			}
		}

		FName ActorName = NAME_None;
		UBOOL HaveName = Parse(Cmd, "Name=", ActorName);

		AActor* Found = NULL;
		APlayerController* Player = GIsClient ? GEngine->Client->Viewports[0]->Actor : NULL;

		if(Class)
		{
			// Find the closest Actor with the given class and optional name
			FLOAT MinDist = 999999.0f;

			foreach(AllActors, AActor, It, Level)
			{
				FLOAT Dist = Player ? FDist(It->Location, Player->Pawn ? Player->Pawn->Location : Player->Location) : 0.0f;

				if(!It->bDeleteMe && It->IsA(Class) && Dist < MinDist && (!HaveName || It->GetFName() == ActorName))
				{
					MinDist = Dist;
					Found   = *It;
				}
			}
		}
		else
		{
			if(HaveName) // Find the first actor with the given name
			{
				foreach(AllActors, AActor, It, Level)
				{
					if(!It->bDeleteMe && It->GetFName() == ActorName)
					{
						Found = *It;
						break;
					}
				}
			}
			else
			{
				if(Player)
					Found = Player->Target;
			}
		}

		if(!Found)
			Ar.Logf("Target not found");

		return Found;
	}

	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar)
	{
		guardFunc;

		if(ParseCommand(&Cmd, "XLIVE"))
			return 0;

		if(ParseCommand(&Cmd, "SHOWLOG"))
		{
			EndFullscreen();
			GLogWindow->Show(1);
			SetFocus(*GLogWindow);
			GLogWindow->Display.ScrollCaret();

			return 1;
		}
		else if(ParseCommand(&Cmd, "HIDELOG"))
		{
			GLogWindow->Show(0);
			return 1;
		}
		else if(ParseCommand(&Cmd, "EDITDEFAULT"))
		{
			UClass* Class;

			if(ParseObject<UClass>(Cmd, "Class=", Class, ANY_PACKAGE))
			{
				UObject* Object = Class->GetDefaultObject();

				// HACK:
				// This command crashes with the original exe because the returned default object does not contain a vtable.
				// Calling the internal constructor initializes it. It's safe to do this each time since the default constructor performs no other initialization.
				UObject::InternalConstructor(Object);

				WObjectProperties* P = new WObjectProperties("EditDefault", 0, "", NULL, 1);
				P->OpenWindow(GLogWindow->hWnd);
				P->Root.SetObjects(&Object, 1);
				P->Show(1);
			}
			else
			{
				Ar.Log("Missing class");
			}

			return 1;
		}
		else if(ParseCommand(&Cmd, "EDITOBJ"))
		{
			UClass*          Class;
			FName            ObjectName;
			UObject*         Outer;
			TArray<UObject*> Objects;

			if(ParseObject<UClass>(Cmd, "Class=", Class, ANY_PACKAGE))
			{
				for(FObjectIterator It; It; ++It)
				{
					if(It->IsA(Class))
						Objects.AddItem(*It);
				}
			}
			else if(ParseObject<UObject>(Cmd, "Outer=", Outer, ANY_PACKAGE))
			{
				for(FObjectIterator It; It; ++It)
				{
					if(It->IsIn(Outer))
						Objects.AddItem(*It);
				}
			}
			else
			{
				if(!Parse(Cmd, "Name=", ObjectName))
					ObjectName = FName(Cmd, FNAME_Find);

				if(ObjectName != NAME_None)
				{
					for(FObjectIterator It; It; ++It)
					{
						if(It->GetFName() == ObjectName)
							Objects.AddItem(*It);
					}
				}
			}

			UBOOL ShowAll = 0;
			ParseUBOOL(Cmd, "All=", ShowAll);

			if(Objects.Num() > 0)
			{
				EndFullscreen();

				if(ShowAll)
				{
					for(INT i = 0; i < Objects.Num(); ++i)
					{
						WObjectProperties* P = new WObjectProperties("EditObj", 0, "", NULL, 1);
						P->OpenWindow(GLogWindow->hWnd);
						P->Root.SetObjects(&Objects[i], 1);
						P->Show(1);
					}
				}
				else
				{
					// Cycle through objects when entering the same command again
					static INT     Index     = 0;
					static UClass* PrevClass = NULL;

					if(PrevClass != Class)
					{
						Index = 0;
						PrevClass = Class;
					}

					WObjectProperties* P = new WObjectProperties("EditObj", 0, "", NULL, 1);
					P->OpenWindow(GLogWindow->hWnd);
					P->Root.SetObjects(&Objects[Index++ % Objects.Num()], 1);
					P->Show(1);
				}
			}
			else
			{
				Ar.Logf("No objects found");
			}

			return 1;
		}
		else if(ParseCommand(&Cmd, "EDITACTOR"))
		{
			AActor* Actor = FindActor(Cmd, Ar);

			if(Actor)
			{
				EndFullscreen();
				WObjectProperties* P = new WObjectProperties("EditActor", 0, "", NULL, 1);
				P->SetNotifyHook(this);
				P->OpenWindow(GLogWindow->hWnd);
				P->Root.SetObjects((UObject**)&Actor, 1);
				P->Show(1);

				foreach(AllActors, AActor, It, Actor->XLevel)
					It->bSelected = 0;

				Actor->bSelected = 1;
			}

			return 1;
		}
		else if(ParseCommand(&Cmd, "ACTORINFO"))
		{
			INT Anim = 0;
			Parse(Cmd, "Anim=", Anim);

			AActor* Actor = FindActor(Cmd, Ar);

			if(Actor)
			{
				EndFullscreen();
				WDebugObject* P = new WDebugObject(Anim, Actor, NULL);
				P->OpenWindow(GLogWindow->hWnd);
				P->Show(1);
			}

			return 1;

		}
		else if(ParseCommand(&Cmd, "COPYACTOR"))
		{
			AActor* Actor = FindActor(Cmd, Ar);

			if(Actor)
			{
				FStringOutputDevice Out;
				UExporter::ExportToOutputDevice(Actor, NULL, Out, "T3D", 0);
				appClipboardCopy(*Out);
			}

			return 1;
		}
		else if(ParseCommand(&Cmd, "COPYLEVEL"))
		{
			ULevel* Level = GetLevel();

			if(Level)
			{
				FStringOutputDevice Out;
				UExporter::ExportToOutputDevice(Level, NULL, Out, "T3D", 0);
				appClipboardCopy(*Out);
			}
			else
			{
				Ar.Log("No level loaded");
			}

			return 1;
		}
		else if(ParseCommand(&Cmd, "PREFERENCES"))
		{
			EndFullscreen();

			if(!Preferences)
			{
				Preferences = new WConfigProperties("Preferences", LocalizeGeneral("AdvancedOptionsTitle", "Window"));
				Preferences->SetNotifyHook(this);
				Preferences->OpenWindow(GLogWindow->hWnd);
				Preferences->ForceRefresh();
			}

			Preferences->Show(1);
			SetFocus(Preferences->hWnd);

			return 1;
		}else if(GIsClient && ParseCommand(&Cmd, "GETRES")){
			Ar.Logf("%ix%i", GEngine->Client->Viewports[0]->SizeX, GEngine->Client->Viewports[0]->SizeY);
			return 1;
		}else if(ParseCommand(&Cmd, "GETRENDEV")){
			if(GEngine && GEngine->GRenDev)
				Ar.Log(GEngine->GRenDev->GetClass()->GetPathName());
			else
				Ar.Logf("No render device in use");

			return 1;
		}else if(!GIsEditor && GIsClient && ParseCommand(&Cmd, "USERENDEV")){
			FString RenderDeviceClass = GetFullRenderDeviceClassName(Cmd);
			UClass* Class = LoadClass<URenderDevice>(NULL, *RenderDeviceClass, NULL, LOAD_NoWarn | LOAD_Quiet, NULL);

			if(Class)
			{
				if(!SwitchRenderDevice(Class))
					Ar.Log("Failed to switch render device");
			}
			else
			{
				Ar.Logf("Failed to find render device '%s'", *RenderDeviceClass);
			}

			return 1;
		}
		else if(ParseCommand(&Cmd, ":q"))
		{
			GIsRequestingExit = 1;

			return 1;
		}

		return 0;

		unguard;
	}
} LauncherExecHook;

static bool RenDevSetOnCommandLine = false;

static void InitEngine()
{
	guardFunc;
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
		RenderDeviceClass = GetFullRenderDeviceClassName(RenderDeviceClass);
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
static void MainLoop()
{
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

	while(GIsRunning && !GIsRequestingExit)
	{
		DOUBLE NewTime  = appSeconds();
		FLOAT DeltaTime = NewTime - OldTime;

		// Handle all incoming messages.
		guard(MessagePump);
		MSG Msg;

		while(PeekMessageA(&Msg,NULL, 0, 0, PM_REMOVE))
		{
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

		if(OldTime > SecondStartTime + 1)
		{
			GEngine->CurrentTickRate = (FLOAT)TickCount / (OldTime - SecondStartTime);
			SecondStartTime = OldTime;
			TickCount = 0;
		}
		unguard;

		// Enforce optional maximum tick rate.
		guard(EnforceTickRate);
		DWORD MaxTickRate = static_cast<DWORD>(GEngine->GetMaxTickRate());

		if(MaxTickRate > 0)
		{
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
		}
		else
		{
			QueryPerformanceCounter(&Counter);

			PreviousTicks = Counter.QuadPart;
		}

		unguard;
	}

	GIsRunning = 0;

	unguard;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
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

		// Show the custom icon in the taskbar and window title bar
		HICON hIcon = LoadIconA(GetModuleHandleA(NULL), MAKEINTRESOURCEA(IDI_ICON1));
		if(hIcon)
		{
			SendMessageA(GLogWindow->hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

			if(GEngine->Client && GEngine->Client->Viewports.Num() > 0)
			{
				SendMessageA((HWND)GEngine->Client->Viewports[0]->GetWindow(), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				SendMessageA((HWND)GEngine->Client->Viewports[0]->GetWindow(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			}
		}

		// Init SWRCFix if it exists
		void* ModDLL = appGetDllHandle("Mod.dll");
		if(ModDLL)
		{
			void(CDECL*InitSWRCFix)(void) = static_cast<void(CDECL*)(void)>(appGetDllExport(ModDLL, "InitSWRCFix"));

			if(InitSWRCFix)
			{
				InitSWRCFix();

				if(!RenDevSetOnCommandLine && GEngine->GRenDev && appStricmp(GEngine->GRenDev->GetClass()->GetPathName(), "D3DDrv.D3DRenderDevice") == 0)
				{
					EndFullscreen(); // D3D doesn't like being created while already in fullscreen
					SwitchRenderDevice(LoadClass<URenderDevice>(NULL, "Mod.ModRenderDevice", NULL, LOAD_NoWarn | LOAD_Quiet, NULL));
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
	}
	catch(...)
	{
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
