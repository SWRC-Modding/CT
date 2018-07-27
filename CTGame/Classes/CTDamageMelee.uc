class CTDamageMelee extends CTDamageType
	native
	abstract;


defaultproperties
{
     DeathString="%o was beaten to death by %k."
     bBypassesShields=True
     RagdollOnDeathProbability=1
     KDamageImpulse=200
     HurtAudioEvent=PAE_HurtMelee
     LeaderKilledOtherEvent=PAE_GoodMeleeLeader
     FriendlyFireEvent=PAE_FriendlyFireMelee
     VisorHitEffect=Class'CTEffects.Blue_Sparks'
}

