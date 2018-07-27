class ACTION_DisplayCriticalMessage extends ScriptedAction;

var(Action) string		Message;
var(Action) float		Duration;
var(Action) color		Color;

function bool InitActionFor(ScriptedController C)
{
	local PlayerController Player;
	ForEach C.DynamicActors( class 'PlayerController', Player )
	{
		if( Player.myHUD != None )
			Player.myHUD.AddCriticalMessage( Message, Duration, Color );
	}

	return false;	
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@Message$" // "$Comment;
	return ActionString@Message;
}


defaultproperties
{
     Duration=3
     Color=(B=200,G=200,R=200,A=200)
     ActionString="display critical message"
}

