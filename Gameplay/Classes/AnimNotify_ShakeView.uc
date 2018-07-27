class AnimNotify_ShakeView extends AnimNotify_Scripted;

var()	float		ShakeRadius;
var()	vector		ShakeMag;
var()	rotator		ShakeRotationMag;
var()	float		InTime;
var()	float		SustainTime;
var()	float		OutTime;
var()	float		Frequency;

event NotifyScript( Actor Owner )
{
	local PlayerController PC;
	local float DistToPC, FadeFactor;

	PC = Owner.Level.GetLocalPlayerController();
	if( PC.Pawn != None )
		DistToPC = VSize( PC.Pawn.Location - Owner.Location );
	else
		DistToPC = VSize( PC.Location - Owner.Location );

	if( DistToPC <= ShakeRadius )
	{
		FadeFactor = DistToPC / ShakeRadius;
		PC.ShakeView(InTime, SustainTime, OutTime, ShakeMag.X * FadeFactor, ShakeMag.Y * FadeFactor, ShakeMag.Z * FadeFactor, ShakeRotationMag.Yaw * FadeFactor, ShakeRotationMag.Pitch * FadeFactor, Frequency);
	}
}


defaultproperties
{
     ShakeRadius=500
     ShakeMag=(X=5,Y=5,Z=7)
     ShakeRotationMag=(Pitch=500,Yaw=500)
     InTime=0.3
     SustainTime=2
     OutTime=0.7
     Frequency=1
}

