class AnimNotify_Footstep extends AnimNotify
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

enum eMaterialWalkType
{
	WalkT_Forward,
	WalkT_Backward,
	WalkT_CrouchWalk,
	WalkT_Scuff,
	WalkT_Slide,
	WalkT_RunForward,
	WalkT_RunBackward,
	WalkT_ArmorSlide,
	WalkT_JumpUp,
	WalkT_JumpDown,
	WalkT_BFPrimary,
	WalkT_BFSecondary
};

var() Sound				 DefaultSound;
var() eMaterialWalkType  TypeOfWalk;




cpptext
{
	// AnimNotify interface.
	virtual void Notify( UMeshInstance *Instance, AActor *Owner );

}

defaultproperties
{
}

