//=============================================================================
// Controller, the base class of players or AI.
//
// Controllers are non-physical actors that can be attached to a pawn to control
// its actions.  PlayerControllers are used by human players to control pawns, while
// AIControFllers implement the artificial intelligence for the pawns they control.
// Controllers take control of a pawn using their Possess() method, and relinquish
// control of the pawn by calling UnPossess().
//
// Controllers receive notifications for many of the events occuring for the Pawn they
// are controlling.  This gives the controller the opportunity to implement the behavior
// in response to this event, intercepting the event and superceding the Pawn's default
// behavior.
//
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class Controller extends Actor
	native
	nativereplication
	abstract;

var Pawn Pawn;

var		Actor		Machine;			// The actor that the player is controlling

var const int		PlayerNum;			// The player number - per-match player number.
var		float		SightCounter;		// Used to keep track of when to check player visibility
var		float		FovAngle;			// X field of view angle in degrees, usually 90.
var globalconfig float	Handedness;
var		bool        bIsPlayer;			// Pawn is a player or a player-bot.
var		bool		bGodMode;			// cheat - when true, can't be killed or hurt

//AI flags
var const bool		bLOSflag;			// used for alternating LineOfSight traces
var		bool		bAdvancedTactics;	// serpentine movement between pathnodes
var		bool		bCanOpenDoors;
var		bool		bCanDoSpecial;
var		bool		bAdjusting;			// adjusting around obstacle
var		bool		bPreparingMove;		// set true while pawn sets up for a latent move
var		bool		bControlAnimations;	// take control of animations from pawn (don't let pawn play animations based on notifications)
var		bool		bNotifyApex;		// event NotifyJumpApex() when at apex of jump
var		bool		bUsePlayerHearing;
var		bool		bJumpOverWall;		// true when jumping to clear obstacle
var		bool		bEnemyAcquired;
var		bool		bShouldWalk;
var		bool		bAdjustFromWalls;	// auto-adjust around corners, with no hitwall notification for controller or pawn
										// if wall is hit during a MoveTo() or MoveToward() latent execution.
var		bool		InAttackState;		//If we're in an attack state (will be used for music manager to check)
var		bool		bEnemyIsPawn;
var		bool		bSmartShot;

// Input buttons.
var input byte
	bRun, bDuck, bFire, bAltFire;

var		vector		AdjustLoc;			// location to move to while adjusting around obstacle

var const Controller	nextController; // chained Controller list

// Navigation AI
var 	float		MoveTimer;
var 	Actor		MoveTarget;		// actor being moved toward
var		vector	 	Destination;	// location being moved toward
var	 	vector		FocalPoint;		// location being looked at
var		Actor		Focus;			// actor being looked at
var		Mover		PendingMover;	// mover pawn is waiting for to complete its move
var	 	float		MinHitWall;		// Minimum HitNormal dot Velocity.Normal to get a HitWall event from the physics
var		float		RespawnPredictionTime;	// how far ahead to predict respawns when looking for inventory
var		int			AcquisitionYawRate;

// Enemy information
var	 	Actor   	Enemy;
var		Actor		Target;
var		float		LastSeenTime;
var		float		WhenEnemyChangedLast;

var float	OldMessageTime;		// to limit frequency of voice messages

// Replication Info
var() class<PlayerReplicationInfo> PlayerReplicationInfoClass;
var PlayerReplicationInfo PlayerReplicationInfo;

var class<Pawn> PawnClass;			// class of pawn to spawn (for players)
var class<Pawn> PreviousPawnClass;	// Holds the player's previous class

var float GroundPitchTime;
var vector ViewX, ViewY, ViewZ;	// Viewrotation encoding for PHYS_Spider

var NavigationPoint StartSpot;  // where player started the match

// for monitoring the position of a pawn
var		vector		MonitorStartLoc;	// used by latent function MonitorPawn()
var		Pawn		MonitoredPawn;		// used by latent function MonitorPawn()
var		float		MonitorMaxDistSq;

//var		AvoidMarker	FearSpots[2];	// avoid these spots when moving

var class<Weapon> LastPawnWeapon;				// used by game stats
native function SetEnemy(Actor Enemy);

