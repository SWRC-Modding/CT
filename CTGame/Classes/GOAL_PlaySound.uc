class GOAL_PlaySound extends GoalObject
	native;

var Sound Sound;
var PawnAudioTable.EPawnAudioEvent AudioCue;
var float CueInterval;
var float CueTimeout;
var Actor CueSubject;
var bool WaitToFinish;
var bool UseSquadQueue;


defaultproperties
{
     CueInterval=-1
}

