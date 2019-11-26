class MPBot extends CTBot native;

var int ChosenSkin;

function PawnDied(Pawn P){
	Super.PawnDied(P);
	GotoState('Dead');
}

function ServerRestartPlayer(){
	local MPPawn MPP;

	Super.ServerRestartPlayer();

	Level.Game.ReStartPlayer(self);

	MPP = MPPawn(Pawn);

	if(MPP != None){
		MPP.ChosenSkin = ChosenSkin;
		MPP.DoCustomizations();
	}
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
TryAgain:
	Sleep(0.25 + MPGame(Level.Game).RespawnWaitTime);
	ServerRestartPlayer();
	Goto('TryAgain');
MPStart:
	Sleep(0.75 + FRand());
	Level.Game.ChangeTeam(Self, 255, false, false);
	ServerRestartPlayer();
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
