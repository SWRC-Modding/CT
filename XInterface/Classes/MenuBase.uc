/*=============================================================================
	Copyright 2001 Digital Extremes. All Rights Reserved.

    There's a whole 'lotta magic ass in here.

    The following code is not yet supported:
        var() Array<SomeMenuWidget> MyWidgets;
        var() Array<int[30]> SomeIntArrays;
        FocusOnWidget for MenuStringList elements
        Default propagation down arrays of MenuButtonEnum.List members.

    The following code will probably crash:
        Emptying an array of widgets while one has focus (MenuStringList)

=============================================================================*/

class MenuBase extends Menu
    /* SBD Removed *** exportstructs*/
    native;

#exec OBJ LOAD FILE=MenuSounds.uax
#exec OBJ LOAD FILE=GUIContent.utx

enum EMenuWidgetPlatform
{
    MWP_All,
    MWP_PC,
    MWP_Console,
};

struct /* SBD Removed ***long*/ MenuWidgetBase // Abstract
{
    var() int Pass;
    var() int bHidden;
    var() EMenuWidgetPlatform Platform; // Defaults to MWP_All; can mask widgets from different platforms.
    var() int bLocked; // If bLocked only show this widget if !PlayerController(Owner).GetEntryLevel().game.bLocked 
    var() Name Style;
};

enum EMenuScaleMode
{
    MSCM_Scale,      // Uniformly scale the whole texture
    MSCM_Stretch,    // Split texture and fill in gaps with streteched blocks of the middle texels.
    MSCM_Fit,        // Calculate whatever scale is required to be ScaleX, ScaleY in normalized screen space.
    MSCM_FitStretch,
};

struct MenuMovie extends MenuWidgetBase
{
	var() MovieTexture MovieTex;

    var() IntBox TextureCoords;     // If 0,0,0,0 the whole texture is used

    var() ERenderStyle RenderStyle; // STY_Alpha is the default
    var() Color DrawColor;          // If 0,0,0,0 fully-opaque white is used

    var() EDrawPivot DrawPivot;     // Default is DP_UpperLeft
    var() float PosX, PosY;         // 0.0, 0.0 is upper-left 1.0, 1.0 is lower-right of the screen
    var() float ScaleX, ScaleY;     // Default is 1.0, 1.0

    var() EMenuScaleMode ScaleMode; // Default is MSCM_Scale
};

struct MenuSprite extends MenuWidgetBase
{
    var() Material WidgetTexture;
    var() IntBox TextureCoords;     // If 0,0,0,0 the whole texture is used

    var() ERenderStyle RenderStyle; // STY_Alpha is the default
    var() Color DrawColor;          // If 0,0,0,0 fully-opaque white is used

    var() EDrawPivot DrawPivot;     // Default is DP_UpperLeft
    var() float PosX, PosY;         // 0.0, 0.0 is upper-left 1.0, 1.0 is lower-right of the screen
    var() float ScaleX, ScaleY;     // Default is 1.0, 1.0

    var() EMenuScaleMode ScaleMode; // Default is MSCM_Scale
};

struct MenuText extends MenuWidgetBase
{
    var() Font MenuFont;
    var() localized String Text;

    var() ERenderStyle RenderStyle; // STY_Alpha is the default
    var() Color DrawColor;          // If 0,0,0,0 fully-opaque white is used

    var() EDrawPivot DrawPivot;     // Default is DP_UpperLeft
    var() float PosX, PosY;         // 0.0, 0.0 is upper-left 1.0, 1.0 is lower-right of the screen
    var() float ScaleX, ScaleY;     // Default is 1.0, 1.0
    
    var() int Kerning;              // Can be negative
    
    var() float MaxSizeX;           // 0 implies none.
    var() int bEllipsisOnLeft;
    var() int bNoFontRemapping;

    var() float TabStops[8];        // Each '\t' found will get a PosX from this array.
    var() int bWordWrap;
};

struct MenuDecoText extends MenuWidgetBase
{
    var() Font MenuFont;

    var() String TextName;

    var() ERenderStyle RenderStyle; // STY_Alpha is the default
    var() Color DrawColor;          // If 0,0,0,0 fully-opaque white is used

    // Draw pivot is always DP_UpperLeft
    var() float PosX, PosY;         // 0.0, 0.0 is upper-left 1.0, 1.0 is lower-right of the screen
    var() float ScaleX, ScaleY;     // Default is 1.0, 1.0

    var() int ColumnCount;
    var() int RowCount;

    var() float TimePerCharacter;
    var() float TimePerLineFeed;
    var() float TimePerLoopEnd;
    var() float TimePerCursorBlink;

    var() float CursorScale;
    var() float CursorOffset;

    var() int bCapitalizeText;
    var() int bPaused;

    var() transient DecoText Text;  // This is loaded from the INT entry specified by TextName

    var() transient float TickAccumulator;
    var() transient float BlinkAccumulator;

    var() transient int CurrentRow;
    var() transient int CurrentColumn;
    var() transient int bShowCursor;
};