const LATENT_MOVETOWARD = 503; // LatentAction number for Movetoward() latent function

replication
{
	reliable if( bNetDirty && (Role==ROLE_Authority) )
		PlayerReplicationInfo, Pawn;
	reliable if( bNetDirty && (Role== ROLE_Authority) && bNetOwner )
		PawnClass;

	// Functions the server calls on the client side.
	reliable if( RemoteRole==ROLE_AutonomousProxy )
		ClientGameEnded, ClientDying, ClientSetRotation, ClientSetLocation,
		/*ClientSwitchToBestWeapon,*/ ClientSetWeapon;
	reliable if (ROLE==ROLE_Authority)
		ClientSwitchToBestWeapon;

	// Functions the client calls on the server.
	unreliable if( Role<ROLE_Authority )
		SendVoiceMessage, SetPawnClass;
	reliable if ( Role < ROLE_Authority )
		ServerRestartPlayer, ServerSwitchGrenade;
}

// Latent Movement.
// Note that MoveTo sets the actor's Destination, and MoveToward sets the
// actor's MoveTarget.  Actor will rotate towards destination unless the optional ViewFocus is specified.

native(502) final latent function MoveToward(actor NewTarget, optional float DestinationOffset, optional bool bShouldWalk);
native(508) final latent function FinishRotation();

//
// Focus direction.
// In UScript, set by 2 functions:
//		SetFocus( actor, optional vector ): sets focus to actor if not none, else vector
//		SetFocusRot( optional rotator): sets focus to rotator
// If Focus actor, focus points to it.
// Else if bRotatetoDesired, focus points in DesiredRotation.
// Else focus points to FocalPoint.
//
function SetFocus( actor Other, optional vector Point )
{
	Focus = Other;
	FocalPoint = Point;
	bRotateToDesired = false;
}

function SetFocusRot( rotator Rot )
{
	DesiredRotation = Rot;
	bRotateToDesired = true;
}

native event rotator GetFocusRotation();
native event vector GetFocusLocation();

// native AI functions
/* LineOfSightTo() returns true if any of several points of Other is visible
  (origin, top, bottom)
*/
native(514) final function bool LineOfSightTo(actor Other);

final function DebugAILog( coerce string S )
{
	if( Pawn != None && Pawn.bDebugAI)
		Log(S); //optional tag
}

/* CanSee() similar to line of sight, but also takes into account Pawn's peripheral vision
*/
//native(533) final function bool CanSee(Pawn Other);

//native(523) final function vector EAdjustJump(float BaseZ, float XYSpeed);

/* PickWallAdjust()
Check if could jump up over obstruction (only if there is a knee height obstruction)
If so, start jump, and return current destination
Else, try to step around - return a destination 90 degrees right or left depending on traces
out and floor checks
*/
native(526) final function bool PickWallAdjust(vector HitNormal);

/* WaitForLanding()
latent function returns when pawn is on ground (no longer falling)
*/
native(527) final latent function WaitForLanding();

//native(540) final function actor FindBestInventoryPath(out float MinWeight);

native(529) final function AddController();
native(530) final function RemoveController();

// Pick best pawn target
native(531) final function pawn PickTarget(out float bestAim, out float bestDist, vector FireDir, vector projStart, float MaxRange);
native(534) final function actor PickAnyTarget(out float bestAim, out float bestDist, vector FireDir, vector projStart);

native final function bool InLatentExecution(int LatentActionNumber); //returns true if controller currently performing latent action specified by LatentActionNumber
// Force end to sleep
native function StopWaiting();

function FriendlyFireCallback(Pawn Ally, class<DamageType> DamageType, int ShotCount);

event MayFall(); //return true if allowed to fall - called by engine when pawn is about to fall

event color GetDebugColor();

function PendingStasis()
{
	bStasis = true;
	Pawn = None;
}

