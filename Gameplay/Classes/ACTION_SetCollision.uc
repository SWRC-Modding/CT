class ACTION_SetCollision extends ScriptedAction;

var(Action) bool bCollideActors;
var(Action) bool bBlockPlayers;
var(Action) bool bBlockActors;
var(Action) bool bCollideWorld;
var(Action) bool bBlockZeroExtentTraces;
var(Action) bool bBlockNonZeroExtentTraces;
var(Action) name CollideActorTag;

function bool InitActionFor(ScriptedController C)
{
	local Actor A;

	if ( CollideActorTag != '' )
	{
		ForEach C.AllActors(class'Actor',A,CollideActorTag)
		{
			A.SetCollision(bCollideActors, bBlockActors, bBlockPlayers);
			A.bCollideWorld = bCollideWorld;
			A.bBlockZeroExtentTraces = bBlockZeroExtentTraces;
			A.bBlockNonZeroExtentTraces = bBlockNonZeroExtentTraces;
		}
	}
	else
	{
		A = C.GetInstigator();
		A.SetCollision(bCollideActors, bBlockActors, bBlockPlayers);
		A.bCollideWorld = bCollideWorld;
		A.bBlockZeroExtentTraces = bBlockZeroExtentTraces;
		A.bBlockNonZeroExtentTraces = bBlockNonZeroExtentTraces;
	}
	return false;	
}


// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(CollideActorTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}


defaultproperties
{
     bCollideActors=True
     bCollideWorld=True
     bBlockZeroExtentTraces=True
     bBlockNonZeroExtentTraces=True
}

