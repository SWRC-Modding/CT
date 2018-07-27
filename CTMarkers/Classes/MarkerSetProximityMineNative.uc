// ====================================================================
//  Class:  CTMarkers.MarkerProximityMine
//
//  Test Marker Object
// ====================================================================

class MarkerSetProximityMineNative extends CTMarker
	native;

var static Name PlaceEvent;
var static Name ArmEvent;

//var class<Emitter> EffectClass;
//should we have sounds and anims lists for each character?
//or specifically labelled sounds and anims?
//the nice thing about this is it would generate the names at script time...

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

function Trigger( actor Other, pawn EventInstigator )
{
	//trigger only if enabled
	if (EventInstigator != Participants[0].Member)
		return;
	//Log("MarkerSetProximityMine::Trigger with Tag: "$Tag);
	if (bMarkerActive && HiddenTag == PlaceEvent)
	{
		//Activate the trap
		if (Anchor != None)
		{
			Anchor.GotoState('Placed');
		}

		HiddenTag = ArmEvent;
	}
	else if (bMarkerActive && HiddenTag == ArmEvent)
	{
		if (Anchor != None)
		{
			ProximityMine(Anchor).Activate(EventInstigator);
		}
		HiddenTag = '';
	}

}



cpptext
{
	//UBOOL Initiate(APawn* Instigator);
	virtual UBOOL Update();

}

defaultproperties
{
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
}

