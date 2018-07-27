// ====================================================================
//  Class:  CTMarkers.MarkerGrenade
//
//  Test Marker Object
// ====================================================================

class MarkerGrenade extends MarkerWeapon
	;

//var			Actor		ArcPreviewActor;

//var static	StaticMesh	ArcPreviewMesh;
//var(Marker)  vector		ArcOffset;


/*
function PostBeginPlay()
{
	local float Scale;
	local vector HorizontalOffset;
	local vector NewDrawScale3D;
	local vector ToTarget;
	local vector StartGrenadeLocation;
	local vector ArcOffset;

	super.PostBeginPlay();

	if (ArcPreviewMesh != None && WeaponTarget != None)
	{
		ArcOffset.X = -155;
		ArcOffset.Y = -36;
		ArcOffset.Z = 0;
		scale = 1.0;
		HorizontalOffset = ArcOffset;
		HorizontalOffset.Z = 0;
		StartGrenadeLocation = Location + (HorizontalOffset >> Rotation);
		StartGrenadeLocation.Z = Location.Z + ArcOffset.Z;
		ToTarget = WeaponTarget.Location - StartGrenadeLocation;
		ToTarget.Z = 0; //zero out z-difference
		ArcPreviewActor = Spawn(class'IntangibleActor',,,StartGrenadeLocation,Rotation);
		//Log("Grenade marker location: "$Location);
		//Log("Grenade marker grenade location: "$StartGrenadeLocation);
		//Log("Grenade marker ToTarget vector: "$ToTarget);
		//Log("Grenade marker ToTarget vector normal: "$Normal(ToTarget));
		ArcPreviewActor.SetStaticMesh(ArcPreviewMesh);
		scale = VSize(ToTarget)/688.0;
		//Log("Grenade marker arc scale: "$scale);
		NewDrawScale3D.X = scale;
		NewDrawScale3D.Y = scale;
		NewDrawScale3D.Z = 1.0;
		ArcPreviewActor.SetDrawScale3D(NewDrawScale3D);
		ArcPreviewActor.bHidden = true;
	}
}
*/

/*
simulated function NotifyTargeted( PlayerController Player )
{
	super.NotifyTargeted(Player);
	if (!bMarkerActive && ArcPreviewActor != None)
		ArcPreviewActor.bHidden = false;
}

simulated function NotifyUnTargeted( PlayerController Player )
{
	super.NotifyUnTargeted(Player);
	if (ArcPreviewActor != None)
		ArcPreviewActor.bHidden = true;
}
*/


defaultproperties
{
     MarkerWeaponType=MWT_Grenade
     GrenadeClass=Class'CTInventory.ThermalDetonator'
     CompleteCue=PAE_GrenadeComplete
     LocationPreviewMesh=StaticMesh'MarkerIcons.SetTrap.BombRadius'
     HUDIconY=1
     ActionString="Grenade area"
     InitiateCue=PAE_GrenadeInitiate
     ConfirmCue=PAE_GrenadeConfirm
     CancelCue=PAE_GrenadeCancel
     CancelConfirmCue=PAE_GrenadeCancelConfirm
     ActivatePromptText="PRESS @ TO ENGAGE GRENADE MANEUVER"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptText="PRESS @ TO CANCEL GRENADE MANEUVER"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
     StaticMesh=StaticMesh'MarkerIcons.Grenade.GrenadeSM1ThrowGrenade'
}

