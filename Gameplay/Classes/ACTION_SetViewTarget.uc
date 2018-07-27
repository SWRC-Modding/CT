class ACTION_SetViewTarget extends ScriptedAction native;

var(Action) name ViewTargetTag;
//var Actor ViewTarget;

function bool InitActionFor(ScriptedController C)
{
	local Actor ViewTarget;
	ViewTarget = None;

	if ( ViewTargetTag == 'Enemy' )
		C.ScriptedFocus = C.Enemy;
	else if ( ViewTargetTag == '' )
		C.ScriptedFocus = None;
	else
	{
		if ( ViewTargetTag != 'None' )
			ForEach C.AllActors(class'Actor',ViewTarget,ViewTargetTag)
				break;

		if ( ViewTarget == None )
			C.bBroken = true;
		C.ScriptedFocus = ViewTarget;
	}
	return false;	
}

function String GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@ViewTargetTag$" // "$Comment;
	return ActionString@ViewTargetTag;
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(ViewTargetTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}

	

defaultproperties
{
     ActionString="set viewtarget"
     bValidForTrigger=False
}

