class ACTION_ShakeView extends ScriptedAction;

var()	vector		ShakeTranslationMag;
var()	rotator		ShakeRotationMag;
var()	float		InTime;
var()	float		SustainTime;
var()	float		OutTime;
var()	float		Frequency;

function bool InitActionFor(ScriptedController C)
{
	local PlayerController PC;

	ForEach C.AllActors(class'PlayerController', PC)
		break;

	if ( PC != None )
		PC.ShakeView(InTime, SustainTime, OutTime, ShakeTranslationMag.X, ShakeTranslationMag.Y, ShakeTranslationMag.Z, ShakeRotationMag.Yaw, ShakeRotationMag.Pitch, Frequency);

	return false;	
}


defaultproperties
{
     ShakeTranslationMag=(X=3,Y=3,Z=3)
     ShakeRotationMag=(Pitch=200,Yaw=200)
     InTime=0.3
     SustainTime=1
     OutTime=0.7
     Frequency=1
     ActionString="ShakeView"
}

