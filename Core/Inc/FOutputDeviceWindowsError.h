/*=============================================================================
	FOutputDeviceWindowsError.h: Windows error message outputter.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

//
//Handle a critical error.
//
class FOutputDeviceWindowsError : public FOutputDeviceError{
public:
	FOutputDeviceWindowsError() : ErrorPos(0),
	                              ErrorType(NAME_None){}

	void Serialize(const TCHAR* Msg, enum EName Event){
		INT Error = GetLastError();

		if(!GIsCriticalError){
			//First appError.
			GIsCriticalError = 1;
			ErrorType        = Event;
			debugf(NAME_Critical, "appError called:");
			debugf(NAME_Critical, "%s", Msg);

			//Windows error.
			debugf(NAME_Critical, "Windows GetLastError: %s (%i)", appGetSystemErrorMessage(Error), Error);

			//Shut down.
			UObject::StaticShutdownAfterError();
			appStrncpy(GErrorHist, Msg, ARRAY_COUNT(GErrorHist));
			appStrncat(GErrorHist, "\r\n\r\n", ARRAY_COUNT(GErrorHist));
			ErrorPos = appStrlen(GErrorHist);

			if(GIsGuarded){
				appStrncat(GErrorHist, GIsGuarded ? LocalizeError("History", "Core") : "History: ", ARRAY_COUNT(GErrorHist));
				appStrncat(GErrorHist, ": ", ARRAY_COUNT(GErrorHist));
			}else{
				HandleError();
			}
		}else{
			debugf(NAME_Critical, "Error reentered: %s", Msg);
		}

		//Propagate the error or exit.
		if(GIsGuarded)
			throw(1);
		else
			appRequestExit(1);
	}

	void HandleError(){
		try{
			GIsGuarded = 0;
			GIsRunning = 0;
			GIsCriticalError = 1;
			GLogHook = NULL;

			UObject::StaticShutdownAfterError();

			GErrorHist[ErrorType == NAME_FriendlyError ? ErrorPos : ARRAY_COUNT(GErrorHist) - 1] = 0;

			GWarn->Log(NAME_Critical, GErrorHist);
			GLog->Flush();

			if(GIsClient || GIsEditor)
				MessageBox(NULL, GErrorHist, "Critical Error", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		}catch(...){}
	}

private:
	INT   ErrorPos;
	EName ErrorType;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
