class ACTION_EnableAIState extends ScriptedAction;

var(Action) class<StateObject>	State;
var(Action) bool bEnable;
var(Action) bool bCreateIfNotFound;

function bool InitActionFor(ScriptedController C)
{
	// trigger event associated with action
	local CTBot Bot;

	Bot = CTBot(C);
	if(Bot != None)
	{
		Bot.EnableState(State, bEnable);
	}
	return false;	
}

function string GetActionString()
{
	if (Comment != "")
		return ActionString@State@bCreateIfNotFound$" // "$Comment;
	return ActionString@State@bCreateIfNotFound;
}


defaultproperties
{
     bCreateIfNotFound=True
     ActionString="disable state"
     bValidForTrigger=False
}

