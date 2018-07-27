//=============================================================================
// The Cone Limit joint class.
//=============================================================================

#exec Texture Import File=Textures\S_KConeLimit.pcx Name=S_KConeLimit Mips=Off MASKED=1

class KConeLimit extends KConstraint
    native
    placeable;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var(KarmaConstraint) float KHalfAngle; // ( 65535 = 360 deg )
var(KarmaConstraint) float KStiffness;
var(KarmaConstraint) float KDamping;

//native final function KSetHalfAngle(float HalfAngle);
//native final function KSetStiffness(float Stiffness);
//native final function KSetDamping(float Damping);




cpptext
{
#ifdef WITH_KARMA
    virtual void KUpdateConstraintParams();
#endif

}

defaultproperties
{
     KHalfAngle=8200
     KStiffness=50
     Texture=Texture'Engine.S_KConeLimit'
     bDirectional=True
}