/* DisplayDebug()
list important controller attributes on canvas
*/
function DisplayDebug(Canvas Canvas, out float YL, out float YPos)
{
	if ( Pawn == None )
	{
		Super.DisplayDebug(Canvas,YL,YPos);
		return;
	}

	Canvas.SetDrawColor(255,0,0);
	Canvas.DrawText("CONTROLLER "$GetItemName(string(self))$" Pawn "$GetItemName(string(Pawn)));
	YPos += YL;
	Canvas.SetPos(4,YPos);

	if ( Enemy != None )
		Canvas.DrawText("     STATE: "$GetStateName()$" Timer: "$TimerCounter$" Enemy "$Enemy.GetHumanReadableName(), false);
	else
		Canvas.DrawText("     STATE: "$GetStateName()$" Timer: "$TimerCounter$" NO Enemy ", false);
	YPos += YL;
	Canvas.SetPos(4,YPos);

	if ( PlayerReplicationInfo == None )
		Canvas.DrawText("     NO PLAYERREPLICATIONINFO", false);
	else
		PlayerReplicationInfo.DisplayDebug(Canvas,YL,YPos);

	YPos += YL;
	Canvas.SetPos(4,YPos);
}

simulated function String GetHumanReadableName()
{
	if(PlayerReplicationInfo != None)
		return PlayerReplicationInfo.GetPlayerName();
	return GetItemName(String(self));
}

function rotator GetViewRotation()
{
	return Rotation;
}

/* Reset()
reset actor to initial state
*/
function Reset()
{
	Super.Reset();
	Enemy = None;
	LastSeenTime = 0;
	StartSpot = None;
}

function bool AvoidCertainDeath()
{
	return false;
}

/* ClientSetLocation()
replicated function to set location and rotation.  Allows server to force new location for
teleports, etc.
*/
function ClientSetLocation( vector NewLocation, rotator NewRotation )
{
	SetRotation(NewRotation);
	If ( (Rotation.Pitch > RotationRate.Pitch)
		&& (Rotation.Pitch < 65536 - RotationRate.Pitch) )
	{
		If (Rotation.Pitch < 32768)
			NewRotation.Pitch = RotationRate.Pitch;
		else
			NewRotation.Pitch = 65536 - RotationRate.Pitch;
	}
	if ( Pawn != None )
	{
		NewRotation.Roll  = 0;
		Pawn.SetRotation( NewRotation );
		Pawn.SetLocation( NewLocation );
	}
}

/* ClientSetRotation()
replicated function to set rotation.  Allows server to force new rotation.
*/
function ClientSetRotation( rotator NewRotation )
{
	if (bIsPlayer) //illegal to set rotation on non-player controllers
		SetRotation(NewRotation);
	if ( Pawn != None )
	{
		NewRotation.Pitch = 0;
		NewRotation.Roll  = 0;
		Pawn.SetRotation( NewRotation );
	}
}

function ClientDying(class<DamageType> DamageType, vector HitLocation)
{
	if ( Pawn != None )
	{
		Pawn.PlayDying(DamageType, HitLocation);
		Pawn.GotoState('Dying');
	}
}

event SoakStop(string problem);

function Possess(Pawn aPawn)
{
	aPawn.PossessedBy(self);
	Pawn = aPawn;
	// preserve Pawn's rotation initially for placed Pawns
	SetFocus(Pawn);
	Restart();
}

// unpossessed a pawn (not because pawn was killed)
function UnPossess()
{
    if ( Pawn != None )
        Pawn.UnPossessed();
    Pawn = None;
}

function class<Weapon> GetLastWeapon()
{
	if ( (Pawn == None) || (Pawn.Weapon == None) )
		return LastPawnWeapon;
	return Pawn.Weapon.Class;
}

/* PawnDied()
 unpossess a pawn (because pawn was killed)
 */
function PawnDied(Pawn P)
{
	if ( Pawn != P )
		return;
	if ( Pawn != None && !Pawn.bDeleteMe )
	{
		Pawn.UnPossessed();
	}
	Pawn = None;
	PendingMover = None;
	if ( !bIsPlayer )
		Destroy();
}

simulated function PawnIncapacitated(Pawn P);
simulated function PawnRevived(Pawn P);
function BeginPawnRevive(Pawn P);


function Restart()
{
	Enemy = None;
}

function bool RestartGame()
{
	return false;
}

event LongFall(); // called when latent function WaitForLanding() doesn't return after 4 seconds

