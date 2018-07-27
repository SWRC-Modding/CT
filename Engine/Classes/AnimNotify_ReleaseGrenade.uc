class AnimNotify_ReleaseGrenade extends AnimNotify
	native;

var() name		Bone;
var() vector	ReleaseDirection;
var() float		ReleaseSpeed;
var() bool		UseFlatTrajectory;

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
     ReleaseDirection=(X=1,Z=1)
     ReleaseSpeed=2000
}

