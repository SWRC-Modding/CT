class ACTION_EndSection extends ScriptedAction;

function ProceedToNextAction(ScriptedController C)
{
	if ( C.IterationCounter > 0 )
	{
		C.ActionNum = C.IterationSectionStart;
		C.IterationCounter--;
	}
	else
	{
		C.ActionNum++;
		C.IterationSectionStart = -1;
	}
}

function bool EndsSection()
{
	return true;
}


defaultproperties
{
     ActionString="end section"
}

