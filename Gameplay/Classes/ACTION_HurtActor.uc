class ACTION_HurtActor extends ScriptedAction;

var(Action)		name				TargetTag;
var(Action)		float				Amount;
var(Action)		vector				Momentum;
var(Action)		class<DamageType>	Type;
var(Action)		bool				bUseOwnerAsInstigator;
var(Action)		bool				bIgnoreDifficulty;

function bool InitActionFor(ScriptedController C)
{
	local Actor a;
	local Pawn Instigator;


	if( bUseOwnerAsInstigator )
		Instigator = C.Pawn;

	if(TargetTag != 'None')
	{
		ForEach C.AllActors(class'Actor', a, TargetTag)
		{
			// Hack to make sure player gets enough damage on Easy
			if( Amount >= 1000 && a.IsA('Pawn') )
			{
				if( Pawn(a).IsHumanControlled() )
					Amount *= 10;
			}
			
			if (bIgnoreDifficulty && a.IsA('Pawn') && (Pawn(a)).GetDifficultyMod() != 0.0)
			{
				a.TakeDamage( Amount * (1.0 / (Pawn(a)).GetDifficultyMod()), Instigator, vect(0,0,0), Momentum, Type);
			}
			else
				a.TakeDamage( Amount, Instigator, vect(0,0,0), Momentum, Type);
		}
	}

	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(TargetTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     ActionString="Hurt actor"
}

