class ACTION_TeleportToPoint extends LatentScriptedAction native;

var(Action) name ActorToTeleportTag;
var(Action) name DestinationTag;	// tag of destination - if none, then use the ScriptedSequence
var(Action) bool bPlaySpawnEffect;
var(Action) bool bNoCheck;

var Actor Dest;

function bool InitActionFor(ScriptedController C)
{
	local Actor P;
	local Pawn Pawn;

	Dest = C.SequenceScript.GetMoveTarget();

	if ( ActorToTeleportTag != '' )
	{
		ForEach C.AllActors(class'Actor',P,ActorToTeleportTag)
			break;
	}

	if ( DestinationTag != '' )
	{
		ForEach C.AllActors(class'Actor',Dest,DestinationTag)
			break;
	}
	
	if (P == None)
		P = C.GetInstigator();
	P.SetLocation(Dest.Location, bNoCheck);
	P.SetRotation(Dest.Rotation);
	if (P.IsA('Pawn'))
	{
		Pawn = Pawn(P);
		Pawn.OldRotYaw = P.Rotation.Yaw;
		if (Pawn.Controller.IsA('PlayerController'))
			Pawn.Controller.SetRotation(Dest.Rotation);
	}
	if ( bPlaySpawnEffect )
		P.PlayTeleportEffect(false,true);
	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(DestinationTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     ActionString="set viewtarget"
     ActionColor=(B=255)
}

