class FOVChanger extends Object;

var() config float CurrentFOV;

var CTPlayer         Player;
var FunctionOverride CTPlayerEndZoomOverride;
var FunctionOverride CTPlayerResetFOVOverride;

function Init(){
	CTPlayerEndZoomOverride = new class'FunctionOverride';
	CTPlayerEndZoomOverride.Init(class'CTPlayer', 'EndZoom', self, 'CTPlayerEndZoom');
	CTPlayerResetFOVOverride = new class'FunctionOverride';
	CTPlayerResetFOVOverride.Init(class'CTPlayer', 'ResetFOV', self, 'CTPlayerResetFOV');
}

function CTPlayerEndZoom(){
	Player = CTPlayer(CTPlayerEndZoomOverride.CurrentSelf);

	Player.EndZoom();
	SetViewFOV();
	SetWeaponFOV();
}

function CTPlayerResetFOV(){
	Player = CTPlayer(CTPlayerResetFOVOverride.CurrentSelf);

	Player.DesiredFOV = Player.DefaultFOV;
	Player.FOVAngle = Player.DefaultFOV;

	SetWeaponFov();
	Player.NoHUDArms(false);
}

function SetViewFOV(){
	if(Player == None)
		return;

	Player.DefaultFOV = CurrentFOV;
	Player.DesiredFOV = Player.DefaultFOV;
	Player.FOVAngle = Player.DefaultFOV;
}

function SetWeaponFOV(){
	if(Player == None)
		return;

	if(Player.Pawn != None && Player.Pawn.Weapon != None &&
	   !Player.Pawn.Weapon.IsA('Boy') &&
	   !Player.Pawn.Weapon.IsA('ATTEWalkerTurret') &&
	   !Player.Pawn.Weapon.IsA('EWebTurret') &&
	   !Player.Pawn.Weapon.IsA('GeoTurret') &&
	   !Player.Pawn.Weapon.IsA('WookieeTurret') &&
	   !Player.Pawn.Weapon.IsA('GunshipTurret') &&
	   !Player.Pawn.Weapon.IsA('MissileTurret') &&
	   !Player.Pawn.Weapon.IsA('HeavyTurret')){
		Player.Pawn.Weapon.ZoomFOVs[0] = CurrentFOV;

		// Changing the default zoom Fovs so that it will not zoom in as much with a higher Fov selected
		Player.Pawn.Weapon.ZoomFOVs[1] = CurrentFOV - (Player.default.DefaultFOV - Player.Pawn.Weapon.default.ZoomFOVs[1]);
		Player.Pawn.Weapon.ZoomFOVs[2] = CurrentFOV - (Player.default.DefaultFOV - Player.Pawn.Weapon.default.ZoomFOVs[2]);
		Player.Pawn.Weapon.ZoomFOVs[3] = CurrentFOV - (Player.default.DefaultFOV - Player.Pawn.Weapon.default.ZoomFOVs[3]);
		Player.Pawn.Weapon.SetWeapFOV(CurrentFOV);
	}
}

function SetFOV(float NewFOV){
	CurrentFOV = FClamp(NewFOV, 85.0, 130);

	SaveConfig();
	SetViewFOV();
	SetWeaponFOV();
}

defaultproperties
{
	CurrentFOV=85.0
}
