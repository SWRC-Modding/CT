class ACTION_PlaySound extends LatentScriptedAction;

var(Action)		sound				Sound;
var(Action)		float				Volume;
var(Action)		name				SoundSourceTag;
//var(Action)		bool	bAttenuate;
var(Action)		bool				bWaitForFinish;
var(Action)		bool				bDontOverrideOtherSounds;
var(Action)	    Actor.ESoundSlot	Slot;

function bool InitActionFor(ScriptedController C)
{
	local float Duration;
	local Actor SoundSource;
	// play appropriate sound
	C.CurrentAction = self;
	if ( Sound != None ){
		if (SoundSourceTag != '')
		{
			ForEach C.AllActors(class'Actor', SoundSource, SoundSourceTag)
			{
				break; //stop at the first one
			}
			if (SoundSource == None)
			{
				Log("ACTION_PlaySound: non-existant SoundSourceTag, not playing");
				return false;
			}
		}
		else
			SoundSource = C.GetSoundSource();

		if (SoundSource == None)
			return false;

		if (SoundSource.IsA('Pawn'))
		{
			if (Pawn(SoundSource).IsDeadOrIncapacitated())
				return false;
		}
		
		SoundSource.PlayOwnedSound(Sound, Slot, Volume, bDontOverrideOtherSounds);
		//Log( "ACTION_PlaySound( "$Sound$" )"  );

		if (bWaitForFinish)
		{
			Duration = C.GetSoundDuration(Sound) + 0.1; // Returned to previous value, bjd
			C.SetTimer(Duration, false);
			return true;
		}
	}
	return false;	
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@Sound$" // "$Comment;
	return ActionString@Sound;
}

function bool CompleteWhenTimer()
{	
	return true;
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(SoundSourceTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     Volume=1
     Slot=SLOT_Misc
     ActionString="play sound"
}