struct /* SBD Removed ***long*/ RenderBounds extends FloatBox
{
    var() float PosX, PosY;
};

struct MenuActiveWidget extends MenuWidgetBase
{
    var() RenderBounds ActiveArea;

    var() const int bHasFocus;
    var() int bDisabled;
    var() int bIgnoreController;
    var() int ContextID;
	var() int bNoMouseOverFocus;
	var() int bStickyDrawFocus;
	var() const int bDrawFocused;

    var() localized String HelpText;

    var() Name OnFocus; // function [WidgetName]OnFocus( int ContextID );
    var() Name OnBlur; // function [WidgetName]OnBlur( int ContextID );

    var() Name OnSelect; // function [WidgetName]OnSelect( int ContextID );
    var() Name OnDoubleClick; // function [WidgetName]OnDoubleClick( int ContextID );

    // The following events can turn regular buttons into spinners. If they
    // are left as None the basic tab-order maneuvring code will kick in.

    var() Name OnLeft; // function [WidgetName]OnLeft( int ContextID );
    var() Name OnRight; // function [WidgetName]OnRight( int ContextID );
    var() Name OnUp; // function [WidgetName]OnUp( int ContextID );
    var() Name OnDown; // function [WidgetName]OnDown( int ContextID );
};

// In all cases, the Blurred widget will provided defaults for the focused version.
// That is, fill in the defaults for the Blurred version and override any you need
// to in the focused version.
//
// Copy order:
//
// Blurred -> Focused
// Blurred -> SelectedBlurred
// SelectedBlurred -> SelectedFocused
// BackgroundBlurred -> BackgroundFocused
// BackgroundFocused -> BackgroundSelected

struct MenuButton extends MenuActiveWidget // Abstract
{
    var() MenuSprite BackgroundBlurred, BackgroundFocused;
    var() int bRelativeBackgroundCoords;    // If set, Background PosX is Blurred.PosX + Background.PosX
};

struct MenuButtonSprite extends MenuButton
{
    var() MenuSprite Blurred, Focused;
};

struct MenuButtonText extends MenuButton
{
    var() MenuText Blurred;
    var() /* SBD Removed ***nonlocalized*/ MenuText Focused;
};

struct MenuButtonEnum extends MenuButtonText
{
    var() localized Array<String> Items;
    var() int Current;
    var() int bNoWrap;
};

struct MenuCheckBox extends MenuButton
{
    // To get RadioButton behaviour, set the Group variable.
    var() Name Group;
    var() const int bSelected;

    var() Name OnToggle; // function [WidgetName]OnToggle( int ContextID );
};

struct MenuCheckBoxSprite extends MenuCheckBox
{
    var() MenuSprite Blurred, Focused, SelectedBlurred, SelectedFocused;
};

struct MenuCheckBoxText extends MenuCheckBox
{
    var() MenuText Blurred;
    var() /*SBD Removed *** nonlocalized*/ MenuText Focused, SelectedBlurred, SelectedFocused;
};

enum MenuBoxFilterMode
{
    MBFM_None, // Allows Alphas & Numerics
    MBFM_Alpha, // Allows only alphas
    MBFM_Numeric, // Allows only numerics
    MBFM_AlphaNumeric, // Allows only alphas and numerics
};

struct MenuEditBox extends MenuButtonText
{
    var() int bNoSpaces; // Defaults to allow spaces
    var() MenuBoxFilterMode FilterMode;

    var() int MaxLength; // Defaults to 0 (no limit)
    var() int MinLength; // Defaults to 0

    var String TextBackup;

    var() float TimePerCursorBlink;
    var() float CursorScale;

    var() transient float BlinkAccumulator;
    var() transient int bShowCursor;
};

struct MenuBindingBox extends MenuButtonText
{
    var() MenuSprite BackgroundSelected;
    var() const int bSelected;
    
    var() String Alias;
    var() int CurrentKey;
    var() Name AnimationName;
    var() Name OnAnim; // function [WidgetName]OnAnim( Name AnimationName );
};

struct MenuStringList extends MenuWidgetBase
{
    var() MenuButtonText Template;

    var() /*SBD Removed *** nonlocalized*/ array<MenuButtonText> Items; // Must call LayoutMenuStringList when this changes

    var() float PosX1, PosY1;       // 0.0, 0.0 is upper-left 1.0, 1.0 is lower-right of the screen
    var() float PosX2, PosY2;       // 0.0, 0.0 is upper-left 1.0, 1.0 is lower-right of the screen
    var() int DisplayCount;
    var() int Position;
    
    var() Name OnScroll; // function [WidgetName]OnScroll( int ContextID );
};

struct MenuLayer extends MenuWidgetBase
{
    var() editinline Menu Layer;    // A MenuClassName will be spawned and assigned on first render if None.
    var() String MenuClassName;     // Done as a string to avoid loading classes we won't need.
};

