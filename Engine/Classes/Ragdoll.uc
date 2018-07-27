//=============================================================================
// The Basic constraint class.
//=============================================================================

class Ragdoll extends KActor
	native
	placeable;

var bool Activated;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	if( KParams != None && KarmaParams(KParams).KStartEnabled )
        Activated = true;        		
}

event PostLoadBeginPlay()
{
	if( Activated )	
		Destroy();	
}

event Trigger( Actor Other, Pawn EventInstigator )
{
	Super.Trigger(Other, EventInstigator);

	if( KIsAwake() )
		Activated = true;
}

function float TakeDamage(float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName)
{
	local float DamageTaken;
	
	DamageTaken = Super.TakeDamage( Damage, instigatedBy, hitlocation, momentum, damageType, BoneName );

	if( KIsAwake() )
		Activated = true;

	return DamageTaken;
}


defaultproperties
{
     Physics=PHYS_KarmaRagDoll
     DrawType=DT_Mesh
     bWorldGeometry=False
     Begin Object Class=KarmaParamsSkel Name=RagdollKParams
         KConvulseSpacing=(Max=2.2)
         KLinearDamping=0.15
         KAngularDamping=0.05
         KBuoyancy=1
         KStartEnabled=True
         KVelDropBelowThreshold=50
         bHighDetailOnly=False
         KFriction=0.9
         KRestitution=0.1
         KImpactThreshold=500
         Name="RagdollKParams"
     End Object
     KParams=KarmaParamsSkel'Engine.Ragdoll.RagdollKParams'
}

