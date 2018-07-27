// ====================================================================
//  Class:  Engine.ActivateItem
//
//  Activate is used in a sense to setup something, whether it's to
//  setup a bomb or engage in the disarming of a trap.  Deactivate is
//  used in a sense to complete the action, such as the completion of
//  setting a bomb, the completion of disarming a trap, or the completion
//  of a door breach.
//
// Created by Jenny Huang on September 22, 2003
// Moved into Engine project by John Hancock on Dec. 11, 2003
// ====================================================================

class ActivateItem extends Actor
	placeable
	native;

enum EMarkerItemDifficulty
{
	MID_Easy,
	MID_Medium,
	MID_Hard,
	MID_Hardest
};

enum EActivateItemType
{
	AIT_HackTerminal,
	AIT_HackDoor,
	AIT_BreachDoor,
	AIT_SetTrap,
	AIT_DisarmTrap,
	AIT_Demolition,
	AIT_Plunger,
	AIT_BactaDispenser,
	AIT_ProximityMine
};

struct ActivationInfo
{
	var()	Name						Event;				// The event to trigger on entering a state
	var()	static class<Emitter>		TransitionEffect;	// The effect to spawn on entering the state
	var()	static Vector				TransitionEffectOffset;	//The offset of the transition effect
	var()	static Sound				TransitionSound;	// The sound to play on entering the state
	var()	static Material				Material;			// The material to use in this state
	var()	static int					MaterialIndex;		// The material index to swap
	var()	static StaticMesh			Mesh;				// The static mesh to use in this state
	var()	static float				Duration;			// Time until transitioning into the next state - usedBy or Tick
	var()	static class<DamageType>	MyDamageType;		// resulting damage
	var()	float						Damage;				// amount of damage
	var()	static float				MomentumTransfer;	// momentum caused by effect
	var()	static float				DamageRadius;		// radius of damage
};

var()		class<Ammunition>    AmmunitionRequired;//the type of inventory required, if necessary
var			EActivateItemType	ActivateItemType;	// type of activate item
var()		EMarkerItemDifficulty Difficulty;		// changes the state durations
var()		name				EventUnderway;		// triggered when the item is used
var			bool				bIsUsable;			// can the item be used by the player
var			bool				bActivated;			// is it setup and activated?
var			bool				bActivating;		// is the player still setting up?
var			bool				bUsed;				// if a state change has occurred because of a player action
var			bool				bPlayerActivated;	// is this activated by the player
var()		bool				bTriggeredByDamage; //whether the object can be triggered by weapons fire
var()		bool				bIsUsableByPlayer;
var			bool				bLockPlayerLocation;	//whether using this item should lock the player location
var			static bool			bSavesProgress;		//whether this object saves the elapsed time if activation is aborted
var			static bool			bNonPlayerUsesTimer;	//whether this non-players use the BeginActivating function to countdown
var			static	BYTE		HUDIconX;			//when there is no marker attached to the item the icon comes from here
var			static  BYTE		HUDIconY;			//when there is no marker attached to the item the icon comes from here
var			float				ElapseSetupTime;	// time that's passed while setting up
var			float				DurationRemaining;	// the amount of time remaining to complete performance
var()		static float		PlayerRange;	// distance within which player can activate item
var()		static float		PlayerOffset;	// the horizontal distance at which the player will snap to
var			float				FractionComplete;
var()		static float		ActivateDotProdMax;
var			Controller			User;				// who is currently using this item
var			class<Actor>		AttachItemClass;

// Prompt vars
//var (Marker) localized String   InventoryPromptText;
var (Marker) localized String	ActivatePromptText;
var (Marker) Array<String>		ActivatePromptButtonFuncs;

function NextState();   
function UseSetupMesh();
function InternalSetup();
native event float DurationMultiplier();

event float DefaultDuration()
{
	return default.DurationRemaining * DurationMultiplier();
}

event StartSetup(Controller Controller)
{
	User = Controller;
	Instigator = Controller.Pawn;
	InternalSetup();
	bActivating = true;
}

