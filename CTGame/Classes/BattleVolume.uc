//Used by the music manager to change the criteria for battles ending.
//When the player is in a BattleVolume and is in battle, the battle will not
//end until all pawn factories in the volume say they are done making baddies.
class BattleVolume extends EngineBattleVolume;

var editconst array<PawnFactory> Factories;

function PostBeginPlay()
{
	Super.PostBeginPlay();
	FindFactories();
}

event FindFactories()
{
	local PawnFactory pf;

	Factories.Remove(0, Factories.length);

	ForEach DynamicActors(class 'PawnFactory', pf)
	{
		if ( Encompasses( pf ) )
		{
			Factories.Insert( Factories.Length, 1 );
			Factories[Factories.Length-1] = pf;
		}
	}
}

event bool AreFactoriesExhausted()
{
	local int i;
	for (i=0; i<Factories.length; i++)
		if (!Factories[i].IsExhausted())
			return false;

	return true;
}

defaultproperties
{
}