// notifications of pawn events (from C++)
// if return true, then pawn won't get notified
event bool NotifyPhysicsVolumeChange(PhysicsVolume NewVolume);
event bool NotifyHeadVolumeChange(PhysicsVolume NewVolume);
event bool NotifyLanded(vector HitNormal);
event bool NotifyHitWall(vector HitNormal, actor Wall);
event bool NotifyBump(Actor Other);
event NotifyHitMover(vector HitNormal, mover Wall);
event NotifyJumpApex();
event NotifyMissedJump();

// notifications called by pawn in script
function NotifyAddInventory(inventory NewItem);
function NotifyTakeHit(pawn InstigatedBy, vector HitLocation, int Damage, class<DamageType> damageType, vector Momentum);

function SetFall();	//about to fall
function PawnIsInPain(PhysicsVolume PainVolume);	// called when pawn is taking pain volume damage

event PreBeginPlay()
{
	AddController();
	Super.PreBeginPlay();
	if ( bDeleteMe )
		return;

	SightCounter = 0.2 * FRand();  //offset randomly
}

event PostBeginPlay()
{
	Super.PostBeginPlay();
	if ( !bDeleteMe && bIsPlayer && (Role == ROLE_Authority) )
	{
		PlayerReplicationInfo = Spawn(PlayerReplicationInfoClass, Self,,vect(0,0,0),rot(0,0,0));
		InitPlayerReplicationInfo();
	}
}

function InitPlayerReplicationInfo()
{
	if (PlayerReplicationInfo.PlayerName == "")
		PlayerReplicationInfo.SetPlayerName(class'GameInfo'.Default.DefaultPlayerName);
}

function bool SameTeamAs(Controller C)
{
	if ( (PlayerReplicationInfo == None) || (C == None) || (C.PlayerReplicationInfo == None)
		|| (PlayerReplicationInfo.Team == None) )
		return false;
	return Level.Game.IsOnTeam(C,PlayerReplicationInfo.Team.TeamIndex);
}

function HandlePickup(Pickup pick)
{
}

simulated event Destroyed()
{
	if ( Role < ROLE_Authority )
    {
    	Super.Destroyed();
		return;
    }

	RemoveController();

	if ( bIsPlayer && (Level.Game != None) )
		Level.Game.logout(self);
	if ( PlayerReplicationInfo != None )
	{
		if ( !PlayerReplicationInfo.bOnlySpectator && (PlayerReplicationInfo.Team != None) )
			PlayerReplicationInfo.Team.RemoveFromTeam(self);
		PlayerReplicationInfo.Destroy();
	}
	Super.Destroyed();
}

/* AdjustView()
by default, check and see if pawn still needs to update eye height
(only if some playercontroller still has pawn as its viewtarget)
Overridden in playercontroller
*/
function AdjustView( float DeltaTime )
{
	local Controller C;

	for ( C=Level.ControllerList; C!=None; C=C.NextController )
		if ( C.IsA('PlayerController') && (PlayerController(C).ViewTarget == Pawn) )
			return;

	Pawn.bUpdateEyeHeight =false;
	Pawn.Eyeheight = Pawn.BaseEyeheight;
}

function bool WantsSmoothedView()
{
	return ( (Pawn != None) && ((Pawn.Physics==PHYS_Walking) || (Pawn.Physics==PHYS_Spider)) && !Pawn.bJustLanded );
}

function GameHasEnded()
{
	GotoState('GameEnded');
}

function ClientGameEnded()
{
	GotoState('GameEnded');
}

simulated event RenderOverlays( canvas Canvas );

//------------------------------------------------------------------------------
// Speech related

function bool AllowVoiceMessage(name MessageType)
{
	if ( Level.TimeSeconds - OldMessageTime < 10 )
		return false;

	OldMessageTime = Level.TimeSeconds;
	return true;
}
function SendVoiceMessage(PlayerReplicationInfo S, PlayerReplicationInfo Recipient, name messagetype, byte messageID, name broadcasttype);

//**************************************************************
// AI related

