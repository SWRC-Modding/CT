/*
*	This is a custom UCC.exe for Star Wars Republic Commando since the game shipped without one.
*	Everything compiles fine with Visual Studio .NET 2003 which is being used to achieve maximum compatibility
*	since it was also used to compile RC
*	The following settings are required in order to compile everything without errors:
*		- Character Set = Not Set	//Important as RC does not use unicode
*		- Struct Member Alignment = 4 Bytes	//Probably not necessary, but just in case...
*		- Calling Convention = __fastcall	//RC uses __fastcall as default calling convention
*/

#include <cstdio>

#include "../../Engine/Inc/Engine.h" //Core and Engine

#include "../../Core/Inc/FOutputDeviceFile.h"
FOutputDeviceFile Log;

#include "../../Core/Inc/FOutputDeviceWindowsError.h" //Error handling using message boxes which is just nicer than having everything in the console
FOutputDeviceWindowsError Error;

#include "../../Core/Inc/FFeedbackContextAnsi.h"
FFeedbackContextAnsi Warn;

#include "../../Core/Inc/FConfigCacheIni.h"

//Variables for ServerCommandlet

FString CurrentCmd; //Contains the next command that is to be passed to UEngine::Exec and is gathered by the input thread
					//the command is then executed by the main thread to avoid issues

/*
*	Allows user input in the console while running a server
*	This function runs in a separate thread in order to not having
*	to pause the main loop while waiting for input
*/
DWORD WINAPI UpdateServerConsoleInput(PVOID){
	TCHAR Cmd[1024];

	while(GIsRunning && !GIsRequestingExit){
		if(std::fgets(Cmd, sizeof(Cmd), stdin)){
			Cmd[appStrlen(Cmd) - 1] = '\0'; //Removing newline added by fgets
			CurrentCmd = Cmd; //Updating CurrentCmd so that it can be executed by the main thread
							  //Nothing has been done in terms of thread safety as so far there haven't been any issues...

			//Returning in case user requested exit in order to not get to fgets again
			if(CurrentCmd == "EXIT" || CurrentCmd == "QUIT")
				return 0;
		}
	}

	return 0;
}

/*
*	Replacement for UServerCommandlet::Main since the one from Engine.dll crashes because it doesn't assign a value to GEngine
*/
void UServerCommandletMain(){
	FString Language;

	if(GConfig->GetString("Engine.Engine", "Language", Language, "System.ini"))
		UObject::SetLanguage(*Language);

	UClass* EngineClass = LoadClass<UEngine>(NULL, "ini:Engine.Engine.GameEngine", NULL, LOAD_NoFail, NULL);

	//Literally the only reason for this function to be rewritten
	//The original one doesn't assign a value to GEngine which leads to a gpf...
	GEngine = ConstructObject<UEngine>(EngineClass);

	GEngine->Init();

	//Creating input thread
	CreateThread(NULL, 0, UpdateServerConsoleInput, NULL, 0, NULL);

	DOUBLE OldTime = appSeconds();

	GIsRunning = 1;

	//Main loop
	while(GIsRunning && !GIsRequestingExit){
		DOUBLE NewTime = appSeconds();

		//Executing console commands that are gathered by UpdateServerConsoleInput in a different thread
		if(CurrentCmd.Len() > 0){
			if(CurrentCmd == "CLS") //In case user wants to clear screen. Can be useful for testing.
				system("cls"); //Hate using system but it's ok here
			else if(!GEngine->Exec(*CurrentCmd, Warn))
				Warn.Log(LocalizeError("Exec", "Core"));

			CurrentCmd.Empty();
		}

		//Update the world
		GEngine->Tick(NewTime - OldTime);
		OldTime = NewTime;

		//Enforce optional maximum tick rate
		FLOAT MaxTickRate = GEngine->GetMaxTickRate();

		if(MaxTickRate > 0.0f){
			FLOAT Delta = (1.0f / MaxTickRate) - (appSeconds() - OldTime);

			appSleep(Delta > 0.0f ? Delta : 0.0f);
		}
	}

	GIsRunning = 0;
}

void ShowBanner(){
	Warn.Log("=======================================");
	Warn.Log("ucc.exe for Star Wars Republic Commando");
	Warn.Log("made by Leon0628");
	Warn.Log("=======================================\n");
}

