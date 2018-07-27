class ACTION_PlayAnim extends LatentScriptedAction;

var(Action) name BaseAnim;
var(Action) float AnimRate;
var(Action) byte AnimIterations;
var(Action) bool bLoopAnim;
var(Action) bool bWaitUntilAnimEnd;
var(Action) float StartFrame;

function bool InitActionFor(ScriptedController C)
{
	// play appropriate animation
	C.AnimsRemaining = AnimIterations;
	if ( PawnPlayBaseAnim(C,true) )
	{
		C.CurrentAction = self;
		C.CurrentAnimation = self;
		return TickedAction();
	}

	return false;	
}

function SetCurrentAnimationFor(ScriptedController C)
{
	if ( C.Pawn.IsAnimating(0) )
		C.CurrentAnimation = self;
	else
		C.CurrentAnimation = None;
}

function bool CompleteOnAnim(int channel)
{
	return (TickedAction() && 0 == channel);
}

function bool PawnPlayBaseAnim(ScriptedController C, bool bFirstPlay)
{
	if ( BaseAnim == '' )
		return false;

	C.bControlAnimations = true;
	if ( bFirstPlay )
		C.Pawn.PlayAnim(BaseAnim,,AnimRate,StartFrame);
	else if ( bLoopAnim || (C.AnimsRemaining > 0) )
		C.Pawn.LoopAnim(BaseAnim,,AnimRate);
	else
		return false;
		
	return true;
}

function bool TickedAction()
{
	// NathanM: Looping the animation will force this action to return immediately
	return ( bWaitUntilAnimEnd && !bLoopAnim );
}

//*****************************************************************************************
// Action Queries

function bool StillTicking(ScriptedController C, float DeltaTime)
{
	if( C.Pawn != None )
	{
		if( C.Pawn.IsPlayingAnim(BaseAnim) )
		{
			return true;
		}
	}

	return false;
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@BaseAnim$" // "$Comment;
	return ActionString@BaseAnim;
}


defaultproperties
{
     AnimRate=1
     bWaitUntilAnimEnd=True
     StartFrame=-1
     ActionString="play animation"
     bValidForTrigger=False
}

