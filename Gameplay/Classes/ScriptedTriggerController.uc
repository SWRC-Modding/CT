//=============================================================================
// ScriptedTriggerController
// used for playing ScriptedTrigger scripts
// A ScriptedTriggerController never has a pawn
//=============================================================================
class ScriptedTriggerController extends ScriptedController;

function InitializeFor(ScriptedTrigger T)
{
	SequenceScript = T;
	ActionNum = 0;
	SequenceScript.SetActions(self);
	GotoState('Scripting', 'Begin');
}

function GameHasEnded() {}
function ClientGameEnded() {}

function DestroyPawn()
{
	if ( Instigator != None )
		Instigator.Destroy();
}

function ClearAnimation() {}

function SetNewScript(ScriptedSequence NewScript)
{
	SequenceScript = NewScript;
	ActionNum = 0;
	Focus = None;
	SequenceScript.SetActions(self);
}

state Scripting
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		Instigator = EventInstigator;
		Super.Trigger(Other,EventInstigator);
	}

	event Tick(float DeltaTime)
	{
		if ( !CurrentAction.StillTicking(self,DeltaTime))
		{
			disable('Tick');
			/*
			if (CurrentAction.TickedAction())
			{
				CompleteAction();
			}
			*/
		}
	}

	function LeaveScripting()
	{
		Destroy();
	}

Begin:
	DebugAILog("ScriptedTriggerController::Scripting::Begin "$self);
	InitforNextAction();
	if ( bBroken )
		GotoState('Broken');
	if ( CurrentAction.TickedAction() )
	{
		Log("ScriptedTriggerController::Scripting::Begin: Enabling Tick");
		enable('Tick');
	}
}

// Broken scripted sequence - for debugging
State Broken
{
Begin:
	warn(" Trigger Scripted Sequence BROKEN "$SequenceScript$" ACTION "$CurrentAction);
}

defaultproperties
{
}

