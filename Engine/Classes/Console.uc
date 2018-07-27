//=============================================================================
// Console - A quick little command line console that accepts most commands.

//=============================================================================
class Console extends Interaction;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

//#exec new TrueTypeFontFactory PACKAGE="Engine" Name=ConsoleFont FontName="Verdana" Height=12 AntiAlias=1 CharactersPerPage=256
#exec TEXTURE IMPORT NAME=ConsoleBK FILE=..\UWindow\TEXTURES\Black.PCX	
#exec TEXTURE IMPORT NAME=ConsoleBdr FILE=..\UWindow\TEXTURES\White.PCX	
	
// Constants.
const MaxHistory=16;		// # of command histroy to remember.

// Variables

var globalconfig byte ConsoleKey;			// Key used to bring up the console

var int HistoryTop, HistoryBot, HistoryCur;
var string TypedStr, History[MaxHistory]; 	// Holds the current command, and the history
var bool bTyping;							// Turn when someone is typing on the console
var bool bIgnoreKeys;						// Ignore Key presses until a new KeyDown is received							

var Name PrevState;

var font ConsoleFont;

var localized string cSay, cTeamSay;

enum HoldingState
{
    StandingStill,
    HoldingLeft,
    HoldingRight,
    HoldingUp,
    HoldingDown,
    HoldingSelect,
    HoldingStart,
    HoldingBack,
};

var() float ControllerRepeatDelayInitial;
var() float ControllerRepeatDelaySubsequent;
var() transient float ControllerRepeatDelayCurrent;
var() transient HoldingState ControllerState;

var() transient float DeltaPrevJoyX, DeltaPrevJoyY, DeltaPrevJoyU, DeltaPrevJoyV;

var() transient float TimeIdle;             // Time since last input.

// CurMenu is the menu stack the currently has focus,
// PrevMenu is the non-current menu stack that most
// recently had focus. If not none, and not flagged
// as bDeleteMe, PrevMenu will be drawn prior to CurMenu.

var() editinline Menu CurMenu;
var() editinline Menu PrevMenu;

var() editinline Menu LoadingMenu;

var	sound MenuBackgroundMusic;

var Menu KeyMenu; // only grabs key/buttons

var() bool UsingMenuRes;
var() String NormalRes;

var() bool	bUIEnabled;	// Whether or not UI has been enabled

var array<string> BufferedConsoleCommands;  // If this is blank, perform the command at Tick

var() transient bool bCleanupMenus;

var() transient float LastElapsedFrameTime;

event NativeConsoleOpen()
{
}

//-----------------------------------------------------------------------------
// Exec functions accessible from the console and key bindings.

// Begin typing a command on the console.
exec function Type()
{
	local PlayerController PC;

	if ( CurMenu != None )
		return;
		
	PC = ViewportOwner.Actor;
	//Log( "ReleaseAllButtons() : "$PC );
	PC.ReleaseAllButtons();
		
	TypedStr="";
	GotoState( 'Typing' );
}
 
exec function Talk()
{
	local PlayerController PC;

	if ( CurMenu != None )
		return;


	if( ViewportOwner == None || ViewportOwner.Actor.Level.NetMode != NM_StandAlone )
	{
		PC = ViewportOwner.Actor;
		//Log( "ReleaseAllButtons() : "$PC );
		PC.ReleaseAllButtons();
	
		TypedStr=cSay$" ";
		GotoState( 'Typing' );
	}
}

exec function TeamTalk()
{
	local PlayerController PC;

	if ( CurMenu != None )
		return;

	if( ViewportOwner == None || ViewportOwner.Actor.Level.NetMode != NM_StandAlone )
	{
		PC = ViewportOwner.Actor;
		//Log( "ReleaseAllButtons() : "$PC );
		PC.ReleaseAllButtons();

		TypedStr=cTeamSay$" ";
		GotoState( 'Typing' );
	}
}

exec function ConsoleOpen();
exec function ConsoleClose();
exec function ConsoleToggle();

simulated function Menu CloseVignettes( Menu M, bool bNoNotCloseVignettesMaybe )
{
    local Menu RM;

    if( M == None || !bUIEnabled )
        return None;

    M.PreviousMenu = CloseVignettes( M.PreviousMenu, bNoNotCloseVignettesMaybe );
    M.UnderlayMenu = CloseVignettes( M.UnderlayMenu, bNoNotCloseVignettesMaybe );
    
    if( M.bVignette == bNoNotCloseVignettesMaybe )
        return M;

    if( M.PreviousMenu != None )
        RM = M.PreviousMenu;
    else if( M.UnderlayMenu != None )
        RM = M.UnderlayMenu;
    else
        RM = None;

	RM.TransferTransientElements( M );

    M.DestroyMenu();
    return RM;
}

simulated event PreLevelChange()
{
	/* SBD Removed ***
    bCheckedURL = false;
	*/
    if( CurMenu != None && bUIEnabled )
    {
        CurMenu = CloseVignettes( CurMenu, true );

        if( CurMenu == None )
            MenuClose();
    }
    ConsoleClose();
}

simulated event PostLevelChange()
{
	/* SBD Removed ***
    bCheckedURL = false;
	*/
    if( CurMenu != None && bUIEnabled )
    {
        CurMenu = CloseVignettes( CurMenu, false );

        if( CurMenu == None )
            MenuClose();
    }
}

function DelayedConsoleCommand(string command)
{
    BufferedConsoleCommands.Length = BufferedConsoleCommands.Length+1;
    BufferedConsoleCommands[BufferedConsoleCommands.Length-1] = Command;
}

event NotifyLevelChange()
{
}

//-----------------------------------------------------------------------------
// Message - By default, the console ignores all output.
//-----------------------------------------------------------------------------

event Message( coerce string Msg, float MsgLife);

function bool IgnoreKeyEvent( EInputKey Key, EInputAction Action )
{
    return( false );
}

function bool KeyIsBoundTo( EInputKey Key, String Binding )
{
    local String KeyName, KeyBinding;
    
    KeyName = ViewportOwner.Actor.ConsoleCommand( "KEYNAME "$Key );
    
    if( KeyName == "" )
        return( false );
    
    KeyBinding = ViewportOwner.Actor.ConsoleCommand( "KEYBINDING "$KeyName );

    if( KeyBinding ~= Binding )
        return( true );

    return( false );
}

