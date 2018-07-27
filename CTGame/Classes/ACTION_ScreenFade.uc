class ACTION_ScreenFade extends ScriptedAction;

var(Action)		float	FadeTime;
var(Action)		Color	FadeColor;

function bool InitActionFor(ScriptedController C)
{
	local CTPlayer PC;

	ForEach C.AllActors(class'CTPlayer', PC)
		break;

	if ( PC == None )
		return false;

	PC.StartScreenFade(FadeTime, FadeColor);

	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     FadeTime=3
     FadeColor=(A=255)
     ActionString="Screen Fade"
}

