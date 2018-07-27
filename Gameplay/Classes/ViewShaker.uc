//=============================================================================
// ViewShaker:  Shakes view of any playercontrollers 
// within the ShakeRadius
//=============================================================================
class ViewShaker extends Triggers;

//-----------------------------------------------------------------------------
// Variables.

var()	float		ShakeRadius;			// radius within which to shake player views
var()	vector		ShakeMag;
var()	rotator		ShakeRotationMag;
var()	float		InTime;
var()	float		SustainTime;
var()	float		OutTime;
var()	float		Frequency;



//-----------------------------------------------------------------------------
// Functions.

function Trigger( actor Other, pawn EventInstigator )
{
	local Controller C;

	for ( C=Level.ControllerList; C!=None; C=C.NextController )
		if ( (PlayerController(C) != None) && (VDistSq(Location, PlayerController(C).ViewTarget.Location) < Square(ShakeRadius)) )		
			C.ShakeView(InTime, SustainTime, OutTime, ShakeMag.X, ShakeMag.Y, ShakeMag.Z, ShakeRotationMag.Yaw, ShakeRotationMag.Pitch, Frequency);
}


defaultproperties
{
     ShakeMag=(Y=5,Z=7)
     ShakeRotationMag=(Pitch=500,Yaw=500)
     InTime=0.1
     SustainTime=0.5
     OutTime=0.4
     Frequency=1
     Texture=Texture'Gameplay.S_SpecialEvent'
}

