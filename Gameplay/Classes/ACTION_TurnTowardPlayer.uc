class ACTION_TurnTowardPlayer extends LatentScriptedAction;

function bool InitActionFor(ScriptedController C)
{
	C.ScriptedFocus = C.GetMyPlayer();
	C.CurrentAction = self;
	return true;	
}

function bool TickedAction()
{
	return true;
}

//*****************************************************************************************
// Action Queries

function bool StillTicking(ScriptedController C, float DeltaTime)
{
	local vector LookDirection, FocusDirection;	
	
	LookDirection = vector( C.Pawn.Rotation );
	FocusDirection = C.Focus.Location - C.Pawn.Location;
	FocusDirection.Z = 0;
	FocusDirection = Normal( FocusDirection );

	// REVISIT: This should use a degrees to radians conversion constant
	if( FocusDirection Dot LookDirection >= cos( ( C.Pawn.MaxAimYaw * 0.5 ) * ( 3.14159 / 180 ) ) )
	{	
		// This is really hacky, but I don't know of another easy way to get the action to terminate
		// perhaps a better solution would be to modify the Tick function in ScriptedController::Scripted
		// to call complete action when the tick is disabled
		C.Trigger( None, None ); 
		return false;
	}
	else
	{		
		return true;	
	}
}

function bool CompleteWhenTriggered()
{
	return true;
}


defaultproperties
{
     ActionString="Turn toward player"
     bValidForTrigger=False
}

