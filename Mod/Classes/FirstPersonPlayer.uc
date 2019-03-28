class FirstPersonPlayer extends Actor transient;

var int LastViewRotationYaw;

function Update(Pawn PlayerPawn){
	local float VelocitySq;
	local vector PlayerLocation;
	local rotator ViewRotation;

	if(bHidden)
		return;

	PlayerLocation = PlayerPawn.Location;

	if(PlayerPawn.bIsCrouched)
		PlayerLocation.Z += 30.0f;

	ViewRotation = PlayerPawn.GetViewRotation();
	ViewRotation.Pitch = 0;

	SetLocation(PlayerLocation);
	SetRotation(ViewRotation);

	VelocitySq = VSizeSq(PlayerPawn.Velocity);

	if(VelocitySq == 0){
		if(PlayerPawn.bIsCrouched){
			if(ViewRotation.Yaw - LastViewRotationYaw > 0)
				LoopAnim('CrouchTurnRight');
			else if(ViewRotation.Yaw - LastViewRotationYaw < 0)
				LoopAnim('CrouchTurnLeft');
			else
				LoopAnim('CrouchBreathe');
		}else{
			if(ViewRotation.Yaw - LastViewRotationYaw > 0)
				LoopAnim('TurnRight');
			else if(ViewRotation.Yaw - LastViewRotationYaw < 0)
				LoopAnim('TurnLeft');
			else
				LoopAnim('ActionBreathe');
		}
	}else if(VelocitySq > 0 && PlayerPawn.Base != None){
		if(PlayerPawn.bIsCrouched)
			LoopAnim('CrouchForward');
		else
			LoopAnim('WalkForward');
	}

	LastViewRotationYaw = ViewRotation.Yaw;
}

defaultproperties
{
	DrawType=DT_Mesh
	Mesh=SkeletalMesh'ModHudArms.FirstPersonPlayer'
}