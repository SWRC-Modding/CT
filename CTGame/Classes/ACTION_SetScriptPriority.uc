class ACTION_SetScriptPriority extends ScriptedAction;

var(Action) float Priority;

function bool InitActionFor(ScriptedController C)
{
	// trigger event associated with action
	local CTBot Bot;

	Bot = CTBot(C);
	if (Bot != None)
	{
		Bot.SetScriptingPriority(Priority);
	}
	return false;	
}

function string GetActionString()
{
	if (Comment != "")
		return ActionString@Priority$" // "$Comment;
	return ActionString@Priority;
}


defaultproperties
{
     ActionString="set priority"
     bValidForTrigger=False
}

