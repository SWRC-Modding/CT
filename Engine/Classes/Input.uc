//=============================================================================
// Input.
//=============================================================================
class Input extends Subsystem;

defaultproperties
{
     MouseWheelDown="PrevWeapon"
     MouseWheelUp="NextWeapon"
     MouseY="Count bYAxis | Axis aMouseY Speed=1.0"
     MouseX="Count bXAxis | Axis aMouseX Speed=1.0 "
     RightBracket="InventoryNext"
     Backslash="ChangeHud"
     LeftBracket="InventoryPrevious"
     Slash="NextWeapon"
     Minus="ShrinkHUD"
     Equals="GrowHUD"
     F12="exit"
     F11="shot"
     F9="QuickLoad"
     F8="ToggleSpectatorMode"
     F7="SwitchTeam"
     F5="QuickSave"
     F4="CancelAllMarkers	;Recall"
     F3="SquadEngage		;Secure Position"
     F2="SetStanceDefensive	;Form Up"
     F1="SetStanceOffensive	;Search & Destroy"
     NumPad6="set playercontroller bZeroRoll True | set playercontroller rotation (pitch=0,yaw=0,roll=0)"
     NumPad5="set playercontroller bZeroRoll False | set playercontroller rotation (pitch=-16384,yaw=32768,roll=16384) | onrelease shot Cube5"
     NumPad4="set playercontroller bZeroRoll False | set playercontroller rotation (pitch=16384,yaw=0,roll=16384) | onrelease shot Cube4"
     NumPad3="set playercontroller bZeroRoll False | set playercontroller rotation (pitch=0,yaw=-16384,roll=0) | onrelease shot Cube3"
     NumPad2="set playercontroller bZeroRoll False | set playercontroller rotation (pitch=0,yaw=16384,roll=32768) | onrelease shot Cube2"
     NumPad1="set playercontroller bZeroRoll False | set playercontroller rotation (pitch=0,yaw=32768,roll=-16384) | onrelease shot Cube1"
     NumPad0="set playercontroller bZeroRoll False | set playercontroller rotation (pitch=0,yaw=0,roll=16384) | onrelease shot Cube0"
     Z="fov 0"
     Y="TeamTalk"
     X="ToggleHeadlamp"
     W="MoveForward"
     V="SpeechMenuToggle"
     T="Talk"
     S="MoveBackward"
     R="ForceReload"
     Q="SwitchGrenade 6"
     F="Use | onrelease StopUse"
     E="AltFire"
     D="StrafeRight"
     C="Duck"
     A="StrafeLeft"
     5="SwitchWeapon 5"
     4="SwitchWeapon 3"
     3="SwitchWeapon 2"
     2="SwitchWeapon 1"
     1="SwitchWeapon 4"
     Delete="LookDown"
     Insert="LookUp"
     Down="MoveBackward"
     Right="StrafeRight"
     Up="MoveForward"
     Left="StrafeLeft"
     Home="CenterView | ACTOR ALIGN SNAPTOFLOOR ALIGN=0"
     End="CenterView | ACTOR ALIGN SNAPTOFLOOR ALIGN=1"
     PageDown="TurnLeft"
     PageUp="TurnRight"
     Space="Jump"
     Escape="ShowMenu"
     Pause="ShowMenu"
     Ctrl="ShowGameStats"
     Shift="Walking"
     Enter="Use | onrelease StopUse   "
     Tab="ToggleWeapon"
     MiddleMouse="fov 0"
     RightMouse="ThrowGrenade"
     LeftMouse="Fire | onrelease StopFire"
     Aliases(0)=(Command="Button bFire | Fire",Alias="Fire")
     Aliases(1)=(Command="Button bAltFire | AltFire",Alias="AltFire")
     Aliases(2)=(Command="Axis aBaseY  Speed=+1200",Alias="MoveForward")
     Aliases(3)=(Command="Axis aBaseY  Speed=-1200",Alias="MoveBackward")
     Aliases(4)=(Command="Axis aBaseX Speed=-1000",Alias="TurnLeft")
     Aliases(5)=(Command="Axis aBaseX  Speed=+1000",Alias="TurnRight")
     Aliases(6)=(Command="Axis aStrafe Speed=-1200",Alias="StrafeLeft")
     Aliases(7)=(Command="Axis aStrafe Speed=+1200",Alias="StrafeRight")
     Aliases(8)=(Command="Jump | Axis aUp Speed=+1200",Alias="Jump")
     Aliases(9)=(Command="Button bDuck | Axis aUp Speed=-1200",Alias="Duck")
     Aliases(10)=(Command="Button bLook",Alias="Look")
     Aliases(11)=(Command="Toggle bLook",Alias="LookToggle")
     Aliases(12)=(Command="ActivateItem",Alias="InventoryActivate")
     Aliases(13)=(Command="NextItem",Alias="InventoryNext")
     Aliases(14)=(Command="PrevItem",Alias="InventoryPrevious")
     Aliases(15)=(Command="Axis aLookUp Speed=+1000",Alias="LookUp")
     Aliases(16)=(Command="Axis aLookUp Speed=-1000",Alias="LookDown")
     Aliases(17)=(Command="Button bSnapLevel",Alias="CenterView")
     Aliases(18)=(Command="Button bRun",Alias="Walking")
     Aliases(19)=(Command="Button bStrafe",Alias="Strafe")
     Aliases(20)=(Command="Button bFreeLook",Alias="FreeLook")
     Aliases(21)=(Command="Button bTurnToNearest",Alias="TurnToNearest")
     Aliases(22)=(Command="Button bTurn180",Alias="Turn180")
}

