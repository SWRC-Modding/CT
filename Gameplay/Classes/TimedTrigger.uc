//=============================================================================
// TimedTrigger: causes an event after X seconds.
//=============================================================================
class TimedTrigger extends Triggers	
	hidecategories(Advanced,Collision,Display,Force,Karma,LightColor,Lighting,Movement,Sound);

#exec Texture Import File=Textures\S_TimedTrigger.bmp Name=S_TimedTrigger Mips=Off MASKED=1

enum ETimerTriggerBehavior
{
	TTB_PauseTimer,
	TTB_RestartTimer
};

var()	float					Delay;
var()	bool					bRepeating;
var()	ETimerTriggerBehavior	TriggerBehavior;

auto state Waiting
{
	function Trigger(Actor Other, Pawn EventInstigator)
	{		
		SetTimer(Delay, false);
		GotoState('Running');
	}
}

state Running
{
	function Timer()
	{		
		TriggerEvent(Event,self,None);
		if(!bRepeating)
			GotoState('Waiting');
		else
			SetTimer(Delay, false);
	}
	function Trigger(Actor Other, Pawn EventInstigator)
	{		
		if( TriggerBehavior == TTB_RestartTimer )
			GotoState('Waiting');
		else if( TriggerBehavior == TTB_PauseTimer )
		{			
			GotoState('Paused');
		}
	}
}

state Paused
{
	function Trigger(Actor Other, Pawn EventInstigator)
	{		
		SetTimer(TimerCounter, false);
		GotoState('Running');
	}  
}


defaultproperties
{
     Delay=1
     bCollideActors=False
     Texture=Texture'Gameplay.S_TimedTrigger'
}

