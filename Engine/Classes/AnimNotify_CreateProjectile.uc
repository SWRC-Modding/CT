class AnimNotify_CreateProjectile extends AnimNotify
	native;

var() class<Projectile>		ProjectileClass;
var() name					Bone;
var() vector				OffsetLocation;
var() rotator				OffsetRotation;
var() vector				DrawScale3D;
var() float					LifeSpan;

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
     DrawScale3D=(X=1,Y=1,Z=1)
}

