class AnimNotify_MeleeHit extends AnimNotify
	native;

var() name		BoneName;
var() vector	Offset;

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
}

