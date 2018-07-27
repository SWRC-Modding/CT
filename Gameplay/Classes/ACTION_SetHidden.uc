class ACTION_SetHidden extends ScriptedAction;

var(Action) bool bHidden;
var(Action) name HideActorTag;

function bool InitActionFor(ScriptedController C)
{
	local Actor A;
	if ( HideActorTag != '' )
	{
		ForEach C.AllActors(class'Actor',A,HideActorTag)
		{
			if( !A.IsA('Controller') && A.Class.Name != 'SpawnPoint' /*&& !A.bStatic*/)
				A.bHidden = bHidden;
		}
	}
	else
	C.GetInstigator().bHidden = bHidden;
	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(HideActorTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}

defaultproperties
{
}

