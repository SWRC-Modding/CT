class ScriptedAction extends Object
		abstract
		native
		hidecategories(Object)
		collapsecategories
		editinlinenew;

var string ActionString;
var bool bValidForTrigger;
var() bool bDisabled;
var Color ActionColor;
var(Action) string Comment;

event bool InitActionFor(ScriptedController C)
{
	return false;
}

function bool EndsSection()
{
	return false;
}

function bool StartsSection()
{
	return false;
}

function ScriptedSequence GetScript(ScriptedSequence S)
{
	return S;
}

function ProceedToNextAction(ScriptedController C)
{	
	C.ActionNum++;
}

function ProceedToSectionEnd(ScriptedController C)
{
	local int Nesting;
	local ScriptedAction A;

	While ( C.ActionNum < C.SequenceScript.Actions.Length )
	{
		A = C.SequenceScript.Actions[C.ActionNum];
		if ( A.StartsSection() )
			Nesting++;
		else if ( A.EndsSection() )
		{
			Nesting--;
			if ( Nesting < 0 )
				return;
		}
		
		C.ActionNum++;
	}
}

event Actor GetActionActor()
{	
	return None;
}

function string GetActionString()
{
	if (Comment != "")
		return ActionString$" // "$Comment;
	else
		return ActionString;
}

// These functions (ContainsPartial*) are used by the editor
// to help with search functionality
function bool ContainsPartialEvent(string StartOfEventName)
{
	return false;
}

function bool ContainsPartialTag(string StartOfTagName)
{
	return false;
}



defaultproperties
{
     ActionString="unspecified action"
     bValidForTrigger=True
     ActionColor=(R=255,A=255)
}

