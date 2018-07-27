class ACTION_PlayLocalSound extends ScriptedAction;

var(Action)		sound	Sound;

function bool InitActionFor(ScriptedController C)
{
	local PlayerController P;

	// play appropriate sound
		ForEach C.DynamicActors(class'PlayerController', P)
			P.ClientPlaySoundLocally(Sound);
	return false;	
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@Sound$" // "$Comment;
	return ActionString@Sound;
}


defaultproperties
{
     ActionString="play sound"
}

