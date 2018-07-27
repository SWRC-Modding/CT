// ====================================================================
// (C) 2002, Epic Games
// ====================================================================

class ExtendedConsole extends Console;

#exec TEXTURE IMPORT NAME=MenuWhite FILE=Textures\White.tga MIPS=0
#exec TEXTURE IMPORT NAME=MenuBlack FILE=Textures\Black.tga MIPS=0
#exec TEXTURE IMPORT NAME=MenuGray  FILE=Textures\Gray.tga MIPS=0

#exec OBJ LOAD FILE=GUIContent.utx

// Visible Console stuff

var globalconfig int MaxScrollbackSize;

var array<string> Scrollback;
var int SBHead, SBPos;	// Where in the scrollback buffer are we
var bool bCtrl;
var bool bConsoleHotKey;

var float   ConsoleSoundVol;

var localized string AddedCurrentHead;
var localized string AddedCurrentTail;


var config EInputKey	LetterKeys[10];
var        EInputKey	NumberKeys[10];

var config bool bSpeechMenuUseLetters;
var config bool bSpeechMenuUseMouseWheel;

var int HighlightRow;

////// Speech Menu
var enum ESpeechMenuState
{
	SMS_Main,
	SMS_Ack,
	SMS_FriendFire,
	SMS_Order,
	SMS_Taunt,
	SMS_PlayerSelect,
} SMState;

var float SMLineSpace;
var float SMMargin, SMTab;
var int SMOffset;
var config float SMOriginX;
var config float SMOriginY;

var localized string  SMStateName[6];
var localized string  SMAllString;
var localized string  SMMoreString;

var sound	SMOpenSound;
var sound   SMAcceptSound;
var sound   SMDenySound;

var string	SMNameArray[48];
var int		SMIndexArray[48];
var int		SMArraySize;

var name SMType;
var int  SMIndex;
////// End Speech Menu

struct StoredPassword
{
	var config string	Server,
						Password;
};

var config array<StoredPassword>	SavedPasswords;
var config string					PasswordPromptMenu;
var string							LastConnectedServer,
									LastURL;


struct ChatStruct
{
	var string	Message;
    var int		Team;
};

var string ChatString;

event ConnectFailure(string FailCode,string URL)
{
	local string			Server;
	local int				Index;

	LastURL = URL;
	Server = Left(URL,InStr(URL,"/"));

	if(FailCode == "NEEDPW")
	{
		for(Index = 0;Index < SavedPasswords.Length;Index++)
		{
			if(SavedPasswords[Index].Server == Server)
			{
				ViewportOwner.Actor.ClearProgressMessages();
				ViewportOwner.Actor.ClientTravel(URL$"?password="$SavedPasswords[Index].Password,TRAVEL_Absolute,false);
				return;
			}
		}

		LastConnectedServer = Server;
		ViewportOwner.Actor.ClientOpenMenu(
			PasswordPromptMenu,
			false,
			URL,
			""
			);
		return;
	}
	else if(FailCode == "WRONGPW")
	{
		ViewportOwner.Actor.ClearProgressMessages();

		for(Index = 0;Index < SavedPasswords.Length;Index++)
		{
			if(SavedPasswords[Index].Server == Server)
			{
				SavedPasswords.Remove(Index,1);
				SaveConfig();
			}
		}

		LastConnectedServer = Server;
		ViewportOwner.Actor.ClientOpenMenu(
			PasswordPromptMenu,
			false,
			URL,
			""
			);
		return;
	}
}


event NotifyLevelChange()
{
	Super.NotifyLevelChange();
//	GUIController(ViewportOwner.GUIController).CloseAll(false);
}


exec function CLS()
{
	SBHead = 0;
	ScrollBack.Remove(0,ScrollBack.Length);
}

function PostRender( canvas Canvas );	// Subclassed in state

event Message( coerce string Msg, float MsgLife)
{
	if (ScrollBack.Length==MaxScrollBackSize)	// if full, Remove Entry 0
	{
		ScrollBack.Remove(0,1);
		SBHead = MaxScrollBackSize-1;
	}
	else
		SBHead++;

	ScrollBack.Length = ScrollBack.Length + 1;

	Scrollback[SBHead] = Msg;
	Super.Message(Msg,MsgLife);
}

