class ACTION_SetThreat extends ScriptedAction;

var(Action)		name			PawnTag;
var(Action)		bool			IsThreat;

function bool InitActionFor(ScriptedController C)
{
	local Actor Actor;

	if (PawnTag != 'None')
	{
		ForEach C.AllActors(class'Actor', Actor, PawnTag)
		{
			if (Actor.IsA('Pawn'))
				Pawn(Actor).bPerceivedAsThreat = IsThreat;
			else
				Actor.bValidEnemy = IsThreat;
		}
	}
	else
		C.Pawn.bPerceivedAsThreat = IsThreat;

	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(PawnTag);
	if ( Caps(PawnTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     IsThreat=True
}