struct MenuScrollBar extends MenuCheckBoxSprite
{
    var() int Position;
    var() int Length;
    var() int DisplayCount;
    
    var() float PosX1, PosY1, PosX2, PosY2;
    var() float MinScaleX, MinScaleY;
    var() Name OnScroll; // function [WidgetName]OnScroll( int ContextID );
    
    var() transient float ClickX, ClickY;
    
    // Natively cast to MenuActiveWidget*, assigned with LayoutMenuScrollBarEx.
    var() transient int PageUpArea, PageDownArea; 
};

struct MenuProgressBar extends MenuWidgetBase
{
	var() MenuSprite	BarBack;		// The unselected portion of the bar
	var() MenuSprite	BarTop;			// The selected portion of the bar
	var() float			BarWidth;		// The width of the bar
	var() float			BarHeight;		// The height of the bar
	var() float			Low;			// The minimum value we should see
	var() float			High;			// The maximum value we should see
	var() float			Value;			// The current value (not clamped)
};

struct MenuActorLight
{
    var() Vector Position;
    var() Color Color;
    var() float Radius;
};

struct MenuActor extends MenuWidgetBase
{
    var() editinline Actor Actor;
    var() float FOV;
    var() Array<MenuActorLight> Lights;
    var() byte AmbientGlow;
};

struct /*SBD Removed *** long*/ MenuScrollArea extends FloatBox
{
    var() Name OnScrollTop;         // function [WidgetName]OnScrollTop();
    var() Name OnScrollPageUp;      // function [WidgetName]OnScrollPageUp();
    var() Name OnScrollLineUp;      // function [WidgetName]OnScrollLineUp();
    var() Name OnScrollLineDown;    // function [WidgetName]OnScrollLineDown();
    var() Name OnScrollPageDown;    // function [WidgetName]OnScrollPageDown();
    var() Name OnScrollBottom;      // function [WidgetName]OnScrollBottom();
    var() Name OnScrollKey;         // function [WidgetName]OnScrollKey( String Key );
	var() int bStickyScrollFocus;	// If true, means that this scroll area will be the active one until a control in another one is selected
	var() const int bScrollFocus;	// Indicates that this scroll area has unconditional focus (despite mouse position).
};

enum ETransitionDir
{
    TD_None,
    TD_Out,
    TD_In,
};

struct FontMapping
{
    var int ResX, ResY;
    var Font OrigFont, DestFont;
    var float ScaleAdjustment;  
};

var private const int WidgetInFocus; // Natively cast to FMenuActiveWidget*
var private const int StructInFocus; // Natively cast to UStructProperty*

var private const int WidgetInAutoFocus; // Natively cast to FMenuActiveWidget*
var private const int StructInAutoFocus; // Natively cast to UStructProperty*

// L33t haxx for IRC menu!
var private const int DefaultFocusWidget; // Natively cast to FMenuActiveWidget*
var private const int DefaultFocusStruct; // Natively cast to UStructProperty*

var() const transient float ResScaleX, ResScaleY;
var() const transient float NormalToScreenScaleX, NormalToScreenScaleY;
var() const transient float ScreenToNormalScaleX, ScreenToNormalScaleY;

var bool bDeferAutoFocus;
var bool bDeferAutoFocusMouseMove;

var() Material MouseCursorTexture;
var() float MouseCursorScale;

var() Material WhiteTexture; // For text cursors.

var() bool bAcceptInput;
var() float MouseRepeatDelayInitial;
var() float MouseRepeatDelaySubsequent;
var() transient float MouseRepeatDelayCurrent;

var() transient String KeyQueue;
var() float KeyQueueTimeout; // For using keys to jump down string lists

var() const class<MenuVirtualKeyboard> VirtualKeyboardClass;

var() ETransitionDir CrossFadeDir;
var() float CrossFadeRate;
var() float CrossFadeLevel;

var() ETransitionDir ModulateDir;
var() float ModulateRate;
var() float ModulateLevel;
var() float ModulateMin;

var() Sound SoundTweenIn;
var() Sound SoundTweenOut;
var() Sound SoundOnFocus;
var() Sound SoundOnSelect;
var() Sound SoundOnError;

var() String ForceFeedbackOnFocus;

var() localized String StringYes;
var() localized String StringNo;
var() localized String StringOn;
var() localized String StringOff;
var() localized String StringOk;
var() localized String StringCancel;
var() localized String StringPercent;

var() float DoubleClickTime;
var transient float ClickTime;
var transient int ClickWidget; // Natively cast to FMenuActiveWidget*

var transient float CanvasSizeX, CanvasSizeY;

var transient bool bHasFocus;

// More default stuff: here's the big picture:
//
// 1) Positions interpolated down arrays.
// 2) Other properties are copied down arrays.
// 3) Sub-widgets like Blurred & Focused are copied as described above.
// 3) Properties from the MenuDefault blocks are copied.
// 4) Widgets are filled in with default values described above.
//
// Non-zero properties aren't overwritten; this is why you'll see some
// double-negative booleans, Vlad forgive me.