event bool KeyEvent( EInputKey Key, EInputAction Action, FLOAT Delta )
{
    if( IgnoreKeyEvent( Key, Action ) )
        return( false );

    if( Action == IST_Press )
    {
        TimeIdle = 0;
        
		/* SBD Removed ***
        if( IsRunningDemo() && !IsSoaking() )
        {
            if
            (
                ( (Key >= IK_Joy1) && (Key <= IK_Joy16) ) ||
                (Key == IK_Space) ||
                (Key == IK_Escape) ||
                (Key == IK_LeftMouse)
            )
            {
                StopRollingDemo( false );
                return( true );
            }
        } 
		*/
    }
    
    return( false );
}

//-----------------------------------------------------------------------------
// State used while typing a command on the console.

function TypingOpen()
{
	if ( CurMenu != None )
		return;

    PrevState = GetStateName();

    bTyping = true;

    if( (ViewportOwner != None) && (ViewportOwner.Actor != None) )
        ViewportOwner.Actor.Typing( bTyping );

    //TypedStr = "";

    GotoState('Typing');
}

function TypingClose()
{
    bTyping = false;

    if( (ViewportOwner != None) && (ViewportOwner.Actor != None) )
        ViewportOwner.Actor.Typing( bTyping );

    TypedStr="";

    if( GetStateName() == 'Typing' )
        GotoState( PrevState ); 
}

state Typing
{
	exec function Type()
	{
		TypedStr="";
		gotoState( '' );
	}
	function bool KeyType( EInputKey Key, optional string Unicode )
	{
		if (bIgnoreKeys)		
			return true;
	
		if( Key>=0x20 && Key<0x100 && Key!=Asc("~") && Key!=Asc("`") )
		{
			if( Unicode != "" )
				TypedStr = TypedStr $ Unicode;
			else
				TypedStr = TypedStr $ Chr(Key);
			return true;
		}
	}
	function bool KeyEvent( EInputKey Key, EInputAction Action, FLOAT Delta )
	{
		local string Temp;

		if (Action== IST_PRess)
		{
			bIgnoreKeys=false;
		}
	
		if( Key==IK_Escape )
		{
			if( TypedStr!="" )
			{
				TypedStr="";
				HistoryCur = HistoryTop;
				return true;
			}
			else
			{
				GotoState( '' );
			}
		}
		else if( global.KeyEvent( Key, Action, Delta ) )
		{
			return true;
		}
		else if( Action != IST_Press )
		{
			return false;
		}
		else if( Key==IK_Enter )
		{
			if( TypedStr!="" )
			{
				// Print to console.
				Message( TypedStr, 6.0 );

				History[HistoryTop] = TypedStr;
				HistoryTop = (HistoryTop+1) % MaxHistory;
				
				if ( ( HistoryBot == -1) || ( HistoryBot == HistoryTop ) )
					HistoryBot = (HistoryBot+1) % MaxHistory;

				HistoryCur = HistoryTop;

				// Make a local copy of the string.
				Temp=TypedStr;
				TypedStr="";
				
				if( !ConsoleCommand( Temp ) )
					Message( Localize("Errors","Exec","Core"), 6.0 );
					
				Message( "", 6.0 );
				GotoState('');
			}
			else
				GotoState('');
				
			return true;
		}
		else if( Key==IK_Up )
		{
			if ( HistoryBot >= 0 )
			{
				if (HistoryCur == HistoryBot)
					HistoryCur = HistoryTop;
				else
				{
					HistoryCur--;
					if (HistoryCur<0)
						HistoryCur = MaxHistory-1;
				}
				
				TypedStr = History[HistoryCur];
			}
			return True;
		}
		else if( Key==IK_Down )
		{
			if ( HistoryBot >= 0 )
			{
				if (HistoryCur == HistoryTop)
					HistoryCur = HistoryBot;
				else
					HistoryCur = (HistoryCur+1) % MaxHistory;
					
				TypedStr = History[HistoryCur];
			}			

		}
		else if( Key==IK_Backspace || Key==IK_Left )
		{
			if( Len(TypedStr)>0 )
				TypedStr = Left(TypedStr,Len(TypedStr)-1);
			return true;
		}
		return true;
	}
	
	function PostRender(Canvas Canvas)
	{
		local float xl,yl;
		local string OutStr;
		local float XBoxOffset;
		
		XBoxOffset = 20;

		// Blank out a space

		Canvas.Style = 1;
		
		Canvas.Font	 = ConsoleFont;
		OutStr = "(>"@TypedStr$"_";
		Canvas.Strlen(OutStr,xl,yl);

		Canvas.SetPos(0,Canvas.ClipY-6-yl-XBoxOffset);
		Canvas.DrawTile( texture 'ConsoleBk', Canvas.ClipX, yl+6,0,0,32,32);

		Canvas.SetPos(0,Canvas.ClipY-8-yl-XBoxOffset);	
		Canvas.SetDrawColor(0,255,0);
		Canvas.DrawTile( texture 'ConsoleBdr', Canvas.ClipX, 2,0,0,32,32);

		Canvas.SetDrawColor(200,200,200,255);
		Canvas.SetPos(0,Canvas.ClipY-3-yl-XBoxOffset);
		Canvas.bCenter = False;
		Canvas.DrawText( OutStr, false );
	}
	
	function BeginState()
	{
		bTyping = true;
		bVisible= true;
		bIgnoreKeys = true;
		HistoryCur = HistoryTop;
		
	}
	function EndState()
	{
		bTyping = false;
		bVisible = false;
	}
}

    
simulated event Tick( float Delta )
{
	LastElapsedFrameTime = Delta;
	
    while( BufferedConsoleCommands.Length > 0 )
    {
        ViewportOwner.Actor.ConsoleCommand(BufferedConsoleCommands[0]);
        BufferedConsoleCommands.Remove(0,1);
    }       

    if( !IsInState('Typing') && !IsInState('ConsoleVisible') )
        TimeIdle += Delta;

    if( ViewportOwner == None)
        return;

    if( CurMenu == None || !bUIEnabled )
        ViewportOwner.bShowWindowsMouse = false;
    else if( CurMenu.TravelMenu == CurMenu )
    {
		if ( ViewportOwner.bWindowsMouseAvailable )
		{
			if( (CurMenu.MouseX != ViewportOwner.WindowsMouseX) || (CurMenu.MouseY != ViewportOwner.WindowsMouseY) )
			{
				CurMenu.MouseX = ViewportOwner.WindowsMouseX;
				CurMenu.MouseY = ViewportOwner.WindowsMouseY;
	            
				CurMenu.HandleInputMouseMove();
			}
	        
			ViewportOwner.bShowWindowsMouse = true;
		}

        CurMenu.bShowMouseCursor = !ViewportOwner.bWindowsMouseAvailable;
    }
    
    if( ViewportOwner.Actor == None ) 
        return;

	/* SBD Removed ***
    if( IsRunningDemo() )
    {
        // log( string(IsRunningDemo()) @ TimeIdle @ TimePerDemo @ curMenu);

        if( (TimePerDemo > 0.0) && (TimeIdle > TimePerDemo) && (curMenu == None) )
            StopRollingDemo( true );
    }
    else if((curMenu != None) && (curMenu.IsA('MenuMain') || curMenu.IsA('MenuPreBeginMain') || 
                                  curMenu.IsA('MenuProfileWarning') || curMenu.IsA('MenuCorruptContent')))
    {
        if( (TimePerTitle > 0.0) && (TimeIdle > TimePerTitle) )
        {
            if( IsSoaking() )
            {
                TimePerTitle = 1;
                TimePerDemo = TimePerSoak;
            }
            else if( IsDebugging() )
            {
                TimePerTitle = 0;
            }
        
            if( (TimePerTitle > 0.0) && (TimeIdle > TimePerTitle) )
            {
                log( "Starting rolling demo. CurMenu:" @ CurMenu.Class );
                StartRollingDemo();
            }
        }
    }
	*/
	
	if ( bCleanupMenus )
	{
		CleanupMenus();
		bCleanupMenus = False;
	}	
}
    
