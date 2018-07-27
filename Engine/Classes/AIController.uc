//=============================================================================
// AIController, the base class of AI.
//
// Controllers are non-physical actors that can be attached to a pawn to control 
// its actions.  AIControllers implement the artificial intelligence for the pawns they control.  
//
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class AIController extends Controller
	native;

var		AIScript MyScript;
var     float		Skill;				// skill, scaled by game difficulty (add difficulty to this value)	
var		vector		FinalDestination;	//Ultimate Goto or FollowDestination

const MAX_ERROR_ANGLE = 60; //the maximum angle by which an AI can miss
const DEG_TO_ROTATOR = 182.04;
const MAXSTATEOBJECTS = 8;

native function AddReflexAnimGoal(name AnimName, optional name BoneName, optional bool Loop, optional float MaxTime, optional int Channel );
native function AddReflexSleepGoal(float MaxTime);

event PreBeginPlay()
{
	Super.PreBeginPlay();
	if ( bDeleteMe )
		return;

	if ( Level.Game != None )
		Skill += Level.Game.GameDifficulty; 
	Skill = FClamp(Skill, 0, 3);
}

/* Reset() 
reset actor to initial state - used when restarting level without reloading.
*/
function Reset()
{
	Super.Reset();

	// by default destroy bots (let game re-create)
	if ( bIsPlayer )
		Destroy();
}

function Trigger( actor Other, pawn EventInstigator )
{
	TriggerScript(Other,EventInstigator);
}

/* TriggerScript()
trigger AI script (this may enable it)
*/
function bool TriggerScript( actor Other, pawn EventInstigator )
{
	if ( MyScript != None )
	{
		MyScript.Trigger(EventInstigator,pawn);
		return true;
	}
	return false;
}

/* NathanM: This isn't necessary for AI controllers as we never use their view rotation
// AdjustView() called if Controller's pawn is viewtarget of a player
function AdjustView(float DeltaTime)
{
	local float TargetYaw, TargetPitch;
	local rotator OldViewRotation,ViewRotation;

	Super.AdjustView(DeltaTime);
	if( !Pawn.bUpdateEyeHeight )
		return;

	// update viewrotation
	ViewRotation = Rotation;
	OldViewRotation = Rotation;			

	if ( Enemy == None )
	{
		ViewRotation.Roll = 0;
		if ( DeltaTime < 0.2 )
		{
			OldViewRotation.Yaw = OldViewRotation.Yaw & 65535;
			OldViewRotation.Pitch = OldViewRotation.Pitch & 65535;
			TargetYaw = float(Rotation.Yaw & 65535);
			if ( Abs(TargetYaw - OldViewRotation.Yaw) > 32768 )
			{
				if ( TargetYaw < OldViewRotation.Yaw )
					TargetYaw += 65536;
				else
					TargetYaw -= 65536;
			}
			TargetYaw = float(OldViewRotation.Yaw) * (1 - 5 * DeltaTime) + TargetYaw * 5 * DeltaTime;
			ViewRotation.Yaw = int(TargetYaw);

			TargetPitch = float(Rotation.Pitch & 65535);
			if ( Abs(TargetPitch - OldViewRotation.Pitch) > 32768 )
			{
				if ( TargetPitch < OldViewRotation.Pitch )
					TargetPitch += 65536;
				else
					TargetPitch -= 65536;
			}
			TargetPitch = float(OldViewRotation.Pitch) * (1 - 5 * DeltaTime) + TargetPitch * 5 * DeltaTime;
			ViewRotation.Pitch = int(TargetPitch);
			SetRotation(ViewRotation);
		}
	}
}
*/

/* PrepareForMove()
Give controller a chance to prepare for a move along the navigation network, from
Anchor (current node) to Goal, given the reachspec for that movement.

Called if the reachspec doesn't support the pawn's current configuration.
By default, the pawn will crouch when it hits an actual obstruction. However,
Pawns with complex behaviors for setting up their smaller collision may want
to call that behavior from here
*/
event PrepareForMove(NavigationPoint Goal, ReachSpec Path);

/* WaitForMover()
Wait for Mover M to tell me it has completed its move
*/
function WaitForMover(Mover M)
{
	if ( (Enemy != None) && (Level.TimeSeconds - LastSeenTime < 3.0) )
		Focus = Enemy;
    PendingMover = M;
	bPreparingMove = true;
	Pawn.Acceleration = vect(0,0,0);
}

/* MoverFinished()
Called by Mover when it finishes a move, and this pawn has the mover
set as its PendingMover
*/
function MoverFinished()
{
	if ( PendingMover.MyMarker.ProceedWithMove(Pawn) )
	{
		PendingMover = None;
		bPreparingMove = false;
	}
}

/* UnderLift()
called by mover when it hits a pawn with that mover as its pendingmover while moving to its destination
*/
function UnderLift(Mover M)
{
	/*
	local NavigationPoint N;

	bPreparingMove = false;
	PendingMover = None;

	// find nearest lift exit and go for that
	if ( (MoveTarget == None) || MoveTarget.IsA('LiftCenter') )
		for ( N=Level.NavigationPointList; N!=None; N=N.NextNavigationPoint )
			if ( N.IsA('LiftExit') && (LiftExit(N).LiftTag == M.Tag)
				&& ActorReachable(N) )
			{
				MoveTarget = N;
				return;
			}
			*/
}


defaultproperties
{
     bCanOpenDoors=True
     bCanDoSpecial=True
     bAdjustFromWalls=True
     MinHitWall=-0.5
}

