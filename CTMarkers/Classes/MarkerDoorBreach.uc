// ====================================================================
//  Class:  CTGame.DoorBreachMarker
//
//  Test Marker Object
// ====================================================================

class MarkerDoorBreach extends MarkerDoorBreachNative
	placeable;

function Trigger( actor Other, pawn EventInstigator )
{
	local Actor Grenade;
	local Vector GrenadeOrigin;
	//trigger only if enabled
	//Log("MarkerDoorBreach::Trigger with Tag: "$Tag);
	if (bMarkerActive)
	{
		if (HiddenTag == DestroySwitchEvent)
		{
			ActivateItem(Anchor).User = EventInstigator.Controller;
			//Log("MarkerDoorBreach::DestroySwitchEvent setting state ChargeLocked");
			bComplete = true;	//the marker can be considered complete here
			Anchor.GotoState('ChargeLocked');
			//Actors[0].Trigger(self, Participants[0].Member);
			HiddenTag = TossGrenadeEvent;
		}
		else if (HiddenTag == TossGrenadeEvent)
		{
			//Log("MarkerDoorBreach::TossGrenadeEvent");
			//Actors[2].Weapon.ThrowGrenade();
			if (TotalParticipants > 1)
			{
				GrenadeOrigin = Participants[1].Member.GetBoneLocation('wrist_L');
				Grenade = Spawn(GrenadeClass,,,GrenadeOrigin);
				
				if (GrenadeTarget != None)
				{
					//throw the grenade towards the grenade target
					//Log("MarkerDoorBreach: Throwing grenade with location "$Grenade.Location$" at target "$GrenadeTarget);
					Grenade.bHidden = false;
					Grenade.Velocity = 2.0f * (GrenadeTarget.Location - Grenade.Location);
					Grenade.Velocity.Z = 120;
					//Log("Throwing grenade with velocity "$Grenade.Velocity);
				}
				//else
				//	Grenade.Velocity = 256.0f * Squad.SquadMembers[2].GetBoneCoords('wrist_L').ZAxis;
				
				Grenade.Instigator = Participants[1].Member;
				Grenade.SetTimer(2.0f, false);
			}
			HiddenTag = '';
		}
	}
}


defaultproperties
{
     DestroySwitchEvent="MDBDestroySwitch"
     TossGrenadeEvent="MDBtossGrenade"
     GrenadeClass=Class'CTInventory.ThermalDetonatorProj'
     MarkerAnims(0)="GestureSignalFast"
     MarkerAnims(1)="DoorBreachLoudSM1Doorman"
     MarkerAnims(2)="DoorBreachLoudSM2Grenademan"
     MarkerAnims(3)="DoorBreachLoudSM3Coverman"
     ActionString="Breaching Door"
     Participants(0)=(IconMesh=StaticMesh'MarkerIcons.DoorBreach.DoorBreachSM1DoormanIcon',bIconOnAnchorPt=True)
     Participants(1)=(IconMesh=StaticMesh'MarkerIcons.DoorBreach.DoorBreachSM2GrenademanIcon',bIconOnAnchorPt=True)
     Participants(2)=(IconMesh=StaticMesh'MarkerIcons.DoorBreach.DoorBreachSM2CovermanIcon',bIconOnAnchorPt=True)
     FinishStance=Class'CTGame.StanceHoldLoose'
     ActivatePromptText="PRESS @ TO ENGAGE DOOR BREACH MANEUVER"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptText="PRESS @ TO CANCEL DOOR BREACH MANEUVER"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
     StaticMesh=StaticMesh'MarkerIcons.DoorBreach.DoorBreachSM1DoormanIcon'
}