event bool KeyEvent( EInputKey Key, EInputAction Action, FLOAT Delta )
{
	if ( CurMenu != None )
		return Super.KeyEvent(Key,Action,Delta);

	if (Key==ConsoleKey)
	{
		if(Action==IST_Release)
			ConsoleOpen();
		return true;
	}

    return Super.KeyEvent(Key,Action,Delta);
}


function PlayConsoleSound(Sound S)
{
	if (ViewportOwner == None || ViewportOwner.Actor == None || ViewportOwner.Actor.Pawn == None ||
		ViewportOwner.Actor.Pawn.Controller == None)
		return;

	if ( ViewportOwner.Actor.Pawn.Controller.IsA('PlayerController') )
		PlayerController(ViewportOwner.Actor.Pawn.Controller).ClientPlaySoundLocally(S);
}

exec function Talk()
{	
	if( ViewportOwner == None || ViewportOwner.Actor.Level.NetMode != NM_StandAlone )
	{
		GotoState( 'Chat' );
	}
}

exec function TeamTalk()
{
	if( ViewportOwner == None || ViewportOwner.Actor.Level.NetMode != NM_StandAlone )
	{
		GotoState( 'TeamChat' );
	}
}


//-----------------------------------------------------------------------------
// State used while typing a command on the console.

event NativeConsoleOpen()
{
	ConsoleOpen();
}

exec function ConsoleOpen()
{
	local PlayerController PC;

	if ( CurMenu != None )
		return;

	PC = ViewportOwner.Actor;
	//Log( "ReleaseAllButtons() : "$PC );
	PC.ReleaseAllButtons();
		
	TypedStr = "";
	GotoState('ConsoleVisible');
}

exec function ConsoleClose()
{
	TypedStr="";
    if( GetStateName() == 'ConsoleVisible' )
	{
        GotoState( '' );
	}
}

exec function ConsoleToggle()
{
    if( GetStateName() == 'ConsoleVisible' )
        ConsoleClose();
    else
        ConsoleOpen();
}

