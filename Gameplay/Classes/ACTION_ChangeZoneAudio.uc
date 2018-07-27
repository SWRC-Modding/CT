class ACTION_ChangeZoneAudio extends ScriptedAction;

var(Action)		name				ZoneInfoTag;
var(Action) class<AudioEnvironment> ZoneAudioClass;


function bool InitActionFor(ScriptedController C)
{
	local ZoneInfo ZoneInfo;

	if(ZoneInfoTag != 'None')
	{
		ForEach C.AllActors(class'ZoneInfo', ZoneInfo, ZoneInfoTag)
		{
			ZoneInfo.ZoneAudioClass = ZoneAudioClass;
		}
	}

	return false;	
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@ZoneInfoTag$" // "$Comment;
	return ActionString@ZoneInfoTag;
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(ZoneInfoTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     ActionString="Change Zone Audio"
}

