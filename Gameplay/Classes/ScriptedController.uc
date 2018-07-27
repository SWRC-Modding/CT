// ScriptedController
// AI controller which is controlling the pawn through a scripted sequence specified by 
// an AIScript

class ScriptedController extends AIController
	native;

var bool bBroken;
var bool bUseScriptFacing;
var bool bUnforceStateOnEvent;

var int ActionNum;
var int AnimsRemaining;
var ScriptedSequence SequenceScript;
var LatentScriptedAction CurrentAction;
var Action_PLAYANIM CurrentAnimation;
var float ScriptPriorityOnEvent;

var Actor ScriptedFocus;
var PlayerController MyPlayerController;
var int NumShots;
var name FiringMode;
var int IterationCounter;
var int IterationSectionStart;

function TakeControlOf(Pawn aPawn)
{
	if ( Pawn != aPawn )
	{
		aPawn.PossessedBy(self);
		Pawn = aPawn;
	}
	GotoState('Scripting');
}

function SetEnemyReaction(int AlertnessLevel);

function DestroyPawn()
{
	if( Pawn != None )
		Pawn.Destroy();

	Destroy();
}

function Pawn GetMyPlayer()
{
	if( MyPlayerController == None || MyPlayerController.Pawn == None )
	{
		ForEach DynamicActors(class'PlayerController',MyPlayerController)
		{
			if ( MyPlayerController.Pawn != None )
				break;
		}
	}

	if ( MyPlayerController == None )
		return None;

	return MyPlayerController.Pawn;
}

function Pawn GetInstigator()
{
	if( Pawn != None )
		return Pawn;
	return Instigator;
}

function Actor GetSoundSource()
{
	if( Pawn != None )
		return Pawn;

	return SequenceScript;
}

function bool CheckIfNearPlayer(float Distance)
{
	local Pawn MyPlayer;

	MyPlayer = GetMyPlayer();
	return( (MyPlayer != None) && (VSize(Pawn.Location - MyPlayer.Location) < Distance+CollisionRadius+MyPlayer.CollisionRadius ) && Pawn.PlayerCanSeeMe() );
}

function ClearScript()
{
	ActionNum = 0;
	CurrentAction = None;
	CurrentAnimation = None;
	ScriptedFocus = None;
	Pawn.SetWalking(false);
	Pawn.ShouldCrouch(false);
}

function SetNewScript(ScriptedSequence NewScript)
{
	MyScript = NewScript;
	SequenceScript = NewScript;
	Focus = Pawn;
	ClearScript();
	SetEnemyReaction(3);
	SequenceScript.SetActions(self);
}

function ClearAnimation()
{
	AnimsRemaining = 0;
	bControlAnimations = false;
	CurrentAnimation = None;
	if( Pawn != None )
		Pawn.PlayWaiting();
}

// Used by editor
function bool ContainsPartialEvent( string StartOfEventName )
{
	if( SequenceScript.ContainsPartialEvent( StartOfEventName ) )
		return true;

	return super.ContainsPartialEvent(StartOfEventName);
}

function LeaveScripting();

