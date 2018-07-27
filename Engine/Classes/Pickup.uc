//=============================================================================
// Pickup items.
//
// Pickup is the base class of actors that when touched by an appropriate pawn, 
// will create and place an Inventory actor in that pawn's inventory chain.  Each 
// pickup class has an associated inventory class (its InventoryType).  Pickups are 
// placed by level designers.  Pickups can only interact with pawns when in their 
// default Pickup state.  Pickups verify that they can give inventory to a pawn by 
// calling the GameInfo's PickupQuery() function.  After a pickup spawns an inventory 
// item for a pawn, it then queries the GameInfo by calling the GameInfo's 
// ShouldRespawn() function about whether it should remain active, enter its Sleep 
// state and later become active again, or destroy itself.
//
// When navigation paths are built, each pickup has an InventorySpot (a subclass 
// of NavigationPoint) placed on it and associated with it 
// (the Pickup's MyMarker== the InventorySpot, 
// and the InventorySpot's markedItem == the pickup).     
//
//=============================================================================
class Pickup extends Actor
	abstract
	placeable
	native
	nativereplication;


//-----------------------------------------------------------------------------
// AI related info.

//var	  NavigationPoint MyMarker;
var() class<Inventory> InventoryType;
var() bool		bInstantRespawn;	  // Can be tagged so this item respawns instantly.
var	  bool		bOnlyReplicateHidden;	// only replicate changes in bHidden (optimization for level pickups)
var		bool	bDropped;
var		bool	bPredictRespawns;	  // high skill bots may predict respawns for this item
var		bool	bAddToNav;			  // whether this pickup creates a NavPt (Marker) for itself
var		bool	bDroppedWeaponStay;	  // dropped weapon pickups shouldn't ever stick around forever - only initial pickups
var()	bool	bForceRespawn;		  // allow the pickup to respawn in SP - for the tutorial
var		bool	bSpecialPickup;       // Pickups that can be  picked up if bCanSpecialPickup is true
var() float     RespawnTime;          // Respawn after this time, 0 for instant.
var	 float RespawnEffectTime;

var() float		DroppedLifetime;
var() float		MPCollisionRadius;
var() float		MPCollisionHeight;

var() localized string PickupMessage; // Human readable description when picked up.
var() sound PickupSound;
var() string PickupForce;  // jdf
var() enum EPickupType
{
	PUT_Health,
	PUT_Ammo,
	PUT_Weapon,
	PUT_Grenade
} PickupType;

var() Mesh AlternateHUDArmsMesh;

//native final function AddToNavigation();			// cache dropped inventory in navigation network
//native final function RemoveFromNavigation();
function AddToNavigation();
function RemoveFromNavigation();
function bool PickupUsedBy( Pawn Other );

static function StaticPrecache(LevelInfo L);

function PostNetBeginPlay()
{
	super.PostNetBeginPlay();
	bDroppedWeaponStay = Level.Game.bWeaponStay;
}

function Destroyed()
{
	//if (MyMarker != None )
	//	MyMarker.markedItem = None;	
	if (Inventory != None )
		Inventory.Destroy();	
}

/* Reset() 
reset actor to initial state - used when restarting level without reloading.
*/
function Reset()
{
	if ( Inventory != None )
		destroy();
	else
	{
		GotoState('Pickup');
		Super.Reset();
	}
}

function RespawnEffect();

// Turns the pickup into a different type of pickup - specificly used by the WildcardCharger
function Pickup Transmogrify(class<Pickup> NewClass) // de
{
	local Pickup NewPickup;

	NewPickup = Spawn(NewClass);
	NewPickup.RespawnTime = RespawnTime;

	/*
	if (MyMarker != None )
	{
		MyMarker.markedItem = NewPickup;
		NewPickup.MyMarker = MyMarker;
		MyMarker = None;
	}
	*/
	Destroy();

	return NewPickup;
}


