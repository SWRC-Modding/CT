//=============================================================================
// Coverpoint
//
// Created: 06Feb2003 John Hancock
//=============================================================================
class CoverPoint extends NavigationPoint
	placeable native;

#exec Texture Import File=..\Engine\Textures\S_Coverpoint.bmp Name=S_Coverpoint Mips=Off MASKED=1

var() bool bOverrideStrafe;						// The user wants to override the generated strafe values
var() bool bOverrideHeight;						// The user wants to override the generated cover height
var() bool bOverrideLean;						// The user wants to override the generated lean values
var() bool bOverrideCorner;
//var() bool bBackToWall;							// Use back-to-the-wall animations if available
var() bool CanLeanLeft, CanLeanRight;			// Can lean left and right
var() bool CanCornerLeft, CanCornerRight;
var   const bool bHadCoverProvider;				// whether there ever was a valid cover provider
var() float FOVdegrees;							// The angle within which this node provides good cover
var() float StrafeDistLeft, StrafeDistRight;	// Distance to strafe left and right
var() float CoverHeight;						// The height of the cover this node repersents
var() float DistanceToCover;					// The distance to the cover
var() Actor CoverProvider;						// The actor that provides cover for this point
var vector LookDir;								// Direction to look while covering
var float minDotProduct;						// Pre-calced version of FOV

native final function bool IsValidForTarget(vector target);

function Trigger( Actor Other, Pawn EventInstigator )
{
	bEnabled = !bEnabled;
}

function PreBeginPlay()
{
	LookDir = vector(Rotation);
	minDotProduct = Cos(PI/360.0 * FOVDegrees); //if in rotator units Cos(PI/32768 * FOVrotunits/2)	
	Super.PreBeginPlay();
}


defaultproperties
{
     FOVdegrees=140
     bCoverPoint=True
     Texture=Texture'Engine.S_Coverpoint'
     bDirectional=True
}

