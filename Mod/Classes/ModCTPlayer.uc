class ModCTPlayer extends CTPlayer;

var config int FovLevel;
var transient FirstPersonPlayer Body;
var bool bDrawBody;

function PostLoadBeginPlay(){
	Super.PostLoadBeginPlay();
	SetFovLevel(FovLevel);

	Body = Spawn(class'FirstPersonPlayer', self);
}

event Destroyed(){
	Super.Destroyed();
	Body.Destroy(); //Cleanup to avoid log spam when loading a savegame
}

exec function ToggleBody(){
	bDrawBody = !bDrawBody;
}

event PlayerTick(float DeltaTime){
	Super.PlayerTick(DeltaTime);

	Body.bHidden = !bDrawBody || Pawn == None || Pawn.bIncapacitated || bBehindview;

	Body.Update(Pawn);
}

function SetViewFov(){
	DefaultFov = 85.0f + FovLevel * 5.0f;
	DesiredFov = DefaultFov;
	FovAngle = DefaultFov;
}

function SetWeaponFov(){
	if(Pawn != None && Pawn.Weapon != None &&
							  !Pawn.Weapon.IsA('Boy') &&
							  !Pawn.Weapon.IsA('ATTEWalkerTurret') &&
							  !Pawn.Weapon.IsA('EWebTurret') &&
							  !Pawn.Weapon.IsA('GeoTurret') &&
							  !Pawn.Weapon.IsA('WookieeTurret') &&
							  !Pawn.Weapon.IsA('GunshipTurret') &&
							  !Pawn.Weapon.IsA('MissileTurret') &&
							  !Pawn.Weapon.IsA('HeavyTurret')){
		Pawn.Weapon.ZoomFOVs[0] = DefaultFov;

		//Changing the default zoom Fovs so that it will not zooom in as much with a higher Fov selected
		Pawn.Weapon.ZoomFOVs[1] = DefaultFov - (85.0f - Pawn.Weapon.Default.ZoomFOVs[1]);
		Pawn.Weapon.ZoomFOVs[2] = DefaultFov - (85.0f - Pawn.Weapon.Default.ZoomFOVs[2]);
		Pawn.Weapon.ZoomFOVs[3] = DefaultFov - (85.0f - Pawn.Weapon.Default.ZoomFOVs[3]);
		Pawn.Weapon.SetWeapFov(DefaultFov);
	}
}

function ResetFOV(){
	DesiredFOV = DefaultFOV;
	FOVAngle = DefaultFOV;

	SetWeaponFov();
	NoHUDArms(false);
}

function SetFovLevel(int NewFovLevel){
	FovLevel = NewFovLevel;

	SaveConfig();
	SetViewFov();
	SetWeaponFov();
}

function EndZoom(){			//Setting the fov for drawing the HudArms
	Super.EndZoom();		//This function is called everytime a weapon is pulled out, holstered or zoomed
	FovAngle = DefaultFov;

	SetWeaponFov();
}

defaultproperties
{
	bDrawBody=true
}