state ConsoleVisible
{
	function bool KeyType( EInputKey Key, optional string Unicode )
	{
		local PlayerController PC;

		if (bIgnoreKeys || bConsoleHotKey)
			return true;

		if (ViewportOwner != none)
			PC = ViewportOwner.Actor;

		if (bCtrl && PC != none)
		{
			if (Key == 3) //copy
			{
				PC.CopyToClipboard(TypedStr);
				return true;
			}
			else if (Key == 22) //paste
			{
				TypedStr = TypedStr$PC.PasteFromClipboard();
				return true;
			}
			else if (Key == 24) // cut
			{
				PC.CopyToClipboard(TypedStr);
				TypedStr="";
				return true;
			}
		}

		if( Key>=0x20 )
		{
			if( Unicode != "" )
				TypedStr = TypedStr $ Unicode;
			else
				TypedStr = TypedStr $ Chr(Key);
            return( true );
		}

		return( true );
	}

	function bool KeyEvent( EInputKey Key, EInputAction Action, FLOAT Delta )
	{
		local string Temp;

		if ( Key==IK_Ctrl )
		{
			if (Action == IST_Press)
				bCtrl = true;
			else if (Action == IST_Release)
				bCtrl = false;
		}

		if (Action == IST_PRess)
			bIgnoreKeys = false;

		if (Key == ConsoleKey)
		{
			if(Action == IST_Press)
				bConsoleHotKey = true;
			else if(Action == IST_Release && bConsoleHotKey)
				ConsoleClose();
			return true;
		}

		if (Key==IK_Escape)
		{
			if (Action==IST_Release)
			{
				if (TypedStr!="")
				{
					TypedStr="";
					HistoryCur = HistoryTop;
				}
				else
	                ConsoleClose();
			}
			return true;
		}

		if ( Action != IST_Press )
            return( true );

		if ( Key==IK_Enter )
		{
			if ( TypedStr!="" )
			{
				// Print to console.

				History[HistoryTop] = TypedStr;
                HistoryTop = (HistoryTop+1) % ArrayCount(History);

				if ( ( HistoryBot == -1) || ( HistoryBot == HistoryTop ) )
                    HistoryBot = (HistoryBot+1) % ArrayCount(History);

				HistoryCur = HistoryTop;

				// Make a local copy of the string.
				Temp=TypedStr;
				TypedStr="";

				if( !ConsoleCommand( Temp ) )
					Message( Localize("Errors","Exec","Core"), 6.0 );

				Message( "", 6.0 );
			}

            return( true );
		}

		if ( Key==IK_Up )
		{
			if ( HistoryBot >= 0 )
			{
				if (HistoryCur == HistoryBot)
					HistoryCur = HistoryTop;
				else
				{
					HistoryCur--;
					if (HistoryCur<0)
                        HistoryCur = ArrayCount(History)-1;
				}

				TypedStr = History[HistoryCur];
			}
            return( true );
		}

		if ( Key==IK_Down )
		{
			if ( HistoryBot >= 0 )
			{
				if (HistoryCur == HistoryTop)
					HistoryCur = HistoryBot;
				else
                    HistoryCur = (HistoryCur+1) % ArrayCount(History);

				TypedStr = History[HistoryCur];
			}
		}
		else if ( Key==IK_Backspace || Key==IK_Left )
		{
			if( Len(TypedStr)>0 )
				TypedStr = Left(TypedStr,Len(TypedStr)-1);
            return( true );
		}
		else if ( Key==IK_PageUp || key==IK_MouseWheelUp )
		{
			if (SBPos<ScrollBack.Length-1)
			{
				if (bCtrl)
					SBPos+=5;
				else
					SBPos++;

				if (SBPos>=ScrollBack.Length)
				  SBPos = ScrollBack.Length-1;
			}

			return true;
		}
		else if ( Key==IK_PageDown || key==IK_MouseWheelDown)
		{
			if (SBPos>0)
			{
				if (bCtrl)
					SBPos-=5;
				else
					SBPos--;

				if (SBPos<0)
					SBPos = 0;
			}
		}

        return( true );
	}

    function BeginState()
	{
		SBPos = 0;
        bVisible= true;
		bIgnoreKeys = true;
		bConsoleHotKey = false;
        HistoryCur = HistoryTop;
		bCtrl = false;
    }
    function EndState()
    {
        bVisible = false;
		bCtrl = false;
		bConsoleHotKey = false;
    }

	function PostRender( canvas Canvas )
	{

		local float fw,fh;
		local float yclip,y;
		local int idx;
		local float XL, YL;

//		Canvas.Font = class'Hud'.static.GetConsoleFont(Canvas);
		yclip = canvas.ClipY*0.5;
		Canvas.StrLen("X",fw,fh);

		Canvas.SetPos(0,0);
		Canvas.SetDrawColor(255,255,255,200);
		Canvas.Style=4;
		Canvas.DrawTileStretched(material'ConsoleBack',Canvas.ClipX,yClip);
		Canvas.Style=1;

		Canvas.SetPos(0,yclip-1);
		Canvas.SetDrawColor(255,255,255,255);
		Canvas.DrawTile(texture 'GUIContent.Menu.BorderBoxA',Canvas.ClipX,2,0,0,64,2);

		Canvas.SetDrawColor(255,255,255,255);

		Canvas.SetPos(0,yclip-5-fh);
		Canvas.DrawText("(>"@TypedStr$"_");

		idx = SBHead - SBPos;
		y = yClip-y-5-(fh*2);

		if (ScrollBack.Length==0)
			return;

		Canvas.SetDrawColor(255,255,255,255);
		y += fh;
		while (y>fh && idx>=0)
		{
			Canvas.StrLen(Scrollback[idx], XL, YL);
			y -= YL;
			if ( y > fh )
			{
				Canvas.SetPos(0,y);
				Canvas.DrawText(Scrollback[idx],false);
			}
			idx--;
		}
	}
}

