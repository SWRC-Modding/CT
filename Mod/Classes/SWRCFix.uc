class SWRCFix extends Object config native transient;

#exec OBJ LOAD FILE="Properties.u"

var() config float   ViewShake;
var() config float   BaseFOV;
var() config float   FpsLimit;
var() config float   FOV;
var() config float   HudArmsFOVFactor;
var() config bool    OverrideDefaultFOV;
var() config bool    LimitHudArmsFOV;
var() config bool    AutoFOV;
var() config bool    EnableCustomMenu;
var() config bool    EnableEditorSelectionFix;
var() config bool    OverrideD3DRenderDevice;

var FunctionOverride CTPlayerEndZoomOverride;
var FunctionOverride CTPlayerResetFOVOverride;

var FunctionOverride PlayerControllerShakeViewOverride;

var FunctionOverride WeaponSetWeapFOVOverride;

var FunctionOverride MenuBaseCallMenuClassOverride;
var FunctionOverride MenuBaseOverlayMenuClassOverride;
var FunctionOverride MenuBaseGotoMenuClassOverride;

event InitScript(){
	// Hook necessary functions

	// CTPlayer
	CTPlayerEndZoomOverride = new class'FunctionOverride';
	CTPlayerEndZoomOverride.Init(class'CTPlayer', 'EndZoom', self, 'CTPlayerEndZoom');
	CTPlayerResetFOVOverride = new class'FunctionOverride';
	CTPlayerResetFOVOverride.Init(class'CTPlayer', 'ResetFOV', self, 'CTPlayerResetFOV');
	// PlayerController
	PlayerControllerShakeViewOverride = new class'FunctionOverride';
	PlayerControllerShakeViewOverride.Init(class'PlayerController', 'ShakeView', self, 'PlayerControllerShakeView');
	// Weapon
	WeaponSetWeapFOVOverride = new class'FunctionOverride';
	WeaponSetWeapFOVOverride.Init(class'Weapon', 'SetWeapFOV', self, 'WeaponSetWeapFOV');

	if(EnableCustomMenu){
		// MenuBase
		MenuBaseCallMenuClassOverride = new class'FunctionOverride';
		MenuBaseCallMenuClassOverride.Init(class'MenuBase', 'CallMenuClass', self, 'MenuBaseCallMenuClass');
		MenuBaseOverlayMenuClassOverride = new class'FunctionOverride';
		MenuBaseOverlayMenuClassOverride.Init(class'MenuBase', 'OverlayMenuClass', self, 'MenuBaseOverlayMenuClass');
		MenuBaseGotoMenuClassOverride = new class'FunctionOverride';
		MenuBaseGotoMenuClassOverride.Init(class'MenuBase', 'GotoMenuClass', self, 'MenuBaseGotoMenuClass');
	}

	// Fixing battle droids using clone weapons in the first mission
	class'Properties.BattleDroidBlasterTM'.default.AttachmentClass = class'Properties.BattleDroidBlasterAttachmentTM';
}

event float GetDefaultFOV(){
	if(OverrideDefaultFOV)
		return BaseFOV;

	return class'CTPlayer'.default.FOVAngle;
}

function CTPlayerEndZoom(){
	local CTPlayer Player;

	Player = CTPlayer(CTPlayerEndZoomOverride.CurrentSelf);

	Player.EndZoom();
	SetViewFOV(Player);
}

function CTPlayerResetFOV(){
	local CTPlayer Player;

	Player = CTPlayer(CTPlayerResetFOVOverride.CurrentSelf);

	Player.DesiredFOV = Player.DefaultFOV;
	Player.FOVAngle = Player.DefaultFOV;

	if(Player.Pawn != None && Player.Pawn.Weapon != None)
		Player.Pawn.Weapon.SetWeapFOV(Player.FOVAngle);

	Player.NoHUDArms(false);
}

function WeaponSetWeapFOV(float NewFOV){
	local Weapon Weapon;
	local float HudArmsFOV;

	Weapon = Weapon(WeaponSetWeapFOVOverride.CurrentSelf);
	HudArmsFOV = GetDefaultFOV() + HUDArmsFOVFactor * (NewFOV - GetDefaultFOV());

	UpdateWeaponZoomFOVs(Weapon);

	if(LimitHudArmsFOV && !Weapon.bWeaponZoom){
		if(Weapon.IsA('Boy'))
			HudArmsFOV = FMin(HudArmsFOV, 90);
		else if(Weapon.IsA('HeavyTurret'))
			HudArmsFOV = FMin(HudArmsFOV, 93);
		else if(Weapon.IsA('WookieeTurret'))
			HudArmsFOV = FMin(HudArmsFOV, 95);
		else if(Weapon.IsA('EWebTurret') || Weapon.IsA('MissileTurret'))
			HudArmsFOV = FMin(HudArmsFOV, 100);
		else if(Weapon.IsA('ATTEWalkerTurret'))
			HudArmsFOV = FMin(HudArmsFOV, 105);
		else if(Weapon.IsA('GunshipTurret'))
			HudArmsFOV = FMin(HudArmsFOV, 107);
		else if(Weapon.IsA('GeoTurret'))
			HudArmsFOV = FMin(HudArmsFOV, 110);
	}

	Weapon.DisplayFOV = HudArmsFOV;
}

