class FirstPersonPlayer extends Actor;

const TURN_SPEED = 100.0f; // Couldn't find a rotation equivalent of Pawn::GroundSpeed so this is hardcoded
const MAX_TURN_ANIM_RATE = 2.0f;

var PlayerController Player;
var int LastViewRotationYaw;

var FirstPersonPlayerShadowCaster ShadowCaster;

function PreBeginPlay(){
	if(Level.NetMode != NM_Standalone)
		Destroy();
	else
		Super.PreBeginPlay();
}

function Tick(float DeltaTime){
	local float Speed;
	local float MaxSpeed;
	local float TurnSpeed;
	local vector PlayerLocation;
	local rotator ViewRotation;
	local Pawn PlayerPawn;
	local Controller C;
	local name AnimName;
	local float AnimRate;

	if(Player == None){
		for(C = Level.ControllerList; C != None; C = C.nextController){
			Player = PlayerController(C);

			if(Player != None)
				break;
		}
	}

	bHidden = Player == None || Player.Pawn == None || Player.Pawn.bIncapacitated || Player.IsInState('Briefing') || Player.bBehindView;

	if(Level.ShadowsEnabled){
		/* if(ShadowCaster == None) */
		/* 	ShadowCaster = Spawn(class'FirstPersonPlayerShadowCaster'); */

		/* ShadowCaster.bHidden = bHidden; */
	}else if(ShadowCaster != None){
		ShadowCaster.Destroy();
		ShadowCaster = None;
	}

	if(bHidden)
		return;

	PlayerPawn = Player.Pawn;
	PlayerLocation = PlayerPawn.Location;
	ViewRotation = PlayerPawn.GetViewRotation();
	ViewRotation.Pitch = 0; // Make the actor stay upright in the world and only rotate it left and right with the camera
	Speed = VSize(PlayerPawn.Velocity);
	TurnSpeed = ViewRotation.Yaw - LastViewRotationYaw;

	if(PlayerPawn.Base != None){
		if(PlayerPawn.bIsCrouched){
			MaxSpeed = PlayerPawn.GroundSpeed * PlayerPawn.CrouchSpeedRatio;
			PlayerLocation.Z += 30.0f;
		}else if(PlayerPawn.bIsWalking){
			MaxSpeed = PlayerPawn.GroundSpeed * PlayerPawn.WalkSpeedRatio;
		}else{
			MaxSpeed = PlayerPawn.GroundSpeed * 0.75f; // Slightly decrease max speed to get a higher anim rate which looks better with the 'WalkForward' anim
																								 // (Can't use 'RunFoward' because it does not look good in first person)
		}

		if(Speed <= MaxSpeed * 0.05f){ // Small threshold so very little movement does not play a walk animation which looks weird
			if(PlayerPawn.bIsCrouched){
				if(TurnSpeed > 0){
					AnimName = 'CrouchTurnRight';
					AnimRate = FMin(TurnSpeed / TURN_SPEED, MAX_TURN_ANIM_RATE);
				}else if(TurnSpeed < 0){
					AnimName = 'CrouchTurnLeft';
					AnimRate = FMin(-TurnSpeed / TURN_SPEED, MAX_TURN_ANIM_RATE);
				}else{
					AnimName = 'CrouchBreathe';
					AnimRate = 1.0;
				}
			}else{
				if(TurnSpeed > 0){
					AnimName = 'TurnRight';
					AnimRate = FMin(TurnSpeed / TURN_SPEED, MAX_TURN_ANIM_RATE);
				}else if(TurnSpeed < 0){
					AnimName = 'TurnLeft';
					AnimRate = FMin(-TurnSpeed / TURN_SPEED, MAX_TURN_ANIM_RATE);
				}else{
					AnimName = 'ActionBreathe';
					AnimRate = 1.0;
				}
			}
		}else{
			if(PlayerPawn.bIsCrouched)
				AnimName = 'CrouchForward';
			else
				AnimName = 'WalkForward';

			AnimRate = Speed / MaxSpeed;
		}
	}else{
		PlayerLocation.Z -= 15.0f;
		AnimName = 'FallBreathe';
		AnimRate = 1.0;
	}

	if(AnimName != 'None'){
		LoopAnim(AnimName, 'None', AnimRate);

		if(ShadowCaster != None)
			ShadowCaster.LoopAnim(AnimName, 'None', AnimRate);
	}

	SetLocation(PlayerLocation);
	SetRotation(ViewRotation);

	if(ShadowCaster != None){
		ShadowCaster.SetLocation(PlayerPawn.Location);
		ShadowCaster.SetRotation(ViewRotation);
	}

	LastViewRotationYaw = ViewRotation.Yaw;
}

defaultproperties
{
	bHidden=True
	DrawType=DT_Mesh
	Mesh=SkeletalMesh'FirstPersonPlayerAnim.FirstPersonPlayer'
}