var(MenuDefault) /*SBD Removed *** nonlocalized*/  MenuText DefaultMenuText;
var(MenuDefault) /*SBD Removed *** nonlocalized*/  MenuDecoText DefaultMenuDecoText;
var(MenuDefault) /*SBD Removed *** nonlocalized*/  MenuButtonSprite DefaultMenuButtonSprite;
var(MenuDefault) /*SBD Removed *** nonlocalized*/  MenuButtonText DefaultMenuButtonText;
var(MenuDefault) /*SBD Removed *** nonlocalized*/  MenuButtonEnum DefaultMenuButtonEnum;
var(MenuDefault) /*SBD Removed *** nonlocalized*/  MenuCheckBoxSprite DefaultMenuCheckBoxSprite;
var(MenuDefault) /*SBD Removed *** nonlocalized*/  MenuCheckBoxText DefaultMenuCheckBoxText;
var(MenuDefault) /*SBD Removed *** nonlocalized*/  MenuEditBox DefaultMenuEditBox;
var(MenuDefault) /*SBD Removed *** nonlocalized*/  MenuBindingBox DefaultMenuBindingBox;
var(MenuDefault) /*SBD Removed *** nonlocalized*/  MenuStringList DefaultMenuStringList;
var(MenuDefault) /*SBD Removed *** nonlocalized*/  MenuScrollBar DefaultMenuScrollBar;

var(MenuDefault) /*SBD Removed *** nonlocalized*/  Array<FontMapping> FontMappings;

enum EHelpTextState
{
    HTS_InitialHidden,
    HTS_FadeUp,
    HTS_Show,
    HTS_FadeDown,
    HTS_Hidden,
};

var() MenuText HelpText;
var() EHelpTextState HelpTextState;
var() float HelpTextStateDelays[5]; // 0 - hidden, 1 - fade up, 2 - show, 3 - fade, 4 - end
var() float HelpTextOpacity;

var() bool DrawRenderBounds;
var() bool DrawScrollAreas;

// To simulate TV overscan region
var() MenuSprite OverscanWidgets[4];

// SBD - Added background movie support
var() editinline String BackgroundMovieName;
var() MenuMovie			BackgroundMovie;
var() MenuSprite		InLevelBackgroundMovieReplacement;


var() String Args;

const SECONDS_PER_HOUR = 3600;
const SECONDS_PER_MINUTE = 60;

simulated native function ClearListStickyFocus( out MenuStringList msl );

simulated native function float GetAppTime();

static simulated function String FormatTime( int Seconds )
{
    local int Minutes, Hours;
    local String Time;

    if( Seconds > SECONDS_PER_HOUR )
    {
        Hours = Seconds / SECONDS_PER_HOUR;
        Seconds -= Hours * SECONDS_PER_HOUR;

        Time = Time $ Hours $ ":";

        Minutes = Seconds / SECONDS_PER_MINUTE;
        Seconds -= Minutes * SECONDS_PER_MINUTE;

        if( Minutes >= 10 )
            Time = Time $ Minutes $ ":";
        else
            Time = Time $ "0" $ Minutes $ ":";
    
        if( Seconds >= 10 )
            Time = Time $ Seconds;
        else
            Time = Time $ "0" $ Seconds;
    
        return( Time );
    }
    else if( Seconds > SECONDS_PER_MINUTE )
    {
        Minutes = Seconds / SECONDS_PER_MINUTE;
        Seconds -= Minutes * SECONDS_PER_MINUTE;

        Time = Time $ Minutes $ ":";

        if( Seconds >= 10 )
            Time = Time $ Seconds;
        else
            Time = Time $ "0" $ Seconds;
    
        return( Time );
    }
    else
    {
        if( Seconds >= 10 )
            Time = "0:" $ Seconds;
        else
            Time = "0:0" $ Seconds;
    
        return( Time );
    }
}

static simulated function String FormatFloat( float f )
{
    local int Truncated;
    local int Whole;
    local int Fractional;
    local String Text;

    if( f < 0 )
    {
        Text = "-";
        f = -f;
    }

    Truncated = f * 100.0f;

    Whole = Truncated / 100;

    Fractional = Truncated - (Whole * 100);

    if( Fractional >= 10 )
        return( Text $ Whole $"."$Fractional );
    else
        return( Text $ Whole $"."$Fractional$"0" );
}

static simulated function String ParseToken(out String Str)
{
    local String Ret;
    local int len;

    Ret = "";
    len = 0;

	// Skip spaces and tabs.
	while( Left(Str,1)==" " || Asc(Left(Str,1))==9 )
		Str = Mid(Str, 1);

	if( Asc(Left(Str,1)) == 34 )
	{
		// Get quoted String.
		Str = Mid(Str, 1);
		while( Str!="" && Asc(Left(Str,1))!=34 )
		{
			Ret = Ret $ Mid(Str,0,1);
            Str = Mid(Str, 1);
		}
		if( Asc(Left(Str,1))==34 )
			Str = Mid(Str, 1);
	}
	else
	{
		// Get unquoted String.
		for( len=0; (Str!="" && Left(Str,1)!=" " && Asc(Left(Str,1))!=9); Str = Mid(Str, 1) )
            Ret = Ret $ Mid(Str,0,1);
	}

	return Ret;
}

