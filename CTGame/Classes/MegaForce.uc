// ====================================================================
//  Class:  CTGame.MegaForce
//  Parent: Engine.Actor
//
//  Class for applying impulses to Karma objects
// ====================================================================

class MegaForce extends Actor
	placeable
	hidecategories(Force,Karma,Marker,LightColor,Lighting,Sound);

#exec Texture Import File=..\Engine\Textures\S_MegaForce.bmp Name=S_MegaForce Mips=Off MASKED=1

enum EImpulseType
{
	IT_Line,
	IT_Sphere
};

var()	bool				bApplyToCenterOfMass;			// Use the object's location (not the hitlocation) when applying impulse. Only valid for IT_Line
var()	bool				bUseEmitterDirection;			// Use the MegaForce's direction. If set to false will use the vector from Megaforce to HitObject
var()	bool				bAutoActivateAffected;			// Automatically wake the affected actors

var()	EImpulseType		ImpulseType;
var()	float				ForceDistance;
var()	float				ForceAmount;


function Trigger( Actor Other, Pawn EventInstigator )
{
	local Actor TestActor;	
	local vector Impulse;
	local vector HitLocation, HitNormal;
	local vector RotationDir;

	switch (ImpulseType)
	{
	case IT_Line:
		Impulse = (vector(Rotation) * ForceAmount);
		ForEach TraceActors( class 'Actor', TestActor, HitLocation, HitNormal, Location + (vector(Rotation) * ForceDistance), Location )
		{
			if( TestActor != None && TestActor.KParams != None )
			{
				if( bAutoActivateAffected )
					TestActor.Trigger( self, None );

                if( TestActor.Physics == PHYS_Karma || TestActor.Physics == PHYS_KarmaRagdoll )
				{
					if( bApplyToCenterOfMass )
						TestActor.TakeDamage( 0, None, TestActor.Location, Impulse, class'CTDamageExplosion', 'root' );
					else
						TestActor.TakeDamage( 0, None, HitLocation, Impulse, class'DamageType' );
				}
			}
		}
		break;
	case IT_Sphere:
		RotationDir = Vector(Rotation);
		ForEach CollidingActors( class 'Actor',  TestActor, ForceDistance, Location )
		{
			if( TestActor != None && TestActor.KParams != None )
			{
				if( bAutoActivateAffected )
					TestActor.Trigger( self, None );

				if( TestActor.Physics == PHYS_Karma || TestActor.Physics == PHYS_KarmaRagdoll )
				{
					if( bUseEmitterDirection )
					{
						Impulse = RotationDir * ForceAmount;
						TestActor.TakeDamage( 0, None, TestActor.Location, Impulse, class'CTDamageExplosion', 'root' );
					}
					else
					{
						Impulse = Normal( TestActor.Location - Location ) * ForceAmount;
						TestActor.TakeDamage( 0, None, TestActor.Location, Impulse, class'CTDamageExplosion', 'root' );
					}
				}
			}
		}
		break;
	}	
}


defaultproperties
{
     ForceDistance=1000
     bHidden=True
     Texture=Texture'CTGame.S_MegaForce'
     bDirectional=True
}

