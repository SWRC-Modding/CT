class ACTION_GenerateStimulus extends ScriptedAction;

var(Action)		Actor.EStimulusType	Type;
var(Action)		Name				StimulusSourceTag;

function bool InitActionFor(ScriptedController C)
{
	local Actor A;
	if (StimulusSourceTag == 'None' && C.Pawn != None)
		C.Pawn.PostStimulusToWorld(Type);
	else if (StimulusSourceTag != 'None')
	{
		ForEach C.DynamicActors( class'Actor', A, StimulusSourceTag )
		{
			A.PostStimulusToWorld(Type);
		}
	}
	return false;	
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@Type$" // "$Comment;
	return ActionString@Type;
}


defaultproperties
{
     ActionString="Generate Stimulus"
}

