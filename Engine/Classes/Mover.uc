//=============================================================================
// The moving brush class.
// This is a built-in Unreal class and it shouldn't be modified.
// Note that movers by default have bNoDelete==true.  This makes movers and their default properties
// remain on the client side.  If a mover subclass has bNoDelete=false, then its default properties must
// be replicated
//=============================================================================
class Mover extends Actor
	native
	nativereplication;

// How the mover should react when it encroaches an actor.
var() enum EMoverEncroachType
{
	ME_StopWhenEncroach,	// Stop when we hit an actor.
	ME_ReturnWhenEncroach,	// Return to previous position when we hit an actor.
   	ME_CrushWhenEncroach,   // Crush the poor helpless actor.
   	ME_IgnoreWhenEncroach,  // Ignore encroached actors.
} MoverEncroachType;

// How the mover moves from one position to another.
var() enum EMoverGlideType
{
	MV_MoveByTime,			// Move linearly.
	MV_GlideByTime,			// Move with smooth acceleration.
} MoverGlideType;

// What classes can bump trigger this mover
var() enum EBumpType
{
	BT_PlayerBump,		// Can only be bumped by player.
	BT_PawnBump,		// Can be bumped by any pawn
	BT_AnyBump,			// Can be bumped by any solid actor
} BumpType;

//-----------------------------------------------------------------------------
// Keyframe numbers.
var() byte       KeyNum;           // Current or destination keyframe.
var byte         PrevKeyNum;       // Previous keyframe.
var() const byte NumKeys;          // Number of keyframes in total (0-3).
var() const byte WorldRaytraceKey; // Raytrace the world with the brush here.
var() const byte BrushRaytraceKey; // Raytrace the brush here.

//-----------------------------------------------------------------------------
// Movement parameters.
var() float      MoveTimes[16];         // Time to spend moving between keyframes.
var() float      StayOpenTime;     // How long to remain open before closing.
var() float      OtherTime;        // TriggerPound stay-open time.
var() int        EncroachDamage;   // How much to damage encroached actors.

//-----------------------------------------------------------------------------
// Mover state.
var() bool       bTriggerOnceOnly; // Go dormant after first trigger.
var() bool       bSlave;           // This brush is a slave.
var() bool		 bUseTriggered;		// Triggered by player grab
var() bool		 bDamageTriggered;	// Triggered by taking damage
var() bool       bDynamicLightMover; // Apply dynamic lighting to mover.
var() bool       bUseShortestRotation; // rot by -90 instead of +270 and so on.
var(ReturnGroup) bool bIsLeader;
var() name       PlayerBumpEvent;  // Optional event to cause when the player bumps the mover.
var() name       BumpEvent;			// Optional event to cause when any valid bumper bumps the mover.
var   actor      SavedTrigger;      // Who we were triggered by.
var() float		 DamageThreshold;	// minimum damage to trigger
var	  int		 numTriggerEvents;	// number of times triggered ( count down to untrigger )
var(ReturnGroup)	  Mover		 Leader;			// for having multiple movers return together
var(ReturnGroup)	  Mover		 Follower;
//var(ReturnGroup) name		 ReturnGroup;		// if none, same as tag
var() float		 DelayTime;			// delay before starting to open
var() int		 ReturnLoopTime;	// Time it takes to interpolate from the last keyframe to the base pos.
var	  int		 ScriptTriggeredKeyframe;	// true if mover is triggered via the AI Script MoverToKeyframe
var() class<DamageType> ImpactDamageType;
//-----------------------------------------------------------------------------
// Audio.
var(MoverSounds) sound      OpeningSound;     // When start opening.
var(MoverSounds) sound      OpenedSound;      // When finished opening.
var(MoverSounds) sound      ClosingSound;     // When start closing.
var(MoverSounds) sound      ClosedSound;      // When finish closing.
var(MoverSounds) sound      MoveAmbientSound; // Optional ambient sound when moving.
var(MoverSounds) sound		LoopSound;		  // Played on Loop

//-----------------------------------------------------------------------------
// Events

