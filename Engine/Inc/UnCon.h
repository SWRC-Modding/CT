/*=============================================================================
	UnCon.h: UConsole game-specific definition
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Contains routines for: Messages, menus, status bar
=============================================================================*/

/*------------------------------------------------------------------------------
	UConsole definition.
------------------------------------------------------------------------------*/

//
// Viewport console.
//
struct UConsole_eventMessage_Parms
{
	class APlayerReplicationInfo* PRI;
	FString S;
	class AZoneInfo* PZone;
	FName N;
};
struct UConsole_eventConnectFailure_Parms
{
    FString FailCode;
    FString URL;
};
class ENGINE_API UConsole : public UObject, public FOutputDevice
{
	DECLARE_CLASS(UConsole,UObject,CLASS_Transient,Engine)

	// Constructor.
	UConsole();

	// UConsole interface.
	virtual void _Init( UViewport* Viewport );
	virtual void PreRender( FSceneNode* Frame );
	virtual void PostRender( FSceneNode* Frame );
	virtual void Serialize( const TCHAR* Data, EName MsgType );
	virtual UBOOL GetDrawWorld();

	// Natives.
	DECLARE_FUNCTION(execConsoleCommand);
	DECLARE_FUNCTION(execSaveTimeDemo);

	// Script events.
    void eventMessage(class APlayerReplicationInfo* PRI, const FString& S, class AZoneInfo* PZone, FName Name)
    {
		UConsole_eventMessage_Parms Parms;
		Parms.PRI=PRI;
        Parms.S=S;
		Parms.PZone=PZone;
		Parms.N=Name;
        ProcessEvent(FindFunctionChecked(NAME_Message),&Parms);
    }
    void eventTick(FLOAT DeltaTime)
    {
        struct {FLOAT DeltaTime; } Parms;
        Parms.DeltaTime=DeltaTime;
        ProcessEvent(FindFunctionChecked(ENGINE_Tick),&Parms);
    }
    void eventVideoChange()
    {
        ProcessEvent(FindFunctionChecked(NAME_VideoChange),NULL);
    }
    void eventPostRender(class UCanvas* C)
    {
        struct {class UCanvas* C; } Parms;
        Parms.C=C;
        ProcessEvent(FindFunctionChecked(ENGINE_PostRender),&Parms);
    }
    void eventPreRender(class UCanvas* C)
    {
        struct {class UCanvas* C; } Parms;
        Parms.C=C;
        ProcessEvent(FindFunctionChecked(ENGINE_PreRender),&Parms);
    }
    DWORD eventKeyType(BYTE Key)
    {
        struct {BYTE Key; DWORD ReturnValue; } Parms;
        Parms.Key=Key;
        Parms.ReturnValue=0;
        ProcessEvent(FindFunctionChecked(NAME_KeyType),&Parms);
        return Parms.ReturnValue;
    }
    DWORD eventKeyEvent(BYTE Key, BYTE Action, FLOAT Delta)
    {
        struct {BYTE Key; BYTE Action; FLOAT Delta; DWORD ReturnValue; } Parms;
        Parms.Key=Key;
        Parms.Action=Action;
        Parms.Delta=Delta;
        Parms.ReturnValue=0;
        ProcessEvent(FindFunctionChecked(NAME_KeyEvent),&Parms);
        return Parms.ReturnValue;
    }
    void eventNotifyLevelChange()
    {
        ProcessEvent(FindFunctionChecked(NAME_NotifyLevelChange),NULL);
    }
    void eventConnectFailure(const FString& FailCode, const FString& URL)
    {
        UConsole_eventConnectFailure_Parms Parms;
        Parms.FailCode=FailCode;
        Parms.URL=URL;
        ProcessEvent(FindFunctionChecked(NAME_ConnectFailure),&Parms);
    }
	UBOOL IsTimeDemo()
	{
		return bTimeDemo;
	}
private:
	// Constants.
	enum {MAX_BORDER     = 6};
	enum {MAX_LINES		 = 64};
	enum {MAX_HISTORY	 = 16};

	// Variables.
    class UViewport* Viewport;
    INT HistoryTop;
    INT HistoryBot;
    INT HistoryCur;
    FStringNoInit TypedStr;
    FStringNoInit History[16];
    INT Scrollback;
    INT numLines;
    INT TopLine;
    INT TextLines;
    FLOAT MsgTime;
    FLOAT MsgTickTime;
    FStringNoInit MsgText[64];
    FName MsgType[64];
    class APlayerReplicationInfo* MsgPlayer[64];
    FLOAT MsgTick[64];
    INT BorderSize;
    INT ConsoleLines;
    INT BorderLines;
    INT BorderPixels;
    FLOAT ConsolePos;
    FLOAT ConsoleDest;
    FLOAT FrameX;
    FLOAT FrameY;
    class UTexture* ConBackground;
    class UTexture* Border;
    BITFIELD bNoStuff:1;
    BITFIELD bTyping:1;
    BITFIELD bNoDrawWorld:1;
    BITFIELD bTimeDemo:1;
    BITFIELD bStartTimeDemo:1;
    BITFIELD bRestartTimeDemo:1;
	BITFIELD bSaveTimeDemoToFile:1;
    FLOAT StartTime;
    FLOAT ExtraTime;
    FLOAT LastFrameTime;
    FLOAT LastSecondStartTime;
    INT FrameCount;
    INT LastSecondFrameCount;
    FLOAT MinFPS;
    FLOAT MaxFPS;
    FLOAT LastSecFPS;
	class UFont* Font;
    FStringNoInit LoadingMessage;
    FStringNoInit SavingMessage;
    FStringNoInit ConnectingMessage;
    FStringNoInit PausedMessage;
    FStringNoInit PrecachingMessage;
    FStringNoInit FrameRateText;
    FStringNoInit AvgText;
    FStringNoInit LastSecText;
    FStringNoInit MinText;
    FStringNoInit MaxText;
    FStringNoInit fpsText;
    FStringNoInit SecondsText;
    FStringNoInit FramesText;
};

/*------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------*/