//-----------------------------------------------------------------------------
// State used while in a menu.

simulated function MenuRenderStack( Canvas C, Menu M, int Depth, bool HasFocus )
{
    local Plane POverlayColor;
    local Color COverlayColor;
    
	if ( !bUIEnabled )
		return;

    if( M.UnderlayMenu != None )
        MenuRenderStack( C, M.UnderlayMenu, Depth + 1, false );

    POverlayColor = M.GetModulationColor();

    C.ColorModulate.X = 1.f;
    C.ColorModulate.Y = 1.f;
    C.ColorModulate.Z = 1.f;
    C.ColorModulate.W = POverlayColor.W;
    
    // Mod2X!
    COverlayColor.R = 128 * POverlayColor.X;
    COverlayColor.G = 128 * POverlayColor.Y;
    COverlayColor.B = 128 * POverlayColor.Z;
    COverlayColor.A = 255;
    
    if( M.bFullscreenOnly==false )
        M.DrawMenu( C, HasFocus );
    
/* SBD Removed ***
    // If we could properly modulate materials I wouldn't have to do this.
    C.Style = 4; // WHY THE FUCK DOESN'T ERenderStyle.STY_Modulated WORK???;
    C.DrawColor = COverlayColor;
    C.SetPos( 0, 0 );
    C.DrawRect( Material'Engine.WhiteTexture', C.ClipX, C.ClipY );
*/
}

simulated function MenuRender( Canvas C )
{
    local Menu NextMenu;

	if ( !bUIEnabled )
		return;

    if( (CurMenu == None) && (PrevMenu == None) )       // No menus visible or active
        return;

    if( CurMenu == None )                               // Last menu fading away
    {
        MenuRenderStack( C, PrevMenu, 0, true );

        if( !PrevMenu.IsVisible() )                     // Prev fully faded away
        {
            if (!PrevMenu.MenuClosed( PrevMenu )) // Need notification if final menu
                LogMenuClosedError( PrevMenu, PrevMenu );
            PrevMenu.DestroyMenu();
            PrevMenu = None;
            MenuClose();
        }

        // Return early to avoid all the menu travel code.
        return;
    }
    else if( PrevMenu == None )                         // No prev
        MenuRenderStack( C, CurMenu, 0, true );
    else if( !PrevMenu.IsVisible() )                    // Prev fully faded away
    {
        MenuRenderStack( C, CurMenu, 0, true );

        if( PrevMenu.TravelMenu == None )
        {
            if (!CurMenu.MenuClosed( PrevMenu ))
                LogMenuClosedError( CurMenu, PrevMenu );
            PrevMenu.DestroyMenu();
            PrevMenu = None;
        }
    }
    else if( PrevMenu.UnderlayMenu == CurMenu )         // Prev used to be the overlay
    {
        MenuRenderStack( C, CurMenu, 1, true );
        MenuRenderStack( C, PrevMenu, 0, false );
    }
    else if( CurMenu.UnderlayMenu == PrevMenu )         // Cur is now overlayed on Prev
        MenuRenderStack( C, CurMenu, 0, true );
    else                                                // Inter-stack transition
    {
        if( CurMenu.GetModulationColor().W < PrevMenu.GetModulationColor().W )
        {
            MenuRenderStack( C, CurMenu, 0, true );
            MenuRenderStack( C, PrevMenu, 0, false );
        }
        else
        {
            MenuRenderStack( C, PrevMenu, 0, false );
            MenuRenderStack( C, CurMenu, 0, true );
        }
    }

    C.Reset();

    // Staying at this menu
    if( CurMenu.TravelMenu == CurMenu )
        return;
    
    if( CurMenu.TravelMenu != None ) // Going/calling/overlaying/underlaying another menu
    {
        NextMenu = CurMenu.TravelMenu;

        // Set the travel menu back so it'll be good when this menu is displayed next.
        if( ( NextMenu.UnderlayMenu == CurMenu ) || ( NextMenu.PreviousMenu == CurMenu ) )
            CurMenu.TravelMenu = CurMenu;
        else
            CurMenu.TravelMenu = None;

        ControllerState = StandingStill;
        ControllerRepeatDelayCurrent = 0;
    }
    else // Closing current menu
    {
        if( CurMenu.PreviousMenu != None )
            NextMenu = CurMenu.PreviousMenu;
        else if( CurMenu.UnderlayMenu != None )
            NextMenu = CurMenu.UnderlayMenu;
        else
            NextMenu = None;

        ControllerState = StandingStill;
        ControllerRepeatDelayCurrent = 0;
    }

    if( (PrevMenu != None) && (PrevMenu.TravelMenu == None) )
        PrevMenu.DestroyMenu();

    if( NextMenu != None )
        CheckResolution( !NextMenu.bRenderLevel );

    PrevMenu = CurMenu;
    CurMenu = NextMenu;
}

