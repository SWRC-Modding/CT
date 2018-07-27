//=============================================================================
// Clone Troooper Player Controller
//
// Created: 2003 Jan 21, John Hancock
//=============================================================================

class CTSquad extends CTSquadNative
	placeable;

#exec Texture Import File=Textures\S_CTSquad.pcx  Name=S_CTSquad Mips=Off MASKED=1

event PostBeginPlay()
{	
	super.PostBeginPlay();
	RallyActor = Spawn(class'RallyIconActor',,,Location,Rotation);
	RallyActor.bHidden = true;
	OptionalRallyActor = Spawn(class'RallyIconActor',,,Location,Rotation);
	OptionalRallyActor.bHidden = true;
	Enable('Tick');
}


defaultproperties
{
     RallyPointMesh=StaticMesh'MarkerIcons.Commands.EngageIcon'
     SearchAndDestroyMesh=StaticMesh'MarkerIcons.SetTrap.DtatonRadius'
     OptionalSDMesh=StaticMesh'MarkerIcons.SetTrap.BombRadius'
     SecureAreaEffect=Class'CTEffects.HoloHUD_SecureArea'
     EngageEffectClass=Class'CTEffects.HoloHUD_EngageEnemy'
     SquadDeathSound=SoundMultiple'Character_Voice.Clone_Advisor_1.CA1_SquadDead'
     SquadIncapSound=SoundMultiple'Character_Voice.Clone_Advisor_1.CA1_SquadIncapacitated'
     Stance=Class'CTGame.StanceFormUp'
     Texture=Texture'CTGame.S_CTSquad'
}

