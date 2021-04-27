class SWRCFix extends Object config native transient;

#exec OBJ LOAD FILE="Properties.u"

var() config float   FpsLimit;
var() config float   FOV;
var() config float   HudArmsFOVFactor;
var() config bool    LimitHudArmsFOV;
var() config bool    EnableCustomMenu;
var() config bool    EnableEditorSelectionFix; // Only here for convenience. The actual fix is in ModRenderDevice

var FunctionOverride CTPlayerEndZoomOverride;
var FunctionOverride CTPlayerResetFOVOverride;

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
	HudArmsFOV = class'CTPlayer'.default.FOVAngle + HUDArmsFOVFactor * (NewFOV - class'CTPlayer'.default.FOVAngle);

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

	DefaultFOV = class'CTPlayer'.default.DefaultFOV;

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
	FOV=85.0
	HUDArmsFOVFactor=1.0
	LimitHudArmsFOV=true
	EnableCustomMenu=true
	EnableEditorSelectionFix=true
}
