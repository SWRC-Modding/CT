class RibbonEmitter extends ParticleEmitter
	native;

struct native RibbonPoint
{
	var() vector Location;
	var() vector AxisNormal;
	var() float Width;
};

enum EGetPointAxis
{
	PAXIS_OwnerX, // owners X axis based on rotation
	PAXIS_OwnerY, // owners Y axis based on rotation
	PAXIS_OwnerZ, // owners Z axis based on rotation
	PAXIS_BoneNormal, // (end - start) or start bone direction if no end bone found
	PAXIS_StartBoneDirection, // start bones direction
	PAXIS_AxisNormal // specified normal
};

// main vars
var(Ribbon) float SampleRate;
var(Ribbon) float DecayRate;
var(Ribbon) int NumPoints;
var(Ribbon) float RibbonWidth;
var(Ribbon) EGetPointAxis GetPointAxisFrom;
var(Ribbon) vector AxisNormal; // used for PAXIS_AxisNormal
var(Ribbon) float MinSampleDist;
var(Ribbon) float MinSampleDot;
var(Ribbon) float PointOriginOffset;

// texture UV scaling
var(RibbonTexture) float RibbonTextureUScale;
var(RibbonTexture) float RibbonTextureVScale;

// axis rotated sheets
var(RibbonSheets) int NumSheets; // number of sheets used
var(RibbonSheets) array<float> SheetScale;

// bone vars (emitter must have an actor with a skeletal mesh as its owner)
var(RibbonBones) vector StartBoneOffset;
var(RibbonBones) vector EndBoneOffset;
var(RibbonBones) name BoneNameStart;
var(RibbonBones) name BoneNameEnd;

// ribbon point array
var(Ribbon) array<RibbonPoint> RibbonPoints;

// flags
var(Ribbon) bool bSamplePoints;
var(Ribbon) bool bDecayPoints;
var(Ribbon) bool bDecayPointsWhenStopped;
var(Ribbon) bool bSyncDecayWhenKilled;
var(RibbonTexture) bool bLengthBasedTextureU;
var(RibbonSheets) bool bUseSheetScale;
var(RibbonBones) bool bUseBones;
var(RibbonBones) bool bUseBoneDistance; // get width from distance between start and end bones

// internal vars
var transient float SampleTimer; // sample timer (samples point at SampleTimer >= SampleRate)
var transient float DecayTimer;
var transient float RealSampleRate;
var transient float RealDecayRate;
var transient int SheetsUsed;
var transient RibbonPoint LastSampledPoint;

var transient bool bKilled; // used to init vars when particle emitter is killed
var transient bool bDecaying;


defaultproperties
{
     SampleRate=0.05
     NumPoints=20
     RibbonWidth=20
     GetPointAxisFrom=PAXIS_AxisNormal
     AxisNormal=(Z=1)
     MinSampleDist=1
     MinSampleDot=0.995
     PointOriginOffset=0.5
     RibbonTextureUScale=1
     RibbonTextureVScale=1
     bSamplePoints=True
     bDecayPoints=True
     MaxParticles=1
     StartSizeRange=(X=(Min=1,Max=1),Y=(Min=1,Max=1),Z=(Min=1,Max=1))
     InitialParticlesPerSecond=10000
     UseRegularSizeScale=False
     AutomaticInitialSpawning=False
}