var(MoverEvents) name		OpeningEvent;	// Event to cause when opening
var(MoverEvents) name		OpenedEvent;	// Event to cause when opened
var(MoverEvents) name		ClosingEvent;	// Event to cause when closing
var(MoverEvents) name		ClosedEvent;	// Event to cause when closed
var(MoverEvents) name		LoopEvent;		// Event to cause when the mover loops
//-----------------------------------------------------------------------------
// Other stuff

//-----------------------------------------------------------------------------
// Internal.
var vector       KeyPos[24];
var rotator      KeyRot[24];
var vector       BasePos, OldPos, OldPrePivot, SavedPos, InitPos;
var rotator      BaseRot, OldRot, SavedRot, InitRot;
var           float       PhysAlpha;       // Interpolating position, 0.0-1.0.
var           float       PhysRate;        // Interpolation rate per second.

// AI related
var(AI)	  NavigationPoint  MyMarker;
var		  bool			bOpening, bClosing, bDelaying, bClientPause;
var		  bool			bClosed, bOpened;	// mover is in closed position, and no longer moving
var		  bool			bPlayerOnly;
var(AI)	  bool			bAutoDoor;	// automatically setup Door NavigationPoint for this mover
var(AI)	  bool			bNoAIRelevance; // don't warn about this mover during path review

// for client side replication
var		vector			SimOldPos;
var		int				SimOldRotPitch, SimOldRotYaw, SimOldRotRoll;
var		vector			SimInterpolate;
var		vector			RealPosition;
var     rotator			RealRotation;
var		int				ClientUpdate;

// Used for controlling antiportals

var array<AntiPortalActor>	AntiPortals;	
var() name					AntiPortalTag;

replication
{
	// Things the server should send to the client.
	reliable if( Role==ROLE_Authority )
		SimOldPos, SimOldRotPitch, SimOldRotYaw, SimOldRotRoll, SimInterpolate, RealPosition, RealRotation;
}

/* StartInterpolation()
when this function is called, the actor will start moving along an interpolation path
beginning at Dest
*/	
simulated function StartInterpolation()
{
	GotoState('');
	bInterpolating = true;
	SetPhysics(PHYS_None);
}

simulated function Timer()
{
	if ( Velocity != vect(0,0,0) )
	{
		bClientPause = false;
		return;		
	}
	if ( Level.NetMode == NM_Client )
	{
		if ( ClientUpdate == 0 ) // not doing a move
		{
			if ( bClientPause )
			{
				if ( VSize(RealPosition - Location) > 3 )
					SetLocation(RealPosition);
				else
					RealPosition = Location;
				SetRotation(RealRotation);
				bClientPause = false;
			}
			else if ( RealPosition != Location )
				bClientPause = true;
		}
		else
			bClientPause = false;
	}
	else 
	{
		RealPosition = Location;
		RealRotation = Rotation;
	}
}

//by default, just keep track of trigger events
function Trigger( actor Other, pawn EventInstigator )
{
	//Log("Mover::Trigger");
	numTriggerEvents++;
}

function UnTrigger( actor Other, pawn EventInstigator )
{
	//Log("Mover::UnTrigger");
	numTriggerEvents--;
}

//-----------------------------------------------------------------------------
// Movement functions.

// Interpolate to keyframe KeyNum in Seconds time.
simulated final function InterpolateTo( byte NewKeyNum, float Seconds )
{
	NewKeyNum = Clamp( NewKeyNum, 0, ArrayCount(KeyPos)-1 );
	if( NewKeyNum==PrevKeyNum && KeyNum!=PrevKeyNum )
	{
		// Reverse the movement smoothly.
		PhysAlpha = 1.0 - PhysAlpha;
		OldPos    = BasePos + KeyPos[KeyNum];
		OldRot    = BaseRot + KeyRot[KeyNum];
	}
	else
	{
		// Start a new movement.
		OldPos    = Location;
		OldRot    = Rotation;
		PhysAlpha = 0.0;
	}

	// Setup physics.
	SetPhysics(PHYS_MovingBrush);
	bInterpolating   = true;
	PrevKeyNum       = KeyNum;
	KeyNum			 = NewKeyNum;
	PhysRate         = 1.0 / FMax(Seconds, 0.005);

	ClientUpdate++;
	SimOldPos = OldPos;
	SimOldRotYaw = OldRot.Yaw;
	SimOldRotPitch = OldRot.Pitch;
	SimOldRotRoll = OldRot.Roll;
	SimInterpolate.X = 100 * PhysAlpha;
	SimInterpolate.Y = 100 * FMax(0.01, PhysRate);
	SimInterpolate.Z = 256 * PrevKeyNum + KeyNum;
}

