class ACTION_IfRandomPct extends ScriptedAction;

var(Action) float Probability;

function ProceedToNextAction(ScriptedController C)
{
	C.ActionNum++;
	if ( FRand() > Probability )
		ProceedToSectionEnd(C);
}

function bool StartsSection()
{
	return true;
}


defaultproperties
{
}

