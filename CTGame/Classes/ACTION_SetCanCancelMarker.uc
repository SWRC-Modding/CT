class ACTION_SetCanCancelMarker extends ScriptedAction;

var(Action)		name			MarkerTag;
var(Action)		bool			bCanCancel;

function bool InitActionFor(ScriptedController C)
{
	local SquadMarker Marker;

	ForEach C.AllActors(class'SquadMarker', Marker, MarkerTag)
	{
		Marker.bCanCancelAfterUnderway = bCanCancel;
	}
	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(MarkerTag);
	if ( Caps(MarkerTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     bCanCancel=True
}

