//=============================================================================
// Trigger: senses things happening in its proximity and generates 
// sends Trigger/UnTrigger to actors whose names match 'EventName'.
//=============================================================================
class Trigger extends Triggers
	native;

#exec Texture Import File=Textures\Trigger.pcx Name=S_Trigger Mips=Off MASKED=1

//-----------------------------------------------------------------------------
// Trigger variables.

// Trigger type.
var() enum ETriggerType
{
	TT_PlayerProximity,	// Trigger is activated by player proximity.
	TT_PawnProximity,	// Trigger is activated by any pawn's proximity
	TT_ClassProximity,	// Trigger is activated by actor of ClassProximityType only
	TT_AnyProximity,    // Trigger is activated by any actor in proximity.
	TT_Shoot,		    // Trigger is activated by player shooting it.
	TT_HumanPlayerProximity,	// Trigger activated by human player (not bot)
	TT_Use,
	TT_SquadMemberProximity,
} TriggerType;

// Human readable triggering message.
var() localized string Message;

// Only trigger once and then go dormant.
var() bool bTriggerOnceOnly;

// For triggers that are activated/deactivated by other triggers.
var() bool bInitiallyActive;
var() bool bMustExceedCountSimultaneously;
var() int TouchThreshold; //allows the trigger to send its event only after it has received this number of touches

var int CurrentTouchCount;


var() class<actor> ClassProximityType;

var() float	RepeatTriggerTime; //if > 0, repeat trigger message at this interval is still touching other
var() float ReTriggerDelay; //minimum time before trigger can be triggered again
var	  float TriggerTime;
var() float DamageThreshold; //minimum damage to trigger if TT_Shoot

var(Events) name ExitEvent;      // The event when it untouches the trigger.
var(Events) name EmptyEvent;	// The event triggered when the trigger is empty

var() actor DirectTrigger;

// store for reset

var bool bSavedInitialCollision;
var bool bSavedInitialActive;

//=============================================================================
// AI related functions

function PreBeginPlay()
{
	Super.PreBeginPlay();

	if ( (TriggerType == TT_PlayerProximity)
		|| (TriggerType == TT_PawnProximity)
		|| (TriggerType == TT_HumanPlayerProximity)
		|| (TriggerType == TT_SquadMemberProximity)
		|| ((TriggerType == TT_ClassProximity) && ClassIsChildOf(ClassProximityType,class'Pawn')) )	
		OnlyAffectPawns(true);
}

function PostBeginPlay()
{
	if ( TriggerType == TT_Shoot )
	{
		bHidden = false;
		bProjTarget = true;
		SetDrawType(DT_None);
	}
	bSavedInitialActive = bInitiallyActive;
	bSavedInitialCollision = bCollideActors;
	Super.PostBeginPlay();
}

/* Reset() 
reset actor to initial state - used when restarting level without reloading.
*/
function Reset()
{
	Super.Reset();

	// collision, bInitiallyactive
	bInitiallyActive = bSavedInitialActive;
	SetCollision(bSavedInitialCollision, bBlockActors, bBlockPlayers );
}	


// when trigger gets turned on, check its touch list

function CheckTouchList()
{
	local Actor A;

	ForEach TouchingActors(class'Actor', A)
		Touch(A);
}

//=============================================================================
// Trigger states.

// Trigger is always active.
state() NormalTrigger
{
}

// Other trigger toggles this trigger's activity.
state() OtherTriggerToggles
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		bInitiallyActive = !bInitiallyActive;
		if ( bInitiallyActive )
			CheckTouchList();
	}
}

// Other trigger turns this on.
state() OtherTriggerTurnsOn
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		local bool bWasActive;

		bWasActive = bInitiallyActive;
		bInitiallyActive = true;
		if ( !bWasActive )
			CheckTouchList();
	}
}

// Other trigger turns this off.
state() OtherTriggerTurnsOff
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		bInitiallyActive = false;
	}
}

//=============================================================================
// Trigger logic.