/* AdjustToss()
return adjustment to Z component of aiming vector to compensate for arc given the target
distance
*/
function vector AdjustToss(float TSpeed, vector Start, vector End, bool bNormalize)
{
	local vector Dest2D, Result, Vel2D;
	local float Dist2D;

	if ( Start.Z > End.Z + 64 )
	{
		Dest2D = End;
		Dest2D.Z = Start.Z;
		Dist2D = VSize(Dest2D - Start);
		TSpeed *= Dist2D/VSize(End - Start);
		Result = SuggestFallVelocity(Dest2D,Start,TSpeed,TSpeed);
		Vel2D = result;
		Vel2D.Z = 0;
		Result.Z = Result.Z + (End.Z - Start.Z) * VSize(Vel2D)/Dist2D;
	}
	else
	{
		Result = SuggestFallVelocity(End,Start,TSpeed,TSpeed);
	}
	if ( bNormalize )
		return TSpeed * Normal(Result);
	else
		return Result;
}

event PrepareForMove(NavigationPoint Goal, ReachSpec Path);
function WaitForMover(Mover M);
function MoverFinished();
function UnderLift(Mover M);

event HearNoise( float Loudness, Actor NoiseMaker);
event SeePlayer( Pawn Seen );	// called when a player (bIsPlayer==true) pawn is seen

exec event StopShake();
exec event ShakeView(float InTime, float SustainTime, float OutTime, float XMag, float YMag, float ZMag, float YawMag, float PitchMag, float Frequency );

//When a character dies (or is incapacitated), the killer is notified by this function
function Killed(Pawn Victim, class<DamageType> damageType);

//When a character dies, EVERY controller is notified by this function
function NotifyKilled(Controller Killer, Controller Killed, pawn KilledPawn, class<DamageType> damageType)
{
	if (Pawn == None)
		return;

	if (Killer != None && Pawn.IsAlly(Killer.Pawn) && Pawn.IsEnemy(KilledPawn))
	{
		if (Pawn.Squad != None)
			Pawn.Squad.EnemyKilled(KilledPawn, Killer, damageType);
		else if (VDistSq(KilledPawn.Location, Pawn.Location) < 1000.0f * 1000.0f)
			Pawn.PlayOwnedCue(PAE_AllyKilledEnemy, 30.0f);
	}
}

//When our Pawn dies (or is incapacitated), the controller is notified through this function
function WasKilledBy(Controller Killer, class<DamageType> damageType)
{
	if (Pawn != None && Pawn.Squad != None)
	{
		//Notify the Squad
		Pawn.Squad.MemberKilled(Pawn, Killer, damageType);
	}
}

function bool FireWeaponAt(Actor A);

native function WeaponFired(); // This function is called everytime the weapon fires a shot
event ThrowScav()
{
	if( Machine != None )
	{
		Machine.Velocity = vector(Rotation) * 1000;
		if( bIsPlayer )
			Machine.SetRotation(Rotation);
		Machine.TakeDamage( 1000, Pawn, Machine.Location, vect(0,0,0), class'DamageThrown' );
		Machine = None;
	}
}

function StopFiring()
{
	bFire = 0;
	bAltFire = 0;
}

simulated function float RateWeapon(Weapon w)
{
    return 0.0;
}

function float WeaponPreference(Weapon W)
{
	return 0.0;
}

/* AdjustAim()
AIController version does adjustment for non-controlled pawns.
PlayerController version does the adjustment for player aiming help.
Only adjusts aiming at pawns
allows more error in Z direction (full as defined by AutoAim - only half that difference for XY)
*/
function rotator AdjustAim(Ammunition FiredAmmunition, vector projStart, float aimerror, optional out int ShouldFire)
{
	return Rotation;
}

// Select the active grenade
exec event SwitchGrenade( optional float F )
{
	local Weapon newGrenade, currentGrenade;

	if ( (Level.Pauser!=None) || (Pawn == None) || (Pawn.Inventory == None) || !Pawn.Weapon.bCanSwitchGrenade )
	{
		return;
	}

	currentGrenade = Pawn.CurrentGrenade;

	if ( (Pawn != None) && (Pawn.Inventory != None) && (currentGrenade != None) && (currentGrenade.Inventory != None) )
	{
        newGrenade = currentGrenade.Inventory.WeaponChange(F, true);	// next item in the inventory after the currentGrenade
		if ( newGrenade == None )
			newGrenade = Pawn.Inventory.WeaponChange(F, true);
	}
	else
		newGrenade = None;

	if ( newGrenade == None )
		return;

	Pawn.CurrentGrenade = newGrenade;
	ServerSwitchGrenade(newGrenade);
}

