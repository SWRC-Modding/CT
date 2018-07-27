class AnimNotify_DestroyEffect extends AnimNotify
	native;

var() name DestroyTag;
var() bool bExpireParticles;

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
     bExpireParticles=True
}

