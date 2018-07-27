// ====================================================================
//  Class:  SpawnPoint
//  Parent: Engine.Keypoint
//
//  Point at which a factory can spawn an actor
// ====================================================================

class SpawnPoint extends Keypoint
	native
	hidecategories(Collision,Force,Karma,LightColor,Lighting,Sound);

#exec Texture Import File=..\Engine\Textures\S_SpawnPoint.bmp Name=S_SpawnPoint Mips=Off MASKED=1

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var() bool						bDrawLinks;					// Draw Links to related Actors
var() bool						PreserveDeadBodies;			// Never destroy bodies spawned from this point
var() bool						DisableEmotes;				// no emotes for pawn from this factory	
var() bool						CanBeLeader;				// Can be fake leader for gestures
var() bool						TossWeaponOnDeath;			// Toss Weapon on Death
var() bool						DebugAI;
var() name						AIScriptTag;				// AI Script associated w/ each Pawn generated from this spawn pt
var() name						PawnTag;					// the tag associated with the pawns generated from this spawn pt
var() name						PawnEvent;					// the event associated with the pawns generated from this spawn pt
var() name						FirstDamagedEvent;			// the first damaged event associated with the pawns generated from this spawn pt
var() array<Pawn.PatrolPoint>	PatrolRoute;				// List of points to walk when on patrol
var() Pawn.EPatrolMode			PatrolMode;					// Way in which to traverse the list of patrol points
var() float						PatrolPriority;				// Priority of Patrol State
var() Pawn.EStartState			StartState;					// Initial State For Pawn
var() EPhysics					InitialPhysics;				// Initial Physics Mode For Actor
var() float						InitialSpeed;				// Initial Speed For Actor
var() int						TeamIndex;					// the current team index for this pawn
var() float						OptimalWeaponDist;			// the optimal dist for this pawn's weapon
var() float						SightRadius;				// Pawn's Sight Radius


function bool ContainsPartialEvent(string StartOfEventName)
{
	local string EventString;
	EventString = string(PawnEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}

	EventString = string(FirstDamagedEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}

	return Super.ContainsPartialEvent(StartOfEventName);
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(AIScriptTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	TagString = string(PawnTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



cpptext
{
	void RenderEditorSelected(FLevelSceneNode* SceneNode,FRenderInterface* RI, FDynamicActor* FDA);

}

defaultproperties
{
     bDrawLinks=True
     TossWeaponOnDeath=True
     PatrolPriority=0.3
     InitialPhysics=PHYS_Falling
     TeamIndex=-1
     OptimalWeaponDist=-1
     SightRadius=-1
     Texture=Texture'CTGame.S_SpawnPoint'
     bDirectional=True
}

