class MPBot extends CTBot native;

var int ChosenSkin;

function GivePawn(){
	Log("GivePawn called");
}

/*function GameHasEnded()
{
	if ( Pawn != None )
		Pawn.bNoWeaponFiring = true;
	GotoState('GameEnded');
}*/

State Dead{
	ignores SeePlayer, HearNoise, KilledBy;

	function PawnDied(Pawn P){
		if(Level.NetMode != NM_Client)
			warn(self$" Pawndied while dead");
	}

	function ServerReStartPlayer(){
		if(Level.NetMode == NM_Client)
			return;

		Level.Game.RestartPlayer(self);
	}
}

defaultproperties
{
	bIsPlayer=true
	RequiredStates(0)=Class'CTGame.StateMercAttack'
	RequiredStates(1)=Class'CTGame.StateInvestigate'
	RequiredStates(2)=Class'CTGame.StateIdle'
	RequiredStates(3)=Class'CTGame.StatePatrol'
}
