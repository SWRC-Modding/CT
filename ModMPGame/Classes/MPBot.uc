class MPBot extends CTBot native;

var int ChosenSkin;

function PawnDied(Pawn P){
	Super.PawnDied(P);
	GotoState('Dead');
}

State Dead{
	ignores SeePlayer, HearNoise, KilledBy;

	function PawnDied(Pawn P){
		if(Level.NetMode != NM_Client)
			warn(self$" PawnDied while dead");
	}

Begin:
	if(Level.Game.bGameEnded)
		GotoState('GameEnded');

	Sleep(0.2);
TryAgain:
	Sleep(0.25 + MPGame(Level.Game).RespawnWaitTime);
	Level.Game.ReStartPlayer(self);
	Goto('TryAgain');
MPStart:
	Level.Game.ChangeTeam(Self, 255, false, false);
	Sleep(0.75 + FRand());
	Level.Game.ReStartPlayer(self);
	Goto('TryAgain');
}

defaultproperties
{
	bIsPlayer=true
	RequiredStates(0)=Class'CTGame.StateMercAttack'
	RequiredStates(1)=Class'CTGame.StateInvestigate'
	RequiredStates(2)=Class'CTGame.StateIdle'
	RequiredStates(3)=Class'CTGame.StatePatrol'
}
