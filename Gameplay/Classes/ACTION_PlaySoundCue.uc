class ACTION_PlaySoundCue extends ScriptedAction;

//var(Action)	Pawn Pawn;
var(Action)	PawnAudioTable.EPawnAudioEvent	Cue;
var(Action) float MinTriggerInterval;

function bool InitActionFor(ScriptedController C)
{
	if (C.Pawn != None && !C.Pawn.IsDeadOrIncapacitated())
		C.Pawn.PlayOwnedCue(Cue, MinTriggerInterval);

	return false;	
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@Cue$" // "$Comment;
	return ActionString@Cue;
}


defaultproperties
{
     ActionString="play sound cue"
     bValidForTrigger=False
}