// Either give this inventory to player Other, or spawn a copy
// and give it to the player Other, setting up original to be respawned.
//
function inventory SpawnCopy( pawn Other )
{
	local inventory Copy;

	if ( Inventory != None )
	{
		Copy = Inventory;
		Inventory = None;
	}
	else
		Copy = spawn(InventoryType,Other,,,rot(0,0,0));

	if (Other.IsA('MPTrandoshan'))
		Copy.AlternateHUDArmsMesh = AlternateHUDArmsMesh;

	Copy.GiveTo( Other );

	return Copy;
}

function StartSleeping()
{
    if (bDropped)
        Destroy();
    else
	GotoState('Sleeping');
}

function AnnouncePickup( Pawn Receiver )
{
	Receiver.HandlePickup(self);
	if ( (Receiver != None) && (PlayerController(Receiver.Controller) != None) )
		PlayerController(Receiver.Controller).ClientPlaySoundLocally(PickupSound);
}

//
// Set up respawn waiting if desired.
//
function SetRespawn()
{
	if (bDroppedWeaponStay)
		return;

	if( Level.Game.ShouldRespawn(self) || bForceRespawn )
		StartSleeping(); 
	else
		Destroy();
}

// HUD Messages

static function string GetLocalString(
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1, 
	optional PlayerReplicationInfo RelatedPRI_2
	)
{
	return Default.PickupMessage;
}

function InitDroppedPickupFor(Inventory Inv)
{	
	SetPhysics(PHYS_Falling);
	GotoState('FallingPickup');
	Inventory = Inv;
	bAlwaysRelevant = false;
	bOnlyReplicateHidden = false;
	bUpdateSimulatedPosition = true;
    bDropped = true;
	if( Level.NetMode != NM_Standalone ) // REVISIT: Is this the correct way to do this??
		LifeSpan = 30;
	bIgnoreEncroachers=false; // handles case of dropping stuff on lifts etc
	bDroppedWeaponStay = false;			// dropped weapons should never have this be true - bug #11143
}

function bool ReadyToPickup(float MaxWait)
{
	return false;
}

event Landed(Vector HitNormal)
{	
	GotoState('Pickup');
}

function FadeOutPickup()
{
	GotoState('FadeOut');
}

//=============================================================================
// Pickup state: this inventory item is sitting on the ground.

auto state Pickup
{
	function bool ReadyToPickup(float MaxWait)
	{
		return true;
	}

	/* ValidTouch()
	 Validate touch (if valid return true to let other pick me up and trigger event).
	*/
	function bool ValidTouch( Pawn Other )
	{		
		//Log("ValidTouch: live player "$Other$" CanPickup "$Other.bCanPickupInventory$" Health "$Other.Health);
		// make sure its a live player
		if ( (Other == None)|| (Other.Health <= 0) || (  !Other.bCanPickupInventory && !( Other.bCanSpecialPickup && bSpecialPickup ) ) )
			return false;

		//Log("ValidTouch: touching wall");
		// make sure not touching through wall
		//if ( !FastTrace(Other.Location, Location ) )
		//	return false;

		// make sure game will let player pick me up
		if( Level.Game.PickupQuery(Other, self) )
		{
			TriggerEvent(Event, self, Other);
			return true;
		}
		return false;
	}
	
	// Player picks up automatically if it's an ammo pickup
	function Touch( actor A )
	{
		local Inventory Copy;
		local Pawn Other;

		Other = Pawn(A);

		if ( (Other != None) && (Other.FindInventoryType(InventoryType) == None) )
			return;

		// If touched by a player pawn, let him pick this up.
		if( ValidTouch(Other) )
		{
			Copy = SpawnCopy(Other);
			AnnouncePickup(Other);
            SetRespawn(); 
            if ( Copy != None )
				Copy.PickupFunction(Other);
		}
	}
	
	// Player activated pickup if it's a weapon pickup.
	function bool PickupUsedBy( Pawn Other )
	{
		local Inventory Copy;

		// If touched by a player pawn, let him pick this up.
		if( ValidTouch(Other) )
		{
			Copy = SpawnCopy(Other);
			AnnouncePickup(Other);
            SetRespawn(); 
            if ( Copy != None )
				Copy.PickupFunction(Other);
			return true;
		}
		return false;
	}

	// Make sure no pawn already touching (while touch was disabled in sleep).
	function CheckTouching()
	{
		local Pawn P;

		ForEach TouchingActors(class'Pawn', P)
			Touch(P);
	}

	function Timer()
	{
		if ( bDropped )
			GotoState('FadeOut');
	}

	function BeginState()
	{
		if ( (Level != None) && (Level.NetMode == NM_Standalone) )
		{
			// Turn off Karma
			KSetBlockKarma(false);
			SetPhysics(PHYS_Falling);
		}

		if ( bDropped )
        {
			AddToNavigation();
			if ( DroppedLifetime > 0 && Level.NetMode != NM_Standalone )
				SetTimer(DroppedLifetime, false);
		}		
	}

	function EndState()
	{
		if ( bDropped )
			RemoveFromNavigation();
	}

Begin:
	CheckTouching();
}

