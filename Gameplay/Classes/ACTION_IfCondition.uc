class ACTION_IfCondition extends ScriptedAction;

var(Action) name TriggeredConditionTag;
var TriggeredCondition T;

function ProceedToNextAction(ScriptedController C)
{
	if ( (T == None) && (TriggeredConditionTag != 'None') )
		ForEach C.AllActors(class'TriggeredCondition',T,TriggeredConditionTag)
			break;

	C.ActionNum++;
	if ( !T.bEnabled )
		ProceedToSectionEnd(C);
}

function bool StartsSection()
{
	return true;
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@T@TriggeredConditionTag$" // "$Comment;
	return ActionString@T@TriggeredConditionTag;
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(TriggeredConditionTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     ActionString="If condition"
}