simulated function LogMenuClosedError( Menu curMenu, Menu closingMenu )
{
    //log( curMenu.class $ "::MenuClosed couldn't handle a " $ closingMenu.class, 'Error');
}

simulated function CheckResolution( bool NeedMenuResolution )
{
    local PlayerController PC;
    local int CurrentX, CurrentY, MenuX, MenuY, i;
    local String MenuRes;

    if( IsOnConsole() )
        return;

    PC = ViewportOwner.Actor;

    if( NeedMenuResolution && UsingMenuRes )
        return;

    if( !NeedMenuResolution && !UsingMenuRes )
        return;
        
    if( !NeedMenuResolution )
    {
        log( "Leaving menu, changing resolution from" @ PC.ConsoleCommand( "GETCURRENTRES" ) @ "to" @ NormalRes );
        DelayedConsoleCommand( "SETRES" @ NormalRes );
        //PC.ConsoleCommand( "SETRES" @ NormalRes );
    }
    else
    {
        NormalRes = PC.ConsoleCommand( "GETCURRENTRES" );
        
        i = InStr( NormalRes, "x" );
        
        if( i > 0 )
        {
            CurrentX = int( Left( NormalRes, i )  );
            CurrentY = int( Right( NormalRes, Len(NormalRes) - i - 1 )  );
        }
                
        if( ( CurrentX == 0 ) || ( CurrentY == 0 ) )
        {
            log( "Couldn't parse GETCURRENTRES result:" @ NormalRes, 'Error' ); 
            return;
        }
        
        MenuX = int( PC.ConsoleCommand("get ini:Engine.Engine.ViewportManager MenuViewportX") );
        MenuY = int( PC.ConsoleCommand("get ini:Engine.Engine.ViewportManager MenuViewportY") );
    
        if( ( MenuX == 0 ) || ( MenuY == 0 ) )
            return;

        if( ( MenuX <= CurrentX ) && ( MenuY <= CurrentY ) )
            return;

        MenuRes = String(MenuX) $ "x" $ String(MenuY);

        log( "Entering menu, changing resolution from" @ NormalRes @ "to" @ MenuRes );

        DelayedConsoleCommand( "SETRES" @ MenuRes  );
        //PC.ConsoleCommand( "SETRES" @ MenuRes  );

        // Stuff back the INI settings just in case they quit while in a menu.
        if( bool( PC.ConsoleCommand("ISFULLSCREEN")) )
        {
            PC.ConsoleCommand("set ini:Engine.Engine.ViewportManager FullscreenViewportX" @ CurrentX );
            PC.ConsoleCommand("set ini:Engine.Engine.ViewportManager FullscreenViewportY" @ CurrentY );
        }
        else
        {
            PC.ConsoleCommand("set ini:Engine.Engine.ViewportManager WindowedViewportX" @ CurrentX );
            PC.ConsoleCommand("set ini:Engine.Engine.ViewportManager WindowedViewportY" @ CurrentY );
        }
    }
    
    UsingMenuRes = NeedMenuResolution;
}


simulated event ManageMenuBackgroundMusic( Menu NewMenu )
{
    local PlayerController PC;
    local string CurrentMapName;

    PC = ViewportOwner.Actor;
    PC.GetCurrentMapName( CurrentMapName );
	// Strip any extension off the level name
	if ( Caps(Right(CurrentMapName, 4)) == ".CTM" )
		CurrentMapName = Left(CurrentMapName, Len(CurrentMapName) - 4);
    
	if ( Caps( CurrentMapName ) != "ENTRY" )
	{
		if ( ( NewMenu == None ) || !NewMenu.bBackgroundMusicDuringLevel )
		{
			// No background music if we're in-game
			StopMenuBackgroundMusic( MenuBackgroundMusic );
			MenuBackgroundMusic = None;
			return;
		}
	}
	
	if ( NewMenu == None )
	{
		if ( None != MenuBackgroundMusic )
			StopMenuBackgroundMusic( MenuBackgroundMusic );

		return;
	}
		
	if ( NewMenu.BackgroundMusic != MenuBackgroundMusic )
	{
		if ( None != MenuBackgroundMusic )
			StopMenuBackgroundMusic( MenuBackgroundMusic );
		
		MenuBackgroundMusic = NewMenu.BackgroundMusic;

		if ( None != MenuBackgroundMusic )
			StartMenuBackgroundMusic( MenuBackgroundMusic );
	}
}

// NOTE: This is clears out any open menus on the stack!

simulated event MenuOpen (class<Menu> MenuClass, String Args)
{
    local PlayerController PC;
	local Menu NewMenu;
    
	if ( !bUIEnabled )
		return;

    log( "MenuOpen:" @ MenuClass @ "(" $ Args $ ")" );

    if (ViewportOwner == None)
    {
        log ("MenuOpen: can't spawn without an owner", 'Error');
        return;
    }

    PC = ViewportOwner.Actor;
    
    if (PC == None)
    {
        log ("MenuOpen: can't spawn without a PlayerController", 'Error');
        return;
    }
    
    NewMenu = PC.Spawn (MenuClass, PC);

    if (NewMenu == None)
    {
        log ("MenuOpen: Failed to open menu of class "$MenuClass, 'Error');
        return;
    }

	NewMenu.TransferTransientElements( CurMenu );

    DestroyMenuTree( CurMenu );
    DestroyMenuTree( PrevMenu );

	CurMenu = NewMenu;

    CheckResolution( !CurMenu.bRenderLevel );

    CurMenu.Init( Args );
    MenuOpenExisting(CurMenu);

	PauseLevelMusic();

	ManageMenuBackgroundMusic( CurMenu );
}

// NOTE: This is clears out any open menus on the stack!

simulated event MenuOpenExisting(Menu m)
{
	if ( !bUIEnabled )
		return;

    if (m == None)
    {
        log ("1 MenuOpenExisting: Failed to open menu "$m, 'Error');
        return;
    }

    if (CurMenu != None && CurMenu != m)
    {
		m.TransferTransientElements( CurMenu );
        MenuClose();
	}
	
    CurMenu = m;
    m.ReopenInit();
    m.FullScreenOnlyTime = m.Level.TimeSeconds;

    ControllerState = StandingStill;
    ControllerRepeatDelayCurrent = 0;

	PauseLevelMusic();

	ManageMenuBackgroundMusic( CurMenu );
	
    GotoState ('Menuing');
}

