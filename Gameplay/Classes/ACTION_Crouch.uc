class ACTION_Crouch extends ScriptedAction;

function bool InitActionFor(ScriptedController C)
{
	C.Pawn.ShouldCrouch(true);
	return false;	
}


defaultproperties
{
     ActionString="crouch"
     bValidForTrigger=False
}

