class RtxLight extends Object within RtxInterface native transient hidecategories(Object, None);

var editconst RtxInterface.RtxHandle Handle;

var(Common) enum ERtxLightType{
	RTXLIGHT_Sphere,
	RTXLIGHT_Rect,
	RTXLIGHT_Disk,
	RTXLIGHT_Cylinder,
	RTXLIGHT_Distant
} Type;

var         bool   bShouldBeDestroyed;
var(Common) bool   bEnabled;
var(Common) bool   bUseShaping;
var(Common) vector Position;
var(Common) color  Color;
var(Common) float  Radiance;

// Used by sphere, rect and disc if bUseShaping
var(Common) struct RtxLightShaping{
	var() vector Direction;
	var() float  ConeAngleDegrees;
	var() float  ConeSoftness;
	var() float  FocusExponent;
} Shaping;

var(SphereLight) struct RtxSphereLight{
	var() float Radius;
} Sphere;

var(RectLight) struct RtxRectLight{
	var() vector XAxis;
	var() vector YAxis;
	var() float  XSize;
	var() float  YSize;
	var() vector Direction;
} Rect;

var(DiskLight) struct RtxDiskLight{
	var() vector XAxis;
	var() vector YAxis;
	var() float  XRadius;
	var() float  YRadius;
	var() vector Direction;
} Disk;

var(CylinderLight) struct RtxCylinderLight{
	var() vector Axis;
	var() float  Length;
	var() float  Radius;
} Cylinder;

var(DistantLight) struct RtxDistantLight{
	var() vector Direction;
	var() float  AngularDiameterDegrees;
} Distant;

native final function Update(); // Must be called whenever a property value has changed

cpptext
{
    virtual void Destroy();
    virtual void PostEditChange(){ Super::PostEditChange(); Update(); }
    void Update();
}

defaultproperties
{
	bEnabled=True
	Color=(R=255,G=255,B=255,A=255)
	Radiance=10000
	Sphere=(Radius=2.5)
}
