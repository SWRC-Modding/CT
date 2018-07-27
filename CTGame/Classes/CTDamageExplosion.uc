// ====================================================================
//  Class:  CTInventory.CTDamageExplosion
//
//  <Enter a description here>
//
// (c) 2003, LucasArts, Inc - All Rights Reserved
// ====================================================================

class CTDamageExplosion extends CTDamageType
	abstract;



defaultproperties
{
     DeathString="%k blew up %o."
     KApplyImpulseToRoot=True
     RagdollOnDeathProbability=1
     KDamageImpulse=800
     KDeathUpKick=700
     HurtAudioEvent=PAE_HurtExplosion
     DeathAudioEvent=PAE_DieExplosion
     AllyDownedByEvent=PAE_DownedByGrenade
     LeaderKilledOtherEvent=PAE_GoodThrowLeader
}

