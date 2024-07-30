class SWRCFix extends Object config native transient;

#exec OBJ LOAD FILE="Properties.u"
#exec OBJ LOAD FILE="CloneTextures.utx"
#exec OBJ LOAD FILE="HudArmsTextures.utx"

struct MPPawnHudArmsShaderPair{
	var Material Pawn;
	var Material HudArms;
};

var() config bool OverrideMPSkins; // Override the default MP skins with the ones that are in the game but unused
var() config array<MPPawnHudArmsShaderPair> MPCloneHudArmsShaders;
var() config array<MPPawnHudArmsShaderPair> MPTrandoHudArmsShaders;

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

var FunctionOverride CTPlayerEndZoomOverride;
var FunctionOverride CTPlayerResetFOVOverride;

var FunctionOverride PlayerControllerShakeViewOverride;

var FunctionOverride WeaponSetWeapFOVOverride;

var FunctionOverride MenuBaseCallMenuClassOverride;
var FunctionOverride MenuBaseOverlayMenuClassOverride;
var FunctionOverride MenuBaseGotoMenuClassOverride;

var FunctionOverride MPPawnSetHudArmTextureOverride;

event InitScript()
{
	local int i;

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

	if(EnableCustomMenu)
	{
		// MenuBase
		MenuBaseCallMenuClassOverride = new class'FunctionOverride';
		MenuBaseCallMenuClassOverride.Init(class'MenuBase', 'CallMenuClass', self, 'MenuBaseCallMenuClass');
		MenuBaseOverlayMenuClassOverride = new class'FunctionOverride';
		MenuBaseOverlayMenuClassOverride.Init(class'MenuBase', 'OverlayMenuClass', self, 'MenuBaseOverlayMenuClass');
		MenuBaseGotoMenuClassOverride = new class'FunctionOverride';
		MenuBaseGotoMenuClassOverride.Init(class'MenuBase', 'GotoMenuClass', self, 'MenuBaseGotoMenuClass');
	}

	MPPawnSetHudArmTextureOverride = new class'FunctionOverride';
	MPPawnSetHudArmTextureOverride.Init(class'MPPawn', 'SetHudArmTexture', self, 'MPPawnSetHudArmTexture');

	if(OverrideMPSkins)
	{
		for(i = 0; i < MPCloneHudArmsShaders.Length && i < arraycount(class'CTCharacters.MPClone'.default.MPSkins); ++i)
			class'CTCharacters.MPClone'.default.MPSkins[i] = MPCloneHudArmsShaders[i].Pawn;
	}
}

simulated function MPPawnSetHudArmTexture(Weapon Weapon)
{
	local int i;
	local MPPawn Pawn;
	local Material Skin;

	Pawn = MPPawn(MPPawnSetHudArmTextureOverride.CurrentSelf);

	Weapon.CopyMaterialsToSkins();

	if(Weapon.HudArmsShaderIndex[0] != -1 && Pawn.Skins.Length > 0)
	{
		if(Pawn.bIsTrandoshan)
		{
			for(i = 0; i < MPTrandoHudArmsShaders.Length; ++i)
			{
				if(Pawn.Skins[0] == MPTrandoHudArmsShaders[i].Pawn)
				{
					Skin = MPTrandoHudArmsShaders[i].HudArms;
					break;
				}
			}
		}
		else
		{
			for(i = 0; i < MPCloneHudArmsShaders.Length; ++i)
			{
				if(Pawn.Skins[0] == MPCloneHudArmsShaders[i].Pawn)
				{
					Skin = MPCloneHudArmsShaders[i].HudArms;
					break;
				}
			}
		}

		if(Skin != None)
		{
			if(Pawn.bIsTrandoshan)
				Weapon.Skins[Weapon.HudArmsShaderIndex[1]] = Skin;
			else
				Weapon.Skins[Weapon.HudArmsShaderIndex[0]] = Skin;
		}
	}
}

event float GetDefaultFOV()
{
	if(OverrideDefaultFOV)
		return BaseFOV;

	return class'CTPlayer'.default.FOVAngle;
}