state Scripting
{
	function DisplayDebug(Canvas Canvas, out float YL, out float YPos)
	{
		Super.DisplayDebug(Canvas,YL,YPos);
		Canvas.DrawText("AIScript "$SequenceScript$" ActionNum "$ActionNum, false);
		YPos += YL;
		Canvas.SetPos(4,YPos);
		CurrentAction.DisplayDebug(Canvas,YL,YPos);
	}

	// UnPossess()
	function UnPossess()
	{		
	}

	function LeaveScripting()
	{
		UnPossess();
	}

	function InitForNextAction()
	{
		SequenceScript.SetActions(self);		

		if( CurrentAction == None )
		{
			LeaveScripting();
			return;
		}		

		MyScript = SequenceScript;

		if( CurrentAnimation == None )
			ClearAnimation();
	}

	function Trigger( actor Other, pawn EventInstigator )
	{
		if( CurrentAction.CompleteWhenTriggered() )		
			CompleteAction();		
	}

	function Timer()
	{
		if( CurrentAction.WaitForPlayer() && CheckIfNearPlayer(CurrentAction.GetDistance()) )		
			CompleteAction();
		else if( CurrentAction.CompleteWhenTimer() )		
			CompleteAction();		
	}

	function AnimEnd(int Channel)
	{
		if ( CurrentAction.CompleteOnAnim(Channel) )
		{			
			CompleteAction();
			//return; //08Apr2004 JAH -- this return seems nonsensical here
			//and we want ClearAnimation to have an opportunity to be called
			//I've also put in a ClearAnimation call at Scripting EndState
			//If this return was there for a good reason, it might be able
			//to be put back... but talk to me first.
		}
		if ( Channel == 0 )
		{
			if ( (CurrentAnimation == None) || !CurrentAnimation.PawnPlayBaseAnim(self,false) )
				ClearAnimation();
		}
		else 
		{
			// FIXME - support for CurrentAnimation play on other channels
			Pawn.AnimEnd(Channel);
		}
	}

	function CompleteAction()
	{		
		ActionNum++;
		GotoState('Scripting','Begin');
	}

	function FailAction()
	{
		Warn(Pawn$" Scripted Sequence FAILURE "$SequenceScript$" ACTION "$CurrentAction);
		
		ActionNum++;
		bBroken = false;
		GotoState('Scripting', 'Begin');
	}


	function SetMoveTarget()
	{
		/*
		local Actor NextMoveTarget;

		//Focus = ScriptedFocus;
		NextMoveTarget = CurrentAction.GetMoveTargetFor(self);		
		if ( NextMoveTarget == None )
		{
			FailAction();
			return;
		}
		//if ( Focus == None )
		//	Focus = NextMoveTarget;
		MoveTarget = NextMoveTarget;
		if ( !ActorReachable(MoveTarget) )
		{
			MoveTarget = FindPathToward(MoveTarget,false);
			if ( Movetarget == None )
			{
				AbortScript();
				return;
			}
			//if ( Focus == NextMoveTarget )
			//	Focus = MoveTarget;				
		}

		//if( Focus == None )		
		//	Focus = Pawn;
		*/
	}

	function AbortScript()
	{		
		LeaveScripting();
	}

	function Tick(float DeltaTime)
	{
		if ( CurrentAction == None || !CurrentAction.StillTicking( self, DeltaTime ) ) 
			disable('Tick');		
	}

	function BeginState()
	{				
	}

	function EndState()
	{
		bUseScriptFacing = true;
		ClearAnimation();
	}

Begin:
	//if (Pawn != None)
	//	Log("ScriptedController::Begin "$CurrentAction$" for "$Pawn);
	InitforNextAction();
	//if (Pawn != None)
	//	Log("ScriptedController::InitForNextAction "$CurrentAction$" completed for "$Pawn);
	if( bBroken )
	{
		Sleep(0.1);
		FailAction();
	}
	if( CurrentAction.TickedAction() )
	{
		enable('Tick');
	}
	if ( CurrentAction.MoveToGoal() )
	{
		Pawn.SetMovementPhysics();
		WaitForLanding();
KeepMoving:

		//SetMoveTarget();
		//if (Pawn != None)
		//	Log("ScriptedController::MoveToward action "$CurrentAction$" for "$Pawn);
		MoveToward(MoveTarget,, ( Pawn.bIsWalking || !Pawn.bCanRun ) );

		/*
		if( MoveTarget != CurrentAction.GetMoveTargetFor( self ) ||
			!Pawn.ReachedDestination( CurrentAction.GetMoveTargetFor( self ) ) )
		{
			Goto('KeepMoving'); //This can cause an infinite loop if MoveToward doesn't succeed and therefore isn't latent
		}
		*/
		CompleteAction();
	}
	else if( CurrentAction.TurnToGoal() )
	{
		Pawn.SetMovementPhysics();
		Focus = CurrentAction.GetMoveTargetFor(self);
		if( Focus == None )
			Focus = Pawn;
		FinishRotation();
		
		CompleteAction();
	}
	else
	{		
		Pawn.Acceleration = vect(0,0,0);
		Focus = ScriptedFocus;
		if ( !bUseScriptFacing )
			SetFocusRot( Pawn.Rotation );		

		if( Focus == None )
			Focus = Pawn;

		FinishRotation();		
	}
End:
	;
}

// Broken scripted sequence - for debugging
State Broken
{
Begin:
	warn(Pawn$" Scripted Sequence BROKEN "$SequenceScript$" ACTION "$CurrentAction);	
	GotoState('BotAI');
}


defaultproperties
{
     bUseScriptFacing=True
     IterationSectionStart=-1
}

