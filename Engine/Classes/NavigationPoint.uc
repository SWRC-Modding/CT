//=============================================================================
// NavigationPoint.
//
// NavigationPoints are organized into a network to provide AIControllers 
// the capability of determining paths to arbitrary destinations in a level
//
//=============================================================================
class NavigationPoint extends Actor
	hidecategories(Lighting,LightColor,Karma,Force)
	native;

#exec Texture Import File=Textures\S_Pickup.pcx Name=S_Pickup Mips=Off MASKED=1
#exec Texture Import File=Textures\SpwnAI.pcx Name=S_NavP Mips=Off MASKED=1

enum ENavConnectionType
{
	NCT_Normal,
	NCT_NoJump,
	NCT_NoDoor,
	NCT_NoPerch,
	NCT_NoFly,
	NCT_NoBottleneck,
	NCT_NotBlocked,
};

//------------------------------------------------------------------------------
// NavigationPoint variables
var const array<ReachSpec> PathList;					//index of reachspecs (used by C++ Navigation code)
var edfindable array<NavigationPoint> ForcedLinks;		// list of NavigationPoints which should always be connected from this path
var edfindable array<NavigationPoint> IgnoredList;		//5Feb2003 JAH -- added in accordance w/ UDN page on Pathing Improvements

var transient int PathSession;						//a path-session ID
var const NavigationPoint nextNavigationPoint;	//for singly linked master list of navigation points
var const ReachSpec	prevLink;
var const float	DistanceToNextObjective;		//used for caching distance to next squad objective
//var int PrevCost;
var int cost;							// added cost to visit this pathnode
var() int ExtraCost;					// Extra weight added by level designer
//var	transient int FearCost;				// extra weight diminishing over time (used for example, to mark path where bot died)
var transient int FutureCost;			// the estimated cost to reach the goal
var() float CeilingHeight;
var() float RadiusOverride;				//an override for the collision radius
var Pawn MostRecentOccupant;			//the occupant of a reserved node
var bool taken;							// set when a creature is occupying this spot (only used for teleporting in)
var() bool		bBlocked;				// this path is currently unuseable 
var() bool		bPropagatesSound;		// this navigation point can be used for sound propagation (around corners)
var() bool		bOneWayPath;			// reachspecs from this path only in the direction the path is facing (180 degrees)
var() bool		bCombatNode;			// is this a combat node?
var	  bool		bCoverPoint;			// is this a cover point
var	bool		bSpecialMove;			// if true, pawn will call SuggestMovePreparation() when moving toward this node
var bool		bNoAutoConnect;			// don't connect this path to others except with special conditions (used by LiftCenter, for example)
var const bool	bOpen;					// A* Variable -- whether we're on the open queue
var const bool  bClosed;				// A* variable -- whether we're on the closed queue
var(Advanced) bool	bNotBased;				// used by path builder - if true, no error reported if node doesn't have a valid base
var const bool  bPathsChanged;			// used for incremental path rebuilding in the editor
var bool		bDestinationOnly;		// used by path building - means no automatically generated paths are sourced from this node
var	bool		bSourceOnly;			// used by path building - means this node is not the destination of any automatically generated path
var bool		bUseRadiusWhenBasing;	// whether we should use the radius when basing the node
var bool		bSquadShouldCrouch;		// the squad should crouch when coming or going from this node
var() bool		bValidSecurePosition;
var(AI) bool	bUseGrenades;			// whether AI should use grenades at this coverpoint/combat point
var(AI) bool	bUseAntiArmor;
var(AI) bool	bUseSniper;
var(AI)	bool	bDontIdleHere;			// when in idle mode, try not to stand here
var const bool	bThrownGrenade;
var const bool	bAntiArmorUsed;
var const bool  bSniperUsed;
var bool bTempNoCollide;				// used during path building
var const bool bRemoveMeOnRebuild;		// remove this node when paths are next rebuilt
var() bool		bEnabled;				// Special properties (cover, combat, perch) are active. LDs can trigger to enable and disable them
var	bool	bMayPreferVsCover;			// an LD specifiable property for combat points that allows them to be preferred vs. a cover point
var() bool  bOverrideCeilingHeight;		// override the auto-calculated ceiling height
var BYTE	TrapCount;					// the number of traps that affect this node
var BYTE	referenceCount;				// Number of AI trying to use this node
var const BYTE	CCRegions[7];			// Connected-Component Regions

native event SetBlocked(bool blocked);
native function ReserveNode(Pawn ReservedBy);
native function ReleaseNode(Pawn ReleasedBy);

function PostBeginPlay()
{
	ExtraCost = Max(ExtraCost,0);
	Super.PostBeginPlay();
}

final function int RefCount()
{
	return referenceCount;
}

event int SpecialCost(Pawn Seeker, ReachSpec Path);

// Accept an actor that has teleported in.
// used for random spawning and initial placement of creatures
event bool Accept( actor Incoming, actor Source )
{
	// Move the actor here.
	taken = Incoming.SetLocation( Location );
	if (taken)
	{
		Incoming.Velocity = vect(0,0,0);
		Incoming.SetRotation(Rotation);
	}
	Incoming.PlayTeleportEffect(true, false);
	TriggerEvent(Event, self, Pawn(Incoming));
	return taken;
}
 
/* SuggestMovePreparation()
Optionally tell Pawn any special instructions to prepare for moving to this goal
(called by Pawn.PrepareForMove() if this node's bSpecialMove==true
*/
event bool SuggestMovePreparation(Pawn Other)
{
	return false;
}

/* ProceedWithMove()
Called by Controller to see if move is now possible when a mover reports to the waiting
pawn that it has completed its move
*/
function bool ProceedWithMove(Pawn Other)
{
	return true;
}

/* MoverOpened() & MoverClosed() used by NavigationPoints associated with movers */
function MoverOpened();
function MoverClosed();


defaultproperties
{
     RadiusOverride=-1
     bPropagatesSound=True
     bValidSecurePosition=True
     bEnabled=True
     bStatic=True
     bHidden=True
     bNoDelete=True
     bCollideWhenPlacing=True
     Texture=Texture'Engine.S_NavP'
     CollisionRadius=80
     CollisionHeight=100
}

