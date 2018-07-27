class ACTION_EnableNavPtSpecial extends ScriptedAction;

var(Action) bool				bSetting;
var(Action)		name			NavPtTag;

function bool InitActionFor(ScriptedController C)
{
	local NavigationPoint Point;

	ForEach C.AllActors(class'NavigationPoint', Point, NavPtTag)
	{
		Point.bEnabled = bSetting;
	}
	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(NavPtTag);
	if ( Caps(NavPtTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}


defaultproperties
{
     bSetting=True
     ActionString="Enable Nav Pt Special"
}

