class ACTION_DestroyActor extends ScriptedAction;

var(Action)		name			DestroyTag;

function bool InitActionFor(ScriptedController C)
{
	local Actor a;

	if(DestroyTag != 'None')
	{
		ForEach C.AllActors(class'Actor', a, DestroyTag)
		{
			a.Destroy();
		}
	}

	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(DestroyTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     ActionString="Destroy actor"
}