////// Speech Menu
exec function SpeechMenuToggle()
{
    if ( GetStateName() == 'SpeechMenuVisible' )
	{
		GotoState('');
		return;
	}

	// Dont let spectators use the speech menu
	if (ViewportOwner.Actor.PlayerReplicationInfo.bOnlySpectator)
		return;

	// Don't show speech menu if no voice pack type
    if ( ViewportOwner.Actor.PlayerReplicationInfo.VoiceType == None )
	{
		log("no PlayerReplicationInfo voicetype for playerid " $ViewportOwner.Actor.PlayerReplicationInfo.PlayerID);
		return;
	}

	GotoState('SpeechMenuVisible');
}

exec function SpeechMenuClose()
{
    if( GetStateName() == 'SpeechMenuVisible' )
	{
		GotoState('');
		return;
	}
}


state SpeechMenuVisible
{
	function bool KeyType( EInputKey Key, optional string Unicode )
	{
		if (bIgnoreKeys)
			return true;
		return false;
	}

	function class<VoicePack> GetVoiceClass()
	{
		if (ViewportOwner == None || ViewportOwner.Actor == None || ViewportOwner.Actor.PlayerReplicationInfo == None)
			return None;
		return (ViewportOwner.Actor.PlayerReplicationInfo.VoiceType);
	}

	// JTODO: Bubble sort. Sorry. But I already wrote the GUIList sort today and its late.
	function SortSMArray()
	{
		local int i,j, tmpInt;
		local string tmpString;

		for(i=0; i<SMArraySize-1; i++)
		{
			for(j=i+1; j<SMArraySize; j++)
			{
				if (SMNameArray[i] > SMNameArray[j])
				{
					tmpString = SMNameArray[i];
					SMNameArray[i] = SMNameArray[j];
					SMNameArray[j] = tmpString;

					tmpInt = SMIndexArray[i];
					SMIndexArray[i] = SMIndexArray[j];
					SMIndexArray[j] = tmpInt;
				}
			}
		}
	}

	// Rebuild the array of options based on the state we are now in.
	function RebuildSMArray()
	{
		local int i;
		local class<VoicePack> tvp;
		local GameReplicationInfo GRI;
		local PlayerReplicationInfo MyPRI;

		SMArraySize = 0;
		SMOffset=0;

		tvp = GetVoiceClass();
		if (tvp == None)
			return;

		if (SMState == SMS_Main)
		{
			for(i=1; i<5; i++)
			{
				SMNameArray[SMArraySize] = SMStateName[i];
				SMIndexArray[SMArraySize] = i;
				SMArraySize++;
			}
		}
		else if (SMState == SMS_Ack)
		{
			for(i=0; i<tvp.Default.numAcks; i++)
			{
				SMNameArray[SMArraySize] = tvp.Default.AckString[i];
				SMIndexArray[SMArraySize] = i;
				SMArraySize++;
			}

			SortSMArray();
		}
		else if (SMState == SMS_Taunt)
		{
			for(i=0; i<tvp.Default.numTaunts; i++)
			{
				SMNameArray[SMArraySize] = tvp.Default.TauntString[i];
				SMIndexArray[SMArraySize] = i;
				SMArraySize++;
			}

			SortSMArray();
		}
		else if (SMState == SMS_FriendFire)
		{
			for(i=0; i<tvp.Default.numFFires; i++)
			{
				SMNameArray[SMArraySize] = tvp.Default.FFireString[i];
				SMIndexArray[SMArraySize] = i;
				SMArraySize++;
			}
			SortSMArray();
		}
		else if (SMState == SMS_Order)
		{
			for(i=0; i<tvp.Default.numOrders; i++)
			{
				SMNameArray[SMArraySize] = tvp.Default.OrderString[i];
				SMIndexArray[SMArraySize] = i;
				SMArraySize++;
			}
			SortSMArray();
		}
		else if(SMState == SMS_PlayerSelect)
		{
			if(ViewportOwner == None || ViewportOwner.Actor == None || ViewportOwner.Actor.PlayerReplicationInfo == None)
				return;

			GRI = ViewportOwner.Actor.GameReplicationInfo;
			MyPRI = ViewportOwner.Actor.PlayerReplicationInfo;

			// First entry is to send to 'all'
			SMNameArray[SMArraySize] = SMAllString;
			SMArraySize++;

			for(i=0; i<GRI.PRIArray.Length; i++)
			{
				// Dont put player on list if myself, not on a team, on the same team, or a spectator.
				if( GRI.PRIArray[i].Team == None || MyPRI.Team == None )
					continue;

				if( GRI.PRIArray[i].Team.TeamIndex != MyPRI.Team.TeamIndex )
					continue;

				if( GRI.PRIArray[i].TeamId == MyPRI.TeamId )
					continue;

				if( GRI.PRIArray[i].bOnlySpectator )
					continue;

				SMNameArray[SMArraySize] = GRI.PRIArray[i].GetPlayerName();
				SMArraySize++;
				// Dont need a number- we use the name direct
			}
		}
	}

	//////////////////////////////////////////////

	function EnterState(ESpeechMenuState newState, optional bool bNoSound)
	{
		SMState = newState;
		RebuildSMArray();

		if (!bNoSound)
			PlayConsoleSound(SMAcceptSound);
	}

	function LeaveState() // Go up a level
	{
		PlayConsoleSound(SMDenySound);

		if (SMState == SMS_Main)
			GotoState('');
		else if (SMState == SMS_PlayerSelect)
			EnterState(SMS_Order, true);
		else
			EnterState(SMS_Main, true);
	}
	// // // // // //
	function HandleInput(int keyIn)
	{
		local int selectIndex;

		// GO BACK - previous state (might back out of menu);
		if (keyIn == -1)
		{
			LeaveState();
			HighlightRow = 0;
			return;
		}

		// TOP LEVEL - we just enter a new state
		if (SMState == SMS_Main)
		{
			switch(keyIn)
			{
				case 1: SMType = 'ACK';
					EnterState(SMS_Ack);
					break;
				case 2: SMType = 'FRIENDLYFIRE';
					EnterState(SMS_FriendFire);
					break;
				case 3: SMType = 'ORDER';
					EnterState(SMS_Order);
					break;
				case 4: SMType = 'TAUNT';
					EnterState(SMS_Taunt);
					break;
			}
			return;
		}

		// Next page on the same level
		if (keyIn == 0 )
		{
			// Check there is a next page!
			if (SMArraySize - SMOffset > 9)
				SMOffset += 9;

			return;
		}

		// Previous page on the same level
		if (keyIn == -2)
		{
			SMOffset = Max(SMOffset - 9, 0);
			return;
		}

		// Otherwise - we have selected something!
		selectIndex = SMOffset + keyIn - 1;
		if (selectIndex < 0 || selectIndex >= SMArraySize) // discard - out of range selections.
			return;

		if (SMState == SMS_Order)
		{
			SMIndex = SMIndexArray[selectIndex];
			EnterState(SMS_PlayerSelect);
		}
		else if (SMState == SMS_PlayerSelect)
		{
			if (SMNameArray[selectIndex] == SMAllString)
				ViewportOwner.Actor.Speech(SMType, SMIndex, "");
			else
				ViewportOwner.Actor.Speech(SMType, SMIndex, SMNameArray[selectIndex]);

			PlayConsoleSound(SMAcceptSound);

			GotoState(''); // Close menu after message
		}
		else
		{
			ViewportOwner.Actor.Speech(SMType, SMIndexArray[selectIndex], "");
			PlayConsoleSound(SMAcceptSound);
			GotoState('');
		}
	}

	//////////////////////////////////////////////

	function string NumberToString(int num)
	{
		local EInputKey key;
		local string s;

		if (num < 0 || num > 9)
			return "";

		if (bSpeechMenuUseLetters)
			key = LetterKeys[num];
		else
			key = NumberKeys[num];

		s = ViewportOwner.Actor.ConsoleCommand( "LOCALIZEDKEYNAME"@string(int(key)) );
		return s;
	}

	function DrawNumbers( canvas Canvas, int NumNums, bool IncZero, bool sizing, out float XMax, out float YMax )
	{
		local int i;
		local float XPos, YPos;
		local float XL, YL;

		XPos = Canvas.ClipX * (SMOriginX+SMMargin);
		YPos = Canvas.ClipY * (SMOriginY+SMMargin);
		Canvas.SetDrawColor(128,255,128,255);

		for(i=0; i<NumNums; i++)
		{
			Canvas.SetPos(XPos, YPos);
			if (!sizing)
				Canvas.DrawText(NumberToString(i+1)$"-", false);
			else
			{
				Canvas.TextSize(NumberToString(i+1)$"-", XL, YL);
				XMax = Max(XMax, XPos + XL);
				YMax = Max(YMax, YPos + YL);
			}

			YPos += SMLineSpace;
		}

		if (IncZero)
		{
			Canvas.SetPos(XPos, YPos);

			if (!sizing)
				Canvas.DrawText(NumberToString(0)$"-", false);

			XPos += SMTab;
			Canvas.SetPos(XPos, YPos);

			if (!sizing)
				Canvas.DrawText(SMMoreString, false);
			else
			{
				Canvas.TextSize(SMMoreString, XL, YL);
				XMax = Max(XMax, XPos + XL);
				YMax = Max(YMax, YPos + YL);
			}
		}
	}

	function DrawCurrentArray( canvas Canvas, bool sizing, out float XMax, out float YMax )
	{
		local int i, stopAt;
		local float XPos, YPos;
		local float XL, YL;

		XPos = (Canvas.ClipX * (SMOriginX+SMMargin)) + SMTab;
		YPos = Canvas.ClipY * (SMOriginY+SMMargin);
		Canvas.SetDrawColor(255,255,255,255);

		stopAt = Min(SMOffset+9, SMArraySize);
		for(i=SMOffset; i<stopAt; i++)
		{
			Canvas.SetPos(XPos, YPos);
			if (!sizing)
				Canvas.DrawText(SMNameArray[i], false);
			else
			{
				Canvas.TextSize(SMNameArray[i], XL, YL);
				XMax = Max(XMax, XPos + XL);
				YMax = Max(YMax, YPos + YL);
			}

			YPos += SMLineSpace;
		}
	}

	//////////////////////////////////////////////

	function int KeyToNumber(EInputKey InKey)
	{
		local int i;

		for(i=0; i<10; i++)
		{
			if (bSpeechMenuUseLetters)
			{
				if (InKey == LetterKeys[i])
					return i;
			}
			else
			{
				if (InKey == NumberKeys[i])
					return i;
			}
		}

		return -1;
	}

	function bool KeyEvent( EInputKey Key, EInputAction Action, FLOAT Delta )
	{
		local int input, NumNums;

		NumNums = Min(SMArraySize - SMOffset, 10);

		// While speech menu is up, dont let user use console. Debateable.
		//if ( KeyIsBoundTo( Key, "ConsoleToggle" ) )
		//	return true;
		//if ( KeyIsBoundTo( Key, "Type" ) )
		//	return true;

		if (Action == IST_Press)
			bIgnoreKeys=false;

		if ( Action != IST_Press )
			return false;

		if ( Key==IK_Escape)
		{
			HandleInput(-1);
			return true ;
		}

		// If 'letters' mode is on, convert input
		input = KeyToNumber(Key);
		if (input != -1)
		{
			HandleInput(input);
			return true;
		}

		// Keys below are only used if bSpeechMenuUseMouseWheel is true
		if (!bSpeechMenuUseMouseWheel)
			return false;

		if ( Key==IK_MouseWheelUp )
		{
			// If moving up on the top row, and there is a previous page
			if (HighlightRow == 0 && SMOffset > 0)
			{
				HandleInput(-2);
				HighlightRow=9;
			}
			else
			{
				HighlightRow = Max(HighlightRow - 1, 0);
			}

			return true;
		}
		if ( Key==IK_MouseWheelDown )
		{
			// If moving down on the bottom row (the 'MORE' row), act as if we hit it, and move highlight to top.
			if (HighlightRow == 9)
			{
				HandleInput(0);
				HighlightRow=0;
			}
			else
			{
				HighlightRow = Min(HighlightRow + 1, NumNums - 1);
			}

			return true;
		}
		else if ( Key==IK_MiddleMouse )
		{

			input = HighlightRow + 1;
			if (input == 10)
				input = 0;

			HandleInput(input);
			HighlightRow=0;
			return true;
		}

		return false;
	}

	function PostRender( canvas Canvas )
	{
		local float XL, YL;
		local int SelLeft, i;
		local float XMax, YMax;

		Canvas.Font = class'HUD'.static.GetSmallFontFor(Canvas);

		// Figure out max key name size
		XMax = 0;
		YMax = 0;
		for(i=0; i<10; i++)
		{
			Canvas.TextSize(NumberToString(i)$"- ", XL, YL);
			XMax = Max(XMax, XL);
			YMax = Max(YMax, YL);
		}
		SMLineSpace = YMax * 1.1;
		SMTab = XMax;

		SelLeft = SMArraySize - SMOffset;

		// First we figure out how big the bounding box needs to be
		XMax = 0;
		YMax = 0;
		DrawNumbers( canvas, Min(SelLeft, 9), SelLeft > 9, true, XMax, YMax);
		DrawCurrentArray( canvas, true, XMax, YMax);
		Canvas.TextSize(SMStateName[SMState], XL, YL);
		XMax = Max(XMax, Canvas.ClipX*(SMOriginX+SMMargin) + XL);
		YMax = Max(YMax, (Canvas.ClipY*SMOriginY) - (1.2*SMLineSpace) + YL);
		// XMax, YMax now contain to maximum bottom-right corner we drew to.

		// Then draw the box
		XMax -= Canvas.ClipX * SMOriginX;
		YMax -= Canvas.ClipY * SMOriginY;
		Canvas.SetDrawColor(255,255,255,255);
		Canvas.SetPos(Canvas.ClipX * SMOriginX, Canvas.ClipY * SMOriginY);
		Canvas.DrawTileStretched(texture 'GUIContent.Menu.BorderBoxD', XMax + (SMMargin*Canvas.ClipX), YMax + (SMMargin*Canvas.ClipY));

		// Draw highlight
		if (bSpeechMenuUseMouseWheel)
		{
			Canvas.SetDrawColor(255,255,255,128);
			Canvas.SetPos( Canvas.ClipX*SMOriginX, Canvas.ClipY*(SMOriginY+SMMargin) + ((HighlightRow - 0.1)*SMLineSpace) );
			Canvas.DrawTileStretched(texture 'GUIContent.Menu.BorderBoxD', XMax + (SMMargin*Canvas.ClipX), 1.1*SMLineSpace );
		}

		// Then actually draw the stuff
		DrawNumbers( canvas, Min(SelLeft, 9), SelLeft > 9, false, XMax, YMax);
		DrawCurrentArray( canvas, false, XMax, YMax);

		// Finally, draw a nice title bar.
		Canvas.SetDrawColor(255,255,255,255);
		Canvas.SetPos(Canvas.ClipX*SMOriginX, (Canvas.ClipY*SMOriginY) - (1.5*SMLineSpace));
		Canvas.DrawTileStretched(texture 'GUIContent.Menu.BorderBoxD', XMax + (SMMargin*Canvas.ClipX), (1.5*SMLineSpace));

		Canvas.SetDrawColor(255,255,128,255);
		Canvas.SetPos(Canvas.ClipX*(SMOriginX+SMMargin), (Canvas.ClipY*SMOriginY) - (1.2*SMLineSpace));
		Canvas.DrawText(SMStateName[SMState]);
	}

    function BeginState()
	{
        bVisible = true;
		bIgnoreKeys = true;
		bCtrl = false;
		HighlightRow=0;

		EnterState(SMS_Main, true);

		PlayConsoleSound(SMOpenSound);
	}

    function EndState()
    {
        bVisible = false;
		bCtrl = false;
    }

	// Close speech menu on level change
	event NotifyLevelChange()
	{
		Global.NotifyLevelChange();
		GotoState('');
	}
}
////// End Speech Menu