// Set the specified keyframe.
final function SetKeyframe( byte NewKeyNum, vector NewLocation, rotator NewRotation )
{
	KeyNum         = Clamp( NewKeyNum, 0, ArrayCount(KeyPos)-1 );
	KeyPos[KeyNum] = NewLocation;
	KeyRot[KeyNum] = NewRotation;
}

// Interpolation ended.
simulated event KeyFrameReached()
{
	local byte OldKeyNum;

	if (ScriptTriggeredKeyframe == KeyNum)
	{
		// Finished interpolating.
		AmbientSound = None;
		if ( (ClientUpdate == 0) && (Level.NetMode != NM_Client) )
		{
			RealPosition = Location;
			RealRotation = Rotation;
		}
		TriggerEvent( Event, Self, Instigator );
		return;
	}

	OldKeyNum  = PrevKeyNum;
	PrevKeyNum = KeyNum;
	PhysAlpha  = 0;
	ClientUpdate--;

	// If more than two keyframes, chain them.
	if( KeyNum>0 && KeyNum<OldKeyNum )
	{
		// Chain to previous.
		InterpolateTo(KeyNum-1,MoveTimes[KeyNum-1]);
	}
	else if( KeyNum<NumKeys-1 && KeyNum>OldKeyNum )
	{
		// Chain to next.
		InterpolateTo(KeyNum+1,MoveTimes[KeyNum]);
	}
	else
	{
		// Finished interpolating.
		AmbientSound = None;
		if ( (ClientUpdate == 0) && (Level.NetMode != NM_Client) )
		{
			RealPosition = Location;
			RealRotation = Rotation;
		}
	}
}

