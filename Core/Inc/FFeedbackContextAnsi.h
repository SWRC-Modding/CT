/*=============================================================================
	FFeedbackContextAnsi.h: Unreal Ansi user interface interaction.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	FFeedbackContextAnsi.
-----------------------------------------------------------------------------*/
//
// Feedback context.
//
class FFeedbackContextAnsi : public FFeedbackContext{
public:
	//Variables
	INT SlowTaskCount;
	INT WarningCount, ErrorCount;
	FContextSupplier* Context;
	FOutputDevice* AuxOut;

	//Constructor
	FFeedbackContextAnsi() : SlowTaskCount(0),
							 WarningCount(0),
							 ErrorCount(0),
							 Context(NULL),
							 AuxOut(NULL){}

	//Functions

	void Serialize(const TCHAR* V, EName Event){
		guard(FFeedbackContextAnsi::Serialize);
		TCHAR Buffer[1024]= "";
		const TCHAR* Temp = V;

		if(Event == NAME_Title){
			SetConsoleTitle(V);

			return; //Prevents the server from spamming the player count to the log
		}else if(Event == NAME_Heading){
			appSprintf(Buffer, "\n--------------------%s--------------------", V);

			Temp = Buffer;
			V = Buffer; //So that the log file also contains the formatted string
		}else if(Event == NAME_Warning || Event == NAME_ExecWarning || Event == NAME_ScriptWarning){
			if(Context)
				appSprintf(Buffer, "%s: %s, %s", Context->GetContext(), *FName(Event), V);
			else
				appSprintf(Buffer, "%s: %s", *FName(Event), V);

			WarningCount++;

			Temp = Buffer;
		}else if(Event == NAME_Error || Event == NAME_Critical){
			if(Context)
				appSprintf(Buffer, "%s: %s, %s", Context->GetContext(), *FName(Event), V);
			else
				appSprintf(Buffer, "%s: %s", *FName(Event), V);

			ErrorCount++;

			Temp = Buffer;
		}

		puts(Temp);
		
		if(GLog != this)
			GLog->Log(Event, V);

		if(AuxOut)
			AuxOut->Log(Event, V);

		unguard;
	}

	void Flush(){
		fflush(stdout);
	}

	UBOOL YesNof(const TCHAR* Fmt, ...){
		TCHAR TempStr[4096];
		GET_VARARGS(TempStr, ARRAY_COUNT(TempStr), Fmt);

		guard(FFeedbackContextAnsi::YesNof);

		if((GIsClient || GIsEditor) && !ParseParam(appCmdLine(), "Silent")){
			printf("%s %s", TempStr, "(Y/N): ");

			INT Ch = getchar();

			getchar(); //Removing newline from input stream

			return Ch == 'Y' || Ch == 'y';
		}else{
			return 1;
		}

		unguard;
	}

	void BeginSlowTask(const TCHAR* Task, UBOOL StatusWindow){
		guard(FFeedbackContextAnsi::BeginSlowTask);

		GIsSlowTask = ++SlowTaskCount > 0;

		unguard;
	}

	void EndSlowTask(){
		guard(FFeedbackContextAnsi::EndSlowTask);

		check(SlowTaskCount>0);

		GIsSlowTask = --SlowTaskCount > 0;

		unguard;
	}

	UBOOL VARARGS StatusUpdatef( INT Numerator, INT Denominator, const TCHAR* Fmt, ... ){
		return 1;
	}

	void SetContext(FContextSupplier* InSupplier){
		Context = InSupplier;
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
