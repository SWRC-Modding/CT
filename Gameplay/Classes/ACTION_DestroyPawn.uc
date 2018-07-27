class ACTION_DestroyPawn extends ScriptedAction;

function bool InitActionFor(ScriptedController C)
{
	C.DestroyPawn();
	return true;
}


defaultproperties
{
     ActionString="destroy pawn"
     bValidForTrigger=False
}