state FallingPickup extends Pickup
{
	function CheckTouching()
	{
	}
	
	function Timer()
	{
		GotoState('FadeOut');
	}

	function BeginState()
	{		
		if ( (Level != None) && (Level.NetMode == NM_Standalone) )
		{
			// Turn on Karma collision for the fall
			KarmaParams(KParams).KMass = 2; // NathanM: Hack to get a better weight
			SetPhysics( PHYS_Karma );
			KSetBlockKarma( true );
			KWake();
		}
		
		if (DroppedLifetime > 0 && Level.NetMode != NM_StandAlone )
		{
			SetTimer(DroppedLifetime, false);
		}		
	}	

	function float TakeDamage( float Damage, Pawn InstigatedBy, Vector HitLocation, Vector Momentum, class<DamageType> DamageType, optional Name BoneName )
	{
		if( Physics == PHYS_Karma )
			KAddImpulse(momentum,HitLocation);

		return 0;
	}
}

State FadeOut extends Pickup
{
	function Tick(float DeltaTime)
	{
		SetDrawScale(FMax(0.01, DrawScale - Default.DrawScale * DeltaTime));
	}
	
	function BeginState()
	{
		RotationRate.Yaw=60000;
		SetPhysics(PHYS_Rotating);
		LifeSpan = 1.0;
	}
	
	function EndState()
	{
		LifeSpan = 0.0;
		SetDrawScale(Default.DrawScale);
		if ( Physics == PHYS_Rotating )
			SetPhysics(PHYS_None);
	}
}

//=============================================================================
// Sleeping state: Sitting hidden waiting to respawn.
function float GetRespawnTime()
{
	return RespawnTime; 
}

State Sleeping
{
	ignores Touch;

	function bool ReadyToPickup(float MaxWait)
	{
		return ( bPredictRespawns && (LatentFloat < MaxWait) );
	}

	function StartSleeping() {}

	function BeginState()
	{
		bHidden = true;
		SetCollision(false, false, false);
	}
	function EndState()
	{
		bHidden = false;
		SetCollision(true, false, false);
	}			
Begin:
	Sleep( GetReSpawnTime() - RespawnEffectTime );
	RespawnEffect();
	Sleep(RespawnEffectTime);
    GotoState('Pickup');
}


defaultproperties
{
     bOnlyReplicateHidden=True
     RespawnEffectTime=0.5
     DroppedLifetime=25
     MPCollisionRadius=84
     MPCollisionHeight=8
     PickupMessage="Item"
     PickupType=PUT_Weapon
     DrawType=DT_Mesh
     bLightingVisibility=False
     bOrientOnSlope=True
     bIgnoreEncroachers=True
     bAlwaysRelevant=True
     bOnlyDirtyReplication=True
     bUseHWOcclusionTests=True
     bCollideActors=True
     bCollideWorld=True
     bBlockZeroExtentTraces=False
     bUseCylinderCollision=True
     NetUpdateFrequency=8
     NetPriority=1.4
     Texture=Texture'Engine.S_Inventory'
     Begin Object Class=KarmaParams Name=PickupKParams
         KLinearDamping=0
         KAngularDamping=0
         KBuoyancy=1
         bHighDetailOnly=False
         KFriction=0.8
         KImpactThreshold=100
         Name="PickupKParams"
     End Object
     KParams=KarmaParams'Engine.Pickup.PickupKParams'
     MessageClass=Class'Engine.PickupMessagePlus'
}