//====================================================================================================

function UpdateWeaponZoomFOVs(Weapon Weapon){
	local float DefaultFOV;

	DefaultFOV = GetDefaultFOV();

	Weapon.ZoomFOVs[0] = FOV;

	// Changing the default zoom Fovs so that it will not zoom in as much with a higher Fov selected
	Weapon.ZoomFOVs[1] = FOV - (DefaultFOV - Weapon.default.ZoomFOVs[1]);
	Weapon.ZoomFOVs[2] = FOV - (DefaultFOV - Weapon.default.ZoomFOVs[2]);
	Weapon.ZoomFOVs[3] = FOV - (DefaultFOV - Weapon.default.ZoomFOVs[3]);
}

function SetViewFOV(PlayerController Player){
	local Weapon Weapon;
	local float CurrentFOV;

	if(Player == None)
		return;

	if(Player.Pawn != None && Player.Pawn.Weapon != None){
		Weapon = Player.Pawn.Weapon;
		UpdateWeaponZoomFOVs(Weapon);
		CurrentFOV = Weapon.ZoomFOVs[Weapon.CurrentZoomFOVIndex];
		Weapon.SetWeapFOV(CurrentFOV);
	}else{
		CurrentFOV = FOV;
	}

	Player.DefaultFOV = FOV;
	Player.DesiredFOV = CurrentFOV;
	Player.FOVAngle = CurrentFOV;
}

event SetFOV(PlayerController Player, float NewFOV){
	FOV = NewFOV;

	SaveConfig();
	SetViewFOV(Player);
}

// View shake

function PlayerControllerShakeView(float InTime, float SustainTime, float OutTime, float XMag, float YMag, float ZMag, float YawMag, float PitchMag, float Frequency){
	if(ViewShake > 0){
		PlayerController(PlayerControllerShakeViewOverride.CurrentSelf).ShakeView(InTime,
		                                                                          SustainTime,
		                                                                          OutTime,
		                                                                          XMag * ViewShake,
		                                                                          YMag * ViewShake,
		                                                                          ZMag * ViewShake,
		                                                                          YawMag * ViewShake,
		                                                                          PitchMag * ViewShake,
		                                                                          Frequency);
	}
}

// Menu stuff

simulated function MenuBaseCallMenuClass(String MenuClassName, optional String Args){
	local MenuBase Menu;

	if(MenuClassName == "XInterfaceCTMenus.CTGameOptionsPCMenu")
		MenuClassName = "Mod.ModGameOptionsMenu";

	Menu = MenuBase(MenuBaseCallMenuClassOverride.CurrentSelf);

	Menu.CallMenu(Menu.Spawn(class<Menu>(DynamicLoadObject(MenuClassName, class'Class')), Menu.Owner), Args);
}

simulated function MenuBaseOverlayMenuClass(String MenuClassName, optional String Args){
	local MenuBase Menu;

	if(MenuClassName == "XInterfaceCTMenus.CTGameOptionsPCMenu")
		MenuClassName = "Mod.ModGameOptionsMenu";

	Menu = MenuBase(MenuBaseOverlayMenuClassOverride.CurrentSelf);

	Menu.OverlayMenu(Menu.Spawn(class<Menu>(DynamicLoadObject(MenuClassName, class'Class')), Menu.Owner), Args);
}

simulated function MenuBaseGotoMenuClass(String MenuClassName, optional String Args){
	local MenuBase Menu;

	if(MenuClassName == "XInterfaceCTMenus.CTGameOptionsPCMenu")
		MenuClassName = "Mod.ModGameOptionsMenu";

	Menu = MenuBase(MenuBaseGotoMenuClassOverride.CurrentSelf);

	Menu.GotoMenu(Menu.Spawn(class<Menu>(DynamicLoadObject(MenuClassName, class'Class')), Menu.Owner), Args);
}

cpptext
{
	void Init();

	static USWRCFix* Instance;
}

defaultproperties
{
	ViewShake=1.0
	FOV=85.0
	BaseFOV=85.0
	HUDArmsFOVFactor=1.0
	LimitHudArmsFOV=True
	AutoFOV=True
	EnableCustomMenu=True
	EnableEditorSelectionFix=True
	OverrideD3DRenderDevice=True
}
