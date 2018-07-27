class ACTION_TriggerEvent extends ScriptedAction native;

var(Action) name Event;

function bool InitActionFor(ScriptedController C)
{
	// trigger event associated with action
	C.TriggerEvent(Event,C.SequenceScript,C.GetInstigator());
	return false;	
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@Event$" // "$Comment;
	return ActionString@Event;
}

function bool ContainsPartialEvent(string StartOfEventName)
{
	local string EventString;
	EventString = string(Event);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}
	return super.ContainsPartialEvent(StartOfEventName);
}



defaultproperties
{
     ActionString="trigger event"
     ActionColor=(G=128)
}

