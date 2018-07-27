class ACTION_ShowActorObjectiveTracker extends ScriptedAction;

var(Action)		name			ActorTag;

function bool InitActionFor(ScriptedController C)
{
	local Actor a;
	local bool found;
	local CTPlayer PC;

	ForEach C.AllActors(class'CTPlayer', PC)
		break;

	if ( PC == None )
		return false;

	found = false;

	if(ActorTag != 'None')
	{
		ForEach C.AllActors(class'Actor', a, ActorTag)
		{
			PC.ShowActorObjectiveTracker(a);
			found = true;
			break;
		}
	}

	if (!found)
	{
		PC.HideObjectiveTracker();
	}

	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(ActorTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     ActionString="Show Actor Objective Tracker"
}

