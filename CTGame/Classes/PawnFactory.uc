// ====================================================================
//  Class:  Factory
//  Parent: Engine.Actor
//
//  Factories contain the logic for spawning new actors at spawn points
// ====================================================================

class PawnFactory extends Factory
	placeable
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var(Factory) class<CTPawn>				Prototype;
var(Factory) name						AIScriptFromFactory;
var(Factory) name						PawnTagFromFactory;				// the tag associated with the pawns generated from this factory
var(Factory) name						FirstDamagedEventFromFactory;	// the first damaged event associated with the pawns generated from this factory
var(Factory) name						PawnEventFromFactory;			// the event associated with the pawns generated from this factory
var(Factory) array<Pawn.PatrolPoint>	PatrolRouteFromFactory;			// List of points to walk when on patrol
var(Factory) Pawn.EPatrolMode			PatrolModeFromFactory;			// Way in which to traverse the list of patrol points
var(Factory) float						PatrolPriorityFromFactory;		// Priority of Patrol State
var(Factory) Pawn.EStartState			StartStateFromFactory;			// Pawn's StartState
var(Factory) int						TeamIndexFromFactory;			// the current team index for this pawn
var(Factory) float						OptimalWeaponDistFromFactory;	// the optimal dist for this pawn's weapon
var(Factory) float						SightRadiusFromFactory;			// pawn sight radius
var(Factory) bool						bAlwaysUseFactorySettings;		// use the factory settings instead of spawn point settings
var(Factory) bool						PreserveDeadBodiesFromFactory;
var(Factory) bool						DisableEmotesFromFactory;
var(Factory) bool						CanBeLeaderFromFactory;
var(Factory) bool						TossWeaponOnDeathFromFactory;
var(Factory) bool						DebugAIFromFactory;


function bool SetupNewActor(Actor NewActor, SpawnPoint SpawnPt)
{
	local Pawn NewPawn;
	local AIScript A;

	Super.SetupNewActor(NewActor, SpawnPt);

	if (NewActor != None)
	{
		NewPawn = Pawn(NewActor);
		if (SpawnMode == SPM_FromFactory || bAlwaysUseFactorySettings)
		{
			NewPawn.AIScriptTag			= AIScriptFromFactory;
			NewPawn.PatrolMode			= PatrolModeFromFactory;
			NewPawn.PatrolRoute			= PatrolRouteFromFactory;
			NewPawn.PatrolPriority		= PatrolPriorityFromFactory;
			NewPawn.StartState			= StartStateFromFactory;
			NewPawn.Tag					= PawnTagFromFactory;
			NewPawn.Event				= PawnEventFromFactory;
			NewPawn.FirstDamagedEvent	= FirstDamagedEventFromFactory;
			NewPawn.bTossWeaponOnDeath	= TossWeaponOnDeathFromFactory;
			NewPawn.bDebugAI			= DebugAIFromFactory;

			if( CanBeLeaderFromFactory )
				NewPawn.bIsSquadLeader = true;

			if( TeamIndexFromFactory >= 0 )
				NewPawn.TeamIndex = TeamIndexFromFactory;

			if( SightRadiusFromFactory >= 0 )
				NewPawn.SightRadius = SightRadiusFromFactory;

			if( PreserveDeadBodiesFromFactory )
				NewPawn.bSaveMyCorpse = PreserveDeadBodiesFromFactory;
		}
		else
		{
			NewPawn.AIScriptTag			= SpawnPt.AIScriptTag;
			NewPawn.PatrolMode			= SpawnPt.PatrolMode;
			NewPawn.PatrolRoute			= SpawnPt.PatrolRoute;
			NewPawn.PatrolPriority		= SpawnPt.PatrolPriority;
			NewPawn.StartState			= SpawnPt.StartState;			
			NewPawn.Tag					= SpawnPt.PawnTag;
			NewPawn.Event				= SpawnPt.PawnEvent;
			NewPawn.FirstDamagedEvent	= SpawnPt.FirstDamagedEvent;
			NewPawn.bTossWeaponOnDeath	= SpawnPt.TossWeaponOnDeath;
			NewPawn.bDebugAI			= SpawnPt.DebugAI;

			if( SpawnPt.CanBeLeader )
				NewPawn.bIsSquadLeader = true;

			if( SpawnPt.TeamIndex >= 0 )
				NewPawn.TeamIndex = SpawnPt.TeamIndex;

			if( SpawnPt.SightRadius >= 0 )
				NewPawn.SightRadius = SpawnPt.SightRadius;

			if( SpawnPt.PreserveDeadBodies )
				NewPawn.bSaveMyCorpse = SpawnPt.PreserveDeadBodies;
		}
		
		// Init the controller
		if( NewPawn.AIScriptTag != '' )
		{
			ForEach AllActors(class'AIScript',A,NewPawn.AIScriptTag)
				break;
			// let the AIScript spawn and init my controller
			if( A != None )			
				A.SpawnControllerFor(NewPawn);			
		}	

		//only do this work if we didn't find an AIScript controller
		if( A == None ) 
		{
			if( (NewPawn.ControllerClass != None) && (NewPawn.Controller == None) )
				NewPawn.Controller = spawn(NewPawn.ControllerClass);
			if( NewPawn.Controller != None )		
				NewPawn.Controller.Possess(NewPawn);
		}

		if( NewPawn.Controller != None && NewPawn.Controller.IsA('CTBot') )
		{
			if ( ( SpawnMode == SPM_FromFactory || bAlwaysUseFactorySettings ) && DisableEmotesFromFactory )
				CTBot(NewPawn.Controller).EmoteProbability = 0;
			else if( SpawnPt != None && SpawnPt.DisableEmotes )
				CTBot(NewPawn.Controller).EmoteProbability = 0;
		}

		if(NewPawn.Controller != None)
			NewPawn.AddDefaultInventory();

		if( NewPawn.Weapon != None )
		{
			if (SpawnMode == SPM_FromFactory || bAlwaysUseFactorySettings)
			{
				if( OptimalWeaponDistFromFactory > 0 )
					NewPawn.Weapon.OptimalDist = OptimalWeaponDistFromFactory;
			}
			else
			{
				if( SpawnPt.OptimalWeaponDist > 0 )
					NewPawn.Weapon.OptimalDist = SpawnPt.OptimalWeaponDist;
			}
		}

		if (NewPawn == None)
			return false;
		else
			return true;
	}
	return false;
}


// Ask whether an event belongs to this actor; used in the editor
function bool ContainsPartialEvent(string StartOfEventName)
{
	local string EventString;
	EventString = string(PawnEventFromFactory);
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
	TagString = string(PawnTagFromFactory);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}


function PostBeginPlay()
{
	InternalPrototype = Prototype;
	Super.PostBeginPlay();	
}



cpptext
{
	void RenderEditorSelected(FLevelSceneNode* SceneNode,FRenderInterface* RI, FDynamicActor* FDA);

}

defaultproperties
{
     PatrolPriorityFromFactory=0.3
     TeamIndexFromFactory=-1
     OptimalWeaponDistFromFactory=-1
     SightRadiusFromFactory=-1
     TossWeaponOnDeathFromFactory=True
}

