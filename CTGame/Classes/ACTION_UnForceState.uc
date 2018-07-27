class ACTION_UnForceState extends ScriptedAction;

function bool InitActionFor(ScriptedController C)
{
	// trigger event associated with action
	local CTBot Bot;

	Bot = CTBot(C);
	if(Bot != None)
	{
		Bot.UnForceState();
	}
	return true;
}


defaultproperties
{
     ActionString="force state"
     bValidForTrigger=False
}

