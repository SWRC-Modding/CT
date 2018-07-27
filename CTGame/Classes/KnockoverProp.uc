// ====================================================================
//  Class:  CTGame.KnockoverProp
//  Parent: Engine.KarmaProp
//
//  Base Class for Props
// ====================================================================

class KnockoverProp extends KarmaProp
	native
	showcategories(Karma)
	placeable;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var bool KnockedOver;
var() static Name KnockoverAnim;

// Default behaviour when triggered is to wake up the physics.
function Trigger( actor Other, pawn EventInstigator )
{
	if (!KnockedOver)
	{
		SetPhysics(PHYS_Karma);
		KWake();
	}
	KnockedOver=true;
}



cpptext
{
	virtual INT AddMyMarker(AActor *S);

}

defaultproperties
{
     KnockoverAnim="PrepSnipeCover"
     bKTakeShot=False
     Physics=PHYS_None
     NavPtLocs(0)=(Offset=(X=165),Yaw=32768)
     NavPtClass=Class'Engine.CoverPoint'
     Begin Object Class=KarmaParams Name=KickoverKParams
         KMass=10
         KLinearDamping=0.01
         KAngularDamping=0.01
         KBuoyancy=1
         KStartAngVel=(Y=-400)
         bHighDetailOnly=False
         KFriction=10
         KImpactThreshold=100
         Name="KickoverKParams"
     End Object
     KParams=KarmaParams'CTGame.KnockoverProp.KickoverKParams'
}