// Ask whether an event belongs to this actor; used in the editor
function bool ContainsPartialEvent(string StartOfEventName)
{
	local string EventString;
	EventString = string(OpeningEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}

	EventString = string(OpenedEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}

	EventString = string(ClosingEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}

	EventString = string(ClosedEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}

	EventString = string(LoopEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}

	EventString = string(PlayerBumpEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}

	EventString = string(BumpEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}

	return super.ContainsPartialEvent(StartOfEventName);
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(AntiPortalTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



//-----------------------------------------------------------------------------
// Mover functions.

// Notify AI that mover finished movement
function FinishNotify()
{
	local Controller C;

	for ( C=Level.ControllerList; C!=None; C=C.nextController )
		if ( (C.Pawn != None) && (C.PendingMover == self) )
			C.MoverFinished();
}

// Handle when the mover finishes closing.
function FinishedClosing()
{
	local Mover M;
	
	// Update sound effects.
	if( !bClosed )
	{
		//Log("__________ Do Closed "$ClosedSound$" __________");
		PlaySound( ClosedSound ); 
	}
	// Handle Events
	
	TriggerEvent( ClosedEvent, Self, Instigator );
	
	// Notify our triggering actor that we have completed.
	if( SavedTrigger != None )
		SavedTrigger.EndEvent();
		
	SavedTrigger = None;
	Instigator = None;
	If ( MyMarker != None )
		MyMarker.MoverClosed();
	bClosed = true;
	FinishNotify(); 
	for ( M=Leader; M!=None; M=M.Follower )
		if ( !M.bClosed )
			return;
	UnTriggerEvent(OpeningEvent, Self, Instigator);
}

// Handle when the mover finishes opening.
function FinishedOpening()
{
	if( !bOpened )
	{
		//Log("__________ Do Opened "$OpenedSound$" __________");
		PlaySound( OpenedSound );
	}
	bOpened = true;
	
	// Trigger any chained movers / Events
	TriggerEvent(Event, Self, Instigator);
	TriggerEvent(OpenedEvent, Self, Instigator);

	If ( MyMarker != None )
		MyMarker.MoverOpened();
	FinishNotify();
}

// Open the mover.
function DoOpen()
{
	if( !bOpening )
	{
		//Log("__________ Do Opening "$OpeningSound$" __________");
		PlaySound( OpeningSound );
	}
	
	bClosed = false;
	bOpening = true; 
	bClosing = false;
	bDelaying = false;
	InterpolateTo( 1, MoveTimes[0] );
	AmbientSound = MoveAmbientSound;
	TriggerEvent(OpeningEvent, Self, Instigator);
	if ( Follower != None )
		Follower.DoOpen();
}

// Close the mover.
function DoClose()
{
	if( !bClosing )
	{
		//Log("__________ Do Closing "$ClosingSound$" __________");
		PlaySound( ClosingSound );
	}
	
	bOpened = false;
	bOpening = false;
	bClosing = true;
	bDelaying = false;
	InterpolateTo( Max(0,KeyNum-1), MoveTimes[Max(0,KeyNum-1)] );	
	UntriggerEvent(Event, self, Instigator);
	AmbientSound = MoveAmbientSound;
	TriggerEvent(ClosingEvent,Self,Instigator);
	if ( Follower != None )
		Follower.DoClose();
}

function DoLoop()
{
	bOpening = false;
	bDelaying = false;
	bClosing = false;
	InterpolateTo( 0, ReturnLoopTime );	// Loop it back to the start.
//	SetLocation( InitPos );				// Set initial location
//	SetRotation( InitRot );				// Initial rotation
	if (LoopEvent != 'None')
		TriggerEvent(LoopEvent,self,Instigator);
	if ( Follower != None )
		Follower.DoLoop();
}

//-----------------------------------------------------------------------------
// Engine notifications.

// When mover enters gameplay.
simulated function BeginPlay()
{
	local AntiPortalActor AntiPortal;
	
	if(AntiPortalTag != '')
	{
		foreach AllActors(class'AntiPortalActor',AntiPortal,AntiPortalTag)
		{
			AntiPortals.Length = AntiPortals.Length + 1;
			AntiPortals[AntiPortals.Length - 1] = AntiPortal;
		}
	}

	// timer updates real position every second in network play
	if ( Level.NetMode != NM_Standalone )
	{
		if ( Level.NetMode == NM_Client )
			settimer(4.0, true);
		else
			settimer(1.0, true);
		if ( Role < ROLE_Authority )
			return;
	}

	if ( Level.NetMode != NM_Client )
	{
		RealPosition = Location;
		RealRotation = Rotation;
	}

	// Init key info.
	Super.BeginPlay();
	KeyNum         = Clamp( KeyNum, 0, ArrayCount(KeyPos)-1 );
	PhysAlpha      = 0.0;

	// Set initial location.
	InitPos = Location;
	Move( BasePos + KeyPos[KeyNum] - Location );

	// Initial rotation.
	InitRot = Rotation;
	SetRotation( BaseRot + KeyRot[KeyNum] );
}

// Immediately after mover enters gameplay.
function PostBeginPlay()
{
	local mover M;

	// Initialize all slaves.
	if( !bSlave )
	{
		//look for slaves with the same tag
		foreach DynamicActors( class 'Mover', M, Tag )
		{
			if( M.bSlave )
			{
				M.GotoState('');
				M.SetBase( Self );
			}
		}
	}
	if (Leader == None)
	{
		Leader = self;
	}
}

function MakeGroupStop()
{
	// Stop moving immediately.
	bInterpolating = false;
	AmbientSound = None;
	GotoState( , '' );

	if ( Follower != None )
		Follower.MakeGroupStop();
}

function MakeGroupReturn()
{
	// Abort move and reverse course.
	bInterpolating = false;
	AmbientSound = None;

	if(bIsLeader || Leader==Self)
	{
		if( KeyNum<PrevKeyNum )
			GotoState( , 'Open' );
		else
			GotoState( , 'Close' );
	}

	if ( Follower != None )
		Follower.MakeGroupReturn();
}
		
// Return true to abort, false to continue.
function bool EncroachingOn( actor Other )
{
	local Pawn P;

	if ( Other == None )
		return false;
	if ( Other.IsA('Pawn') && Pawn(Other).Controller == None )
	{
		if( Other.Physics == PHYS_KarmaRagdoll )
			Other.Destroy();
		else
			Other.TakeDamage(10000, None, Other.Location, vect(0,0,0), ImpactDamageType);

		return false;
	}
	if ( Other.IsA('Pickup') )
	{
		if ( !Other.bAlwaysRelevant && (Other.Owner == None) )
			Other.Destroy();
		return false;
	}
	if ( Other.IsA('Projectile') )
	{
		Other.Destroy();
		return false;
	}

	// Damage the encroached actor.
	if( EncroachDamage != 0 )
		Other.TakeDamage( EncroachDamage, Instigator, Other.Location, vect(0,0,0), ImpactDamageType );

	// If we have a bump-player event, and Other is a pawn, do the bump thing.
	P = Pawn(Other);
	if( P!=None && (P.Controller != None) && P.IsPlayerPawn() )
	{
		if ( PlayerBumpEvent!='' )
			Bump( Other );
		if ( (P != None) && (P.Controller != None) && (P.Base != self) && (P.Controller.PendingMover == self) )
			P.Controller.UnderLift(self);	// pawn is under lift - tell him to move
	}

	// Stop, return, or whatever.
	if( MoverEncroachType == ME_StopWhenEncroach )
	{
		Leader.MakeGroupStop();
		return true;
	}
	else if( MoverEncroachType == ME_ReturnWhenEncroach )
	{
		Leader.MakeGroupReturn();
		if ( Other.IsA('Pawn') )
			Pawn(Other).PlayMoverHitSound();
		return true;
	}
	else if( MoverEncroachType == ME_CrushWhenEncroach )
	{
		// Kill it.
		Other.KilledBy( Instigator, ImpactDamageType );
		return false;
	}
	else if( MoverEncroachType == ME_IgnoreWhenEncroach )
	{
		// Ignore it.
		return false;
	}
}

// When bumped by player.
function Bump( actor Other )
{
	local pawn  P;

	P = Pawn(Other);
	if ( bUseTriggered && (P != None) && !P.IsHumanControlled() && P.IsPlayerPawn() )
	{
		Trigger(P,P);
		P.Controller.WaitForMover(self);
	}	
	if ( (BumpType != BT_AnyBump) && (P == None) )
		return;
	if ( (BumpType == BT_PlayerBump) && !P.IsPlayerPawn() )
		return;
	//if ( (BumpType == BT_PawnBump) && P.bAmbientCreature ) //TODO: Fix to use Team Info
	//	return;
	TriggerEvent(BumpEvent, self, P);

	if ( (P != None) && P.IsPlayerPawn() )
		TriggerEvent(PlayerBumpEvent, self, P);
}

// When damaged
function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
{
	if ( bDamageTriggered && (Damage >= DamageThreshold) )
	{
		if ( (AIController(instigatedBy.Controller) != None) && (instigatedBy.Controller.Focus == self) )		
			instigatedBy.Controller.StopFiring();		

		self.Trigger(self, instigatedBy);
	}

	return Damage;
}

//========================================================================
// Master State for OpenTimed mover states (for movers that open and close)

state() Locked
{
	function BeginState()
	{
		if (MyMarker != None)
			MyMarker.SetBlocked(true);

	}
	function EndState()
	{
		if (MyMarker != None)
			MyMarker.SetBlocked(false);
	}

Close:
	if (!bClosed)
	{
		DoClose();
		FinishInterpolation();
		FinishedClosing();
	}
}

state OpenTimedMover
{
	function DisableTrigger();
	function EnableTrigger();

Open:
	bClosed = false;
	DisableTrigger();
	if ( DelayTime > 0 )
	{
		bDelaying = true;
		Sleep(DelayTime);
	}
	DoOpen();
	FinishInterpolation();
	FinishedOpening();
	Sleep( StayOpenTime );
	if( bTriggerOnceOnly )
		GotoState('');
Close:
	DoClose();
	FinishInterpolation();
	FinishedClosing();
	EnableTrigger();
}

// Open when stood on, wait, then close.
state() StandOpenTimed extends OpenTimedMover
{
	function Attach( actor Other )
	{
		local pawn  P;

		P = Pawn(Other);
		if ( (BumpType != BT_AnyBump) && (P == None) )
			return;
		if ( (BumpType == BT_PlayerBump) && !P.IsPlayerPawn() )
			return;
		if ( (BumpType == BT_PawnBump) && (Other.Mass < 10) )
			return;
		SavedTrigger = None;
		GotoState( 'StandOpenTimed', 'Open' );
	}

	function DisableTrigger()
	{
		Disable( 'Attach' );
	}

	function EnableTrigger()
	{
		Enable('Attach');
	}
}

// Open when bumped, wait, then close.
state() BumpOpenTimed extends OpenTimedMover
{
	function Bump( actor Other )
	{
		//10Dec2003 JAH -- Added ability to bump follower to trigger
		//entire return group
		if ( Leader != None && Leader != self)
		{
			Leader.Bump(Other);
			return;
		}
		if ( (BumpType != BT_AnyBump) && (Pawn(Other) == None) )
			return;
		if ( (BumpType == BT_PlayerBump) && !Pawn(Other).IsPlayerPawn() )
			return;
		if ( (BumpType == BT_PawnBump) && (Other.Mass < 10) )
			return;
		Global.Bump( Other );
		SavedTrigger = None;
		Instigator = Pawn(Other);
		Instigator.Controller.WaitForMover(self);
		GotoState( 'BumpOpenTimed', 'Open' );
	}

	function DisableTrigger()
	{
		Disable( 'Bump' );
	}

	function EnableTrigger()
	{
		Enable('Bump');
	}
}

// When triggered, open, wait, then close.
state() TriggerOpenTimed extends OpenTimedMover
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		numTriggerEvents++;
		//Log("Mover::TriggerOpenTimed::Trigger "$self$" by "$Other$" Instigator: "$EventInstigator);
		SavedTrigger = Other;
		Instigator = EventInstigator;
		if ( SavedTrigger != None )
			SavedTrigger.BeginEvent();
		GotoState( 'TriggerOpenTimed', 'Open' );
	}

	function DisableTrigger()
	{
		Disable( 'Trigger' );
	}

	function EnableTrigger()
	{
		Enable('Trigger');
	}
}