simulated event TransferTransientElements(Menu m)
{
	local MenuBase mb;

	if ( m == None )
	{
		return;
	}
	
	Super.TransferTransientElements(m);

	if ( BackgroundMovie.MovieTex != None )
	{
		// We've still got our own movie.
		return;
	}
	
	if ( m.IsA('MenuBase') )
	{
		mb = MenuBase(m);
		if ( (mb != None) && (mb.BackgroundMovieName != "") && (mb.BackgroundMovieName == BackgroundMovieName) )
		{
			BackgroundMovie.MovieTex = mb.BackgroundMovie.MovieTex;
			mb.BackgroundMovie.MovieTex = None;
		}
	}
}

simulated function Init( String A )
{
	local String LevelName;
    Args = A;
    
    GetPlayerOwner().GetCurrentMapName( LevelName );
	LevelName = Caps( LevelName );
	// Strip any extension off the level name
	if ( Caps(Right(LevelName, 4)) == ".CTM" )
		LevelName = Left(LevelName, Len(LevelName) - 4);
    
    if ( (Caps(LevelName) != "ENTRY") && (Caps(LevelName) != "MPCUSTOM") )
    {
		if ( BackgroundMovieName != "" )
		{
			InLevelBackgroundMovieReplacement.bHidden = 0;
			
			if ( Level.IsDedicatedServer() )
				InLevelBackgroundMovieReplacement.DrawColor.A = 255;
		}
    }
    else
    {
		InLevelBackgroundMovieReplacement.bHidden = 1;
	
		StartBackgroundMovie();	
	}
}

simulated event StartBackgroundMovie()
{
	if ( (BackgroundMovieName != "") && (BackgroundMovie.MovieTex == None) )
	{
		BackgroundMovie.MovieTex=new class'MovieTexture';
		BackgroundMovie.MovieTex.MovieFilename=BackgroundMovieName;
		BackgroundMovie.MovieTex.MaxFrameRate = 0.0;
		BackgroundMovie.MovieTex.InitializeMovie();
	}
	else if ( BackgroundMovie.MovieTex != None && !BackgroundMovie.MovieTex.Movie.IsPlaying() )
	{
		log("starting menu movie");
		BackgroundMovie.MovieTex.Movie.Play(BackgroundMovie.MovieTex.MovieFilename, False, True);
	}    
}

simulated event StopBackgroundMovie()
{
	if ( BackgroundMovie.MovieTex != None )
	{
		log("stopping menu movie");
		BackgroundMovie.MovieTex.Movie.StopNow();
	}
}

simulated function DestroyMenu()
{
	if ( !bPersistent )
	{
		StopBackgroundMovie();
	}
	
	Super.DestroyMenu();		
}

native simulated event DrawMenu( Canvas C, bool HasFocus );

simulated event PostEditChange();

simulated function Plane GetModulationColor()
{
    local Plane C;

    C.X = ModulateLevel;
    C.Y = ModulateLevel;
    C.Z = ModulateLevel;
    C.W = CrossFadeLevel;

    return( C );
}

simulated function bool IsVisible()
{
    return( (CrossFadeDir != TD_None) || (CrossFadeLevel != 0.0) );
}

native simulated function HandleInputLeft();
native simulated function HandleInputRight();
native simulated function HandleInputUp();
native simulated function HandleInputDown();
native simulated function HandleInputSelect();
native simulated function HandleInputStart();
native simulated function HandleInputBack();
native simulated function bool HandleInputKey( Interactions.EInputKey Key );
native simulated function bool HandleInputKeyRaw( Interactions.EInputKey Key );
native simulated function HandleInputMouseDown();
native simulated function HandleInputMouseUp();
native simulated function HandleInputMouseMove();
native simulated function bool HandleInputWheelUp();
native simulated function bool HandleInputWheelDown();

native simulated function FocusOnNothing();
native simulated function FocusOnWidget( MenuActiveWidget Widget );
native simulated function SelectWidget( MenuActiveWidget Widget );
native simulated function DeSelectCheckbox( MenuCheckBox Checkbox );

native simulated function SetDefaultFocusWidget( MenuActiveWidget Widget );

native simulated function LayoutMenuStringList( MenuStringList StringList );
native simulated function LayoutMenuDecoText( MenuDecoText DecoText );
native simulated function LayoutMenuScrollBar( MenuScrollBar MenuScrollBar );
native simulated function LayoutMenuScrollBarEx( MenuScrollBar MenuScrollBar, MenuActiveWidget PageUpArea, MenuActiveWidget PageDownArea );

native simulated function int GetWrappedTextHeight( Canvas C, MenuText MenuText );

