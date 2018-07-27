class ACTION_WaitForTimer extends LatentScriptedAction;

var(Action) float PauseTime;

function bool InitActionFor(ScriptedController C)
{
	C.CurrentAction = self;
	C.SetTimer(PauseTime, false);
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
		return ActionString@PauseTime$" // "$Comment;
	return ActionString@PauseTime;
}


defaultproperties
{
     ActionString="Wait for timer"
}

