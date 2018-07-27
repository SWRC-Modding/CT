class ACTION_Revive extends ScriptedAction;

var(Action) name				PawnTag;

function bool InitActionFor(ScriptedController C)
{
	local Pawn Pawn;

	if ( PawnTag != '' )
	{
		ForEach C.AllActors(class'Pawn',Pawn,PawnTag)
		{
			if (Pawn.IsInState('Incapacitated'))
				Pawn.Revive();
		}
	}
	return false;	
}

function string GetActionString()
{
	if (Comment != "")
		return ActionString@PawnTag$" // "$Comment;
	return ActionString@PawnTag;
}


defaultproperties
{
     ActionString="Revive"
}

