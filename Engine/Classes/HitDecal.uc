//=============================================================================
// HitDecal
// base class of CT damage decals
//=============================================================================
class HitDecal extends Projector;

var float LifeTime;

//function PostBeginPlay()
simulated event PostBeginPlay()
{
	AttachProjector();
	AbandonProjector(LifeTime);
	Destroy();
}

/* Reset() 
reset actor to initial state - used when restarting level without reloading.
*/
simulated function Reset()
{
	Destroy();
}


defaultproperties
{
     LifeTime=15
     FOV=0
     MaxTraceDistance=32
     bProjectTerrain=False
     bProjectStaticMesh=False
     bProjectActor=False
     bClipBSP=True
     bUseHWOcclusionTests=False
}

