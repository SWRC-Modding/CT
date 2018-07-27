class ACTION_ShowMarker extends ScriptedAction;

var(Action)		name			MarkerTag;
var(Action)		bool			bShow;

function bool InitActionFor(ScriptedController C)
{
	local SquadMarker Marker;

	local CTPlayer PC;

	ForEach C.AllActors(class'CTPlayer', PC)
		break;

	ForEach C.AllActors(class'SquadMarker', Marker, MarkerTag)
	{
		Marker.AlwaysVisible = bShow;
		if (bShow)
			Marker.NotifyTargeted(PC);
		else
			Marker.NotifyUnTargeted(PC);
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
     bShow=True
     ActionString="Show Marker"
}

