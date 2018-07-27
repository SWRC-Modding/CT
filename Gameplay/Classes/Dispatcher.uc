//=============================================================================
// Dispatcher: receives one trigger (corresponding to its name) as input, 
// then triggers a set of specifid events with optional delays.
//=============================================================================
class Dispatcher extends Actor
	placeable;

#exec Texture Import File=Textures\Dispatch.pcx Name=S_Dispatcher Mips=Off MASKED=1

enum EDispatcherBehavior
{	
	DBV_Pause,
	DBV_Reset,
	DBV_StartOver,
	DBV_Ignore,
	DBV_OnceOnly
};

struct DispatcherEvent
{
	var() name Event;
	var() float Delay;
	var() float	DelayMax;
	var() Actor SoundSource;
	var() Sound Sound;
	var() bool  bWaitForSound;
};

//-----------------------------------------------------------------------------
// Dispatcher variables.

var() bool					 bLoop;

var() EDispatcherBehavior	 TriggerBehavior;
var() array<DispatcherEvent> Events;	// Events to generate.
var int EventCounter;				// Internal counter.

//Editor function for determining whether an event matches this thing
function bool ContainsPartialEvent(string StartOfEventName)
{
	local string EventString;
	local int i;
	for (i = 0; i < Events.Length; ++i)
	{
		EventString = string(Events[i].Event);
		if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
		{
			return true;
		}
	}
	return Super.ContainsPartialEvent(StartOfEventName);
}


//=============================================================================
// Dispatcher logic.

//
// When dispatcher is triggered...
//
function Trigger( actor Other, pawn EventInstigator )
{
	GotoState('Dispatch');
}

//
// Dispatch events.
//
state Dispatch
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		switch ( TriggerBehavior )
		{
		case DBV_Pause:			
			GotoState('');
			break;
		case DBV_Reset:
            EventCounter = 0;
			GotoState('');
			break;
		case DBV_StartOver:
			EventCounter = 0;
			break;
		}		
	}

Begin:
	while( EventCounter < Events.Length )
	{
		Sleep( RandRange( Events[EventCounter].Delay, Events[EventCounter].Delay + Events[EventCounter].DelayMax ) );
		if( Events[EventCounter].Event != '' )
			TriggerEvent(Events[EventCounter].Event,self,Instigator);
		if( Events[EventCounter].Sound != None )
		{
			if (Events[EventCounter].SoundSource != None)
				Events[EventCounter].SoundSource.PlaySound( Events[EventCounter].Sound );
			else
				PlaySound( Events[EventCounter].Sound );
			if (Events[EventCounter].bWaitForSound)
				Sleep(GetSoundDuration(Events[EventCounter].Sound) + 0.1);
		}

		EventCounter++;
	}
	EventCounter=0;
	if( bLoop )		
		Goto('Begin');	
	else if( TriggerBehavior == DBV_OnceOnly )
		GotoState('Completed');
	else
		GotoState('');
}

state Completed
{
	ignores Trigger;	
}


defaultproperties
{
     bHidden=True
     Texture=Texture'Gameplay.S_Dispatcher'
}

