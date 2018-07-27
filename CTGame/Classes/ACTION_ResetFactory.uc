class ACTION_ResetFactory extends ScriptedAction;

var(Action) name FactoryTag;

function bool InitActionFor(ScriptedController C)
{
	local Factory F;
	
	if ( FactoryTag != '' )
	{
		ForEach C.AllActors(class'Factory',F,FactoryTag)
		{
			F.RemainingItems = F.Capacity;
		}
	}
	return false;	
}


// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(FactoryTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}

defaultproperties
{
}