function CTPlayerEndZoom()
{
	local CTPlayer Player;

	Player = CTPlayer(CTPlayerEndZoomOverride.CurrentSelf);

	Player.EndZoom();
	SetViewFOV(Player);
}

function CTPlayerResetFOV()
{
	local CTPlayer Player;

	Player = CTPlayer(CTPlayerResetFOVOverride.CurrentSelf);

	Player.DesiredFOV = Player.DefaultFOV;
	Player.FOVAngle = Player.DefaultFOV;

	if(Player.Pawn != None && Player.Pawn.Weapon != None)
		Player.Pawn.Weapon.SetWeapFOV(Player.FOVAngle);

	Player.NoHUDArms(false);
}

function WeaponSetWeapFOV(float NewFOV)
{
	local Weapon Weapon;
	local float HudArmsFOV;

	Weapon = Weapon(WeaponSetWeapFOVOverride.CurrentSelf);
	HudArmsFOV = GetDefaultFOV() + HUDArmsFOVFactor * (NewFOV - GetDefaultFOV());

	UpdateWeaponZoomFOVs(Weapon);

	if(LimitHudArmsFOV && !Weapon.bWeaponZoom)
	{
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

function UpdateWeaponZoomFOVs(Weapon Weapon)
{
	local float DefaultFOV;

	DefaultFOV = GetDefaultFOV();

	Weapon.ZoomFOVs[0] = FOV;

	// Changing the default zoom Fovs so that it will not zoom in as much with a higher Fov selected
	Weapon.ZoomFOVs[1] = FOV - (DefaultFOV - Weapon.default.ZoomFOVs[1]);
	Weapon.ZoomFOVs[2] = FOV - (DefaultFOV - Weapon.default.ZoomFOVs[2]);
	Weapon.ZoomFOVs[3] = FOV - (DefaultFOV - Weapon.default.ZoomFOVs[3]);
}

function SetViewFOV(PlayerController Player)
{
	local Weapon Weapon;
	local float CurrentFOV;

	if(Player == None)
		return;

	if(Player.Pawn != None && Player.Pawn.Weapon != None)
	{
		Weapon = Player.Pawn.Weapon;
		UpdateWeaponZoomFOVs(Weapon);
		CurrentFOV = Weapon.ZoomFOVs[Weapon.CurrentZoomFOVIndex];
		Weapon.SetWeapFOV(CurrentFOV);
	}
	else
	{
		CurrentFOV = FOV;
	}

	Player.DefaultFOV = FOV;
	Player.DesiredFOV = CurrentFOV;
	Player.FOVAngle = CurrentFOV;
}

event SetFOV(PlayerController Player, float NewFOV)
{
	FOV = NewFOV;

	SaveConfig();
	SetViewFOV(Player);
}

// View shake

function PlayerControllerShakeView(float InTime, float SustainTime, float OutTime, float XMag, float YMag, float ZMag, float YawMag, float PitchMag, float Frequency)
{
	if(ViewShake > 0)
	{
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

simulated function MenuBaseCallMenuClass(String MenuClassName, optional String Args)
{
	local MenuBase Menu;

	if(MenuClassName == "XInterfaceCTMenus.CTGameOptionsPCMenu")
		MenuClassName = "Mod.ModGameOptionsMenu";

	Menu = MenuBase(MenuBaseCallMenuClassOverride.CurrentSelf);

	Menu.CallMenu(Menu.Spawn(class<Menu>(DynamicLoadObject(MenuClassName, class'Class')), Menu.Owner), Args);
}

simulated function MenuBaseOverlayMenuClass(String MenuClassName, optional String Args)
{
	local MenuBase Menu;

	if(MenuClassName == "XInterfaceCTMenus.CTGameOptionsPCMenu")
		MenuClassName = "Mod.ModGameOptionsMenu";

	Menu = MenuBase(MenuBaseOverlayMenuClassOverride.CurrentSelf);

	Menu.OverlayMenu(Menu.Spawn(class<Menu>(DynamicLoadObject(MenuClassName, class'Class')), Menu.Owner), Args);
}

simulated function MenuBaseGotoMenuClass(String MenuClassName, optional String Args)
{
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
	static UBOOL     RenderingReady; // Used by ModRenderDevice to only render once everything is started up. This avoids loading tons of textures for no reason.
}

defaultproperties
{
	ViewShake=1.0
	FOV=85.0
	BaseFOV=85.0
	FpsLimit=60
	HUDArmsFOVFactor=1.0
	LimitHudArmsFOV=True
	AutoFOV=True
	EnableCustomMenu=True
	EnableEditorSelectionFix=True
	OverrideMPSkins=True
	MPCloneHudArmsShaders(0)=(Pawn=Shader'CloneTextures.CloneTextures.CloneCommandoWhite_Shader',HudArms=Shader'HudArmsTextures.HudArms.HudArmsWhite_Shader')
	MPCloneHudArmsShaders(1)=(Pawn=Shader'CloneTextures.CloneTextures.MP_CloneCommandoD_Shader',HudArms=Shader'HudArmsTextures.HudArms.MP_HudArmsD_Shader')
	MPCloneHudArmsShaders(2)=(Pawn=Shader'CloneTextures.CloneTextures.MP_CloneCommandoB_Shader',HudArms=Shader'HudArmsTextures.HudArms.MP_HudArmsB_Shader')
	MPCloneHudArmsShaders(3)=(Pawn=Shader'CloneTextures.CloneTextures.MP_CloneCommandoC_Shader',HudArms=Shader'HudArmsTextures.HudArms.MP_HudArmsC_Shader')
	MPCloneHudArmsShaders(4)=(Pawn=Shader'CloneTextures.CloneTextures.MP_CloneCommandoA_Shader',HudArms=Shader'HudArmsTextures.HudArms.MP_HudArmsA_Shader')
	MPCloneHudArmsShaders(5)=(Pawn=Shader'CloneTextures.CloneTextures.CloneCommando40_Shader',HudArms=Shader'HudArmsTextures.HudArms.HudArmsWhite_Shader')
	MPCloneHudArmsShaders(6)=(Pawn=Shader'CloneTextures.CloneTextures.CloneCommando62_Shader',HudArms=Shader'HudArmsTextures.HudArms.HudArmsWhite_Shader')
	MPCloneHudArmsShaders(7)=(Pawn=Shader'CloneTextures.CloneTextures.CloneCommando07_Shader',HudArms=Shader'HudArmsTextures.HudArms.HudArmsWhite_Shader')
	MPCloneHudArmsShaders(8)=(Pawn=Shader'CloneTextures.CloneTextures.CloneCommando38_Shader',HudArms=Shader'HudArmsTextures.HudArms.HudArms_Shader')
	MPTrandoHudArmsShaders(0)=(Pawn=Shader'CloneTextures.TrandoshanMercTextures.TrandoshanMerc_Shader',HudArms=Shader'HudArmsTextures.HudArms.tHudArms_Shader')
	MPTrandoHudArmsShaders(1)=(Pawn=Shader'CloneTextures.TrandoshanMercTextures.MP_TrandoshanMercA_Shader',HudArms=Shader'HudArmsTextures.HudArms.MP_tHudArmsA_Shader')
	MPTrandoHudArmsShaders(2)=(Pawn=Shader'CloneTextures.TrandoshanMercTextures.MP_TrandoshanMercB_Shader',HudArms=Shader'HudArmsTextures.HudArms.MP_tHudArmsB_Shader')
	MPTrandoHudArmsShaders(3)=(Pawn=Shader'CloneTextures.TrandoshanMercTextures.MP_TrandoshanMercC_Shader',HudArms=Shader'HudArmsTextures.HudArms.MP_tHudArmsC_Shader')
	MPTrandoHudArmsShaders(4)=(Pawn=Shader'CloneTextures.TrandoshanMercTextures.MP_TrandoshanMercD_Shader',HudArms=Shader'HudArmsTextures.HudArms.MP_tHudArmsD_Shader')
}
