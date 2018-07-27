class ACTION_Label extends ScriptedAction;

var(Action) string LabelName;

function string GetActionString()
{
	return ActionString @ LabelName;
}


defaultproperties
{
     ActionString="label"
}

