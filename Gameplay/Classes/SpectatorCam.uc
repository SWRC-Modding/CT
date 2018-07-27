//=============================================================================
// SpectatorCam.
//=============================================================================
class SpectatorCam extends KeyPoint notplaceable;

var() bool bSkipView; // spectators skip this camera when flipping through cams
var() float FadeOutTime;	// fade out time if used as EndCam


defaultproperties
{
     FadeOutTime=5
     bClientAnim=True
     Texture=Texture'Engine.S_Camera'
     CollisionRadius=20
     CollisionHeight=40
     bDirectional=True
}

