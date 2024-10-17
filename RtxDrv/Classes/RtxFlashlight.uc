class RtxFlashlight extends Object config(Rtx) hidecategories(Object);

var() bool   bIsOn;

struct FlashlightCone{
	var() config float Angle;
	var() config float Radius;
	var() config float Radiance;
	var() config float Softness;
	var() config color Color;
};

var() config Vector         FirstPersonOffset;
var() config float          FocusDistance;
var() config FlashlightCone OuterCone;
var() config FlashlightCone MainCone;
var() config FlashlightCone InnerCone;

var RtxLight OuterLight;
var RtxLight MainLight;
var RtxLight InnerLight;

function CalcLightPositionAndDirection(Pawn Pawn, out vector Position, out vector Direction)
{
	local vector  ViewPosition;
	local rotator ViewRotation;
	local vector X,Y,Z;

	ViewPosition    = Pawn.Location;
	ViewPosition.Z += Pawn.EyeHeight;

	if(Pawn.IsLocallyControlled())
	{
		ViewRotation = Pawn.Controller.Rotation;
		Position     = ViewPosition + QuatRotateVector(QuatFromRotator(ViewRotation), FirstPersonOffset);
	}
	else
	{
		ViewRotation = Pawn.GetBoneRotation('LightAttach');
		GetAxes(ViewRotation, X, Y, Z);
		Position     = Pawn.GetBoneLocation('LightAttach') + Y * 5; // FIXME: Don't hardcode this
	}

	if(FocusDistance >= 1.0)
		Direction = (ViewPosition + vector(ViewRotation) * FocusDistance) - Position;
	else
		Direction = vector(ViewRotation);
}

function UpdateLight(Pawn Pawn, RtxLight Light, FlashlightCone Cone)
{
	Light.Color                    = Cone.Color;
	Light.Radiance                 = Cone.Radiance;
	Light.Sphere.Radius            = Cone.Radius;
	Light.Shaping.ConeAngleDegrees = Cone.Angle;
	Light.Shaping.ConeSoftness     = Cone.Softness;

	CalcLightPositionAndDirection(Pawn, Light.Position, Light.Shaping.Direction);
	Light.Update();
}

function Init()
{
	local Rtx Rtx;

	Rtx = class'Rtx'.static.GetInstance();

	OuterLight             = Rtx.CreateLight();
	OuterLight.Type        = RTXLIGHT_Sphere;
	OuterLight.bEnabled    = false;
	OuterLight.bUseShaping = true;
	OuterLight.Update();

	MainLight              = Rtx.CreateLight();
	MainLight.Type         = RTXLIGHT_Sphere;
	OuterLight.bEnabled    = false;
	MainLight.bUseShaping  = true;
	MainLight.Update();

	InnerLight             = Rtx.CreateLight();
	InnerLight.Type        = RTXLIGHT_Sphere;
	OuterLight.bEnabled    = false;
	InnerLight.bUseShaping = true;
	InnerLight.Update();
}

function Exit()
{
	local Rtx Rtx;

	Rtx = class'Rtx'.static.GetInstance();

	Rtx.DestroyLight(OuterLight);
	Rtx.DestroyLight(MainLight);
	Rtx.DestroyLight(InnerLight);
}

function Update(Pawn Pawn, bool bOn)
{
	if(bOn != bIsOn)
	{
		OuterLight.bEnabled = bOn;
		MainLight.bEnabled  = bOn;
		InnerLight.bEnabled = bOn;
		bIsOn               = bOn;
	}

	if(bIsOn && Pawn != None)
	{
		UpdateLight(Pawn, OuterLight, OuterCone);
		UpdateLight(Pawn, MainLight, MainCone);
		UpdateLight(Pawn, InnerLight, InnerCone);
	}
}

defaultproperties
{
	FirstPersonOffset=(X=0,Y=13,Z=4)
	FocusDistance=256
	OuterCone=(Angle=30,Color=(R=176,G=249,B=255,A=255),Radiance=10000,Radius=2,Softness=0.02)
	MainCone=(Angle=25,Color=(R=253,G=255,B=217,A=255),Radiance=15000,Radius=3,Softness=0.05)
	InnerCone=(Angle=5,Color=(R=254,G=248,B=205,A=255),Radiance=20000,Radius=4,Softness=0)
}
