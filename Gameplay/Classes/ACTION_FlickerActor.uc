class ACTION_FlickerActor extends LatentScriptedAction;

struct FlickerActorData
{
	var Actor	FlickerActor;
	var bool	InitialState;
};

var(Action) name					ActorTag;
var			array<FlickerActorData>	FlickerActors;
var(Action) float					FlickerTime;
var(Action) float					FlickerProbability;

function bool InitActionFor(ScriptedController C)
{
	local Actor A;
	local FlickerActorData TempData;

	C.CurrentAction = self;

	ForEach C.DynamicActors( class'Actor', A, ActorTag )
	{
		TempData.FlickerActor = A;
		TempData.InitialState = A.bHidden;
		FlickerActors[ FlickerActors.length ] = TempData;
	}

	return true;	
}

function bool TickedAction()
{
	return true;
}

//*****************************************************************************************
// Action Queries

function bool StillTicking(ScriptedController C, float DeltaTime)
{
	local int i;	

    for( i = 0; i < FlickerActors.length; i++ )
	{
		if( frand() < FlickerProbability )
		{
			FlickerActors[i].FlickerActor.bHidden = !FlickerActors[i].FlickerActor.bHidden;
		}
	}

	FlickerTime -= DeltaTime;

	if( FlickerTime <= 0 )
	{
		for( i = 0; i < FlickerActors.length; i++ )
		{
			FlickerActors[i].FlickerActor.bHidden = FlickerActors[i].InitialState;
		}

		return false;
	}
	else 
		return true;
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@ActorTag$" // "$Comment;
	return ActionString@ActorTag;
}


defaultproperties
{
}

