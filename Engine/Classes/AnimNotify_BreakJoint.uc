class AnimNotify_BreakJoint extends AnimNotify
	native;

var()	bool	DestroyChildren;	
var()	name	Bone;

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

