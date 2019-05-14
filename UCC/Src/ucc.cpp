/*
 * This is a custom UCC.exe for Star Wars Republic Commando since the game shipped without one.
 * Everything compiles fine with Visual Studio .NET 2003 which is being used to achieve maximum compatibility
 * since it was also used to compile RC
 * The following settings are required in order to compile everything without errors:
 * - Character Set = Not Set
 * - Struct Member Alignment = 4 Bytes
 * - Calling Convention = __fastcall
 */

#include "../../Core/Inc/Core.h"
#include "../../Core/Inc/FOutputDeviceFile.h"
#include "../../Core/Inc/FOutputDeviceWindowsError.h"
#include "../../Core/Inc/FFeedbackContextUCC.h"
#include "../../Core/Inc/FConfigCacheIni.h"

void UServerCommandletMain(); // Defined in ServerCommandlet.cpp

void ShowBanner(FOutputDevice& Out){
	Out.Log("=======================================");
	Out.Log("ucc.exe for Star Wars Republic Commando");
	Out.Log("made by Leon0628");
	Out.Log("=======================================");
	Out.Log("");
}

int __cdecl main(int argc, char** argv){
	int ExitCode = EXIT_SUCCESS;
	FOutputDeviceFile Log;
	FOutputDeviceWindowsError Error;
	FFeedbackContextUCC Warn;

	GIsStarted = 1;

	try{
		GIsGuarded = 1;

		FString CmdLine;

		for(int i = 1; i < argc; ++i)
			CmdLine += FString(argv[i]) + " ";

		appInit("SWRepublicCommando", *CmdLine, &Log, &Error, &Warn, FConfigCacheIni::Factory, 1);
		UObject::SetLanguage("int");

		if(argc > 1){
			// Initializing global state
			GIsUCC = GIsClient = GIsServer = GIsEditor = GIsScriptable = GLazyLoad = 1;

			FString ClassName = argv[1];
			TArray<FRegistryObjectInfo> List;

			UObject::GetRegistryObjects(List, UClass::StaticClass(), UCommandlet::StaticClass(), 0); // Loading list of commandlets declared in .int files

			for(int i = 0; i < List.Num(); ++i){ // Looking Token up in list and autocompleting class name if found
				FString FullName = List[i].Object;
				FString ShortName = FullName;

				while(ShortName.InStr(".") >= 0) // Removing package name so that only class name remains
					ShortName = ShortName.Mid(ShortName.InStr(".") + 1);

				if(ClassName == FullName || ClassName + "Commandlet" == FullName ||  // Checking against "PackageName.ClassName (+ Commandlet)"
				   ClassName == ShortName || ClassName + "Commandlet" == ShortName){ // Checking against "ClassName (+ Commandlet)"
					ClassName = List[i].Object;

					break;
				}
			}

			DWORD LoadFlags = LOAD_NoWarn | LOAD_Quiet;

			if(ClassName == "Editor.MakeCommandlet")
				LoadFlags |= LOAD_DisallowFiles;

			UClass* Class = LoadClass<UCommandlet>(NULL, *ClassName, NULL, LoadFlags, NULL);

			if(!Class) // If class failed to load appending "Commandlet" and trying again
				Class = LoadClass<UCommandlet>(NULL, *(ClassName + "Commandlet"), NULL, LoadFlags, NULL);

			if(Class){
				UCommandlet* Commandlet = ConstructObject<UCommandlet>(Class);
				UCommandlet* Default = Cast<UCommandlet>(Class->GetDefaultObject());

				if(Default->ShowBanner)
					ShowBanner(Warn);

				Warn.Logf("Executing %s", Class->GetFullName());
				Warn.Log("");

				GIsClient = Default->IsClient;
				GIsEditor = Default->IsEditor;
				GIsServer = Default->IsServer;
				GLazyLoad = Default->LazyLoad;

				// Contains only the command-line options that are passed to the commandlet
				FString CommandletCmdLine;

				for(int i = 2; i < argc; ++i)
					CommandletCmdLine += FString(argv[i]) + " ";

				Commandlet->InitExecution();
				Commandlet->ParseParms(*CommandletCmdLine);

				if(Default->LogToStdout){ // Redirecting commandlet output to console
					Warn.AuxOut = GLog;
					GLog = &Warn;
				}
				
				if(ClassName == "Engine.Server" || ClassName == "Engine.ServerCommandlet")
					UServerCommandletMain(); // The ServerCommandlet has a special Main function
				else
					ExitCode = Commandlet->Main(CommandletCmdLine);

				if(Default->ShowErrorCount){
					Warn.Log("");
					Warn.Logf("%s - %i error(s), %i warning(s)", Warn.ErrorCount == 0 ? "Success" : "Failure", Warn.ErrorCount, Warn.WarningCount);
				}

				if(Default->LogToStdout){
					Warn.AuxOut = NULL;
					GLog = &Log;
				}
			}else{
				ShowBanner(Warn);
				Warn.Logf("Commandlet %s not found", argv[1]);
			}
		}else{
			ShowBanner(Warn);
			Warn.Log("Usage:");
			Warn.Log("    ucc <command> <parameters>");
		}

		// This prevents an infinite loop during garbage collection when there are compile errors with ucc make
		if(Warn.ErrorCount == 0)
			appPreExit();
		else
			ExitCode = EXIT_FAILURE;

		GIsGuarded = 0;
	}catch(...){
		GIsGuarded = 0;
		GLog = &Log;
		ExitCode = EXIT_FAILURE;
		Error.HandleError();
	}

	appExit();

	return ExitCode;
}