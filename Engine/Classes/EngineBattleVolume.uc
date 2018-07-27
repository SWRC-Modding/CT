//Parent to BattleVolume class which is needed in Engine so the music manager can access it
class EngineBattleVolume extends Volume
	native;

event bool AreFactoriesExhausted()
{
	return true;
}


defaultproperties
{
     bBlockZeroExtentTraces=False
}

