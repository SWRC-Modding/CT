class LatentScriptedAction extends ScriptedAction
	abstract
	native;

function DisplayDebug(Canvas Canvas, out float YL, out float YPos)
{
	Canvas.DrawText("Action "$GetActionString(), false);
	YPos += YL;
	Canvas.SetPos(4,YPos);
}

function bool InitActionFor(ScriptedController C)
{
	C.CurrentAction = self;
	return true;
}

//*****************************************************************************************
// Action Completion Queries

function bool CompleteWhenTriggered()
{
	return false;
}

function bool CompleteOnAnim(int Channel)
{
	return false;
}

function bool CompleteWhenTimer()
{
	return false;
}

function bool WaitForPlayer()
{
	return false;
}

function bool TickedAction()
{
	return false;
}

//*****************************************************************************************
// Action Queries

function bool StillTicking(ScriptedController C, float DeltaTime)
{
	return false;
}

function bool MoveToGoal()
{
	return false;
}

function bool TurnToGoal()
{
	return false;
}

function Actor GetMoveTargetFor(ScriptedController C)
{
	return C.SequenceScript.GetMoveTarget();
}

function float GetDistance()
{
	return 0;
}


defaultproperties
{
}

