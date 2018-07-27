//=============================================================================
// Menu base class
// Copyright 2001 Digital Extremes - All Rights Reserved.
// Confidential.
//=============================================================================
class Menu extends Actor
    abstract
    transient
    native
    dependsOn(Interactions)
    config
;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

// The UnderlayMenu variables keep a stack of menus to be drawn over top of
// eachother. The head of the list is pointed to by the Console's CurMenu
// pointer and is the foremost window (the one that receives input). They're
// drawn in reverse order (from the tail of the list/bottom of the stack on
// upwards).
//
// When a menu wants to spawn an overlay menu, it takes the newly spawned 
// menu and sets the UnderlayMenu pointer to self. Then, the next time it returns
// from the Draw event it returns the new menu.

// The console draw code has been adapted to detect overlay menus returning
// none from their draw events; if an menu returns None, the console first
// checks the menu's UnderlayMenu pointer and, if set, will destroy the current
// menu and then continue menuing on in the Next menu. If the stack is empty
// the console jumps out of menuing mode.

// This does mean, however that should a background menu decide that it wants
// to close itself or change to another menu, it will be ignored until it is
// at the top of the stack again. Too bad. Don't do that.

var() editinline Menu UnderlayMenu;

// The PreviousMenu variables keep a list of menus in series so that when 
// one closes the previous one can be brought back up.

var() editinline Menu PreviousMenu;

// The TravelMenu variable will be returned by the next DrawMenu call.

var() editinline Menu TravelMenu;

var() editinline Sound BackgroundMusic;
var() bool			   bBackgroundMusicDuringLevel;

// The LastInputSource variable is managed by the Console and can be used
// to modify things like auto-focus behaviour based on the form of input given.

enum InputSource
{
    IS_None,
    IS_Mouse,
    IS_Keyboard,
    IS_Controller
};

var transient InputSource LastInputSource;

enum InputModifier
{
    IM_None,
    IM_Alt,
    IM_Shift,
    IM_Ctrl
};

var transient InputModifier LastInputModifier;

var() bool bRenderLevel;
var() bool bPersistent;
var() bool bNetworkOnly; // this menu is only valid if there is a network connection
var() bool bFullscreenOnly; // this menu must be drawn fullscreen
var() bool bIgnoresInput;
var() bool bFullscreenExclusive;
var() float FullScreenOnlyTime;
var() int FullscreenPriority;

var() bool bShowMouseCursor;
var() bool bHideMouseCursor;	// Explicitly hide the mouse cursor
var() float MouseX, MouseY;

// This is a lovely hack that allows us to bind space to "Select" but have it still
// work with editboxes and key auto-repeat.
var() bool bRawKeyboardInput;

var() bool bVignette;

var() bool bRequiresEthernetLink;

simulated event PreBeginPlay()
{
    Super.PreBeginPlay();
    TravelMenu = self;
}

// Init is called after a the code constructing a menu has finished initializing it.
// It should generally be used instead of PreBeginPlay.

simulated event Init( String Args );

simulated event DrawMenu ( Canvas C, bool HasFocus );

simulated function Plane GetModulationColor();
simulated function bool IsVisible()
{
    return true;
}

// This event will be sent to a menu when a menu closes and focus is returned to this menu,
// or, when the last menu in the call stack has finished fading out, it will get a MenuClosed
// event with (self) as it's ClosingMenu.

simulated function bool MenuClosed( Menu ClosingMenu ); //return true if handled

simulated function HandleInputLeft();
simulated function HandleInputRight();
simulated function HandleInputUp();
simulated function HandleInputDown();

simulated function HandleInputSelect();  // The "A" Button or whatever (Space on PC)
simulated function HandleInputStart();   // The "Start" Button
simulated event HandleInputBack();    // The "B" or "Back" Button (Escape on PC)

simulated function bool HandleInputKey( Interactions.EInputKey Key ); // Gives menus a chance to ride the raw input. Return true to consume.
simulated function bool HandleInputKeyRaw( Interactions.EInputKey Key );
simulated function HandleInputMouseDown();
simulated function HandleInputMouseUp();
simulated function HandleInputMouseMove();

simulated function bool HandleInputWheelUp();
simulated function bool HandleInputWheelDown();

simulated function bool HandleInputGamePad( String ButtonName )
{
	return( false );
}

simulated function ReopenInit();

simulated function DestroyMenu()
{
    if (!bPersistent)
    {
        //log(Name$".Destroy()", 'Log');
        Destroy();
    }
}

event PostLevelChange();

simulated function bool IgnoreKeyEvent( Interactions.EInputKey Key, Interactions.EInputAction Action )
{
    return( false );
}

simulated function bool KeyIsBoundTo( Interactions.EInputKey Key, String Binding )
{
    local String KeyName, KeyBinding;
    
    KeyName = PlayerController(Owner).ConsoleCommand( "KEYNAME "$Key );
    
    if( KeyName == "" )
        return( false );
    
    KeyBinding = PlayerController(Owner).ConsoleCommand( "KEYBINDING "$KeyName );

    if( KeyBinding ~= Binding )
        return( true );

    return( false );
}

// This is a shameful hack for the scoreboards.
simulated function bool ToggleVisibility();
simulated function CallMenu( Menu InTravelMenu, optional String Args );

function float GetCanvasXSize();
function float GetCanvasYSize();

function UpdateLoadingProgress(float LoadingRatioCompleted) {}
function SetInfoOptions(String InfoPic, String Title, String InfoText, String NewLevel, bool ShowHints) {}
function bool PauseOnLoadingFinish() { return False; }

simulated event TransferTransientElements(Menu m) {}



cpptext
{
	// NOTE we have to expose this directly through C++, since
	//		we will want to draw the menu when actor script calls might be
	//		unavailable (during level loads).
	virtual void NativeDrawMenu( UCanvas* C, UBOOL HasFocus ) {}

}

defaultproperties
{
     bShowMouseCursor=True
     DrawType=DT_None
     bUnlit=True
     bGameRelevant=True
     RemoteRole=ROLE_None
     CollisionRadius=0
     CollisionHeight=0
}