state Chat
{
	function string GetSayType()
	{
		return cSay;
	}
	
	function bool KeyType( EInputKey Key, optional string Unicode )
	{
		if (bIgnoreKeys)
			return true;
			
		if (Key>=0x20)
		{
			ChatString = ChatString $ Chr(Key);
			return true;
		}
			
		return false;
	}

	function bool KeyEvent( EInputKey Key, EInputAction Action, FLOAT Delta )
	{
		// While chat is up, dont let user use console. Debateable.
		//if ( KeyIsBoundTo( Key, "ConsoleToggle" ) )
		//	return true;
		//if ( KeyIsBoundTo( Key, "Type" ) )
		//	return true;

		if (Action == IST_Press)
			bIgnoreKeys=false;

		if ( Key==IK_Escape)
		{
			GotoState('');
			return true ;
		}
		/*else if( global.KeyEvent( Key, Action, Delta ) )
		{
			return true;
		}*/
		else if ( Action != IST_Press )
		{
			return false;
		}
		else if ( Key==IK_BackSpace || Key==IK_Left || Key==IK_Delete)
		{
			if ( Len(ChatString) > 0 )
				ChatString = Left(ChatString, Len(ChatString) - 1);
		}
		else if ( Key==IK_Enter )
		{
			if ( Len(ChatString) > 0 )
			{
				if ( GetSayType() == cSay )
				{
					ViewportOwner.Actor.ConsoleCommand("Say" $ " " $ ChatString);
				}
				else if ( GetSayType() == cTeamSay )
				{
					ViewportOwner.Actor.ConsoleCommand("TeamSay" $ " " $ ChatString);
				}
			}
			GotoState('');
		}

		return true;
	}

	function PostRender( canvas Canvas )
	{
		local float xl,yl;
		local string OutStr;
		local float XBoxOffset;
		
		XBoxOffset = 20;

		Canvas.Style = 1;
		
		Canvas.Font	 = class'Hud'.static.GetConsoleFont(Canvas);
		OutStr = GetSayType() $ "> " @ ChatString $ "_";
		Canvas.Strlen(OutStr,xl,yl);

		Canvas.SetDrawColor(255,255,255,255);
		Canvas.SetPos(0,Canvas.ClipY-6-yl-XBoxOffset);
		Canvas.DrawTileStretched( texture 'GUIContent.Menu.BorderBoxD', Canvas.ClipX, yl+6);

		Canvas.SetDrawColor(200,200,255,255);
		Canvas.SetPos(8,Canvas.ClipY-1-yl-XBoxOffset);
		Canvas.bCenter = False;
		Canvas.DrawText( OutStr, false );
	}

    function BeginState()
	{
        bVisible = true;
		bIgnoreKeys = true;
		bCtrl = false;
		HighlightRow=0;

		ChatString = "";
		
		PlayConsoleSound(SMOpenSound);
	}

    function EndState()
    {
        bVisible = false;
		bCtrl = false;
		ChatString = "";
    }

	// Close speech menu on level change
	event NotifyLevelChange()
	{
		ChatString = "";
		Global.NotifyLevelChange();
		GotoState('');
	}
}

