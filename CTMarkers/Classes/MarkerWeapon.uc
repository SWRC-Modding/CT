// ====================================================================
//  Class:  CTMarkers.MarkerAntiArmor
//
//  Test Marker Object
// ====================================================================

class MarkerWeapon extends CTMarker
	abstract
	native;

enum EMarkerWeaponType
{
	MWT_Sniper,
	MWT_AntiArmor,
	MWT_Grenade,
};

var static EMarkerWeaponType MarkerWeaponType;
var(Marker) Actor WeaponTarget;
var(Marker) float WeaponTargetRadius;
var static float Accuracy;
var static float FOV;
var(Marker) class<Weapon> GrenadeClass;

var static PawnAudioTable.EPawnAudioEvent CompleteCue;

var(Marker)		StaticMesh	LocationPreviewMesh;
var				Actor		LocationPreviewActor;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

function PostBeginPlay()
{
	local float scale;
	local vector ToTarget;

	super.PostBeginPlay();
	//SetStaticMesh(Participants[MarkerWeaponType].IconMesh);
	//REVISIT: move grenade target down to floor?

	Anchor.bDrawHUDMarkerIcon = true;
	if (LocationPreviewMesh != None && WeaponTarget != None)
	{
		scale = 1.0;
		//Log("Created Bomb LocationPreviewActor");
		LocationPreviewActor = Spawn(class'IntangibleActor',,,WeaponTarget.Location,WeaponTarget.Rotation);
		LocationPreviewActor.SetStaticMesh(LocationPreviewMesh);
		if (LocationPreviewMesh == default.LocationPreviewMesh)
			scale = WeaponTargetRadius/512.0;
		LocationPreviewActor.SetDrawScale(scale);
		LocationPreviewActor.bHidden = true;
	}
	if (WeaponTarget != None)
	{
		//orient hologram towards WeaponTarget
		ToTarget = WeaponTarget.Location - Location;
		SetRotation(Rotator(ToTarget));
	}
}

simulated function NotifyTargeted( PlayerController Player )
{
	super.NotifyTargeted(Player);
	if (!bMarkerActive && LocationPreviewActor != None)
		LocationPreviewActor.bHidden = false;
}

simulated function NotifyUnTargeted( PlayerController Player )
{
	super.NotifyUnTargeted(Player);
	if (LocationPreviewActor != None)
		LocationPreviewActor.bHidden = true;
}

function Trigger( actor Other, pawn EventInstigator )
{
	if (bMarkerActive)
	{
		if (HiddenTag == 'MSknockOver' && Anchor.IsA('KnockoverProp'))
		{
			Anchor.Trigger(Other, EventInstigator);
			HiddenTag = 'None';
		}
	}
}




cpptext
{
	virtual UBOOL Initiate(APawn* Instigator);
	virtual UBOOL Update();
	virtual UBOOL FinishMarker();

}

defaultproperties
{
     WeaponTargetRadius=512
     HUDIconY=8
     Participants(0)=(bIconOnAnchorPt=True)
     bCanCancelAfterUnderway=True
     bCancelIfMembersNeeded=True
     ActivatePromptText="PRESS @ TO ENGAGE ANTI-ARMOR MANEUVER"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptText="PRESS @ TO CANCEL ANTI-ARMOR MANEUVER"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
}

