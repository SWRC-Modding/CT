class ACTION_PlayMovie extends LatentScriptedAction;

var(Action)		string			MovieName;
var(Action)		bool			bLooping;
var(Action)		bool			bInterruptable;
var(Action)		bool			bPauseLevelMusic;
var(Action)		bool			bPauseLevelSound;
var(Action)		bool			bWaitForMovieEnd;

function bool InitActionFor(ScriptedController C)
{
	local CTPlayer PC;

	ForEach C.AllActors(class'CTPlayer', PC)
		break;

	if ( PC == None )
		return false;

	C.ScriptedFocus = None;
	C.CurrentAction = self;

	PC.PlayMovie( bLooping, bInterruptable, bPauseLevelMusic, bPauseLevelSound, MovieName );

	return bWaitForMovieEnd;	
}

function bool TickedAction()
{
	return bWaitForMovieEnd;
}

function bool StillTicking(ScriptedController C, float DeltaTime)
{
	local CTPlayer PC;

	ForEach C.AllActors(class'CTPlayer', PC)
		break;

	if ( PC == None )
		return false;

	if ( !PC.IsMoviePlaying() )
	{
		// This is really hacky, but I don't know of another easy way to get the action to terminate
		// perhaps a better solution would be to modify the Tick function in ScriptedController::Scripted
		// to call complete action when the tick is disabled
		C.Trigger( None, None ); 
		return false;
	}
	
	return true;
}

function bool CompleteWhenTriggered()
{
	return bWaitForMovieEnd;
}


// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	if ( Caps(StartOfTag) == Left(Caps(MovieName), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     bPauseLevelMusic=True
     bPauseLevelSound=True
     ActionString="Show Menu Screen"
}

