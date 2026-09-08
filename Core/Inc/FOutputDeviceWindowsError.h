/*=============================================================================
	FOutputDeviceWindowsError.h: Windows error message outputter.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef NO_CRASH_MESSAGEBOX
#include "Window.h"
#endif

//
//Handle a critical error.
//
class FOutputDeviceWindowsError : public FOutputDeviceError{
public:
	FOutputDeviceWindowsError()
		: ErrorPos(0)
		, ErrorType(NAME_None)
	{
	}

	void Serialize(const TCHAR* Msg, enum EName Event)
	{
		if(!GIsCriticalError)
		{
			//First appError.
			GIsCriticalError = 1;
			ErrorType        = Event;
			debugf(NAME_Critical, "appError called:");
			debugf(NAME_Critical, "%s", Msg);

			//Windows error.
			const INT Error = GetLastError();
			debugf(NAME_Critical, "Windows GetLastError: %s (%i)", appGetSystemErrorMessage(Error), Error);

			//Shut down.
			UObject::StaticShutdownAfterError();
			appStrncpy(GErrorHist, Msg, ARRAY_COUNT(GErrorHist));
			appStrncat(GErrorHist, "\r\n\r\n", ARRAY_COUNT(GErrorHist));
			ErrorPos = appStrlen(GErrorHist);

			if(GIsGuarded)
			{
				appStrncat(GErrorHist, "History: ", ARRAY_COUNT(GErrorHist));
				appStrncat(GErrorHist, ": ", ARRAY_COUNT(GErrorHist));
			}
			else
			{
				HandleError();
			}
		}
		else
		{
			debugf(NAME_Critical, "Error reentered: %s", Msg);
		}

		//Propagate the error or exit.
		if(GIsGuarded)
			throw(1);
		else
			appRequestExit(1);
	}

	void HandleError()
	{
		try
		{
			GIsGuarded       = 0;
			GIsRunning       = 0;
			GIsCriticalError = 1;
			GLogHook         = NULL;

			UObject::StaticShutdownAfterError();

			GErrorHist[ErrorType == NAME_FriendlyError ? ErrorPos : ARRAY_COUNT(GErrorHist) - 1] = 0;

			GWarn->Log(NAME_Critical, GErrorHist);
			GLog->Flush();

#ifndef NO_CRASH_MESSAGEBOX
			if(GIsClient || GIsEditor)
			{
				try
				{
					if(GConfig)
					{
						FString ext = UObject::GetLanguage();

						if(ext.Len() <= 0)
							ext = "int";

						GConfig->SetString("IDDIALOG_CrashBox", "BugReportURL", "https://github.com/SWRC-Modding/CT", *("Window." + ext));
					}

					WCrashBoxDialog CrashBox("Critical Error", GErrorHist);
					// work around WndProc error handling
					GIsCriticalError = 0;
					CrashBox.DoModal();
					GIsCriticalError = 1;
				}
				catch(...)
				{
					MessageBoxA(NULL, GErrorHist, "Critical Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
				}
			}
#endif
		}
		catch(...)
		{
		}
	}

private:
	INT   ErrorPos;
	EName ErrorType;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