//
// See whether the other actor is relevant to this trigger.
//
function bool IsRelevant( actor Other )
{
	if( !bInitiallyActive )
		return false;
	switch( TriggerType )
	{
		case TT_HumanPlayerProximity:
			return (Pawn(Other) != None) && Pawn(Other).IsHumanControlled();
		case TT_PlayerProximity:
			return (Pawn(Other) != None) && (Pawn(Other).IsPlayerPawn() || Pawn(Other).WasPlayerPawn());
		case TT_PawnProximity:
			return (Pawn(Other) != None);
		case TT_ClassProximity:
			return ClassIsChildOf(Other.Class, ClassProximityType);
		case TT_AnyProximity:
			if (!bAcceptsProjectors && ClassIsChildOf(Other.Class, class'Projector'))
				return false;
			return true;
		case TT_Shoot:
			return ( (Projectile(Other) != None) && (Projectile(Other).Damage >= DamageThreshold) );
		case TT_SquadMemberProximity:
			return ((Pawn(Other) != None) && Pawn(Other).Squad != None && Pawn(Other).Squad.PlayerSquad);
	}
}
//
// Called when something touches the trigger.
//
function Touch( actor Other )
{
	if( IsRelevant( Other ) )
	{
		if ( ReTriggerDelay > 0 )
		{
			if ( Level.TimeSeconds - TriggerTime < ReTriggerDelay )
				return;
			TriggerTime = Level.TimeSeconds;
		}
		++CurrentTouchCount;
		if (CurrentTouchCount < TouchThreshold)
			return;

		//Log("Trigger::Touch "$self$" by "$Other);
		// Broadcast the Trigger message to all matching actors.
		if( TriggerType != TT_Shoot )
		{
			TriggerEvent(Event, self, Other.Instigator);
			if (DirectTrigger != None)
				DirectTrigger.Trigger(self, Other.Instigator);
					
			if( (Message != "") && (Other.Instigator != None) )
				// Send a string message to the toucher.
				Other.Instigator.ClientMessage( Message );

			if( bTriggerOnceOnly )
				// Ignore future touches.
				SetCollision(false);
			else if ( RepeatTriggerTime > 0 )
				SetTimer(RepeatTriggerTime, false);
		}
	}
}

function Timer()
{
	local bool bKeepTiming;
	local Actor A;

	bKeepTiming = false;

	ForEach TouchingActors(class'Actor', A)
		if ( IsRelevant(A) )
		{
			bKeepTiming = true;
			Touch(A);
		}

	if ( bKeepTiming )
		SetTimer(RepeatTriggerTime, false);
}

function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
{
	Damage = CalculateDamageFrom(damageType, Damage);
	if ( bInitiallyActive && (TriggerType == TT_Shoot) && (Damage >= DamageThreshold) && (instigatedBy != None) )
	{
		if ( ReTriggerDelay > 0 )
		{
			if ( Level.TimeSeconds - TriggerTime < ReTriggerDelay )
				return 0;
			TriggerTime = Level.TimeSeconds;
		}
		// Broadcast the Trigger message to all matching actors.
		TriggerEvent(Event, self, instigatedBy);
		if (DirectTrigger != None)
			DirectTrigger.Trigger(self, instigatedBy);

		if( Message != "" )
			// Send a string message to the toucher.
			instigatedBy.Instigator.ClientMessage( Message );

		if( bTriggerOnceOnly )
			// Ignore future touches.
			SetCollision(False);
	}

	return Damage;
}

//
// When something untouches the trigger.
//
function UnTouch( actor Other )
{
	if( IsRelevant( Other ) )
	{
		if (bMustExceedCountSimultaneously)
		{
			--CurrentTouchCount;
			if (CurrentTouchCount > 0)
				return;
		}
		UntriggerEvent(event, self, Other.Instigator);
		if( DirectTrigger != None )
			DirectTrigger.Untrigger(self, Other.Instigator);
		if (ExitEvent != 'None')
			TriggerEvent(ExitEvent, self, Other.Instigator);
		if (CurrentTouchCount == 0 && EmptyEvent != 'None')
			TriggerEvent(EmptyEvent, self, Other.Instigator);
	}
}

function UsedBy( Pawn user )
{
	if( TriggerType == TT_Use )
	{
		TriggerEvent(Event, self, user);
		if (DirectTrigger != None)
			DirectTrigger.Trigger(self, user);
	}
}

// This function is used by the editor to indicate to the search function
// that an event is important to this guy
function bool ContainsPartialEvent(string StartOfEventName)
{
	local string EventString;
	EventString = string(ExitEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}

	return super.ContainsPartialEvent(StartOfEventName);
}



defaultproperties
{
     bTriggerOnceOnly=True
     bInitiallyActive=True
     TouchThreshold=1
     bAcceptsProjectors=False
     InitialState="NormalTrigger"
     Texture=Texture'Engine.S_Trigger'
}

