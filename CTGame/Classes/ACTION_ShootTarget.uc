class ACTION_ShootTarget extends LatentScriptedAction;

var(Action) name	TargetTag;
var(Action) float	Duration;

function bool InitActionFor(ScriptedController C)
{
	local Actor	ShootTarget;

	C.CurrentAction = self;

	if( Duration > 0 )
		C.SetTimer(Duration, false);	

	if( ShootTarget == None && TargetTag != '' )
	{
		ForEach C.AllActors(class'Actor',ShootTarget,TargetTag)
			break;
	}

	if( ShootTarget != None && C.Pawn.Weapon != None && C.IsA('CTBot') )
	{
		CTBot(C).AddScriptedShootGoal(ShootTarget,Duration);
		C.Target = ShootTarget;
	}

	return true;
}

function bool CompleteWhenTimer()
{
	return ( Duration > 0 );
}

function bool CompleteWhenTicked()
{
	return true;
}

function bool StillTicking(ScriptedController C, float DeltaTime)
{
	if( C.Target == None || C.Target.bDeleteMe )
	{
		C.Target = None;
		return false;
	}

	if( C.Target.IsA('Pawn') && Pawn(C.Target).Health <= 0 )
		return false;

	if( C.Target.IsA('Prop') && Prop(C.Target).Health <= 0 )
		return false;

	if( C.Target.IsA('DroidDispenser') && DroidDispenser(C.Target).Health <= 0 )
		return false;

	return true;
}


defaultproperties
{
     ActionString="shoot target"
}

