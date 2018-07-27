class ACTION_GotoState extends ScriptedAction;

var(Action)		name				TargetTag;
var(Action)		name				State;
var(Action)		name				Label;

function bool InitActionFor(ScriptedController C)
{
	local Actor a;

	if(TargetTag != 'None')
	{
		ForEach C.AllActors(class'Actor', a, TargetTag)
		{
			a.GotoState(State, Label);
		}
	}

	return false;	
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@State$" // "$Comment;
	return ActionString@State;
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(TargetTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     ActionString="Goto State"
}

