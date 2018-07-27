#exec Texture Import File=Textures\S_FluidSurf.pcx Name=S_FluidSurf Mips=Off MASKED=1

class FluidSurfaceInfo extends Info
	showcategories(Movement,Collision,Lighting,LightColor,Karma,Force)
	native
	noexport
	placeable;

var () enum EFluidGridType
{
	FGT_Square,
	FGT_Hexagonal
} FluidGridType;

var () float						FluidGridSpacing; // distance between grid points
var () int							FluidXSize; // num vertices in X direction
var () int							FluidYSize; // num vertices in Y direction

var () float						FluidHeightScale; // vertical scale factor

var () float						FluidSpeed; // wave speed
var () float						FluidDamping; // between 0 and 1

var () float						FluidNoiseFrequency;
var () range						FluidNoiseStrength;

var () bool							TestRipple;
var () float						TestRippleSpeed;
var () float						TestRippleStrength;
var () float						TestRippleRadius;

var () float						UTiles;
var () float						UOffset;
var	() float						VTiles;
var () float						VOffset;
var () float						AlphaCurveScale;
var () float						AlphaHeightScale;
var () byte 						AlphaMax;

var () float						ShootStrength; // How hard to ripple water when shot
var () float						ShootRadius; // How large a radius is affected when water is shot

// How much to ripple the water when interacting with actors
var () float						RippleVelocityFactor;
var () float						TouchStrength;

// Class of effect spawned when water surface it shot or touched by an actor
var () class<Actor>					ShootEffect;
var () bool							OrientShootEffect;

var () class<Actor>					TouchEffect;
var () bool							OrientTouchEffect;

// Bitmap indicating which water verts are 'clamped' ie. dont move
var const array<int>				ClampBitmap;

// Terrain used for auto-clamping water verts if below terrain level.
var () edfindable TerrainInfo		ClampTerrain;

var () bool							bShowBoundingBox;
var () bool							bUseNoRenderZ;
var () float						NoRenderZ;

// Amount of time to simulate during postload before water is first displayed
var () float						WarmUpTime;

// Rate at which fluid sim will be updated
var () float						UpdateRate;

var () color						FluidColor;

// Sim storage
var transient const array<float>	Verts0;
var transient const array<float>	Verts1;
var transient const array<byte>		VertAlpha;

var transient const int				LatestVerts;

var transient const box				FluidBoundingBox;	// Current world-space AABB
var transient const vector			FluidOrigin;		// Current bottom-left corner

var transient const float			TimeRollover;
//var transient const float			AverageTimeStep;
//var transient const int  			StepCount;
var transient const float			TestRippleAng;

var transient const FluidSurfacePrimitive Primitive;
var transient const array<FluidSurfaceOscillator>	Oscillators;
var transient const bool			bHasWarmedUp;

// Functions

// Ripple water at a particlar location.
// Ignores 'z' componenet of position.
native final function Pling(vector Position, float Strength, optional float Radius);

// Default behaviour when shot is to apply an impulse and kick the KActor.
simulated function float TakeDamage(float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName)
{
	//Log("FS TakeDam:"$hitlocation@damageType);

	// Vibrate water at hit location.
	Pling(hitLocation, ShootStrength, ShootRadius);	

	// If present, spawn splashy hit effect.
	/*
	if( (ShootEffect != None) && EffectIsRelevant(HitLocation,false) )
	{
		if(OrientShootEffect)
			spawn(ShootEffect, self, , hitLocation, rotator(momentum));
		else
			spawn(ShootEffect, self, , hitLocation);
	}
	*/

	return Damage;
}

simulated function Touch(Actor Other)
{
	local vector touchLocation;

	Super.Touch(Other);

	if( (Other == None) || !Other.bDisturbFluidSurface )
		return;

	touchLocation = Other.Location;

	Pling(touchLocation, ShootStrength, Other.CollisionRadius);

	// JTODO: Fix for non-horizontal fluid
	touchLocation.Z = Location.Z;
	if( (TouchEffect != None) && EffectIsRelevant(touchLocation,false) )
	{
		if(OrientTouchEffect)
			spawn(TouchEffect, self, , touchLocation, rotator(Other.Velocity));
		else
			spawn(TouchEffect, self, , touchLocation);
	}
}


defaultproperties
{
     FluidGridType=FGT_Hexagonal
     FluidGridSpacing=24
     FluidXSize=48
     FluidYSize=48
     FluidHeightScale=1
     FluidSpeed=170
     FluidDamping=0.5
     FluidNoiseFrequency=60
     FluidNoiseStrength=(Min=-70,Max=70)
     TestRippleSpeed=3000
     TestRippleStrength=-20
     TestRippleRadius=48
     UTiles=1
     VTiles=1
     AlphaHeightScale=10
     AlphaMax=128
     ShootStrength=-50
     RippleVelocityFactor=-0.05
     TouchStrength=-50
     WarmUpTime=2
     UpdateRate=50
     DrawType=DT_FluidSurface
     bHidden=False
     bNoDelete=True
     bUnlit=True
     bUseHWOcclusionTests=True
     bCollideActors=True
     bProjTarget=True
     Texture=Texture'Engine.S_FluidSurf'
     TypeOfMaterial=MT_Water
     bEdShouldSnap=True
}

