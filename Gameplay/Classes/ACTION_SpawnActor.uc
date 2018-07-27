class ACTION_SpawnActor extends ScriptedAction;

var(Action)		class<Actor>	ActorClass;
var(Action)		vector			LocationOffset;
var(Action)		rotator			RotationOffset;
var(Action)		bool			bOffsetFromScriptedPawn;
var(Action)		name			ActorTag;

function bool InitActionFor(ScriptedController C)
{
	local vector loc;
	local rotator rot;
	local actor a;

	if ( bOffsetFromScriptedPawn )
	{
		loc = C.Pawn.Location + LocationOffset;
		rot = C.Pawn.Rotation + RotationOffset;
	}
	else
	{
		loc = C.SequenceScript.Location + LocationOffset;
		rot = C.SequenceScript.Rotation + RotationOffset;
	}
	a = C.Spawn(ActorClass,,,loc,rot);
	a.Instigator = C.Pawn;
	if ( ActorTag != 'None' )
		a.Tag = ActorTag;
	return false;	
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@ActorClass$" // "$Comment;
	return ActionString@ActorClass;
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(ActorTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}


defaultproperties
{
     ActionString="Spawn actor"
}

