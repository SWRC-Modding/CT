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
		MPP.PatrolRoute = BotSupport(Owner).BotPatrolRoute;
	}
}

State Dead{
	ignores SeePlayer, HearNoise, KilledBy;

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

state GameEnded{
	ignores SeePlayer, HearNoise, KilledBy, NotifyBump, HitWall, NotifyHeadVolumeChange, NotifyPhysicsVolumeChange, Falling, TakeDamage;

	function ServerReStartPlayer();

	function BeginState(){
		if(Pawn != None ){
			if(Pawn.Weapon != None)
				Pawn.Weapon.HolderDied();

			Pawn.bPhysicsAnimUpdate = false;
			Pawn.StopAnimating();
			Pawn.SimAnim[0].RateScale = 0;
			Pawn.SetCollision(true,false,false);
			Pawn.Velocity = vect(0,0,0);
			Pawn.SetPhysics(PHYS_None);
			Pawn.bIgnoreForces = true;

			// The original code calls Pawn.UnPossessed here which is wrong and causes a crash
			// UnPossess calls Pawn.UnPossessed internally but also sets the controller's Pawn
			// property to None which was missing before.
			UnPossess();
		}
	}
}

cpptext
{
	virtual int Tick(FLOAT DeltaTime, ELevelTick TickType);
}

defaultproperties
{
	bIsPlayer=true
	RequiredStates(0)=Class'CTGame.StateMercAttack'
	RequiredStates(1)=Class'CTGame.StateInvestigate'
	RequiredStates(2)=Class'CTGame.StateIdle'
	RequiredStates(3)=Class'CTGame.StatePatrol'
}