simulated event MenuCall(class<Menu> MenuClass, String Args)
{
    local PlayerController PC;
	local Menu NewMenu;
    
	if ( !bUIEnabled )
		return;

    log( "MenuCall:" @ MenuClass @ "(" $ Args $ ")" );

    if (ViewportOwner == None)
    {
        log ("MenuCall: can't spawn without an owner", 'Error');
        return;
    }

    PC = ViewportOwner.Actor;
    
    if (PC == None)
    {
        log ("MenuCall: can't spawn without a PlayerController", 'Error');
        return;
    }
    
    NewMenu = PC.Spawn (MenuClass, PC);

    if (NewMenu == None)
    {
        log ("MenuCall: Failed to open menu of class "$MenuClass, 'Error');
        return;
    }
    
	MenuCallExisting( NewMenu, Args );
}

simulated event MenuCallExisting(Menu m, string Args)
{
	if ( !bUIEnabled )
		return;

    log("MenuCallExisting"@m@CurMenu);
    if (m == None)
    {
        log ("1 MenuCallExisting: Failed to open menu "$m, 'Error');
        return;
    }

    if (CurMenu != None && CurMenu != m)
    {
		m.TransferTransientElements(CurMenu);
        CurMenu.CallMenu(m, Args);
    }
    else
    {
        CurMenu = m;
        CurMenu.Init(Args);
        //m.ReopenInit();
        
		ManageMenuBackgroundMusic( CurMenu );        
    }

    ControllerState = StandingStill;
    ControllerRepeatDelayCurrent = 0;

    GotoState ('Menuing');
}

simulated function DestroyMenuTree( out Menu M )
{
	if ( !bUIEnabled )
		return;

    if( M == None )
        return;

    DestroyMenuTree( M.PreviousMenu );
    DestroyMenuTree( M.UnderlayMenu );

    M.DestroyMenu();
    M = None;
}

simulated event MenuClose()
{
	if ( !bUIEnabled )
		return;

    DestroyMenuTree( CurMenu );
    DestroyMenuTree( PrevMenu );

    if( IsInState('Typing') )
        TypingClose();

    TimeIdle = 0;

    CheckResolution( false );

	if ( None != MenuBackgroundMusic )
	{
		StopMenuBackgroundMusic(MenuBackgroundMusic);
		MenuBackgroundMusic = None;
	}

	ResumeLevelMusic();

    GotoState ('');
    
	bCleanupMenus = True;
}

simulated function KeyMenuOpenExisting(Menu m)
{
	if ( !bUIEnabled )
		return;

    if (m == None)
    {
        log ("1 MenuOpenExisting: Failed to open menu "$m, 'Error');
        return;
    }

    KeyMenuClose();

    KeyMenu = m;
    m.ReopenInit();

    GotoState ('KeyMenuing');
}

simulated function KeyMenuClose()
{
	if ( !bUIEnabled )
		return;

    KeyMenu = None;
    if( IsInState('Typing') )
        TypingClose();
    TimeIdle = 0;
    GotoState ('');
}

native function CreateLoadingMenu(class<Menu> MenuClass);
native function DestroyLoadingMenu();

simulated event LoadingStarted(string Menu, String Pic, string Title, string Text, bool ShowHints)
{
	local class<Menu> MenuClass;

	if ( !bUIEnabled )
		return;

	MenuClass = class<Menu>( DynamicLoadObject( Menu, class'Class' ) );
    if( MenuClass == None )
    {
        log( "Could not load menu! ["$Menu$"]", 'Error' );
        return;
    }

	CreateLoadingMenu(MenuClass);
	
	if ( LoadingMenu != None )
	{
		LoadingMenu.Init("");
		LoadingMenu.SetInfoOptions(Pic, Title, Text, "", ShowHints);
	}
}

simulated event UpdateLoadingProgress(float LoadingRatioCompleted)
{
	if ( !bUIEnabled )
		return;

	if ( LoadingMenu != None )
	{
		LoadingMenu.UpdateLoadingProgress(LoadingRatioCompleted);
	}
}

simulated event LoadingFinished()
{
	local Menu lm;
    local PlayerController PC;

	if ( !bUIEnabled )
		return;

	if ( CurMenu != None )
	{
		MenuClose();
	}

	if ( (LoadingMenu != None) && LoadingMenu.PauseOnLoadingFinish() )
	{
		if (ViewportOwner == None)
		{
			log ("LoadingFinished: can't spawn without an owner", 'Error');
			return;
		}

		PC = ViewportOwner.Actor;
	    
		if (PC == None)
		{
			log ("LoadingFinished: can't spawn without a PlayerController", 'Error');
			return;
		}
	
		PC.SetPause(true);
		
		lm = PC.Spawn( LoadingMenu.Class, PC );
		
		lm.TransferTransientElements( LoadingMenu );
		lm.Init( "LOADINGDONE" );
		
		MenuOpenExisting( lm );
	}
	
	DestroyLoadingMenu();
	
	bCleanupMenus=True;
}

simulated event bool ShowingMenu()
{
	if ( !bUIEnabled )
		return false;

	return CurMenu != None;
}

simulated event bool ShowingMenuClass( string MenuClassName )
{
	local class<Menu> CheckClass;
	
	if ( !bUIEnabled )
		return false;
		
	if ( CurMenu == None )
		return false;
		
    CheckClass = class<Menu>( DynamicLoadObject( MenuClassName, class'Class' ) );

	if ( CurMenu.Class != CheckClass )
		return false;
		
	return true;
}

simulated event RestartMenuBackgroundMusic( bool bOnlyIfNotPlaying )
{
	if ( !bUIEnabled )
		return;

	if ( CurMenu != None )
	{
		if ( !bOnlyIfNotPlaying || !IsMenuBackgroundMusicPlaying(CurMenu.BackgroundMusic) )
		{
			StopMenuBackgroundMusic(CurMenu.BackgroundMusic);
			StartMenuBackgroundMusic(CurMenu.BackgroundMusic);
		}
	}
}

native function bool IsMenuBackgroundMusicPlaying(Sound s);
native function StartMenuBackgroundMusic(Sound s);
native function StopMenuBackgroundMusic(Sound s);
native function PauseLevelMusic();
native function ResumeLevelMusic();

native function CleanupMenus();

