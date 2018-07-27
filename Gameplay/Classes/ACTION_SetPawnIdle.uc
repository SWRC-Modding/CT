class ACTION_SetPawnIdle extends ScriptedAction;

var(Action) Pawn.EIdleState IdleState;

function bool InitActionFor(ScriptedController C)
{
	// play appropriate animation
	C.Pawn.CurrentIdleState = IdleState;
	C.Pawn.ChangeAnimation();

	return false;	
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@IdleState$" // "$Comment;
	return ActionString@IdleState;
}


defaultproperties
{
     ActionString="set pawn idle"
     bValidForTrigger=False
}

