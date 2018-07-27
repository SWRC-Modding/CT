class ACTION_SpawnStatusEffect extends ScriptedAction;

var(Action)		class<StatusEffect>		StatusEffectClass;
var(Action)		float					MaxDuration;
var(Action)		float					Radius;
var(Action)		float					DamagePerSecond;
var(Action)		name					ActorTag;
var(Action)		bool					AffectTaggedActor;

function bool InitActionFor(ScriptedController C)
{
	local Actor Originator;

	if( ActorTag != '' )
	{
		ForEach C.AllActors( class'Actor', Originator, ActorTag )
		{
			if( AffectTaggedActor )			
				StatusEffectClass.Static.AddStatusEffectTo( Originator, C, MaxDuration, DamagePerSecond );				
			
			StatusEffectClass.Static.CreateStatusEffects( Originator, MaxDuration, Radius, DamagePerSecond );
		}		
	}
	else if( C.Pawn != None )
		StatusEffectClass.Static.CreateStatusEffects( C.Pawn, MaxDuration, Radius, DamagePerSecond );
	else
		StatusEffectClass.Static.CreateStatusEffects( C, MaxDuration, Radius, DamagePerSecond );

	return false;	
}


defaultproperties
{
     ActionString="Spawn Status Effect"
}