state KeyMenuing
{
    event bool KeyEvent( EInputKey Key, EInputAction Action, FLOAT Delta )
    {
        local bool rc;

        if( IgnoreKeyEvent( Key, Action ) )
            return( false );

        if( KeyMenu.IgnoreKeyEvent( Key, Action ) )
            return( false );

        if( Action == IST_Press )
        {
            rc = KeyMenu.HandleInputKeyRaw( Key ); // may cause KeyMenu to be None
            if( KeyMenu != None )
                KeyMenu.LastInputSource = IS_None;
            //ControllerRepeatDelayCurrent = 0;
            if( rc )
                return( true );
        }
        return Super.KeyEvent( Key, Action, Delta );
    }
}

state Menuing
{
    function bool KeyType( EInputKey Key, optional string Unicode )
    {
        local bool rc;

        if( (CurMenu == None) || (CurMenu.TravelMenu != CurMenu) )
            return( false );

        CurMenu.LastInputSource = IS_Keyboard;
        rc = CurMenu.HandleInputKey( Key );
        CurMenu.LastInputSource = IS_None;
        ControllerState = StandingStill;
        ControllerRepeatDelayCurrent = 0;
        
        if( rc )
            return( true );
    }

    function bool IgnoreKeyEvent( EInputKey Key, EInputAction Action )
    {
        if( KeyIsBoundTo( Key, "ConsoleToggle" ) )
            return( true );

        if( KeyIsBoundTo( Key, "Type" ) )
            return( true );

        return( global.IgnoreKeyEvent( Key, Action ) );
    }

    event bool KeyEvent( EInputKey Key, EInputAction Action, FLOAT Delta )
    {
        local bool rc;

        if( CurMenu != None && CurMenu.bIgnoresInput )
            return false;

        if( (CurMenu == None) || (CurMenu.TravelMenu != CurMenu) )
            return( true );

        if( (Action == IST_Press) && !CurMenu.bRawKeyboardInput )
        {
            if( IgnoreKeyEvent( Key, Action ) )
                return( false );

            if( CurMenu.IgnoreKeyEvent( Key, Action ) )
                return( false );
        }

        if( Action == IST_Press )
        {
            CurMenu.LastInputSource = IS_Keyboard;
            rc = CurMenu.HandleInputKeyRaw( Key );
            CurMenu.LastInputSource = IS_None;
            ControllerState = StandingStill;
            ControllerRepeatDelayCurrent = 0;
            
            if( rc )
                return( true );
        }

        if( Action == IST_Axis )
        {
            if( ( Key == IK_MouseX ) || ( Key == IK_MouseY ) )
            {
                TimeIdle = 0;

				if ( !ViewportOwner.bWindowsMouseAvailable )
				{
					if ( Key == IK_MouseX )
					{
						// COMPLETE HACK.
						// The mouse delta does not appear to be mirroring
						// changes in framerate, so we'll have to do it manually.
						// NOTE 0.01667 == 1 frame @ 60 FPS
						if( LastElapsedFrameTime > 0.01667 )
							Delta *= (LastElapsedFrameTime / 0.01667);

						// Adjust for differing screen resolutions
						Delta *= CurMenu.GetCanvasXSize() / 800.0;
						
						CurMenu.MouseX += Delta * 0.015;
						if ( CurMenu.MouseX > CurMenu.GetCanvasXSize() )
							CurMenu.MouseX = CurMenu.GetCanvasXSize();
						else if ( CurMenu.MouseX < 0.0f ) 
							CurMenu.MouseX = 0.0f;
					}
					else if ( Key == IK_MouseY )
					{
						// COMPLETE HACK.
						// The mouse delta does not appear to be mirroring
						// changes in framerate, so we'll have to do it manually.
						// NOTE 0.01667 == 1 frame @ 60 FPS
						if( LastElapsedFrameTime > 0.01667 )
							Delta *= (LastElapsedFrameTime / 0.01667);
						
						// Adjust for differing screen resolutions						
						Delta *= CurMenu.GetCanvasYSize() / 600.0;
						
						CurMenu.MouseY -= Delta * 0.015f;
						if ( CurMenu.MouseY > CurMenu.GetCanvasYSize() )
							CurMenu.MouseY = CurMenu.GetCanvasYSize();
						else if ( CurMenu.MouseY < 0.0f ) 
							CurMenu.MouseY = 0.0f;
					}

					CurMenu.HandleInputMouseMove();

					return ( true );
				}

                return( false );
            }
            else if( (Key == IK_JoyX) || (Key == IK_JoyY) || (Key == IK_JoyU) || (Key == IK_JoyV) )
            {
                if( Delta < -0.4 )
                    Delta = -1;
                else if( Delta > 0.4 )
                    Delta = 1;
                else
                    Delta = 0;
            
                if( Key == IK_JoyX )
                {
                    if( Delta != DeltaPrevJoyX )
                    {
                        TimeIdle = 0;

                        if( Delta < 0 )
                        {
                            if( ControllerState != HoldingLeft )
                            {
                                CurMenu.LastInputSource = IS_Controller;
                                CurMenu.HandleInputLeft();
                                CurMenu.LastInputSource = IS_None;
                                ControllerState = HoldingLeft;
                                ControllerRepeatDelayCurrent = ControllerRepeatDelayInitial;
                            }
                        }
                        else if( Delta > 0 )
                        {
                            if( ControllerState != HoldingRight )
                            {
                                CurMenu.LastInputSource = IS_Controller;
                                CurMenu.HandleInputRight();
                                CurMenu.LastInputSource = IS_None;
                                ControllerState = HoldingRight;
                                ControllerRepeatDelayCurrent = ControllerRepeatDelayInitial;
                            }
                        }
                        else
                        {
                            if( ( ControllerState == HoldingLeft ) || ( ControllerState == HoldingRight ) )
                            {
                                ControllerState = StandingStill;
                                ControllerRepeatDelayCurrent = 0;
                            }
                        }

                        DeltaPrevJoyX = Delta;
                    }
                }
                else if( Key == IK_JoyY )
                {
                    if( Delta != DeltaPrevJoyY )
                    {
                        TimeIdle = 0;

                        if( Delta > 0 )
                        {
                            if( ControllerState != HoldingUp )
                            {
                                CurMenu.LastInputSource = IS_Controller;
                                CurMenu.HandleInputUp();
                                CurMenu.LastInputSource = IS_None;
                                ControllerState = HoldingUp;
                                ControllerRepeatDelayCurrent = ControllerRepeatDelayInitial;
                            }
                        }
                        else if( Delta < 0 )
                        {
                            if( ControllerState != HoldingDown )
                            {
                                CurMenu.LastInputSource = IS_Controller;
                                CurMenu.HandleInputDown();
                                CurMenu.LastInputSource = IS_None;
                                ControllerState = HoldingDown;
                                ControllerRepeatDelayCurrent = ControllerRepeatDelayInitial;
                            }
                        }
                        else
                        {
                            if( ( ControllerState == HoldingDown ) || ( ControllerState == HoldingUp ) )
                            {
                                ControllerState = StandingStill;
                                ControllerRepeatDelayCurrent = 0;
                            }
                        }

                        DeltaPrevJoyY = Delta;
                    }
                }
                else if( Key == IK_JoyU )
                {
                    if( Delta != DeltaPrevJoyU )
                    {
                        TimeIdle = 0;

                        if( Delta < 0 )
                        {
                            if( ControllerState != HoldingLeft )
                            {
                                CurMenu.LastInputSource = IS_Controller;
                                CurMenu.HandleInputLeft();
                                CurMenu.LastInputSource = IS_None;
                                ControllerState = HoldingLeft;
                                ControllerRepeatDelayCurrent = ControllerRepeatDelayInitial;
                            }
                        }
                        else if( Delta > 0 )
                        {
                            if( ControllerState != HoldingRight )
                            {
                                CurMenu.LastInputSource = IS_Controller;
                                CurMenu.HandleInputRight();
                                CurMenu.LastInputSource = IS_None;
                                ControllerState = HoldingRight;
                                ControllerRepeatDelayCurrent = ControllerRepeatDelayInitial;
                            }
                        }
                        else
                        {
                            if( ( ControllerState == HoldingLeft ) || ( ControllerState == HoldingRight ) )
                            {
                                ControllerState = StandingStill;
                                ControllerRepeatDelayCurrent = 0;
                            }
                        }

                        DeltaPrevJoyU = Delta;
                    }
                }
                else if( Key == IK_JoyV )
                {
                    if( Delta != DeltaPrevJoyV )
                    {
                        TimeIdle = 0;

                        if( Delta > 0 )
                        {
                            if( ControllerState != HoldingUp )
                            {
                                CurMenu.LastInputSource = IS_Controller;
                                CurMenu.HandleInputUp();
                                CurMenu.LastInputSource = IS_None;
                                ControllerState = HoldingUp;
                                ControllerRepeatDelayCurrent = ControllerRepeatDelayInitial;
                            }
                        }
                        else if( Delta < 0 )
                        {
                            if( ControllerState != HoldingDown )
                            {
                                CurMenu.LastInputSource = IS_Controller;
                                CurMenu.HandleInputDown();
                                CurMenu.LastInputSource = IS_None;
                                ControllerState = HoldingDown;
                                ControllerRepeatDelayCurrent = ControllerRepeatDelayInitial;
                            }
                        }
                        else
                        {
                            if( ( ControllerState == HoldingUp ) || ( ControllerState == HoldingDown ) )
                            {
                                ControllerState = StandingStill;
                                ControllerRepeatDelayCurrent = 0;
                            }
                        }

                        DeltaPrevJoyV = Delta;
                    }
                }
            }
        }
        else if( Action == IST_Press )
        {
            TimeIdle = 0;
            
            if( !CurMenu.bRawKeyboardInput )
            {
                switch( Key )
                {
                    case IK_Escape:
                        CurMenu.LastInputSource = IS_Keyboard;
                        CurMenu.HandleInputBack();
                        CurMenu.LastInputSource = IS_None;
                        ControllerState = HoldingBack;
                        ControllerRepeatDelayCurrent = 0;
                        break;

                    case IK_Space:
                        CurMenu.LastInputSource = IS_Keyboard;
                        CurMenu.HandleInputSelect();
                        CurMenu.LastInputSource = IS_None;
                        ControllerState = HoldingSelect;
                        ControllerRepeatDelayCurrent = 0;
                        break;

                    case IK_Enter:
                        CurMenu.LastInputSource = IS_Keyboard;
                        CurMenu.HandleInputSelect();
                        CurMenu.LastInputSource = IS_None;
                        ControllerState = HoldingSelect;
                        ControllerRepeatDelayCurrent = 0;
                        break;

                    case IK_Left:
                        CurMenu.LastInputSource = IS_Keyboard;
                        CurMenu.HandleInputLeft();
                        CurMenu.LastInputSource = IS_None;
                        ControllerState = HoldingLeft;
                        ControllerRepeatDelayCurrent = 0;
                        break;

                    case IK_Right:
                        CurMenu.LastInputSource = IS_Keyboard;
                        CurMenu.HandleInputRight();
                        CurMenu.LastInputSource = IS_None;
                        ControllerState = HoldingRight;
                        ControllerRepeatDelayCurrent = 0;
                        break;

                    case IK_Up:
                        CurMenu.LastInputSource = IS_Keyboard;
                        CurMenu.HandleInputUp();
                        CurMenu.LastInputSource = IS_None;
                        ControllerState = HoldingUp;
                        ControllerRepeatDelayCurrent = 0;
                        break;

                    case IK_Down:
                        CurMenu.LastInputSource = IS_Keyboard;
                        CurMenu.HandleInputDown();
                        CurMenu.LastInputSource = IS_None;
                        ControllerState = HoldingDown;
                        ControllerRepeatDelayCurrent = 0;
                        break;
                }
            }
            
            // Gamepad pass code stuff
            
            switch( Key )
            {
                case IK_GameA:
                    if( CurMenu.HandleInputGamePad( "A" ) )
                        return( true );
                    break;

                case IK_GameB:
                    if( CurMenu.HandleInputGamePad( "B" ) )
                        return( true );
                    break;

                case IK_GameX:
                    if( CurMenu.HandleInputGamePad( "X" ) )
                        return( true );
                    break;

                case IK_GameY:
                    if( CurMenu.HandleInputGamePad( "Y" ) )
                        return( true );
                    break;

                case IK_GameBlack:
                    if( CurMenu.HandleInputGamePad( "K" ) )
                        return( true );
                    break;

                case IK_GameWhite:
                    if( CurMenu.HandleInputGamePad( "W" ) )
                        return( true );
                    break;

                case IK_GameLTrig:
                    if( CurMenu.HandleInputGamePad( "LT" ) )
                        return( true );
                    break;

                case IK_GameRTrig:
                    if( CurMenu.HandleInputGamePad( "RT" ) )
                        return( true );
                    break;

                case IK_GameDUp:
                    if( CurMenu.HandleInputGamePad( "U" ) )
                        return( true );
                    break;

                case IK_GameDDown:
                    if( CurMenu.HandleInputGamePad( "D" ) )
                        return( true );
                    break;

                case IK_GameDLeft:
                    if( CurMenu.HandleInputGamePad( "L" ) )
                        return( true );
                    break;

                case IK_GameDRight:
                    if( CurMenu.HandleInputGamePad( "R" ) )
                        return( true );
                    break;
            }

            switch( Key )
            {
                case IK_GameBack: // Back button
                case IK_GameB: // B button
                    CurMenu.LastInputSource = IS_Controller;
                    CurMenu.HandleInputBack();
                    CurMenu.LastInputSource = IS_None;
                    ControllerState = HoldingBack;
                    ControllerRepeatDelayCurrent = ControllerRepeatDelayInitial;
                    break;

                case IK_GameStart: // Start button
                    CurMenu.LastInputSource = IS_Controller;
                    CurMenu.HandleInputStart();
                    CurMenu.LastInputSource = IS_None;
                    ControllerState = HoldingStart;
                    ControllerRepeatDelayCurrent = ControllerRepeatDelayInitial;
                    break;

                case IK_GameA: // A button
                    CurMenu.LastInputSource = IS_Controller;
                    CurMenu.HandleInputSelect();
                    CurMenu.LastInputSource = IS_None;
                    ControllerState = HoldingSelect;
                    ControllerRepeatDelayCurrent = ControllerRepeatDelayInitial;
                    break;

				/* SBD Removed ***
                case IK_JoyPovLeft:
				*/
                case IK_GameDLeft:
                    CurMenu.LastInputSource = IS_Controller;
                    CurMenu.HandleInputLeft();
                    CurMenu.LastInputSource = IS_Controller;
                    ControllerState = HoldingLeft;
                    ControllerRepeatDelayCurrent = ControllerRepeatDelayInitial;
                    break;


				/* SBD Removed ***
                case IK_JoyPovRight:
				*/
                case IK_GameDRight:
                    CurMenu.LastInputSource = IS_Controller;
                    CurMenu.HandleInputRight();
                    CurMenu.LastInputSource = IS_None;
                    ControllerState = HoldingRight;
                    ControllerRepeatDelayCurrent = ControllerRepeatDelayInitial;
                    break;

				/* SBD Removed ***
                case IK_JoyPovUp:
				*/
                case IK_GameDUp:
                    CurMenu.LastInputSource = IS_Controller;
                    CurMenu.HandleInputUp();
                    CurMenu.LastInputSource = IS_None;
                    ControllerState = HoldingUp;
                    ControllerRepeatDelayCurrent = ControllerRepeatDelayInitial;
                    break;

				/* SBD Removed ***
                case IK_JoyPovDown:
				*/
                case IK_GameDDown:
                    CurMenu.LastInputSource = IS_Controller;
                    CurMenu.HandleInputDown();
                    CurMenu.LastInputSource = IS_None;
                    ControllerState = HoldingDown;
                    ControllerRepeatDelayCurrent = ControllerRepeatDelayInitial;
                    break;

                case IK_RightMouse:
                    CurMenu.LastInputModifier = IM_Alt;
                case IK_LeftMouse:
                case IK_MiddleMouse:
                    CurMenu.LastInputSource = IS_Mouse;
                    CurMenu.HandleInputMouseDown();
                    CurMenu.LastInputSource = IS_None;
                    CurMenu.LastInputModifier = IM_None;
                    break;

                case IK_MouseWheelUp:
                    CurMenu.LastInputSource = IS_Mouse;
                    if( !CurMenu.HandleInputWheelUp() )
                        CurMenu.HandleInputUp();
                    CurMenu.LastInputSource = IS_None;
                    break;

                case IK_MouseWheelDown:
                    CurMenu.LastInputSource = IS_Mouse;
                    if( !CurMenu.HandleInputWheelDown() )
                        CurMenu.HandleInputDown();
                    CurMenu.LastInputSource = IS_None;
                    break;
            }
        }
        else if( Action == IST_Release )
        {
            TimeIdle = 0;

            ControllerState = StandingStill;
            ControllerRepeatDelayCurrent = 0;

            switch( Key )
            {
                case IK_RightMouse:
                    CurMenu.LastInputModifier = IM_Alt;
                case IK_LeftMouse:
                case IK_MiddleMouse:
                    CurMenu.LastInputSource = IS_Mouse;
                    CurMenu.HandleInputMouseUp();
                    CurMenu.LastInputSource = IS_None;
                    CurMenu.LastInputModifier = IM_None;
                    break;
            }
        }
        
        return( true ); // Consume all input while in menus
    }

    simulated event Tick( float dt )
    {
        global.Tick( dt );

        if( CurMenu == None )
            return;

        if( ControllerRepeatDelayCurrent == 0 )
            return;

        ControllerRepeatDelayCurrent -= dt;

        if( ControllerRepeatDelayCurrent > 0 )
            return;
        else
            ControllerRepeatDelayCurrent = ControllerRepeatDelaySubsequent;

        CurMenu.LastInputSource = IS_Controller;

        switch( ControllerState )
        {
            case HoldingLeft:
                CurMenu.HandleInputLeft();
                break;

            case HoldingRight:
                CurMenu.HandleInputRight();
                break;

            case HoldingUp:
                CurMenu.HandleInputUp();
                break;

            case HoldingDown:
                CurMenu.HandleInputDown();
                break;

            case HoldingStart:
                CurMenu.HandleInputStart();
                break;

            case HoldingSelect:
                CurMenu.HandleInputSelect();
                break;

            case HoldingBack:
                CurMenu.HandleInputBack();
                break;

            default: 
                ControllerRepeatDelayCurrent = 0;
                break;
        }

        CurMenu.LastInputSource = IS_None;
    }
}




cpptext
{
	void StartMenuBackgroundMusic(USound* sound);
	void StopMenuBackgroundMusic(USound* sound);
	bool IsMenuBackgroundMusicPlaying(USound* sound);

	void PauseLevelMusic();
	void ResumeLevelMusic();

}

defaultproperties
{
     ConsoleKey=192
     HistoryBot=-1
     ConsoleFont=Font'OrbitFonts.OrbitBold8'
     cSay="Say"
     cTeamSay="TeamSay"
     ControllerRepeatDelayInitial=2
     ControllerRepeatDelaySubsequent=0.2
     bUIEnabled=True
     bRequiresTick=True
}

