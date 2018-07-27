class ACTION_SetPhysics extends ScriptedAction;

var(Action) Actor.EPhysics	NewPhysicsMode;
var(Action) name			ActorTag;

function bool InitActionFor(ScriptedController C)
{
	local Actor A;

	if( ActorTag != '' )
	{
		ForEach C.DynamicActors(class'Actor',A,ActorTag)
			A.SetPhysics(NewPhysicsMode);
	}
	else
		C.GetInstigator().SetPhysics(NewPhysicsMode);

	return false;	
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@NewPhysicsMode$" // "$Comment;
	return ActionString@NewPhysicsMode;
}


defaultproperties
{
     ActionString="change physics to "
}