native simulated event OverlayMenu( Menu InOverlayMenu, optional String Args );
native simulated event GotoMenu( Menu InTravelMenu, optional String Args );
native simulated event CallMenu( Menu InTravelMenu, optional String Args );
native simulated event CloseMenu();

native simulated function CenterArray( out MenuWidgetBase First, float Spacing, optional float Center );
native simulated function FitBorderBox( out MenuSprite BorderBox, out MenuWidgetBase First, optional float ExtraSpacing );

function ClientPlayForceFeedback( String EffectName )
{
    local PlayerController PC;
    PC = GetPlayerOwner();
    if( (PC != None) && PC.bEnableGUIForceFeedback )
        PC.ClientPlayForceFeedback( EffectName );
}

simulated event OnFocusChange()
{
    local PlayerController PC;

    if( SoundOnFocus == None )
        return;

    PC = PlayerController( Owner );

    if( PC == None )
        return;

    PC.PlaySound( SoundOnFocus );

    //if( WidgetInFocus != 0 )
    //    ClientPlayForceFeedback( ForceFeedbackOnFocus );
}

simulated event OnSelectionChange()
{
    local PlayerController PC;

    if( SoundOnSelect == None )
        return;

    PC = PlayerController( Owner );
    assert( PC != None );

    PC.PlaySound(SoundOnSelect);
}

simulated function ChildAnimEnd( Actor A, int i );

static simulated function String ParseOption( String Options, String InKey )
{
    return( class'GameInfo'.static.ParseOption( Options, InKey ) );
}

// workers
function PlayerController GetPlayerOwner()
{
    return PlayerController(Owner);
}

function Actor GetPersistSpawner() // todo: entry's levelinfo used to serve this purpose
{
    return self;//PlayerController(Owner);
}

simulated function AddListItem( out MenuStringList msl, String s, int context )
{
    local int i;

    i = msl.Items.Length;
    msl.Items[i].Focused.Text = s;
    msl.Items[i].Blurred.Text = s;
    msl.Items[i].ContextID = context;
}

simulated function String GetSelectedListItem( out MenuStringList msl )
{
    local int i;
    for( i=0; i<msl.Items.Length; i++ )
    {
        if( msl.Items[i].bHasFocus == 1 )
            return msl.Items[i].Focused.Text;
    }
    return "";
}

simulated function DoNothing();

simulated function SetupActor(out Actor playerActor, out Actor weaponActor, String className, String weapName, vector actorPos, rotator actorRot, optional float size)
{
    //local xUtil.PlayerRecord PlayerRecord;
    //local Mesh m;
    local StaticMesh sm;
//    local xUtil.WeaponRecord wRec;
    local class<Actor> menuSkelMeshActor;
    local class<Actor> menuStatMeshActor;

    assert( className != "" );

    //PlayerRecord = class'xUtil'.static.FindPlayerRecord( charName );
    //assert( PlayerRecord.MeshName != "" );

	/*
    menuSkelMeshActor = class<Actor>(DynamicLoadObject("XInterface.MenuActorSkeletalMesh", class'Class'));
    playerActor = Spawn( menuSkelMeshActor, self,, actorPos, actorRot );
    assert( playerActor != None );

    m = Mesh( DynamicLoadObject( meshName, class'Mesh' ) );
    assert( m != None );

    playerActor.LinkMesh( m );
	*/

	menuSkelMeshActor = class<Actor>(DynamicLoadObject(className, class'Class'));
	playerActor = Spawn( menuSkelMeshActor, self,, actorPos, actorRot );

    //playerActor.Skins[0] = Material(DynamicLoadObject(PlayerRecord.BodySkinName, class'Material'));
    //playerActor.Skins[1] = Material(DynamicLoadObject(PlayerRecord.FaceSkinName, class'Material'));
    playerActor.Style = STY_AlphaZ;

    if (size > 0.0 )
        playerActor.SetDrawScale( size );

	if (weapName == "")
		return;

    menuStatMeshActor = class<Actor>(DynamicLoadObject("XInterface.MenuActorStaticMesh", class'Class'));
    weaponActor = Spawn( menuStatMeshActor, playerActor );
    assert( weaponActor != None );

	 // TODO: removed weap afinity
//      if( weapName == "" )
//  		 weapName = PlayerRecord.WepAffinity.WepString;

//      wRec = class'xUtil'.static.FindWeaponRecord(weapName);
//      sm = StaticMesh(DynamicLoadObject(wRec.AttachmentMeshName, class'StaticMesh'));
	sm = StaticMesh(DynamicLoadObject( weapName, class'StaticMesh' ));

    weaponActor.SetStaticMesh(sm);
    
 /*   if (size > 0.0 )
        weaponActor.SetDrawScale( wRec.AttachmentDrawScale*size );
    else
        weaponActor.SetDrawScale( wRec.AttachmentDrawScale );
*/
    weaponActor.bHidden = false;

    if( !playerActor.AttachToBone( weaponActor, 'weaponAttach_R' ) )
        log( "Couldn't attach weapon!", 'Error' );

    //playerActor.LoopAnim( 'RelaxBreatheIdle2' );
	Pawn(playerActor).CurrentIdleState = AS_Relaxed;
}