state TeamChat extends Chat
{
	function string GetSayType()
	{
		return cTeamSay;
	}
}

////// End Chat


defaultproperties
{
     MaxScrollbackSize=128
     ConsoleSoundVol=0.3
     AddedCurrentHead="Added Server:"
     AddedCurrentTail="To Favorites!"
     LetterKeys(0)=IK_Q
     LetterKeys(1)=IK_W
     LetterKeys(2)=IK_E
     LetterKeys(3)=IK_R
     LetterKeys(4)=IK_A
     LetterKeys(5)=IK_S
     LetterKeys(6)=IK_D
     LetterKeys(7)=IK_F
     LetterKeys(8)=IK_Z
     LetterKeys(9)=IK_X
     NumberKeys(0)=IK_0
     NumberKeys(1)=IK_1
     NumberKeys(2)=IK_2
     NumberKeys(3)=IK_3
     NumberKeys(4)=IK_4
     NumberKeys(5)=IK_5
     NumberKeys(6)=IK_6
     NumberKeys(7)=IK_7
     NumberKeys(8)=IK_8
     NumberKeys(9)=IK_9
     bSpeechMenuUseMouseWheel=True
     SMMargin=0.015
     SMOriginX=0.01
     SMOriginY=0.3
     SMStateName(0)="Speech Menu"
     SMStateName(1)="Acknowledge"
     SMStateName(2)="Alert"
     SMStateName(3)="Command"
     SMStateName(4)="Taunt"
     SMStateName(5)="Player Select"
     SMAllString="[ALL]"
     SMMoreString="[MORE]"
}

