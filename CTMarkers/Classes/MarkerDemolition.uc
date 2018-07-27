// ====================================================================
//  Class:  CTMarkers.MarkerDemolition
//
//  Test Marker Object
// ====================================================================

class MarkerDemolition extends MarkerDemolitionNative
	placeable;

function Trigger( actor Other, pawn EventInstigator )
{
	//trigger only if enabled
	Log("MarkerDemolition::Trigger with Tag: "$Tag);
	//ignore similar events from anyone who isn't the marker participant
	if (EventInstigator != Participants[0].Member)
		return;

	if (bMarkerActive && HiddenTag == PlaceEvent)
	{
		if (Anchor != None)
		{
			ActivateItem(Anchor).StartSetup(EventInstigator.Controller);
		}

		HiddenTag = ''; //clear out the tag, it can't be reused
	}

}


defaultproperties
{
     PlaceEvent="MDplaceCharge"
     MarkerAnims(0)="DemolitionSetChargeStart"
     MarkerAnims(1)="DemolitionSetChargeLoop"
     MarkerAnims(2)="DemolitionSetChargeStop"
     ActionString="Setting Charge"
     Participants(0)=(IconMesh=StaticMesh'MarkerIcons.Demolition.DemolitionSM1DemolitionSet',bIconOnAnchorPt=True)
     PreferredPawnClass=Class'CTCharacters.Commando62Delta'
     ActivatePromptText="PRESS @ TO ENGAGE DEMOLITION MANEUVER"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptText="PRESS @ TO CANCEL DEMOLITION MANEUVER"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
     StaticMesh=StaticMesh'MarkerIcons.Demolition.DemolitionSM1DemolitionSet'
}

