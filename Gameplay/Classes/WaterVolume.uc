class WaterVolume extends PhysicsVolume;

var string EntrySoundName, ExitSoundName, EntryActorName;

function PostBeginPlay()
{
	Super.PostBeginPlay();

	if ( EntrySoundName != "" )
		EntrySound = Sound(DynamicLoadObject(EntrySoundName,class'Sound'));
	if ( ExitSoundName != "" )
		ExitSound = Sound(DynamicLoadObject(ExitSoundName,class'Sound'));
	if ( EntryActorName != "" )
		EntryActor = class<Actor>(DynamicLoadObject(EntryActorName,class'Class'));	
}


defaultproperties
{
     EntrySoundName="PlayerSounds.FootstepWater1"
     ExitSoundName="GeneralImpacts.ImpactSplash2"
     FluidFriction=2.4
     bWaterVolume=True
     bDistanceFog=True
     DistanceFogColor=(B=128,G=64,R=32,A=64)
     DistanceFogStart=8
     DistanceFogEnd=2000
     KExtraLinearDamping=0.8
     KExtraAngularDamping=0.1
     LocationName="under water"
}

