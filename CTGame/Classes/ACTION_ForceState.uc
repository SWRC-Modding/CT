class ACTION_ForceState extends ScriptedAction;

var(Action) class<StateObject>	State;
var(Action) bool				bCreateIfNotFound;
var(Action) float				TimeLength;

function bool InitActionFor(ScriptedController C)
{
	// trigger event associated with action
	local CTBot Bot;

	Bot = CTBot(C);
	if(Bot != None)
	{
		Bot.ForceState(State, bCreateIfNotFound, TimeLength);
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
     TimeLength=1e+010
     ActionString="force state"
     bValidForTrigger=False
}