//=================================================================
// Other Mover States

// Toggle when triggered.
state() TriggerToggle
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		numTriggerEvents++;
		SavedTrigger = Other;
		Instigator = EventInstigator;
		if ( SavedTrigger != None )
			SavedTrigger.BeginEvent();
		if( KeyNum==0 || KeyNum<PrevKeyNum )
			GotoState( 'TriggerToggle', 'Open' );
		else
			GotoState( 'TriggerToggle', 'Close' );
	}
Open:
	bClosed = false;
	if ( DelayTime > 0 )
	{
		bDelaying = true;
		Sleep(DelayTime);
	}
	DoOpen();
	FinishInterpolation();
	FinishedOpening();
	if ( SavedTrigger != None )
		SavedTrigger.EndEvent();
	Stop;
Close:		
	DoClose();
	FinishInterpolation();
	FinishedClosing();
}

state() TriggerLoop
{	
	function Trigger( actor Other, pawn EventInstigator )
	{
		SavedTrigger = Other;
		Instigator = EventInstigator;
		UntriggerEvent(Event, self, Instigator);
		if ( SavedTrigger != None )
			SavedTrigger.BeginEvent();
		GotoState( 'TriggerLoop', 'Open' );
	}

Open:
	Disable('Trigger');
	if ( DelayTime > 0 )
	{
		bDelaying = true;
		Sleep(DelayTime);
	}
	DoOpen();
	FinishInterpolation();
	FinishedOpening();
	DoLoop();			
	FinishInterpolation();
	if( bTriggerOnceOnly )
		GotoState('');
	goto 'Open';
}

