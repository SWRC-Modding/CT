// ====================================================================
//  Class:  CTMarkers.MarkerDisarmTrap
//
//  Test Marker Object
// ====================================================================

class MarkerDisarmTrap extends MarkerDisarmTrapNative
	placeable;

//var class<Emitter> EffectClass;
//should we have sounds and anims lists for each character?
//or specifically labelled sounds and anims?
//the nice thing about this is it would generate the names at script time...

function Trigger( actor Other, pawn EventInstigator )
{
	//trigger only if enabled
	Log("MarkerDisarmTrap::Trigger with Tag: "$Tag);
	if (bMarkerActive && HiddenTag == RemoveEvent)
	{
		if (Anchor != None)
		{
			Anchor.bHidden = true;
			Anchor.Destroy();
		}

		HiddenTag = ''; //clear out the tag, it can't be reused
	}

}

simulated function NotifyTargeted( PlayerController Player )
{
	super.NotifyTargeted(Player);
	Anchor.NotifyTargeted(Player);
}

simulated function NotifyUnTargeted( PlayerController Player )
{
	super.NotifyUnTargeted(Player);
	Anchor.NotifyUnTargeted(Player);
}



defaultproperties
{
     RemoveEvent="MDTremoveTrap"
     MarkerAnims(0)="TrapPlantDisarmStart"
     MarkerAnims(1)="TrapPlantDisarmLoop"
     MarkerAnims(2)="TrapPlantDisarmStop"
     ActionString="Disarming Trap"
     Participants(0)=(IconMesh=StaticMesh'MarkerIcons.SetTrap.TrapSM1PlantDisarmIcon',bIconOnAnchorPt=True)
     PreferredPawnClass=Class'CTCharacters.Commando62Delta'
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
     StaticMesh=StaticMesh'MarkerIcons.SetTrap.TrapSM1PlantDisarmIcon'
}