function ServerSwitchGrenade(Weapon newGrenade)
{
	Pawn.CurrentGrenade = newGrenade;
}

exec function SwitchToBestWeapon()
{
	local float rating;

	if ( Pawn == None || Pawn.Inventory == None )
		return;

    if ( (Pawn.PendingWeapon == None) || (AIController(self) != None) )
    {
		Pawn.PendingWeapon = Pawn.Inventory.RecommendWeapon(rating);
		if ( Pawn.PendingWeapon == Pawn.Weapon )
			Pawn.PendingWeapon = None;
		if ( Pawn.PendingWeapon == None )
			return;
    }

	StopFiring();

	if ( Pawn.Weapon == None )
		Pawn.ChangedWeapon();

	else if ( Pawn.Weapon != Pawn.PendingWeapon )
		Pawn.Weapon.PutDown(Pawn.PendingWeapon);
}

// server calls this to force client to switch
simulated function ClientSwitchToBestWeapon()
{
    SwitchToBestWeapon();
}

function ClientSetWeapon( class<Weapon> WeaponClass )
{
    local Inventory Inv;
	local int Count;

    for( Inv = Pawn.Inventory; Inv != None; Inv = Inv.Inventory )
    {
		Count++;
		if ( Count > 1000 )
			return;

        if( !ClassIsChildOf( Inv.Class, WeaponClass ) )
            continue;

	    if( Pawn.Weapon == None )
        {
            Pawn.PendingWeapon = Weapon(Inv);
    		Pawn.ChangedWeapon();
        }
	    else if ( Pawn.Weapon != Weapon(Inv) )
        {
    		Pawn.PendingWeapon = Weapon(Inv);
			Pawn.Weapon.PutDown(Pawn.PendingWeapon);
		}
        return;
    }
}

function SetPawnClass(string inClass)
{
    local class<Pawn> pClass;
    pClass = class<Pawn>(DynamicLoadObject(inClass, class'Class'));
    if ( pClass != None )
        PawnClass = pClass;
}

function ChangedWeapon()
{
	if ( Pawn.Weapon != None )
		LastPawnWeapon = Pawn.Weapon.Class;
}

event ServerReStartPlayer()
{
	if ( Level.NetMode == NM_Client )
		return;
	if ( Pawn != None )
		ServerGivePawn();
}

function ServerGivePawn();

event MonitoredPawnAlert();

function StartMonitoring(Pawn P, float MaxDist)
{
	MonitoredPawn = P;
	MonitorStartLoc = P.Location;
	MonitorMaxDistSq = MaxDist * MaxDist;
}

function bool AutoTaunt()
{
	return false;
}

function bool DontReuseTaunt(int T)
{
	return false;
}
// **********************************************
// Controller States

state GameEnded
{
ignores SeePlayer, HearNoise, KilledBy, NotifyBump, HitWall, NotifyPhysicsVolumeChange, NotifyHeadVolumeChange, Falling, TakeDamage;

	function BeginState()
	{
		if ( Pawn != None )
		{
			if ( Pawn.Weapon != None )
				Pawn.Weapon.HolderDied();
			Pawn.bPhysicsAnimUpdate = false;
			Pawn.StopAnimating();
			Pawn.SimAnim[0].RateScale = 0;
			Pawn.SetCollision(true,false,false);
			Pawn.Velocity = vect(0,0,0);
			Pawn.SetPhysics(PHYS_None);
			Pawn.UnPossessed();
			Pawn.bIgnoreForces = true;
		}
		if ( !bIsPlayer )
			Destroy();
	}
}


defaultproperties
{
     FovAngle=90
     MinHitWall=-1
     AcquisitionYawRate=20000
     PlayerReplicationInfoClass=Class'Engine.PlayerReplicationInfo'
     bHidden=True
     RotationRate=(Pitch=3072,Yaw=30000,Roll=2048)
     bHiddenEd=True
}
