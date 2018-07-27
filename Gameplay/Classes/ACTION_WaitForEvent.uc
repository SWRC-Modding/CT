class ACTION_WaitForEvent extends LatentScriptedAction native;

var(Action) name ExternalEvent;	//tag to give controller (to affect triggering)
var TriggeredCondition T;
var(Action) float ScriptPriorityOnTrigger;
var(Action) bool UnforceStateOnTrigger;

function bool InitActionFor(ScriptedController C)
{
	if ( T == None )
		ForEach C.AllActors(class'TriggeredCondition',T,ExternalEvent)
			break;

	if ( (T != None) && T.bEnabled )
		return false;
	
	C.CurrentAction = self;
	C.Tag = ExternalEvent;
	C.ScriptPriorityOnEvent = ScriptPriorityOnTrigger;
	C.bUnforceStateOnEvent = UnforceStateOnTrigger;
	return true;
}

function bool CompleteWhenTriggered()
{
	return true;
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@ExternalEvent$" // "$Comment;
	return ActionString@ExternalEvent;
}

function bool ContainsPartialEvent(string StartOfEventName)
{
	local string EventString;
	EventString = string(ExternalEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}
	return super.ContainsPartialEvent(StartOfEventName);
}



defaultproperties
{
     ScriptPriorityOnTrigger=-1
     ActionString="Wait for external event"
     ActionColor=(B=255,R=0)
}

