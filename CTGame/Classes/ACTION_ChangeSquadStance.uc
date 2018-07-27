class ACTION_ChangeSquadStance extends ScriptedAction;

var(Action)		class<SquadStance> StanceType;
var(Action)		name				ObjectiveTag;

function bool InitActionFor(ScriptedController C)
{
	local CTPlayer PC;
	local Actor StanceObjective;

	ForEach C.AllActors(class'CTPlayer', PC)
		break;

	if (ObjectiveTag != '')
	{
		ForEach C.AllActors(class'Actor',StanceObjective,ObjectiveTag)
			break;
	}

	if ( PC == None )
		return false;
	
	PC.Pawn.Squad.SetStance(StanceType, false, StanceObjective);

	return false;	
}



defaultproperties
{
     ActionString="Change Squad Stance"
}