state() Looping
{	
Begin:	
	if ( DelayTime > 0 )
	{
		bDelaying = true;
		Sleep(DelayTime);
	}
	DoOpen();
	FinishInterpolation();
	FinishedOpening();
	DoLoop();			
	FinishInterpolation();	
	goto 'Begin';
}

// Open when triggered, close when get untriggered.
state() TriggerControl
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		numTriggerEvents++;
		SavedTrigger = Other;
		Instigator = EventInstigator;
		if ( SavedTrigger != None )
			SavedTrigger.BeginEvent();
		GotoState( 'TriggerControl', 'Open' );
	}
	function UnTrigger( actor Other, pawn EventInstigator )
	{
		numTriggerEvents--;
		if ( numTriggerEvents <=0 )
		{
			numTriggerEvents = 0;
			SavedTrigger = Other;
			Instigator = EventInstigator;
			SavedTrigger.BeginEvent();
			GotoState( 'TriggerControl', 'Close' );
		}
	}

Begin:
	if (numTriggerEvents > 0)
		goto 'Open';
	Stop;
Open:
	bClosed = false;
	if ( DelayTime > 0 )
	{
		bDelaying = true;
		Sleep(DelayTime);
	}
	DoOpen();
	FinishInterpolation();
	FinishedOpening();
	SavedTrigger.EndEvent();
	if( bTriggerOnceOnly )
		GotoState('');
	Stop;
