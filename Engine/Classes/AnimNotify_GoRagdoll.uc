class AnimNotify_GoRagdoll extends AnimNotify
	native;

var()	vector				InitialImpulse;
var()	float				ImpulseMagnitude;
var()	class<DamageType>	ImpulseDamageType;
var()	vector				InitialVelocity;
var()	float				VelocityMagnitude;
var()	name				Bone;
var()	bool				PriorityRagdoll;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)



cpptext
{
	// AnimNotify interface.
	virtual void Notify( UMeshInstance *Instance, AActor *Owner );

}

defaultproperties
{
     ImpulseDamageType=Class'Engine.DamageThrown'
}

