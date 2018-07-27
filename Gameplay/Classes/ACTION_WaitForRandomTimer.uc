class ACTION_WaitForRandomTimer extends LatentScriptedAction;

var(Action) range PauseTime;

function bool InitActionFor(ScriptedController C)
{
	C.CurrentAction = self;
	C.SetTimer( RandRange( PauseTime.Min, PauseTime.Max ), false);
	return true;
}

function bool CompleteWhenTriggered()
{
	return true;
}

function bool CompleteWhenTimer()
{
	return true;
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@PauseTime.max$" // "$Comment;
	return ActionString@PauseTime.max;
}


defaultproperties
{
     ActionString="Wait for timer"
}

