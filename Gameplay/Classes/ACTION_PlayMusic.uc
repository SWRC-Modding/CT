class ACTION_PlayMusic extends ScriptedAction;

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
		if ( A.IsA('PlayerController') )
			PlayerController(A).PlayMusic( Music, OverrideFadeDefaults, FadeInTime, FadeInType, FadeOutTime, FadeOutType );

	return false;	
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@Music$" // "$Comment;
	return ActionString@Music;
}


defaultproperties
{
     ActionString="play music"
}

