/*=============================================================================
	FFeedbackContextCmd.h: Command line user interface interaction.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	FFeedbackContextCmd.
-----------------------------------------------------------------------------*/

#include <cstdio>
#include <Windows.h>

/*
 * Windows command prompt feedback context.
 */
class FFeedbackContextCmd : public FFeedbackContext{
public:
	// Variables
	INT SlowTaskCount;
	FContextSupplier* Context;
	FOutputDevice* AuxOut;

	// Constructor
	FFeedbackContextCmd() : SlowTaskCount(0),
							Context(NULL),
							AuxOut(NULL){}

	// Functions

	void Serialize(const TCHAR* V, EName Event){
		guard(FFeedbackContextCmd::Serialize);

		TCHAR Buffer[1024]= "";
		const TCHAR* Temp = V;

		if(Event == NAME_Title){
			SetConsoleTitle(V);

			return; // Prevents the server from spamming the player count to the log
		}else if(Event == NAME_Heading){
			appSprintf(Buffer, "--------------------%s--------------------", V);

			Temp = Buffer;
			V = Buffer; // So that the log file also contains the formatted string
		}else if(Event == NAME_Warning || Event == NAME_ExecWarning || Event == NAME_ScriptWarning || Event == NAME_Error || Event == NAME_Critical){
			if(Context)
				appSprintf(Buffer, "%s: %s, %s", *Context->GetContext(), *FName(Event), V);
			else
				appSprintf(Buffer, "%s: %s", *FName(Event), V);

			if(Event == NAME_Error || Event == NAME_Critical)
				++ErrorCount;
			else
				++WarningCount;

			Temp = Buffer;
		}

		std::puts(Temp);

		if(GLog != this)
			GLog->Log(Event, V);

		if(AuxOut)
			AuxOut->Log(Event, V);

		unguard;
	}

	void Flush(){
		std::fflush(stdout);
	}

	UBOOL YesNof(const TCHAR* Fmt, ...){
		TCHAR TempStr[4096];
		GET_VARARGS(TempStr, ARRAY_COUNT(TempStr), Fmt);

		guard(FFeedbackContextCmd::YesNof);

		if((GIsClient || GIsEditor) && !ParseParam(appCmdLine(), "Silent")){
			std::printf("%s %s", TempStr, "(Y/N): ");

			INT Ch = std::getchar();

			std::getchar(); // Removing newline from input stream

			return Ch == 'Y' || Ch == 'y';
		}else{
			return 1;
		}

		unguard;
	}

	void BeginSlowTask(const TCHAR* Task, UBOOL StatusWindow){
		guard(FFeedbackContextCmd::BeginSlowTask);

		GIsSlowTask = ++SlowTaskCount > 0;

		unguard;
	}

	void EndSlowTask(){
		guard(FFeedbackContextCmd::EndSlowTask);

		check(SlowTaskCount>0);

		GIsSlowTask = --SlowTaskCount > 0;

		unguard;
	}

	UBOOL VARARGS StatusUpdatef(INT Numerator, INT Denominator, const TCHAR* Fmt, ...){
		return 1;
	}

	void SetContext(FContextSupplier* InSupplier){
		Context = InSupplier;
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
