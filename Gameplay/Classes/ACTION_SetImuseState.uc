class ACTION_SetImuseState extends ScriptedAction;

var(Action) Sound Music;
var(Action) bool  OverrideFadeDefaults;
var(Action) float FadeInTime;
var(Action) float FadeOutTime;
var(Action) Actor.EMusicFadeType FadeInType;
var(Action) Actor.EMusicFadeType FadeOutType;

function bool InitActionFor(ScriptedController C)
{
	local Controller A;

	For ( A=C.Level.ControllerList; A!=None; A=A.nextController )
		if ( A.IsA('PlayerController') && PlayerController(A).ViewTarget != None )
			PlayerController(A).ViewTarget.SetIMuseState( Music, OverrideFadeDefaults, FadeInTime, FadeInType, FadeOutTime, FadeOutType );
	return false;	
}


defaultproperties
{
     ActionString="set iMuse state"
}

