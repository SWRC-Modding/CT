class ACTION_EnableBlockedComputations extends ScriptedAction;

var(Action) bool				bSetting;

function bool InitActionFor(ScriptedController C)
{
	if (!bSetting)
	{
		C.Level.bDisableBlockedRegionRecompute = true;
	}
	else
	{
		C.Level.bDisableBlockedRegionRecompute = false;
		C.Level.RegenerateBlockedRegions();
	}
	return false;
}

function string GetActionString()
{
	if (Comment != "")
		return ActionString$" // "$Comment;
	return ActionString;
}


defaultproperties
{
     bSetting=True
     ActionString="Enable Blocked Computations"
}

