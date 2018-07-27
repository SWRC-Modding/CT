//=============================================================================
// Counter: waits until it has been triggered 'NumToCount' times, and then
// it sends Trigger/UnTrigger events to actors whose names match 'EventName'.
//=============================================================================
class Counter extends Triggers;

#exec Texture Import File=Textures\Counter.pcx Name=S_Counter Mips=Off MASKED=1

//-----------------------------------------------------------------------------
// Counter variables.

var() byte       NumToCount;                // Number to count down from.
var() bool       bShowMessage;              // Display count message?
var() localized  string CountMessage;       // Human readable count message.
var() localized  string CompleteMessage;    // Completion message.
var   byte       OriginalNum;               // Number to count at startup time.

//-----------------------------------------------------------------------------
// Counter functions.

//
// Init for play.
//
function BeginPlay()
{
	OriginalNum = NumToCount;
}

/* Reset() 
reset actor to initial state - used when restarting level without reloading.
*/
function Reset()
{
	NumToCount = OriginalNum;
}

//
// Counter was triggered.
//
function Trigger( actor Other, pawn EventInstigator )
{
	local string S;
	local string Num;
	local int i;

	if( NumToCount > 0 )
	{
		if( --NumToCount == 0 )
		{
			// Trigger all matching actors.
			if( bShowMessage && CompleteMessage != "" )
			{
				// Send message to local player reqardless of whether they were the instigator
				//EventInstigator.ClientMessage( CompleteMessage );
				Level.GetLocalPlayerController().ClientMessage( CompleteMessage );
			}
			TriggerEvent(Event,Other,EventInstigator);
		}
		else if( bShowMessage && CountMessage != "" )
		{
			// Still counting down.
			switch( NumToCount )
			{
				case 1:  Num="one"; break;
				case 2:  Num="two"; break;
				case 3:  Num="three"; break;
				case 4:  Num="four"; break;
				case 5:  Num="five"; break;
				case 6:  Num="six"; break;
				default: Num=string(NumToCount); break;
			}
			S = CountMessage;
			while( InStr(S, "%i") >= 0 )
			{
				i = InStr(S, "%i");
				S = Left(S,i) $ Num $ Mid(S,i+2);
			}
			// Send message to local player reqardless of whether they were the instigator
			//EventInstigator.ClientMessage( S );
			Level.GetLocalPlayerController().ClientMessage( S );
		}
	}
}


defaultproperties
{
     NumToCount=2
     CountMessage="Only %i more to go..."
     CompleteMessage="Completed!"
     Texture=Texture'Gameplay.S_Counter'
}

