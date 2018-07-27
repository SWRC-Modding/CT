class ACTION_MoveToPoint extends LatentScriptedAction native;

var(Action) name DestinationTag;	// tag of destination - if none, then use the ScriptedSequence
var Actor MoveTarget;
var(Action) bool WalkToPoint;
var(Action) bool OrientToPoint;


function bool InitActionFor(ScriptedController C)
{
	C.Pawn.ShouldCrouch( false );
	if( C.Pawn.Physics != PHYS_Flying )
		C.Pawn.SetWalking( WalkToPoint );
	if ( MoveTarget == None && DestinationTag != '' )
	{
		ForEach C.AllActors(class'Actor',MoveTarget,DestinationTag)
			break;
	}
	if (MoveTarget != None && CTBot(C) != None)
		CTBot(C).AddScriptedGotoGoal(MoveTarget,,OrientToPoint);

	return Super.InitActionFor( C );;	
}


function bool MoveToGoal()
{
	return true;
}

function Actor GetMoveTargetFor(ScriptedController C)
{
	if ( MoveTarget != None )
		return MoveTarget;

	MoveTarget = C.SequenceScript.GetMoveTarget();
	if ( DestinationTag != '' )
	{
		ForEach C.AllActors(class'Actor',MoveTarget,DestinationTag)
			break;
	}
	if ( AIScript(MoveTarget) != None )
		MoveTarget = AIScript(MoveTarget).GetMoveTarget();
	return MoveTarget;
}


function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@DestinationTag$" // "$Comment;
	return ActionString@DestinationTag;
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
     ActionString="Move to point"
     bValidForTrigger=False
     ActionColor=(G=255,R=0)
}

