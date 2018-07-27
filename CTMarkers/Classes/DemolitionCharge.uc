// ====================================================================
//  Class:  CTGame.DemolitionCharge
//
// Created by 2004, February 29, John Hancock
// Begun as duplicate of SetTrap. Kept SetTrap as
// separate in case we wanted to go back or use both
// ====================================================================

class DemolitionCharge extends ExplosiveCharge
	;

state() Armed
{
	function BeginState()
	{
		Super.BeginState();
		ActivateItemType=AIT_Plunger;
		AttachItemClass=class'DemolitionPlungerObject';
	}
}


defaultproperties
{
     ChargeUnplaced=(Material=HardwareShader'MarkerIconTextures.Clone.MarkerDynamic_DemolitionChargeHolo')
     ChargePlaced=(TransitionSound=Sound'GEN_Sound.Impacts_Explos.imp_GEN_demChrgSet_01',Material=Shader'MarkerIconTextures.Demolition.DemolitionCharge_Inactive',Duration=10)
     ChargeArmed=(TransitionSound=Sound'GEN_Sound.Interface.int_GEN_demChrgArm_01',Material=Shader'MarkerIconTextures.Demolition.DemolitionCharge_Active')
     ChargeExplode=(TransitionEffect=Class'CTEffects.Explosion_Sparks',TransitionSound=Sound'GEN_Sound.Impacts_Explos.exp_GEN_demoCharge_01',MyDamageType=Class'CTGame.CTDamageExplosion',Damage=256,DamageRadius=256)
     ExplosionStimulus=ST_DemolitionExplode
     ActivateItemType=AIT_Demolition
     bTriggeredByDamage=False
     bSavesProgress=True
     bNonPlayerUsesTimer=True
     DurationRemaining=10
     AttachItemClass=Class'CTInventory.DemolitionChargeObject'
     ActivatePromptText="PRESS AND HOLD @ TO PLACE DEMOLITION CHARGE"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     StaticMesh=StaticMesh'MarkerIcons.Demolition.DemolitionCharge'
     NavPtLocs(0)=(Offset=(X=85),Yaw=32768)
}