event bool HasRequiredInventory(Pawn Pawn)
{
	local Ammunition AmmoType;

	if (AmmunitionRequired == None)
		return true;
	AmmoType = Ammunition(Pawn.FindInventoryType(AmmunitionRequired));
	if (AmmoType == None)
		return false;
	return (AmmoType.AmmoAmount > 0);
}

function DeductAmmo()
{
	local Pawn Pawn;
	local Ammunition AmmoType;

	if (AmmunitionRequired != None && User != None)
	{
		Pawn = User.Pawn;
		if (Pawn.Squad != None)
			Pawn = Pawn.Squad.SquadLeader;
		AmmoType = Ammunition(Pawn.FindInventoryType(AmmunitionRequired));
		if (AmmoType == None)
			return;

		AmmoType.DeductAmmo();
	}
}

function PostBeginPlay()
{
	DurationRemaining = DefaultDuration();
	SetLocation(self.Location);
	super.PostBeginPlay();
}

event PostLoadBeginPlay()
{
	//Log("ActivateItem::PostLoadBeginPlay");
	//only abort the setup if the item was usable and User is a valid player
	if (User != None && User.bIsPlayer && bIsUsable)
		AbortSetup();
	Super.PostLoadBeginPlay();
}

function MakeTransition(ActivationInfo info)
{
	if (info.Mesh != None)
		SetStaticMesh(info.Mesh);
	if (info.Material != None)
	{
		CopyMaterialsToSkins();
		Skins[info.MaterialIndex] = info.Material;
	}
	if (info.TransitionSound != None)
		PlaySound(info.TransitionSound);
	if ( info.TransitionEffect != None )
		Spawn(info.TransitionEffect,,,self.Location+info.TransitionEffectOffset);
	if ( info.Event != 'None' )
		TriggerEvent( info.Event, Self, None );
	if (info.Damage > 0 && info.DamageRadius > 0)
		HurtRadius(info.Damage,info.DamageRadius, info.MyDamageType, info.MomentumTransfer, self.Location );
}

function BeginActivating(ActivationInfo State, float deltaTime)
{
	local float Duration;
	//local PlayerController PC;

	if (User == None)
	{
		AbortSetup();
		return;
	}
	//if we're not supposed to use the auto-timer, just return
	if (User != None && !User.bIsPlayer && !bNonPlayerUsesTimer)
		return;

	//continue activation if we have a user and
	//if our user is the player, make sure he is still looking at us
	//and holding down the use button
	if (User != None && (!User.bIsPlayer || 
		(PlayerController(User).bUse && PlayerController(User).TargetType == TGT_Panel)))
	{	
		Duration = State.Duration * DurationMultiplier();
		//Log("ElapseSetupTime "$ElapseSetupTime$" ChangeTime "$State.Duration);
		
		if (ElapseSetupTime + deltaTime >= Duration)
		{
			NextState();
			bActivating = false;
			DurationRemaining = 0.0;
		}
		else
		{
			ElapseSetupTime += deltaTime;
			DurationRemaining -= deltaTime;
			FractionComplete = ElapseSetupTime / Duration;
		}
	}
	else
	{
		AbortSetup();
	}
}

event AbortSetup()
{
	bActivating = false;
	bPlayerActivated = false;
	User = None;
	if (!bSavesProgress)
	{
		DurationRemaining = DefaultDuration();
		ElapseSetupTime = 0;
		FractionComplete = 0;
		GotoState('Auto');
	}
}

// Ask whether an event belongs to this actor; used in the editor
event bool ContainsPartialEvent(string StartOfEventName)
{
	local string EventString;
	EventString = string(EventUnderway);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}
	return Super.ContainsPartialEvent(StartOfEventName);
}

function bool ActivateInfoContainsPartialEvent(ActivationInfo I, string StartOfEventName)
{
	local string EventString;
	EventString = string(I.Event);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}
	return false;
}


defaultproperties
{
     bIsUsable=True
     bIsUsableByPlayer=True
     PlayerRange=192
     PlayerOffset=80
     ActivatePromptText="PRESS AND HOLD @ TO ACTIVATE"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     bDrawHUDMarkerIcon=True
     bCollideActors=True
     bProjTarget=True
     bImmobile=True
     bEdShouldSnap=True
}

