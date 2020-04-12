/*=============================================================================
	FFeedbackContextWindows.h: Unreal Windows user interface interaction.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	FFeedbackContextWindows.
-----------------------------------------------------------------------------*/

#include "UnMsg.h"

//
// Feedback context.
//
class FFeedbackContextWindows : public FFeedbackContext
{
	/* Context information for warning and error messages */
	FContextSupplier*	Context;

public:
	// Variables.
	INT					SlowTaskCount, WarningCount, ErrorCount;
	DWORD				hWndProgressBar, hWndProgressText, hWndProgressDlg, hWndMapCheckDlg;

	// Constructor.
	FFeedbackContextWindows()
		: FFeedbackContext()
		, SlowTaskCount( 0 )
		, hWndProgressBar( 0 )
		, hWndProgressText( 0 )
		, hWndProgressDlg( 0 )
		, hWndMapCheckDlg( 0 )
	{}
	void Serialize( const TCHAR* V, EName Event )
	{
		TCHAR Temp[1024]=TEXT("");

		if( Event==NAME_UserPrompt && (GIsClient || GIsEditor) )
		{
			::MessageBox( NULL, V, (TCHAR *)*LocalizeError("Warning",TEXT("Core")), MB_OK|MB_TASKMODAL );
		}
		else if( Event==NAME_Title )
		{
			return;
		}
		else if( Event==NAME_Heading )
		{
			appSprintf( Temp, TEXT("--------------------%s--------------------"), (TCHAR*)V );
			V = Temp;
		}
		else if( Event==NAME_SubHeading )
		{
			appSprintf( Temp, TEXT("%s..."), (TCHAR*)V );
			V = Temp;
		}
		else if( Event==NAME_Error || Event==NAME_Warning || Event==NAME_ExecWarning || Event==NAME_ScriptWarning )
		{
			if( Context )
			{
				appSprintf( Temp, TEXT("%s : %s, %s"), *Context->GetContext(), *FName(Event), (TCHAR*)V );
				V = Temp;
			}
			if(Event == NAME_Error)
				ErrorCount++;
			else
				WarningCount++;
		}

		GLog->Serialize( V, Event );
	}
	UBOOL YesNof( const TCHAR* Fmt, ...)
	{
		TCHAR TempStr[4096];
		GET_VARARGS( TempStr, ARRAY_COUNT(TempStr), Fmt );
		if( GIsClient || GIsEditor )
			return( ::MessageBox( NULL, TempStr, (TCHAR *)*LocalizeError("Question",TEXT("Core")), MB_YESNO|MB_TASKMODAL ) == IDYES);
		else
			return 0;
	}
	void MapCheck_Show()
	{

	}
	// This is the same as MapCheck_Show, except it won't display the error box if there are no errors in it.
	void MapCheck_ShowConditionally()
	{

	}
	void MapCheck_Hide()
	{

	}
	void MapCheck_Clear()
	{

	}
	void MapCheck_Add( INT InType, void* InActor, const TCHAR* InMessage )
	{

	}
	void BeginSlowTask( const TCHAR* Task, UBOOL StatusWindow, UBOOL )
	{
		::ShowWindow( (HWND)hWndProgressDlg, SW_SHOW );
		if( hWndProgressBar && hWndProgressText )
		{
			{	// flush all messages
				MSG mfm_msg;
				while(::PeekMessage(&mfm_msg, (HWND)hWndProgressDlg, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&mfm_msg);
					DispatchMessage(&mfm_msg);
				}
			}
		}
		GIsSlowTask = ++SlowTaskCount>0;
	}
	void EndSlowTask()
	{
		check(SlowTaskCount>0);
		GIsSlowTask = --SlowTaskCount>0;
		if( !GIsSlowTask )
			::ShowWindow( (HWND)hWndProgressDlg, SW_HIDE );
	}
	UBOOL VARARGS StatusUpdatef( INT Numerator, INT Denominator, const TCHAR* Fmt, ... )
	{
		TCHAR TempStr[4096];
		GET_VARARGS( TempStr, ARRAY_COUNT(TempStr), Fmt );
		if( GIsSlowTask && hWndProgressBar && hWndProgressText )
		{
			{	// flush all messages
				MSG mfm_msg;
				while(::PeekMessage(&mfm_msg, (HWND)hWndProgressDlg, 0, 0, PM_REMOVE)) {
					TranslateMessage(&mfm_msg);
					DispatchMessage(&mfm_msg);
				}
			}
		}
		return 1;
	}
	void SetContext( FContextSupplier* InSupplier )
	{
		Context = InSupplier;
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
