//=============================================================================
// Footstep info: A class which contains a collection of mappings from 
// material type to sounds to play
//=============================================================================
class FootstepInfo extends Object
	dependsOn(AnimNotify_Footstep)
	native
	abstract;


struct FootstepSoundInfo {
	var() Actor.EMaterialType StepOnMaterial;
	var() Sound 			  FootstepSound;	
};

struct FootstepSoundMap {
	var() editinline Array<FootstepSoundInfo> Footsteps;
	var() AnimNotify_Footstep.eMaterialWalkType WalkType;
};

var() Array<FootstepSoundMap> FootstepSounds;


defaultproperties
{
}

