class ACTION_PlayPropAnim extends ScriptedAction;

var(Action) name BaseAnim;
var(Action) float AnimRate;
var(Action) float StartFrame;
var(Action)	int	Channel;
var(Action) name AnimPropTag;

function bool InitActionFor(ScriptedController C)
{
	local Actor A;

	// play appropriate animation
	if ( AnimPropTag != '' )
	{
		ForEach C.AllActors(class'Actor',A,AnimPropTag)
			A.PlayAnim(BaseAnim,,AnimRate,StartFrame,Channel);
	}
	else
		C.GetInstigator().PlayAnim(BaseAnim,,AnimRate,StartFrame,Channel);
	
	return false;
	
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@BaseAnim$" // "$Comment;
	return ActionString@BaseAnim;
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(AnimPropTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     AnimRate=1
     StartFrame=-1
     ActionString="play prop animation"
}