int main(int argc, char** argv){
	GIsStarted = 1;

	try{
		GIsGuarded = 1;

		FString CmdLine;

		for(int i = 1; i < argc; i++)
			CmdLine += FString(argv[i]) + " ";

		appInit("SWRepublicCommando", *CmdLine, &Log, &Error, &Warn, FConfigCacheIni::Factory, 1);
		UObject::SetLanguage("int");

		if(argc > 1){
			//Initializing global state
			GIsUCC = GIsClient = GIsServer = GIsEditor = GIsScriptable = GLazyLoad = 1;

			FString ClassName = argv[1];
			TArray<FRegistryObjectInfo> List;

			UObject::GetRegistryObjects(List, UClass::StaticClass(), UCommandlet::StaticClass(), 0); //Loading list of commandlets declared in .int files

			for(int i = 0; i < List.Num(); i++){ //Looking Token up in list and autocompleting class name if found
				FString Str = List[i].Object;

				if(ClassName == Str || ClassName + "Commandlet" == Str){ //Checking against "PackageName.ClassName (+ Commandlet)"
					ClassName = List[i].Object;

					break;
				}

				while(Str.InStr(".") >= 0) //Removing package name so that only class name remains
					Str = Str.Mid(Str.InStr(".") + 1);

				if(ClassName == Str || ClassName + "Commandlet" == Str){ //Checking against "ClassName (+ Commandlet)" and adding "PackageName"
					ClassName = List[i].Object;

					break;
				}
			}

			DWORD LoadFlags = LOAD_NoWarn | LOAD_Quiet;

			if(ClassName == "Editor.MakeCommandlet")
				LoadFlags |= LOAD_DisallowFiles;

			UClass* Class = LoadClass<UCommandlet>(NULL, *ClassName, NULL, LoadFlags, NULL);

			if(!Class) //If class failed to load appending "Commandlet" and trying again
				Class = LoadClass<UCommandlet>(NULL, *(ClassName + "Commandlet"), NULL, LoadFlags, NULL);

			if(Class){
				UCommandlet* Commandlet = ConstructObject<UCommandlet>(Class);
				UCommandlet* Default = static_cast<UCommandlet*>(Class->GetDefaultObject());

				if(Default->ShowBanner)
					ShowBanner();

				Warn.Logf("Executing %s\n", Class->GetFullName());

				GIsClient = Default->IsClient;
				GIsEditor = Default->IsEditor;
				GIsServer = Default->IsServer;
				GLazyLoad = Default->LazyLoad;

				//Contains only the command-line options that are passed to the commandlet to avoid problems with some commandlets
				FString CommandletCmdLine;

				for(int i = 2; i < argc; i++)
					CommandletCmdLine += FString(argv[i]) + " ";

				Commandlet->InitExecution();
				Commandlet->ParseParms(*CommandletCmdLine);

				if(Default->LogToStdout){ //Redirecting commandlet output to console
					Warn.AuxOut = GLog;
					GLog = &Warn;
				}
				
				if(ClassName == "Engine.ServerCommandlet")
					UServerCommandletMain(); //The ServerCommandlet has a special Main function
				else
					Commandlet->Main(CommandletCmdLine);

				if(Default->ShowErrorCount)
					Warn.Logf("\n%s - %i error(s), %i warning(s)", Warn.ErrorCount == 0 ? "Success" : "Failure", Warn.ErrorCount, Warn.WarningCount);

				if(Default->LogToStdout){
					Warn.AuxOut = NULL;
					GLog = &Log;
				}
			}else{
				ShowBanner();
				Warn.Logf("Commandlet %s not found", argv[1]);
			}
		}else{
			ShowBanner();
			Warn.Log("Usage:");
			Warn.Log("    ucc CommandletName <parameters>");
		}

		//For some reason this results in an infinite loop during garbage collection when there are compile errors with ucc make
		//Hopefully only a temporary fix...
		if(Warn.ErrorCount == 0)
			appPreExit();

		GIsGuarded = 0;
	}catch(...){
		GIsGuarded = 0;
		GLog = &Log;
		Error.HandleError();
	}

	appExit();
}