simulated exec function Menu( String Command )
{
	local int i;

    if( Command ~= "renderbounds" )
        DrawRenderBounds = !DrawRenderBounds;
    else if( Command ~= "scrollareas" )
        DrawScrollAreas = !DrawScrollAreas;
    else if( Command ~= "overscan" )
    {
		if( OverscanWidgets[i].bHidden == 0 )
		{
			for( i = 0; i < ArrayCount(OverscanWidgets); i++ )
				OverscanWidgets[i].bHidden = 1;
		}
		else
		{
			for( i = 0; i < ArrayCount(OverscanWidgets); i++ )
				OverscanWidgets[i].bHidden = 0;
				
			// Left bar    
			OverscanWidgets[0].ScaleX = (1.f - 0.93f) * 0.5f;

			// Right bar    
			OverscanWidgets[1].ScaleX = (1.f - 0.93f) * 0.5f;

			// Top bar    
			OverscanWidgets[2].ScaleY = (1.f - 0.85f) * 0.5f;

			// Bottom bar    
			OverscanWidgets[3].ScaleY = (1.f - 0.85f) * 0.5f;
		}
    }
}

simulated function CallMenuClass( String MenuClassName, optional String Args )
{
    CallMenu( Spawn( class<Menu>( DynamicLoadObject( MenuClassName, class'Class' ) ), Owner ), Args );
}

simulated function OverlayMenuClass( String MenuClassName, optional String Args )
{
    OverlayMenu( Spawn( class<Menu>( DynamicLoadObject( MenuClassName, class'Class' ) ), Owner ), Args );
}

simulated function GotoMenuClass( String MenuClassName, optional String Args )
{
    GotoMenu( Spawn( class<Menu>( DynamicLoadObject( MenuClassName, class'Class' ) ), Owner ), Args );
}

simulated function string MakeQuotedString( string in )
{
    return "\""$in$"\"";
}

simulated function SetPositionSprite( out MenuSprite w, float x, float y )
{
    w.PosX = x;
    w.PosY = y;
}

simulated function SetPositionEditBox( out MenuEditBox w, float x, float y )
{
    w.Blurred.PosX = x;
    w.Focused.PosX = x;
    w.BackgroundBlurred.PosX = x;
    w.BackgroundFocused.PosX = x;

    w.Blurred.PosY = y;
    w.Focused.PosY = y;
    w.BackgroundBlurred.PosY = y;
    w.BackgroundFocused.PosY = y;
}

simulated function SetPositionButtonText( out MenuButtonText w, float x, float y )
{
    w.Blurred.PosX = x;
    w.Focused.PosX = x;
    w.BackgroundBlurred.PosX = x;
    w.BackgroundFocused.PosX = x;

    w.Blurred.PosY = y;
    w.Focused.PosY = y;
    w.BackgroundBlurred.PosY = y;
    w.BackgroundFocused.PosY = y;
}

simulated function SetPositionButtonSprite( out MenuButtonSprite w, float x, float y )
{
    w.Blurred.PosX = x;
    w.Focused.PosX = x;
    w.BackgroundBlurred.PosX = x;
    w.BackgroundFocused.PosX = x;

    w.Blurred.PosY = y;
    w.Focused.PosY = y;
    w.BackgroundBlurred.PosY = y;
    w.BackgroundFocused.PosY = y;
}

simulated function SetPositionButtonEnum( out MenuButtonEnum w, float x, float y )
{
    w.Blurred.PosX = x;
    w.Focused.PosX = x;
    w.BackgroundBlurred.PosX = x;
    w.BackgroundFocused.PosX = x;

    w.Blurred.PosY = y;
    w.Focused.PosY = y;
    w.BackgroundBlurred.PosY = y;
    w.BackgroundFocused.PosY = y;
}

simulated function string LoadSaveCommand(string cmd, int dev, optional string opts)
{
    return ConsoleCommand("LOADSAVE "$cmd$" DEVICE="$dev$" "$opts);
}

simulated function string ProfilesCommand(string cmd, int dev, optional string opts)
{
    return ConsoleCommand("PROFILE "$cmd$" DEVICE="$dev$" "$opts);
}

simulated function UpdateDecoField(out MenuDecoText decoText, string Marker, string info)
{
    local int Row;
    for (Row=0; Row<decoText.Text.Rows.Length; Row++)
        if (UpdateTextField(decoText.Text.Rows[Row], Marker, info))
            return;
}

simulated function OverlayErrorMessageBox( String OperationName )
{
    OverlayMenuClass( "XInterfaceLive.MenuLiveErrorMessage", OperationName );
}

simulated function string GetProfileName()
{
    return ProfilesCommand("GETCURRENTPROFILE", 0);
}

simulated function string CheckName(string inName)
{
	Log("Checking name "$inName);
    return ProfilesCommand("CHECK_NAME NAME=" $ inName, 0);
}

