class ACTION_GotoLabel extends ScriptedAction;

var(Action) string LabelName;

function ProceedToNextAction(ScriptedController C)
{
	local int N;
	local ACTION_Label A;

	for ( N = 0; N < C.SequenceScript.Actions.Length; N++ )
	{
		A = ACTION_Label( C.SequenceScript.Actions[N] );
		if ( A != none && A.LabelName == LabelName )
		{
			C.ActionNum = N+1;
			return;
		}
	}
	warn("Cannot find label" @ LabelName);
	C.ActionNum++;
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString @ LabelName$" // "$Comment;
	return ActionString @ LabelName;
}


defaultproperties
{
     ActionString="go to label"
}