Close:
	if( StayOpenTime > 0)
	{
		bDelaying = true;
		Sleep(StayOpenTime);
	}
	DoClose();
	FinishInterpolation();
	FinishedClosing();
}

// Start pounding when triggered.
state() TriggerPound
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		numTriggerEvents++;
		SavedTrigger = Other;
		Instigator = EventInstigator;
		GotoState( 'TriggerPound', 'Open' );
	}
	function UnTrigger( actor Other, pawn EventInstigator )
	{
		numTriggerEvents--;
		if ( numTriggerEvents <= 0 )
		{
			numTriggerEvents = 0;
			SavedTrigger = None;
			Instigator = None;
			GotoState( 'TriggerPound', 'Close' );
		}
	}
	function BeginState()
	{
		//numTriggerEvents = 0;
	}

Open:
	bClosed = false;
	if ( DelayTime > 0 )
	{
		bDelaying = true;
		Sleep(DelayTime);
	}
	DoOpen();
	FinishInterpolation();
	Sleep(OtherTime);
Close:
	DoClose();
	FinishInterpolation();
	Sleep(StayOpenTime);
	if( bTriggerOnceOnly )
		GotoState('');
	if( SavedTrigger != None )
		goto 'Open';
}

//-----------------------------------------------------------------------------
// Bump states.


// Open when bumped, close when reset.
state() BumpButton
{
	function Bump( actor Other )
	{
		if ( (BumpType != BT_AnyBump) && (Pawn(Other) == None) )
			return;
		if ( (BumpType == BT_PlayerBump) && !Pawn(Other).IsPlayerPawn() )
			return;
		if ( (BumpType == BT_PawnBump) && (Other.Mass < 10) )
			return;
		Global.Bump( Other );
		SavedTrigger = Other;
		Instigator = Pawn( Other );
		Instigator.Controller.WaitForMover(self);
		GotoState( 'BumpButton', 'Open' );
	}
	function BeginEvent()
	{
		bSlave=true;
	}
	function EndEvent()
	{
		bSlave     = false;
		Instigator = None;
		GotoState( 'BumpButton', 'Close' );
	}
Open:
	bClosed = false;
	Disable( 'Bump' );
	if ( DelayTime > 0 )
	{
		bDelaying = true;
		Sleep(DelayTime);
	}
	DoOpen();
	FinishInterpolation();
	FinishedOpening();
	if( bTriggerOnceOnly )
		GotoState('');
	if( bSlave )
		Stop;
Close:
	DoClose();
	FinishInterpolation();
	FinishedClosing();
	Enable( 'Bump' );
}


defaultproperties
{
     MoverEncroachType=ME_ReturnWhenEncroach
     MoverGlideType=MV_GlideByTime
     NumKeys=2
     StayOpenTime=4
     ImpactDamageType=Class'Engine.Crushed'
     bClosed=True
     Physics=PHYS_MovingBrush
     bNoDelete=True
     bAlwaysRelevant=True
     bOnlyDirtyReplication=True
     bShadowCast=True
     bCollideActors=True
     bBlockActors=True
     bBlockPlayers=True
     bBlockKarma=True
     bIsMover=True
     RemoteRole=ROLE_SimulatedProxy
     NetPriority=2.7
     InitialState="BumpOpenTimed"
     CollisionRadius=160
     CollisionHeight=160
     bEdShouldSnap=True
     bPathColliding=True
}