simulated function xUtil.PlayerRecord GetUnusedPlayerRecord()
{
    local int i;
    local xUtil.PlayerRecord pr;

    do 
    {
        pr = class'xUtil'.static.GetRandPlayerRecord(true);
        if (pr.DefaultName ~= CheckName(pr.DefaultName))
            break;
    }
    until (++i > 100);

    return pr;
}

simulated function GetProfileList( out Array<string> Profiles )
{
    local string profileNames;
    local int cnt;
    local int index;
    local int isSharing;

	/* SBD Removed ***
    if (PlayerController(Owner).IsSharingScreen())
        isSharing = 1;
	*/

    profileNames = ProfilesCommand("LIST", 0, "UNIQUE="$isSharing);

    if (profileNames == "")
        return;

    do
    {
        index = InStr(profileNames, ",");
        if (index < 0)
        {
            Profiles[cnt] = profileNames;
            break;
        }
        
        Profiles[cnt++] = Left(profileNames, index);
        ProfileNames = Right(profileNames, Len(ProfileNames) - index - 1);
    }
    until (false);
}

simulated function bool AllowDefaultProfileUse()
{
    local array<string> profiles;
    local int i;

    ProfilesCommand("USING_DEFAULT_PROFILE SET=0", 0);

    GetProfileList(profiles);

    // if there are valid profiles avail, must select one
    for (i=0; i < profiles.Length; ++i)
    {
        if (bool(ProfilesCommand("VERIFY_PROFILE NAME="$profiles[i], 0)) && 
            (/*SBD Removed *** !PlayerController(Owner).IsSharingScreen() || */
            !bool(ProfilesCommand("PROFILE INUSE", 0, "NAME="$profiles[i]))))
        {
            return false;
        }
    }

    // check profile limit & free space
    if (bool(ProfilesCommand("REACHED_PROFILE_LIMIT", 0)) ||
        int(ProfilesCommand("SPACE FREE", 0)) < int(ProfilesCommand("SPACE PROFILE", 0)))
    {
        ProfilesCommand("USING_DEFAULT_PROFILE SET=1", 0);
        RandomizeDefaultProfile();
        return true;
    }

    // must create one
    return false;
}

simulated function RandomizeDefaultProfile()
{
    local xUtil.PlayerRecord pr;

    pr = GetUnusedPlayerRecord();
    
	/* SBD Removed ***
    if (!PlayerController(Owner).IsSharingScreen())
    {
	    UpdateURL("Name", pr.DefaultName, true);
	    UpdateURL("Character", pr.DefaultName, true);
    }
	

    PlayerController(Owner).UpdatePlayer(pr.DefaultName, pr.DefaultName);
	*/
}

function float GetCanvasXSize()
{
	return CanvasSizeX;
}

function float GetCanvasYSize()
{
	return CanvasSizeY;
}

simulated native event string RetrieveLocalizedKeyName(int key);


// 4978008 Asshole-Rog-Orc-Fem-Cha ascended to demigoddess-hood. 302 [312]

defaultproperties
{
     bDeferAutoFocus=True
     bDeferAutoFocusMouseMove=True
     WhiteTexture=Texture'Engine.MenuWhite'
     bAcceptInput=True
     MouseRepeatDelayInitial=0.5
     MouseRepeatDelaySubsequent=0.05
     KeyQueueTimeout=0.5
     CrossFadeRate=2
     CrossFadeLevel=1
     ModulateRate=2
     ModulateLevel=1
     ModulateMin=0.2
     StringYes="YES"
     StringNo="NO"
     StringOn="ON"
     StringOff="OFF"
     StringOk="OK"
     StringCancel="CANCEL"
     StringPercent="%"
     DoubleClickTime=0.5
     HelpTextStateDelays(0)=1.5
     HelpTextStateDelays(1)=0.5
     HelpTextStateDelays(2)=3
     HelpTextStateDelays(3)=0.5
     HelpTextStateDelays(4)=4
     OverscanWidgets(0)=(WidgetTexture=Texture'Engine.MenuWhite',DrawColor=(B=128,G=128,R=128,A=255),ScaleY=1,ScaleMode=MSCM_Fit,Pass=10,bHidden=1)
     OverscanWidgets(1)=(DrawPivot=DP_UpperRight,PosX=1,ScaleY=1,bHidden=1)
     OverscanWidgets(2)=(ScaleX=1,bHidden=1)
     OverscanWidgets(3)=(DrawPivot=DP_LowerLeft,PosY=1,ScaleX=1,bHidden=1)
     BackgroundMovie=(DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,PosX=0.5,PosY=0.5,ScaleX=1,ScaleY=1,ScaleMode=MSCM_Fit)
     InLevelBackgroundMovieReplacement=(WidgetTexture=Texture'GUIContent.Menu.blacksquare',DrawColor=(A=192),bHidden=1,Style="FullScreen")
     bAlwaysTick=True
}

