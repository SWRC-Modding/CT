//=============================================================================
// Parent class of all weapons.
// FIXME - should bReplicateInstigator be true???
//=============================================================================
class Weapon extends Inventory
	abstract
	native
	nativereplication;


enum EFireMode
{	
	FM_SemiAuto,		//You have to press fire every time you want to shoot, probably only for player weapons
	FM_Burst,			//Each fire request will generate a BurstNumber of shots
	FM_Automatic,		//Fire constantly 
	FM_AnimationDriven	//PlaceHolder
};

enum EWeaponType
{
	WT_None,			//Attached Scav Droid and Turrets
	WT_Primary,
	WT_Secondary,
	WT_Thrown,
	WT_Heal,
	WT_Turret,
	WT_Marker
};

enum EPawnWeaponAnimation
{
	PWA_Fire,
	PWA_Holster,
	PWA_Load,
	PWA_Idle,
	PWA_Melee,
	PWA_Throw,
	PWA_Reload
};

// 0: Blaster  1: Sniper  2: AA  3:Pistol  4: Shotgun  5: Elite Beam  6: Conc Rifle  7: Rocket Launcher  8: Bowcaster  9: SMG  10: Trandoshan Rifle 
enum EWeaponIndexType
{	
	WI_Blaster,
	WI_Sniper,
	WI_AntiArmor,
	WI_Pistol,
	WI_Shotgun,
	WI_EliteBeam,
	WI_ConRifle,
	WI_RocketLauncher,
	WI_BowCaster,
	WI_SMG,
	WI_TrandoshanRifle
};

//-----------------------------------------------------------------------------
// Weapon ammo information:
var		class	<Ammunition>	AmmoName;		// Type of ammo used.
var		travel	Ammunition		AmmoType;		// Inventory Ammo being used.
var()	int						PickupAmmoCount;// Amount of ammo initially in pick-up item.
var()	int						InitAmmoAmount;	// Amount of ammo at initial level load

var()	travel	int				ReloadCount;	// Amount of ammo depletion before reloading. 0 if no reloading is done.
var		int						FireCount;		// Number of shots fired off - used for occasional traces of instant hit weapons
var()	name					OutOfAmmoEvent;	// Event to fire off when the weapon is out of ammo and auto switches to the next weapon - no autoswitch in MP

//-----------------------------------------------------------------------------
// Weapon firing/state information:

//var		bool		bPointing;				// Indicates weapon is being pointed
var		bool		bWeaponUp;				// Used in Active State
var		bool		bChangeWeapon;			// Used in Active State
var		bool		bCanThrow;				// if true, player can toss this weapon out
var		bool		bRapidFire;				// used by pawn animations in determining firing animation, and for net replication
var		bool		bForceReload;
var		bool		bCanReload;				// all charge weapons except for the pistol cannot reload
var		bool		bSpectated;
var		bool		bMeleeWeapon;			// Weapon is only a melee weapon
var		bool		bSteadyToggle;
var		bool		bForceFire;
var		bool		bForceAltFire;
var()	bool		bForcesWeaponChange;	// forces a weapon change
var		bool		bAutoFire;
var		bool		bJustTravelled;			// true if we just traveled from another level
var		bool		bApplyKick;				// should there be a kick
var()	bool		bRecoilShake;			// randomize up/down recoil kick to simulate shake?
var		bool		bWeaponZoom;			// is the weapon zoomed?
var		bool		bPlayingIdle;			// is it playing the idle anim?  if so, don't interrupt w/ the breathe anim
var		bool		bLoopMelee;				// loop melee anim between BeginMeleeLoopFrame and AnimNotify to end loop
var()	bool		bLoopMuzzleFX;			// loop the muzzle fx
var()   bool	    bZoomedAltFireCapable;  // Allowed to alt fire (melee) when zoomed
var		bool		bZoomedUsesNoHUDArms;	// Whether or not HUDArms appear when zoomed
var		bool		bCanSwitchGrenade;		// Can switch grenade
var		bool		bSaveAsLastWeapon;		// Rememeber this weapon as the last weapon
var()   bool		bCanThrowGrenadesWhenZoomed; // Can throw grenades when zoomed

var		Weapon		NewWeapon;
var		int			AutoSwitchPriority;
var     vector		FireOffset;				// Offset from first person eye position for projectile/trace start
var 	float		ShakeMag;				// how far to roll view as it shakes - no helmet roll currently
var() 	float		ShakeTime;				// should be RollTime - how long to roll - no helmet roll currently
var() 	vector		ShakeVert;
var		float		RollRate;				// no helmet roll currently
var()	vector		ShakeSpeed;
var()	float		NumShakes;
var		texture		Reticle;
var		Powerups	Affector;				// powerup chain currently affecting this weapon

//var		float		BurstTime;				// the amount of time before AI stops trying to shoot weapon
var(AI)	int			BurstCount;				// the number of shots AI will try to shoot with the weapon

var		EFireMode	FireMode;
var		EWeaponType WeaponType;

var		const vector FPFireOffset;			//The Offset when firing the weapon from first person view
var		const vector FPGrenadeOffset;		//The Offset when firing the grenade from first person view

//-----------------------------------------------------------------------------
// Recoil

var()	float		RecoilConstant;			// spring constant - higher for tighter springs
var()	float		RecoilDampening;		// factor at which spring stops recoil
var()	float		DisableRecoil;			// threshhold in pitch and yaw at which to disable recoil
var()	float		XRecoilMultiplier;		// xbox only
var()	float		XShakeMultiplier;		// xbox only
var()	rotator		WeaponKick;				// amount of kick
var		rotator		RecoilVelocity;			// velocity of recoil
var		rotator		InitRotation;			// initial view rotation before kick is applied

//-----------------------------------------------------------------------------
// AI information
var()	float			AimError;			// Aim Error for bots (note this value doubled if instant hit weapon)
var()	float			ZoomedAimError;
var()	float			ZoomDamageMultiplier;
var()	float			OptimalDist;		// The best distance for this weapon
var()	Range			ValidRange;			// The min and max range for this weapon

//-----------------------------------------------------------------------------
// Sound Assignments
var()	sound 		FireSound;
var()	sound		WeaponEmptySound;
var()	sound		GrenadeEmptySound;
var()	sound		HolsterSound;

//-----------------------------------------------------------------------------
// messages
var()	Localized string	MessageNoAmmo;
var		Localized string	WeaponDescription;
var		Color				NameColor;	// used when drawing name on HUD

//-----------------------------------------------------------------------------
// HUD related vars
var		float	DisplayFOV;			// FOV for drawing the weapon in first person view
var()   float   ZoomFOVs[4];		// FOVs when the player zooms -- could be more than one (max 4)
var()	int		NumZoomFOVs;		// Number of FOVs
var		int		CurrentZoomFOVIndex;// Index of current FOV
var()	VisionMode		VisionWeapon;		// Vision mode for this weapon
var()	VisionMode		VisionWeaponZoom;	// Vision mode for this weapon when zoomed

//-----------------------------------------------------------------------------
// first person Muzzle Flash

var			class <Emitter>	MuzzleClass;		// first person muzzle effect class
var					Emitter	MuzzleFlash;		// first person muzzle flash
var					byte	FlashCount;			// when incremented, draw muzzle flash for current frame
var					name	MuzzleBone;			// bone to attach the muzzle flash

//-----------------------------------------------------------------------------
// Additional Attachment Effects

var()	class<Emitter>	BreechEffectClass;
var		Emitter			BreechEffect;
var()	vector			BreechOffset;
var()	name			BreechAttachBone;

var()	bool			bHasAmmoNeedle;
var()	float			FullClipAngle;			// angle when the clip is full
var()	float			EmptyClipAngle;			// angle when the clip is empty
var		float			AnglePerShot;			// amount of rotation each ammo gives
var()	float			NeedleAngle;			// angle to empty or full

//-----------------------------------------------------------------------------
// Variables that determine when the next valid fire time is
var		float		NextShotTime;				// the next game time at which you can fire
var()	float		ShotDelay;					// the number of seconds between shots
var		float		LastTimeSeconds;

var		float		WeaponAccuracy;

//-----------------------------------------------------------------------------
// Idle
var()	float		PlayIdleTime;		// time that elapses before playing the idle animation
var		float		StartIdleTime;		// time when the player first begins their idle state

//-----------------------------------------------------------------------------
// Melee
var()	float					MeleeRange;
var()	float					MeleeDamage;
var()	class<DamageType>		MeleeDamageType;
var()	class<MaterialTypeInfo>	MeleeHitEffectsByMaterial;
var()	class<StatusEffect>		MeleeStatusEffect;
var()	float					MeleeStatusEffectDuration;
var()	float					MeleeStatusEffectDamagePerSec;
var()	float					BeginMeleeLoopFrame;// value between 0-1

//-----------------------------------------------------------------------------
// Anim 
var		name					LoadAnim;
var		name					FireAnim;
var		name					HolsterAnim;

//-----------------------------------------------------------------------------
// Sound queue info

var()		int					soundQ[5];
var()		int					CurrIndex;

//-----------------------------------------------------------------------------
// Misc
var(Events)	name				FirstTimeSwitchEvent;	// event name to fire off if switching to weapon for the first time
var		EWeaponIndexType		WeaponIndex;			// index ID
var()   float					PlayerSpeedModifier;    // If the player is carrying this weapon, then his overall speed at any point is multiplied by this

var int HudArmsShaderIndex[2];

// Network replication
//
replication
{
	// Variables the server replicates to the client.
	reliable if( bNetOwner && bNetDirty && (Role==ROLE_Authority) )
		AmmoType, Affector, ReloadCount;

	reliable if( Role==ROLE_Authority )
		bForceFire, bForceAltFire, bApplyKick, LastTimeSeconds;

	// Functions the server replicates to the client
	reliable if( Role==ROLE_Authority )
		ClientWeaponEvent, ClientWeaponSet, ClientForceReload, ClientThrowGrenade, ClientReleaseGrenade, ClientFire, ClientAltFire, ClientThink, ClientGotoState, ClientIsInState;

	reliable if( ROLE==ROLE_Authority)
		ClientReset, ClientSwapSecondaryWeapon, Putdown;

	// functions called by client on server
	reliable if( Role<ROLE_Authority )
		ServerFire, ServerStopFire, ServerForceReload, ServerAltFire, ServerThrowGrenade, ServerZoom;	
}

native function PlayOwnedSoundOnQueue(Sound WeaponFire);


simulated function ClientGotoState( optional name NewState, optional name Label )
{
	GotoState( NewState, Label );
}

simulated function bool ClientIsInState( optional name State )
{
	return IsInState( State );
}


function float GetDamageRadius()
{
	return AmmoType.GetDamageRadius();
}

// fixme - make IsFiring() weapon state based
function bool IsFiring()
{
	return (   (Instigator != None) 
			&& (Instigator.Controller != None) 
			&& ((Instigator.Controller.bFire !=0) || (Instigator.Controller.bAltFire != 0)) );

}

simulated event PostNetBeginPlay()
{
	super.PostNetBeginPlay();

	if ( Role == ROLE_Authority )
		return;

	if ( (Instigator == None) || (Instigator.Controller == None) )
		SetHand(0);
	else
		SetHand(Instigator.Controller.Handedness);	

}

//////////////////////////////////////////////////
// Debugging
//////////////////////////////////////////////////

simulated function DisplayDebug(Canvas Canvas, out float YL, out float YPos)
{
	local WeaponAttachment W;

	Super.DisplayDebug(Canvas, YL, YPos);
	
	W = WeaponAttachment(ThirdPersonActor);

	Canvas.SetDrawColor(255,255,0);
	YPos += YL;
	Canvas.SetPos(4,YPos);

	//Canvas.DrawText("Weapon Accuracy: "$WeaponAccuracy$"   Pointing: "$bPointing);
	//YPos += YL;
	//Canvas.SetPos(4,YPos);

	YPos += YL;
	Canvas.SetPos(4,YPos);
	Canvas.DrawText("     3rd Person Offset: "$W.EffectLocationOffset[0]);
	YPos += YL;
	Canvas.SetPos(4,YPos);
	Canvas.DrawText("     1st Person Offset: "$W.EffectLocationOffset[1]);
}


//=============================================================================
// Inventory travelling across servers.

// give the inventory in Pre so we can decided if we need the defaultInventory or not
// in AcceptInventory.
event TravelPreAccept()
{
	local PlayerStart Start;

	// don't travel the secondary weapon if bClearSecondary is set to true in PlayerStart
	if ( (WeaponType == WT_Secondary) && (WeaponIndex != WI_Pistol) && (Level != None) && (Level.Game != None) && Owner != None )
	{
		Start = PlayerStart(Level.Game.FindPlayerStart(Pawn(Owner).Controller));
		if ( (Start != None) && Start.bClearSecondary)
		{
			if (AmmoType != None)
				Pawn(Owner).DeleteInventory(AmmoType);
			return;
		}
	}

	bJustTravelled=true;
	Super.TravelPreAccept();

	if ( Pawn(Owner) == None )
		return;
	if ( ReloadCount == 0 )
		ReloadCount = Default.ReloadCount;
}

event TravelPostAccept()
{
	Super.TravelPostAccept();

	if ( Pawn(Owner) == None )
		return;
	if ( self == Pawn(Owner).Weapon )
	{
		BringUp(self);
	}
	else ClientGotoState('');
	bJustTravelled = false;
}

simulated function Destroyed()
{
	Super.Destroyed();
	if (AmmoType != None)
		AmmoType.Destroy();
	if( (Pawn(Owner)!=None) && (Pawn(Owner).Weapon == self) )
		Pawn(Owner).Weapon = None;
	if (MuzzleFlash != None)
		MuzzleFlash.Destroy();
	if (BreechEffect != None)
		BreechEffect.Destroy();
}

simulated function ClientSwapSecondaryWeapon(Pawn Other, Ammunition Ammo)
{
	//Log("ClientSwapSecondaryWeapon self "$self$" Ammo "$Ammo$" AmmoType "$AmmoType$" PendingWeapon "$Other.PendingWeapon);
	
	if (ROLE == ROLE_Authority)
		return;

	if (Other != None)
		Other.Weapon = None;

	if (AmmoType == None)
		AmmoType = Ammo;

	SetOwner(Other);
}

function ThrowAwayWeapon(Pawn WeaponOwner, Weapon WeaponToToss)
{
    local Vector TossVel;

	if ( (WeaponOwner != None) && (WeaponToToss != None) )
	{
		TossVel = Vector(WeaponOwner.GetViewRotation());
		TossVel = TossVel * ((Velocity Dot TossVel) + 500) + Vect(0,0,200);
		WeaponOwner.TossWeapon(WeaponToToss, TossVel);
		WeaponOwner.DeleteInventory(WeaponToToss.AmmoType);
		WeaponOwner.DeleteInventory(WeaponToToss);
	}
}

function Pickup DropFrom(vector StartLocation, rotator StartRotation)
{
	local Pickup P;

	P = super.DropFrom(StartLocation,StartRotation);
	if ( P != None && PickupAmmoCount == 0 && bCanReload )
		P.FadeOutPickup();
	return P;
}

function SetMesh()
{
	local string meshName;

	if (Default.Mesh == None || string(Default.Mesh.Name) == "")
		return;

	if (AlternateHUDArmsMesh != None)
	{
		LinkMesh(AlternateHUDArmsMesh);
		return;
	}

	meshName = "HudArms.t"$string(Default.Mesh.Name);
	AlternateHUDArmsMesh = Mesh(DynamicLoadObject(meshName, class'Mesh'));

	if (AlternateHUDArmsMesh != None)
		LinkMesh( AlternateHUDArmsMesh );
}

function bool GiveTo(Pawn Other)
{
	local bool good, bSwapWeapon;
	local Weapon WeapToToss;

	if (Other.IsA('MPTrandoshan'))
	{
		SetMesh();
	}
	else if (Mesh != Default.Mesh && Default.Mesh != None)
	{
		LinkMesh(Default.Mesh);
	}

	if ( Other.ShouldTossWeapon(self) )
	{
		WeapToToss = Other.CurrentTossableWeapon;
		if (WeapToToss != None)
		{
			// swap out the current weapon if it's a tossable weapon
			if (WeapToToss == Other.Weapon)
				WeapToToss.PutDown(self);
			ThrowAwayWeapon(Other, WeapToToss);
			bSwapWeapon = true;
		}
	}

	// set the new CurrentTossableWeapon
	if (Level.Game != None && Level.Game.WeaponIsTossable(self) && Level.NetMode == NM_Standalone)
		Other.CurrentTossableWeapon = self;

	if( Super.GiveTo(Other) )
	{	
		// make sure the auto fire is set properly for this weapon - AI uses auto fire, player doesn't always
		if ( bAutoFire && (FireMode != FM_Automatic) && Other.Controller.IsA('PlayerController') )
			bAutoFire = false;

		ClientGotoState('');	//cg moved state change from super GiveTo() to here (Inventory -> Weapon)
		good = true;
	}
	else
	{
		good = false;
	}
			
	bTossedOut = false;
	Instigator = Other;

	// don't give more ammo if this is called during a level transition.
	if (!bJustTravelled)
	{	
		GiveAmmo(Other);
		bJustTravelled = false;
	}

	if ( (Pawn(Owner) != None) && (PlayerController(Pawn(Owner).Controller) != None) )
	{
		if (PlayerController(Pawn(Owner).Controller).FirstTimeSwitch[WeaponIndex] == -1)
			PlayerController(Pawn(Owner).Controller).FirstTimeSwitch[WeaponIndex] = 1;
	}

	if (bSwapWeapon)
		ClientSwapSecondaryWeapon(Other, AmmoType);
  	ClientWeaponSet(true);
	return good;
}

//=============================================================================
// Weapon rendering
// Draw first person view of inventory
simulated event RenderOverlays( canvas Canvas )
{
	local rotator NewRot;
	local vector NewLoc;
	local actor Temp;
	local bool bPlayerOwner;
	local int Hand;
	local  PlayerController PlayerOwner;

	if ( Instigator == None )
		return;

	PlayerOwner = PlayerController(Instigator.Controller);

	if ( PlayerOwner != None )
	{
		bPlayerOwner = true;
		Hand = PlayerOwner.Handedness;
		if (  Hand == 2 )
			return;
	}

	if( Instigator != None && Instigator.Controller != None )
	{
		PlayerController(Instigator.Controller).PlayerCalcView( Temp, NewLoc, NewRot );
		SetLocation(NewLoc);
		SetRotation(NewRot);
	}
	else
	{
		SetLocation( Instigator.Location + Instigator.CalcDrawOffset(self) );
		NewRot = Instigator.GetViewRotation();

		if ( Hand == 0 )
			newRot.Roll = 2 * Default.Rotation.Roll;
		else
			newRot.Roll = Default.Rotation.Roll * Hand;

		setRotation(newRot);
	}
	
	Canvas.DrawActor(self, false, false, DisplayFOV);
}

//////////////////////////////////////////////////
// Weapon //Logic
//////////////////////////////////////////////////

simulated function ClientReset()
{
	GotoState('Idle');
}

simulated exec function SetWeapFOV(float newfov)
{
	if ( Pawn(Owner).Weapon != self)
		return;

	DisplayFOV = newFOV;
}

/* AmmoStatus()
return percent of full ammo (0 to 1 range)
*/
function float AmmoStatus()
{
	return AmmoType.GetAmmoAmount()/AmmoType.MaxAmmo;
}

simulated function bool HasAmmo()
{
	if (AmmoType != None)
		return AmmoType.HasAmmo();
	else
		return false;
}

simulated function float RateSelf()
{
	local float CurrentRating;

    if ( !HasAmmo() )
        CurrentRating = -2;
	else if ( Pawn(Owner).Controller == None )
		return 0;
	else
		CurrentRating = Instigator.Controller.RateWeapon(self);
	return CurrentRating;
}

// Has second lock-on? <- For rocket launchers; sucks, but needs to go here
simulated function bool HasSecondLockOn()
{
	return false;
}

//-------------------------------------------------------

function ClientWeaponEvent(name EventType);

/* HandlePickupQuery()
If picking up another weapon of the same class, add its ammo.
If ammo count was at zero, check if should auto-switch to this weapon.
*/
function bool HandlePickupQuery( Pickup Item )
{
	local int OldAmmo, NewAmmo;
	local Pawn P;

	if (Item == None)
		return false;

	if (Item.InventoryType == Class)
	{
		/* jH: This makes no sense.  The check for bWeaponStay should be in pickup.  Below is already a check for item.inventory == NULL
		if ( Level.Game.bWeaponStay && ((item.inventory == None) || item.inventory.bTossedOut) )
			return true;
		*/
		P = Pawn(Owner);
		if ( AmmoType != None )
		{
			if (AmmoType.AmmoAmount == AmmoType.MaxAmmo)
				return true;
			OldAmmo = AmmoType.GetAmmoAmount();
			if ( Item.Inventory != None )
				NewAmmo = Weapon(Item.Inventory).PickupAmmoCount;
			else
				NewAmmo = class<Weapon>(Item.InventoryType).Default.PickupAmmoCount;
			if ( AmmoType.AddAmmo(NewAmmo) && (OldAmmo == 0) 
				&& (P.Weapon.class != item.InventoryType) )
			{
				ClientWeaponSet(true);
			}

			if (ReloadCount == 0)
			{
				ReloadCount = Default.ReloadCount;
				if (Pawn(Owner).Weapon == self)
					ServerForceReload();
			}
		}
		Item.AnnouncePickup(Pawn(Owner));
		Item.SetRespawn(); 
		return true;
	}
	if ( Inventory == None )
		return false;

	return Inventory.HandlePickupQuery(Item);
}

// set which hand is holding weapon
simulated function setHand(float Hand)
{
	if ( Hand == 2 )
	{
		PlayerViewOffset.Y = 0;
		FireOffset.Y = 0;
		return;
	}

	//LinkMesh(Default.Mesh); //MHi uh, no. Don't need and can't be doin this or it kills the trando meshes.

	if ( Hand == 0 )
	{
		PlayerViewOffset.X = Default.PlayerViewOffset.X * 0.88;
		PlayerViewOffset.Y = -0.2 * Default.PlayerViewOffset.Y;
		PlayerViewOffset.Z = Default.PlayerViewOffset.Z * 1.12;
	}
	else
	{
		PlayerViewOffset.X = Default.PlayerViewOffset.X;
		PlayerViewOffset.Y = Default.PlayerViewOffset.Y * Hand;
		PlayerViewOffset.Z = Default.PlayerViewOffset.Z;
	}
	FireOffset.Y = Default.FireOffset.Y * Hand;
}


function bool UseSquadAmmo( Pawn Other )
{
	local Pawn SquadLeader;
	local Ammunition LeaderAmmoType;

	//Log("Weapon::UseSquadAmmo 1");
	if (AmmoName == None)
		return false;

	SquadLeader = Other.Squad.SquadLeader;

	LeaderAmmoType = Ammunition(SquadLeader.FindInventoryType(AmmoName));
	if (LeaderAmmoType == None)
		return false;

	//Log("Weapon::UseSquadAmmo 2");
	//if the leader doesn't have a full clip, give up
	if (LeaderAmmoType.AmmoAmount < default.ReloadCount)
		return false;
	
	//Log("Weapon::UseSquadAmmo 3");
	AmmoType = Ammunition(Other.FindInventoryType(AmmoName));
	if (AmmoType != None)
	{
		AmmoType.AmmoAmount = default.ReloadCount;
		LeaderAmmoType.DeductAmmoAmount(default.ReloadCount);
	}
	else
	{
		AmmoType = Spawn(AmmoName);	// Create ammo type required
		Other.AddInventory(AmmoType);		// and add to player's inventory
		AmmoType.AmmoAmount = default.ReloadCount;
		LeaderAmmoType.DeductAmmoAmount(default.ReloadCount);
	}
	return true;
}

function GiveAmmo( Pawn Other )
{
	if ( AmmoName == None )
		return;
	AmmoType = Ammunition(Other.FindInventoryType(AmmoName));
	if ( AmmoType != None )
		AmmoType.AddAmmo(PickUpAmmoCount);
	else
	{
		AmmoType = Spawn(AmmoName);	// Create ammo type required
		Other.AddInventory(AmmoType);		// and add to player's inventory
		if (WeaponType == WT_Secondary || Level.NetMode != NM_StandAlone )
			AmmoType.AmmoAmount = PickUpAmmoCount;
		else if (InitAmmoAmount != -1)
			AmmoType.AmmoAmount = InitAmmoAmount;
		else
			AmmoType.AmmoAmount = AmmoType.MaxAmmo;
	}
}	

simulated function HolderDied()
{
	if ( bWeaponZoom && (Pawn(Owner) != None) && PlayerController(Pawn(Owner).Controller) != None  )
	{
		PlayerController(Pawn(Owner).Controller).ResetFOV();
	}
	
	GotoState('');
}

function AdjustWeaponAccuracy()
{
	//normally make no changes to weapon accuracy when firing
}

//////////////////////////////////////////////////
// Pawn Animation
//////////////////////////////////////////////////

//play weapon type anims on 3rd party pawns
simulated function PlayPawnAnimation(EPawnWeaponAnimation whichAnim)
{
	local bool bDoFXNow;

	local WeaponAttachment WA;
	WA = WeaponAttachment(ThirdPersonActor);
	if (WA == None) //need the weapon attachment to do anything, so if none, got to bail
	{
		return;
	}

	switch (whichAnim)
	{
		case PWA_Fire:
			WA.IncrementFlashCount(FireMode == FM_Automatic);
			break;
		case PWA_Holster:
//			WA.IncrementHolsterCount();
			WA.bChangeHolsterCount = true;   // delay incrementing holstercount.  calling this rep'd function now causes a channel corruption
			                                 // that causes the no-shooting bug.

			break;
		case PWA_Load:
			WA.IncrementLoadCount();
			break;
		case PWA_Melee:
			WA.IncrementMeleeCount();
			break;
		case PWA_Throw:
			WA.IncrementThrowCount();
			break;
		case PWA_Reload:
			WA.IncrementReloadCount();
			break;
		case PWA_Idle:
			//TODO
			//WA.IncrementIdleCount();
			break;
		default:
			return;
	}

	//listen servers need to call ThirdPersonEffects now. Clients call it from PostNetReceive and dedicated servers don't care.
	//note we only call it in the case of 3rd person instigators, of course - and also for splitscreen
	bDoFXNow = (Level.NetMode == NM_ListenServer && Instigator != None && !Instigator.IsLocallyControlled());
	bDoFXNow = bDoFXNow || Level.IsSplitScreen(); //splitscreen; TODO - need to do the FX now if we're in splitscreen

	//in cases that we're in here in a SP game, go ahead and do the FX now, since there's no other chance
	if (whichAnim == PWA_Fire)
		bDoFXNow = bDoFXNow || (Level.NetMode == NM_Standalone);

	if (bDoFXNow)
		WA.ThirdPersonEffects();
}

simulated function bool IsIdle()
{
	return false;
}

// play the up, down, left, right anims
simulated function PlayTurnAnim(float deltaYaw, float deltaPitch)
{
	if (deltaYaw == 0 && deltaPitch == 0)
	{
		bPlayingIdle = false;
		return;
	}
	else
	{
		bPlayingIdle = true;		
		if ( deltaPitch > 0 && HasAnim('Up') )
			PlayAnim('Up');
		else if ( deltaPitch < 0 && HasAnim('Down') )
			PlayAnim('Down');
		if ( deltaYaw < 0 && HasAnim('Left') )
			PlayAnim('Left');
		else if ( deltaYaw > 0 && HasAnim('Right') )
			PlayAnim('Right');		
	}
}

function CheckAnimating();
simulated event TurretFire();					// spawn the projectiles from the corresponding turret muzzle

//**************************************************************************************
// ADDITIONAL ATTACHMENT EFFECTs
//**************************************************************************************
simulated function SpawnBreechEffect()
{
	local vector BoneLoc;
	local name BoneName;
	local Pawn P;

	if ( (Level.NetMode != NM_DedicatedServer) && (BreechEffectClass != None) && (Pawn(Owner) != None) )
	{
		if ( Pawn(Owner).Controller.IsA('PlayerController') )
		{
			BoneLoc = GetBoneLocation(BreechAttachBone);
			BreechEffect = Spawn(BreechEffectClass,,,BoneLoc);
			AttachToBone(BreechEffect, BreechAttachBone);
			BreechEffect.SetRelativeLocation(vect(0,0,0)); 
		}
		else
		{
			P = Instigator;
			BoneName = P.GetWeaponBoneFor(P.Weapon);
			BoneLoc = P.GetBoneLocation(BoneName);
			BreechEffect = Spawn(BreechEffectClass,,,BoneLoc);
			P.AttachToBone(BreechEffect, BoneName);	
			BreechEffect.SetRelativeLocation(BreechOffset); 
		}
		BreechEffect.LifeSpan = 0;
		BreechEffect.SetRelativeRotation(rot(0,0,0));
	}
}

simulated event LoadAttachment()
{
	if (BreechEffect != None)
		BreechEffect.bHidden = false;
	
	ManageAmmoEffect(true);
}

simulated event HolsterAttachment()
{
	if (BreechEffect != None)
		BreechEffect.bHidden = true;
}

// manages the spawning and loading of shells into some weapons (shotguns, anti-armor, etc.)
simulated function ManageAmmoEffect(bool bLoad)
{
	local rotator NewAmmoAngle;

	if (!bHasAmmoNeedle)
		return;

	NewAmmoAngle = GetBoneRotation('needle', SPACE_Mesh);

	if (AnglePerShot == 0)
	{
		FullClipAngle = NewAmmoAngle.Roll - NeedleAngle;
		EmptyClipAngle = NewAmmoAngle.Roll + NeedleAngle;
		AnglePerShot = (EmptyClipAngle - FullClipAngle) / default.ReloadCount;
	}
	NewAmmoAngle.Roll = EmptyClipAngle - ReloadCount * AnglePerShot;
	SetBoneRotation('needle', NewAmmoAngle, SPACE_Mesh);
}

simulated function DetachFromPawn(Pawn P)
{
	super.DetachFromPawn(P);
	if (BreechEffect != None)
	{
		BreechEffect.Destroy();
		BreechEffect = None;
	}
}

//**************************************************************************************
// TARGETING
//**************************************************************************************
event bool IsTargeting()
{
	return false;
}

event bool IsTargetLocked()
{
	return false;
}

event float GetTargetLockTime()
{
	return 0.0f;
}

event float GetElapsedTargetLockTime()
{
	return 0.0f;
}

event texture GetLockReticle()
{
	return None;
}

event bool IsSecondaryTargetLocked()
{
	return false;
}

event float GetSecondaryTargetLockTime()
{
	return 0.0f;
}

event float GetElapsedSecondaryTargetLockTime()
{
	return 0.0f;
}

//**************************************************************************************
// FIRE
//**************************************************************************************
simulated function Fire( float Value )
{
	if ( !AmmoType.HasAmmo() )	
	{
		if (WeaponEmptySound != None)
			PlaySound(WeaponEmptySound);
		return;
	}

	if( AmmoType.HasAmmo() && ( Level.TimeSeconds >= NextShotTime ) && ((ReloadCount != 0) || !bCanReload) )
	{		
		ClientFire();
		ServerFire();
	}
}

event ServerFire()
{	
	local Pawn PawnOwner;

	if ( bCanReload && (ReloadCount == 0) )
		return;

	PawnOwner = Pawn(Owner);

	if (FireMode == FM_Automatic || !PawnOwner.IsHumanControlled() )
		bAutoFire = true;

	if( AmmoType == None )
		GiveAmmo(PawnOwner);

	//Log(""$self$": Level Time "$Level.TimeSeconds$" NextShotTime "$NextShotTime);

	if( AmmoType.HasAmmo() && ( Level.TimeSeconds >= NextShotTime ) )
	{	
		if (!IsInState('NormalFire'))
			GotoState('NormalFire');
		CalculateFire();
		if (WeaponType != WT_Turret && WeaponType != WT_Marker && WeaponType != WT_Heal)
			ReloadCount--;
		if( PawnOwner.Controller != None )
			PawnOwner.Controller.WeaponFired();
		if (ROLE == ROLE_Authority) //clients will call this from the replicated to client ClientFire func
			PlayFiring();
		if ( ((Level.NetMode != NM_Standalone) && (!Level.IsSplitScreen())) || Pawn(Owner).Controller.IsA('AIController') )
		{
			if (bAutoFire && FireSound != None && FireSound.Looping && !WeaponAttachment(ThirdPersonActor).bPlayingFireSound)
			{
				WeaponAttachment(ThirdPersonActor).PlayOwnedSound(FireSound);
				WeaponAttachment(ThirdPersonActor).bPlayingFireSound = true;
				WeaponAttachment(ThirdPersonActor).LastLoopingSound = FireSound;
			}
			else if (FireSound != None && (!bAutoFire || !FireSound.Looping))
			{
				PlayOwnedSoundOnQueue(FireSound);
			}
		}
	}
}

function CalculateFire()
{
	local vector Start,X,Y,Z;
	local rotator AdjustedAim;
	local int ShouldFire;

	ShouldFire = 1;
	AdjustWeaponAccuracy();		
	GetAxes(Instigator.GetViewRotation(),X,Y,Z);
	Start = GetFireStart(X,Y,Z);
	AdjustedAim = Instigator.AdjustAim(AmmoType, Start, AimError, ShouldFire);

	if( ShouldFire != 0 )
		AmmoType.DoFire( Start, AdjustedAim );	
}

simulated function ClientFire()
{
	local PlayerController P;
	local vector start,X,Y,Z;
	local rotator dir;
	local int ShouldFire;

	if (Level.NetMode == NM_DedicatedServer) //func is replicated server->client, so shouldn't be in here if server only
		return;

	if (FireMode == FM_Automatic || !Pawn(Owner).Controller.IsA('PlayerController') )
		bAutoFire = true;

	P = PlayerController(Instigator.Controller);
	if (P != None)
	{
		if (Level.NetMode == NM_Client) //server already called this "server side"
			PlayFiring();

		// HUD shake
		if (ShakeTime > 0)
			P.ShakeView(0.0, ShakeTime - 0.1, 0.1, ShakeVert.X, ShakeVert.Y, ShakeVert.Z, 0, 0, 1);

		// Recoil
		if (!bApplyKick)
		{
			InitRotation = P.Rotation;
			InitRotation = PlayerController(Instigator.Controller).Rotation;
			bApplyKick = true;
		}
		WeaponRecoil();
		CreateMuzzleEffect();
	}

	// If this weapon is instant hit and doesn't have a tracer effect play the Tracer Fire Client Side.
	if (( AmmoType != None ) && ( AmmoType.bInstantHit == true ) && ( AmmoType.TracerEffect == None ))
	{
		ShouldFire = 1;
		GetAxes(Instigator.GetViewRotation(),X,Y,Z);
		start = GetFireStart(X,Y,Z);
		dir = Instigator.AdjustAim(AmmoType, Start, AimError, ShouldFire);
		// last element indicates that this is pure visual, don't replicate the tracer
		AmmoType.CreateTracerFire(start,dir,false);
	}

	if (!Pawn(Owner).Controller.IsA('AIController'))
	{
		if (bAutoFire && FireSound != None && FireSound.Looping && !WeaponAttachment(ThirdPersonActor).bPlayingFireSound)
		{
			WeaponAttachment(ThirdPersonActor).PlayOwnedSound(FireSound);
			WeaponAttachment(ThirdPersonActor).bPlayingFireSound = true;
			WeaponAttachment(ThirdPersonActor).LastLoopingSound = FireSound;
		}
		else if (FireSound != None && (!bAutoFire || !FireSound.Looping))
		{
			PlayOwnedSoundOnQueue(FireSound);
		}
	}
	
	if ( Affector != None )
		Affector.FireEffect();
	if (!IsInState('NormalFire'))
		GotoState('NormalFire');
	ManageAmmoEffect(false);
}

simulated function CreateMuzzleEffect()
{
	local int i;

	// spawn it once
	if ( MuzzleFlash == None )
		SpawnMuzzleFlash();
	// reset it
	else if (!bLoopMuzzleFX)
	{
		// if for whatever reason the emitters get destroyed, destroy it and respawn a new one
		if (MuzzleFlash.Emitters.Length == 0)
		{
			MuzzleFlash.Destroy();
			SpawnMuzzleFlash();
		}
		else
		{
			MuzzleFlash.LightType = LT_Steady;
			for (i = 0; i < MuzzleFlash.Emitters.Length; i++)
				MuzzleFlash.Emitters[i].Trigger();
		}
	}
	else
	{
		MuzzleFlash.SetRelativeLocation(vect(0,0,0)); 
		MuzzleFlash.SetRelativeRotation(rot(0,0,0));
	}
}

simulated function SpawnMuzzleFlash()
{
	local vector Start;
	local int i;

	if ( (MuzzleClass != None) && (MuzzleBone != '') )
	{
		Start = GetBoneLocation(MuzzleBone);
		MuzzleFlash = Spawn(MuzzleClass,,, Start);
		AttachToBone(MuzzleFlash, MuzzleBone);	
		MuzzleFlash.SetRelativeLocation(vect(0,0,0)); 
		MuzzleFlash.SetRelativeRotation(rot(0,0,0));			

		// "just in case" property settings
		MuzzleFlash.AutoDestroy = false;
		MuzzleFlash.LightType = LT_None; 
		for (i = 0; i < MuzzleFlash.Emitters.Length; i++)
		{				
			MuzzleFlash.Emitters[i].TriggerDisabled = false;
			MuzzleFlash.Emitters[i].ResetOnTrigger = true;
			MuzzleFlash.Emitters[i].AutoDestroy = false;
		}
	}
}

simulated function PlayFiring()
{
	NextShotTime = Level.TimeSeconds + ShotDelay;
	if (bWeaponZoom && HasAnim('AimFire'))
		PlayAnim('AimFire',,,,1);
	else
		PlayAnim(FireAnim,,,,1); //play the fire anim on me, the WEAPON

	PlayPawnAnimation(PWA_Fire);
}

simulated function bool RepeatFire()
{
	return bRapidFire;
}

simulated event StopFire( float Value )
{
	if ( FireMode == FM_Automatic && Instigator != None && !Instigator.IsLocallyControlled() )
		Instigator.PawnStopFiring();

	if (bAutoFire)
	{
		bAutoFire = false;

		StopSound(ThirdPersonActor, FireSound, 0.1);
		//PlayOwnedSound(FireEndSound); //MHi - when we get one

		ServerStopFire();
		ClientThink();					//don't get stuck in the NormalFire state
	}

	if ( bLoopMuzzleFX && MuzzleFlash != None )
	{
		MuzzleFlash.FadeOut();
		MuzzleFlash = None;
	}
}

event ServerStopFire()
{
	bAutoFire = false;
	if (WeaponAttachment(ThirdPersonActor) != None)
	{
		WeaponAttachment(ThirdPersonActor).bPlayingFireSound = false;
		WeaponAttachment(ThirdPersonActor).IncrementStopFireCount();
		
		//if ( bLoopMuzzleFX && WeaponAttachment(ThirdPersonActor).MuzzleFlash != None )
		WeaponAttachment(ThirdPersonActor).StopMuzzleFlash();
	}

	if ((Level.NetMode == NM_ListenServer || Level.NetMode == NM_StandAlone) && Instigator != None && !Instigator.IsLocallyControlled())
	{		
		StopSound(ThirdPersonActor, FireSound, 0.1);
		if ( FireMode == FM_Automatic )
			Instigator.PawnStopFiring();
	}
	//NextShotTime = Level.TimeSeconds + ShotDelay;
}

simulated event vector GetFireStart(vector X, vector Y, vector Z)
{	
	local Pawn P;
	local vector BoneLocation;
	local rotator BoneOrientation;
	local name BoneName;
	local vector Start, Offset;	

	P  = Pawn(Owner);
	if( P.IsHumanControlled() )
	{
		Offset = FPFireOffset;
		if (bWeaponZoom)
			Offset.Y = 0;
		Start = P.Location + P.EyePosition() + (X*Offset.X) + (Y*Offset.Y) + (Z*Offset.Z);		
	}
	else
	{
		BoneName = P.GetWeaponBoneFor(P.Weapon);
		BoneLocation = P.GetBoneLocation( BoneName );
		BoneOrientation = P.GetBoneRotation( BoneName );
		Start = BoneLocation;
		if( ThirdPersonActor != None )
			Start += ( WeaponAttachment(ThirdPersonActor).MuzzleOffset >> BoneOrientation );
	}
	return Start;
}
	
function FireWeapon();				// called from an anim notify
//**************************************************************************************
// IDLE
//**************************************************************************************
simulated function PlayIdleAnim()
{	
	local float	VelocitySq;

	VelocitySq = VSizeSq(Pawn(Owner).Velocity);

	// if we're not moving and we haven't marked the begin idle time yet, mark it now
	if ( !bWeaponZoom && VelocitySq <= 0.0 )
	{
		if (StartIdleTime == -1)
			StartIdleTime = Level.TimeSeconds;

		if ( (Level.TimeSeconds - StartIdleTime) >= PlayIdleTime )
		{			
			PlayAnim('Idle');		// will get more idle anims and randomize later
			PlayPawnAnimation(PWA_Idle);
			StartIdleTime = Level.TimeSeconds;
			bPlayingIdle = true;
			return;
		}
	}
	else
		StartIdleTime = -1;

	if ( (VelocitySq < 51076) && (VelocitySq >= 0.04) )		// 226*226 = 51076; 0.2*0.2 = 0.04
	{
		if (bWeaponZoom && HasAnim('Aim'))
			PlayAnim('Aim');
		else
			PlayAnim('Walk');
	}
	else if ( VelocitySq >= 51076 )
	{
		if (bWeaponZoom && HasAnim('Aim'))
			PlayAnim('Aim');
		else
			PlayAnim('Run');
	}
	else if (!bPlayingIdle) /*( (VSize(Instigator.Velocity) < 0.2) && !IsAnimating() )*/
	{		
		if (bWeaponZoom && HasAnim('AimBreathe'))
			PlayAnim('AimBreathe');
		else
			PlayAnim('ActionBreathe');		
	}
}
	
//**************************************************************************************
// RELOAD
//**************************************************************************************

/* Force reloading even though clip isn't empty.  Called by player controller exec function,
and implemented in idle state */
simulated event ForceReload()
{
	// full clip - no need to reload
	// don't reload if it's a charge weapon
	// 12May2004 JAH - Removed the check on reload count is 1 caused AI not to reload properly
	if ( (ReloadCount == Default.ReloadCount) || !AmmoType.HasAmmo() || (ReloadCount == AmmoType.AmmoAmount) || !bCanReload )
	{
		GotoState('Idle');
		return;
	}

	if ( HasAnim('Reload') || !Instigator.IsHumanControlled() )
		ServerForceReload();
}

event ServerForceReload()
{
	bForceReload = true;

	if ( AmmoType.HasAmmo() && !IsInState('Reloading') )
	{		
		GotoState('Reloading');
		ClientForceReload();
	}
}

simulated function ClientForceReload()
{
	if (ROLE == ROLE_Authority)
		return;
	bForceReload = true;
	GotoState('Reloading');

}

simulated function PlayReloading()
{
	PlayAnim('Reload');

	PlayPawnAnimation(PWA_Reload);
}

simulated function bool NeedsToReload()
{
	local bool val;		

	val = (( bForceReload || (Default.ReloadCount > 0 && ReloadCount <= 0) ) && AmmoType.HasAmmo() );
	
	if( val && Instigator != None && !Instigator.IsHumanControlled() )
	{
		Instigator.PostStimulusToIndividual(ST_Reloading, Instigator.Controller);		
		return false;
	}
	else
		return val;
}
	
//**************************************************************************************
// GRENADE
//**************************************************************************************
simulated function ThrowGrenade( float Value )
{
	if ( (Pawn(Owner).CurrentGrenade == None) || (Pawn(Owner).CurrentGrenade.AmmoType == None) || (bWeaponZoom && !bCanThrowGrenadesWhenZoomed))
		return;	

	if ( HasAnim('ThrowGrenade') || Instigator.IsA('AIController') )
	{
		if ( Pawn(Owner).CurrentGrenade.AmmoType.HasAmmo() || Pawn(Owner).CurrentGrenade.ObtainAmmo() )
		{
			ServerThrowGrenade();
			ClientThrowGrenade();
		}
		else
			PlaySound(Pawn(Owner).CurrentGrenade.GrenadeEmptySound);
	}

}

event ServerThrowGrenade()
{
	if ( Pawn(Owner).CurrentGrenade.AmmoType.HasAmmo() )
		GotoState('GrenadeThrow');
}

simulated function ClientThrowGrenade()
{	
	if ( ROLE == ROLE_Authority )
		return;
    GotoState('GrenadeThrow');
}

simulated function PlayThrowingGrenade()
{
	PlayAnim('ThrowGrenade'); //play the throw anim on me, the WEAPON
	PlayPawnAnimation(PWA_Throw);
}

// callback spawn grenade event
event ReleaseGrenade()
{
	local vector Start,X,Y,Z;
	local rotator AdjustedAim;

	bCanSwitchGrenade = true;
	GetAxes(Instigator.GetViewRotation(),X,Y,Z);
	Start = GetGrenadeStart(X,Y,Z);
	AdjustedAim = Instigator.AdjustAim(Pawn(Owner).CurrentGrenade.AmmoType, Start, AimError);			
	Pawn(Owner).CurrentGrenade.AmmoType.DoFire( Start, AdjustedAim );	
	ClientReleaseGrenade();
}

simulated function ClientReleaseGrenade()
{
	bCanSwitchGrenade = true;
}

function vector GetGrenadeStart(vector X, vector Y, vector Z)
{	
	local Pawn P;	
	local vector Start;

	P  = Pawn(Owner);

	if( P.Controller.IsA('PlayerController') )
		Start = P.Location + P.EyePosition() + (X*FPGrenadeOffset.X) + (Y*FPGrenadeOffset.Y) + (Z*FPGrenadeOffset.Z);
	else
	{
		Start = Instigator.GetBoneLocation( P.GetWeaponBoneFor( P.Weapon ) );
	}
	return Start;
}
	
//**************************************************************************************
// MELEE (ALT FIRE)
//**************************************************************************************
simulated function AltFire( float Value )
{
	if ( Owner != None && Pawn(Owner).Health <= 0 )
		return;
	if (IsInState('Melee'))
	{
		bLoopMelee = true;
		ServerAltFire();
	}
	else if ( (!bWeaponZoom || bZoomedAltFireCapable) && (HasAnim('MeleeAttack') || Instigator.IsA('AIController') ) )
	{	
		ClientAltFire();
		ServerAltFire();
	}
}

function ServerAltFire()
{	
	if (IsInState('Melee'))
		bLoopMelee = true;
	else
		GotoState('Melee');
	//ClientAltFire();
}

simulated function ClientAltFire()
{
	if (ROLE < ROLE_Authority)
		GotoState('Melee');
}

simulated function PlayMelee()
{
	local name LoopAnimName;

	//play the melee animation on me, the WEAPON
	if (bLoopMelee)		
	{
		if (HasAnim('MeleeAttackLoop'))
			LoopAnimName = 'MeleeAttackLoop';
		else
			LoopAnimName = 'MeleeAttack';
		PlayAnim(LoopAnimName,,,BeginMeleeLoopFrame);
		bLoopMelee = false;
	}
	else
		PlayAnim('MeleeAttack');

	PlayPawnAnimation(PWA_Melee);
}

simulated event DoMeleeAttack()
{
	local Actor Other;
	local Pawn PawnOwner;
	local vector Start;
	local vector TraceEnd, HitLocation, HitNormal;
	local EMaterialType HitMaterialType;	
	local name HitBone;
	local int ShouldFire;
	local rotator AdjustedAim;

	if (Pawn(Owner) != None)
		PawnOwner = Pawn(Owner);
	else
		return;

	Start = PawnOwner.Location + PawnOwner.EyePosition();	
	AdjustedAim = PawnOwner.AdjustAim(None, Start, 0, ShouldFire);
	TraceEnd = Start + Vector(AdjustedAim) * MeleeRange;
	//TraceEnd = Start + (Vector(PlayerController(Instigator.Controller).Rotation) * MeleeRange);
	Other = Trace(HitLocation,HitNormal,TraceEnd,Start,true, , , HitBone, HitMaterialType);

	if (Other != None)
	{
		if ( MeleeHitEffectsByMaterial != None && Level.NetMode != NM_DedicatedServer )		
			MeleeHitEffectsByMaterial.Static.SpawnEffectsFor(self, HitMaterialType, HitLocation, HitNormal);

		if( MeleeStatusEffect != None && Other.IsA('Pawn') )
			MeleeStatusEffect.Static.AddStatusEffectTo( Other, Instigator, MeleeStatusEffectDuration, MeleeStatusEffectDamagePerSec );

		Other.TakeDamage( MeleeDamage, Pawn(Owner), HitLocation, -HitNormal * MeleeDamageType.default.KDamageImpulse, MeleeDamageType, HitBone);
	}
}

//**************************************************************************************
// WEAPON CHANGE
//**************************************************************************************
simulated function int GetInventoryGroup()
{
	return Level.GRI.WeaponInventoryGroup(self);
}

// Change weapon to that specificed by F matching inventory weapon's Inventory Group.
simulated function Weapon WeaponChange( byte F, bool bSilent )
{	
	local Weapon newWeapon;

	//Log("WeaponChange F "$F$" Level.GRI.WeaponInventoryGroup(self) "$Level.GRI.WeaponInventoryGroup(self)$" self "$self);
	
	if ( GetInventoryGroup() == F )
	{
		if ( !AmmoType.HasAmmo() && !ObtainAmmo() && Level.NetMode == NM_StandAlone )
		{
			if ( Inventory == None )
				newWeapon = None;
			else
				newWeapon = Inventory.WeaponChange(F, bSilent);
			if ( !bSilent && (newWeapon == None) && (Instigator != None) && Instigator.IsHumanControlled() )
				Instigator.ClientMessage( ItemName$MessageNoAmmo );
			return newWeapon;
		}
		else
		{
			if (Instigator != None && Instigator.Weapon != self)
				return self;
			else 
			{
				newWeapon = Inventory.WeaponChange(F, bSilent);
				if (newWeapon == None)
					return self;
				else
					return newWeapon;
			}
		}
	}
	else if ( Inventory == None )
		return None;
	else
		return Inventory.WeaponChange(F, bSilent);
}

// Change weapon to first weapon that matches WeaponClass
simulated function Weapon WeaponChangeClass( class<Weapon> WeaponClass )
{	
	local Weapon newWeapon;

	//Log("WeaponChange F "$F$" InventoryGroup "$InventoryGroup$" self "$self);
	
	if ( IsA(WeaponClass.Name) )
	{
		if ( !AmmoType.HasAmmo() )
		{
			if ( Inventory == None )
				newWeapon = None;
			else
				newWeapon = Inventory.WeaponChangeClass(WeaponClass);
			return newWeapon;
		}
		else
			return self;
	}
	else if ( Inventory == None )
		return None;
	else
		return Inventory.WeaponChangeClass(WeaponClass);
}

event bool CanSwitchWeapon()
{
	return true;
}

simulated function bool PutDown(weapon NextWeapon)
{
	//Log("Global PutDown");
	bChangeWeapon = true;
	NewWeapon = NextWeapon;
	GotoState('DownWeapon');
	return true;
}

simulated function TweenDown()
{
	// If zoomed in, zoom out

	if ( bWeaponZoom && (Pawn(Owner) != None) && PlayerController(Pawn(Owner).Controller) != None  )
	{
		PlayerController(Pawn(Owner).Controller).EndZoom();
	}

	// add on the primary weapon attachments
	if ( (Level.NetMode == NM_Standalone) && (WeaponType == WT_Primary) && (NewWeapon != None) && (NewWeapon.WeaponType == WT_Primary) )
		PlayAnim('AttachHolster');
	// switch to the secondary weapon
	else
		PlayAnim(HolsterAnim);

	PlayPawnAnimation(PWA_Holster);
	PlaySound(HolsterSound);
}

simulated function BringUp(weapon PrevWeapon)
{
	local PlayerController PC;

	if (Instigator == None && Pawn(Owner) != None)
		Instigator = Pawn(Owner);

	if ( PrevWeapon != None )
	{
		// special case for the elite beam since it gets tossed when it runs out of ammo
		if ( (PrevWeapon.WeaponIndex != WI_EliteBeam) || ((PrevWeapon.WeaponIndex == WI_EliteBeam) && PrevWeapon.HasAmmo()) )
		{
			if ( (PrevWeapon.WeaponType == WT_Primary) || (PrevWeapon.Weapontype == WT_Secondary) )
				Pawn(Owner).OldWeapon = PrevWeapon;
			else
				Pawn(Owner).OldWeapon = None;
		}
	}
	else
		Pawn(Owner).OldWeapon = None;
	
	if ( Instigator.IsHumanControlled() )
	{
		PC = PlayerController(Instigator.Controller);
		SetHand(PC.Handedness);
		PC.EndZoom();

		// first time bringing up this weapons?
		if (PC.FirstTimeSwitch[WeaponIndex] == 1)
		{
			if (FirstTimeSwitchEvent != '')
				TriggerEvent(FirstTimeSwitchEvent, self, Pawn(Owner));
			PC.FirstTimeSwitch[WeaponIndex] = 0;
		}
	}	
	if (Level.Game != None && Level.Game.WeaponIsTossable(self))
		Pawn(Owner).CurrentTossableWeapon = self;
	bWeaponUp = false;
	GotoState('Active');
}

simulated function PlaySelect()
{
	if ( (Level.NetMode == NM_Standalone) && (WeaponType == WT_Primary) && (Pawn(Owner).OldWeapon != None) && (Pawn(Owner).OldWeapon.WeaponType == WT_Primary) )
		PlayAnim('AttachLoad',,,0);			// tweening messes up transition between secondary and primary weapons
	else
		PlayAnim(LoadAnim,,,0);				// tweening messes up transition between secondary and primary weapons

	PlayPawnAnimation(PWA_Load);
}

// Find the previous weapon (using the Inventory group)
simulated function Weapon PrevWeapon(Weapon CurrentChoice, Weapon CurrentWeapon)
{
	local int InvGrp;
	local int CurrentChoiceInvGrp;
	local int CurrentWeaponInvGrp;

	if ( AmmoType.HasAmmo() && (WeaponType == WT_Primary || WeaponType == WT_Secondary) )
	{
		InvGrp = GetInventoryGroup();
		if ( CurrentChoice != None && CurrentWeapon != None )
		{
			CurrentChoiceInvGrp = CurrentChoice.GetInventoryGroup();
			CurrentWeaponInvGrp = CurrentWeapon.GetInventoryGroup();
		}

		if ( (CurrentChoice == None) )
		{
			if ( CurrentWeapon != self )
				CurrentChoice = self;
		}
		else if ( InvGrp > CurrentChoiceInvGrp )
		{
			if ( (InvGrp < CurrentWeaponInvGrp)
				|| (CurrentChoiceInvGrp > CurrentWeaponInvGrp) )
				CurrentChoice = self;
		}
		else if ( (CurrentChoiceInvGrp > CurrentWeaponInvGrp)
				&& (InvGrp < CurrentWeaponInvGrp) )
			CurrentChoice = self;
	}
	if ( Inventory == None )
	{
		if ( WeaponType == WT_Primary || WeaponType == WT_Secondary )
			return CurrentChoice;
		else
			return CurrentWeapon;
	}
	else
		return Inventory.PrevWeapon(CurrentChoice,CurrentWeapon);
}

simulated function Weapon NextWeapon(Weapon CurrentChoice, Weapon CurrentWeapon)
{
	local int InvGrp;
	local int CurrentChoiceInvGrp;
	local int CurrentWeaponInvGrp;

	if ( AmmoType.HasAmmo() && (WeaponType == WT_Primary || WeaponType == WT_Secondary) )
	{
		InvGrp = GetInventoryGroup();
		if ( CurrentChoice != None && CurrentWeapon != None )
		{
			CurrentChoiceInvGrp = CurrentChoice.GetInventoryGroup();
			CurrentWeaponInvGrp = CurrentWeapon.GetInventoryGroup();
		}

		if ( (CurrentChoice == None) )
		{
			if ( CurrentWeapon != self )
				CurrentChoice = self;
		}

		else if ( InvGrp < CurrentChoiceInvGrp )
		{
			if ( (InvGrp > CurrentWeaponInvGrp)
				|| (CurrentChoiceInvGrp < CurrentWeaponInvGrp) )
				CurrentChoice = self;
		}
		else if ( (CurrentChoiceInvGrp < CurrentWeaponInvGrp)
				&& (InvGrp > CurrentWeaponInvGrp) )
			CurrentChoice = self;
	}
	if ( Inventory == None )
	{
		if ( WeaponType == WT_Primary || WeaponType == WT_Secondary )
			return CurrentChoice;
		else
			return CurrentWeapon;
	}
	else
		return Inventory.NextWeapon(CurrentChoice,CurrentWeapon);
}

function SwitchToWeaponWithAmmo()
{
	if (Level.NetMode == NM_StandAlone)
	{
		if (OutOfAmmoEvent != '')
			TriggerEvent(OutOfAmmoEvent, self, Pawn(Owner));
		// if local player, switch weapon
		Instigator.Controller.SwitchToBestWeapon();
		if ( bChangeWeapon )
		{
			GotoState('DownWeapon');
			return;
		}
		else
			GotoState('Idle');
	}
	else
		GotoState('Idle');
}

// Return the switch priority of the weapon (normally AutoSwitchPriority, but may be
// modified by environment (or by other factors for bots)
simulated event float SwitchPriority() 
{
	if ( !Pawn(Owner).IsHumanControlled() )
		return RateSelf();
	else if ( (AmmoType != None) && !AmmoType.HasAmmo() )
	{
		if ( Pawn(Owner).Weapon == self )
			return -0.5;
		else
			return -1;
	}
	else 
		return default.AutoSwitchPriority;
}

// Compare self to current weapon.  If better than current weapon, then switch
simulated function ClientWeaponSet(bool bOptionalSet)
{
	if ( Pawn(Owner) != None )
		Pawn(Owner).SetHudArmTexture(self);

	Instigator = Pawn(Owner); //weapon's instigator isn't replicated to client
	if ( Instigator == None )
	{
		GotoState('PendingClientWeaponSet');
		return;
	}
	else if ( IsInState('PendingClientWeaponSet') )
		GotoState('');

	// we always want a non-throwable weapon in the player's hand	
	if ( (WeaponType == WT_Thrown) || (WeaponType == WT_Marker) || (Instigator.Weapon == self) )
		return;

	if ( (WeaponType == WT_Heal) && (PlayerController(Instigator.Controller).TargetType != TGT_FriendToHeal) )
		return;

	if ( Instigator.Weapon == None )
	{
		Instigator.PendingWeapon = self;
		Instigator.ChangedWeapon();
		return;	
	}

	if ( bOptionalSet && (Instigator.IsHumanControlled() && PlayerController(Instigator.Controller).bNeverSwitchOnPickup) )
		return;

	if ( (Instigator.Weapon.SwitchPriority() < SwitchPriority()) )
	{
		//W = Instigator.PendingWeapon;
		Instigator.PendingWeapon = self;
		GotoState('');

		if ( !Instigator.Weapon.PutDown(self) )
			Instigator.PendingWeapon = self;
		return;
	}
	GotoState('');
}

simulated function Weapon RecommendWeapon( out float rating )
{
	local Weapon Recommended;
	local float oldRating;

	if ( Instigator != None && Instigator.IsHumanControlled() )
	{
		if ( (self.WeaponType == WT_Primary) || (self.WeaponType == WT_Secondary) )
			rating = SwitchPriority();
		else
			rating = -99;
	}
	else
	{
		rating = RateSelf();
		if ( (Instigator != None) && (self == Instigator.Weapon) && (Instigator.Controller.Enemy != None) 
			&& AmmoType.HasAmmo() )
			rating += 0.21; // tend to stick with same weapon
			rating += Instigator.Controller.WeaponPreference(self);
	}
	if ( inventory != None )
	{
		Recommended = inventory.RecommendWeapon(oldRating);
		if ( (Recommended != None) && (oldRating > rating) )
		{
			rating = oldRating;
			return Recommended;
		}
	}
	return self;
}
	
//**************************************************************************************
// ZOOM
//**************************************************************************************

simulated function Zoom(bool ZoomIn)
{
	// only want clients and human players to play zoom anim
	if ( !Instigator.IsHumanControlled() )
		return;
	ServerZoom(ZoomIn);
	SetZoom(ZoomIn);
	PlayIdleAnim();
}

simulated function SetZoom(bool ZoomIn)
{
	if (!ZoomIn)		
		CurrentZoomFOVIndex = 0;
	bWeaponZoom = ZoomIn;
}

function ServerZoom(bool ZoomIn)
{
	SetZoom(ZoomIn);
}
	
simulated function float GetNextZoomFOV()
{
	CurrentZoomFOVIndex = (CurrentZoomFOVIndex + 1) % NumZoomFOVs;
	return ZoomFOVs[CurrentZoomFOVIndex];
}


//**************************************************************************************
// RECOIL
//**************************************************************************************
simulated function WeaponRecoil()
{
	local PlayerController P;
	local Rotator RandomKick;
	
	if ( Instigator != None && Instigator.IsLocallyControlled() )
	{
		P = PlayerController(Instigator.Controller);
		if ( P != None )
		{
			if (bRecoilShake)
				RandomKick.Pitch = 2 * WeaponKick.Pitch * (FRand()-0.5f);
			else
				RandomKick.Pitch = WeaponKick.Pitch;
			RandomKick.Yaw = 2 * WeaponKick.Yaw * (FRand()-0.5f);		// randomize the left/right recoil
			RecoilVelocity += RandomKick;
		}
	}
}
	
//**************************************************************************************
// MISC
//**************************************************************************************

function bool ObtainAmmo()
{
	if (Instigator.IsHumanControlled())
		return false;

	if (AmmoType.bSquadUsesPlayerClips && Instigator.Squad != None)
	{
		return UseSquadAmmo(Pawn(Owner));
	}
	else if ( AmmoType.bInfiniteClipsForNonPlayers )
	{
		GiveAmmo(Pawn(Owner));
		return true;
	}
	return false;
}

//========================================================================
//	THINK
//========================================================================
function ServerThink()
{
	local bool bForce, bForceAlt;

	if (AmmoType == None)
		return;

	bForce = bForceFire;
	bForceAlt = bForceAltFire;
	bForceFire = false;
	bForceAltFire = false;

	if ( (Instigator == None) || (Instigator.Controller == None) )
	{
		GotoState('');
		return;
	}

	if (!IsInState('Idle') && (StartIdleTime != -1))
		StartIdleTime = -1;	

	if ( !Instigator.IsHumanControlled() )
	{
		if ( !AmmoType.HasAmmo() )
		{
			ObtainAmmo();
		}
	}
	if ( !AmmoType.HasAmmo() && Instigator.IsLocallyControlled() && !AmmoType.bIsRecharge )
		SwitchToWeaponWithAmmo();
	else if ( Instigator.Weapon != self )
		GotoState('Idle');
	else if ( bForce || bAutoFire )
		Fire(0);
	else 
		GotoState('Idle');

	if ( NeedsToReload() && AmmoType.HasAmmo() )
	{
		ServerForceReload();
		return;
	}
}

simulated function ClientThink()
{
	if ( (Instigator == None) || (Instigator.Controller == None) )
	{
		GotoState('');
		return;
	}
	if (!IsInState('Idle') && (StartIdleTime != -1))
		StartIdleTime = -1;	
	if ( bChangeWeapon )
		GotoState('DownWeapon');
	else if ( Instigator.PressingFire() && HasAmmo() )
		Global.Fire(0);
	else if (NeedsToReload() && AmmoType.HasAmmo() )
		ForceReload();
	else
		GotoState('Idle');
}

//========================================================================
//	STATE
//========================================================================
state NormalFire
{
	simulated function AnimEnd(int Channel)
	{
		if (Channel == 1)
		{
			if (ROLE == ROLE_Authority)
				ServerThink();
			else 
				ClientThink();
		}
	}

	simulated function BeginState()
	{
		//Log("NormalFire BeginState "$self$" Time "$Level.TimeSeconds);
	}

	simulated function EndState()
	{
		if (bDeleteMe)
			return;

		//Log("NormalFire EndState "$self$" Time "$Level.TimeSeconds);
		StopSound(ThirdPersonActor, FireSound, 0.1);
		ServerStopFire();
		PlayAnim('None',,,,1); //clear out any FireAnim
	}

	simulated function Tick(float DeltaTime)
	{
		if ( !AmmoType.HasAmmo() && !AmmoType.bIsRecharge && !ObtainAmmo() )
		{
			SwitchToWeaponWithAmmo();
			return;
		}
		if ( bAutoFire && Instigator != None && Instigator.PressingFire() )
			Fire(0);
	}

	simulated function bool PutDown(weapon NextWeapon)
	{
		//Log("NormalFire PutDown "$self);
		bChangeWeapon = true;
		NewWeapon = NextWeapon;
		GotoState('DownWeapon');
		return true;
	}
}

state Idle
{
	function bool IsIdle()
	{
		return true;
	}

	simulated function BeginState()
	{
		//Log("Idle BeginState "$self$" CurrentWeapon "$Pawn(Owner).Weapon$" HasAmmo "$AmmoType.HasAmmo());
		//if ( NeedsToReload() && AmmoType.HasAmmo() )
		//	ServerForceReload();		//GotoState('Reloading');
		if ( !AmmoType.HasAmmo() && !AmmoType.bIsRecharge ) 
			SwitchToWeaponWithAmmo();	//Instigator.Controller.SwitchToBestWeapon();  //Goto Weapon that has Ammo
		else if ( Instigator.PressingFire() )
			Fire(0);
		else if ( Instigator.PressingAltFire() ) 
			AltFire(0);
	}

	simulated function EndState()
	{
		//Log("Idle EndState "$self);
	}
	
	simulated function AnimEnd(int Channel)
	{
		if ( Channel == 0 && bPlayingIdle )
			bPlayingIdle = false;
	}

	simulated function bool PutDown(weapon NextWeapon)
	{
		bChangeWeapon = true;
		NewWeapon = NextWeapon;
		GotoState('DownWeapon');
		return true;
	}

	simulated function Tick(float deltaTime)
	{	
		if ( (Pawn(Owner) != None) && (Pawn(Owner).Controller != None) && Pawn(Owner).Controller.IsA('PlayerController') && (Level.NetMode != NM_DedicatedServer) )
			PlayIdleAnim();

		if( bAutoFire && Instigator != None && Instigator.PressingFire() && Role < ROLE_Authority )
			Fire(0);
	}
}

state Reloading
{
	simulated function ServerForceReload() {}
	simulated function Fire(float value) {}
	simulated function Zoom(bool ZoomIn) {}
	//simulated function AltFire(float value) {}
	//simulated function ThrowGrenade(float value) {}
	simulated function bool PutDown(weapon NextWeapon) 
	{ 
		bChangeWeapon = true;
		NewWeapon = NextWeapon;
		GotoState('DownWeapon');
		return true;
	}

	simulated function float GetNextZoomFOV()
	{
		return ZoomFOVs[CurrentZoomFOVIndex];
	}

	event bool CanSwitchWeapon()
	{
		return true;
	}

	simulated function BeginState()
	{	
		local Pawn PawnOwner;

		bForceReload = false;		// delete variable?

		if( Owner != None )
		{
			PawnOwner = Pawn(Owner);
			if( PawnOwner.Controller != None && PawnOwner.Controller.IsA('PlayerController') )
			{
				PlayerController(PawnOwner.Controller).ResetFOV();
				SetZoom(false);
			}
		}

		if( Instigator.IsHumanControlled() )
			PlayReloading();
		else		
			FinishedReloading();		
	}

	simulated function AnimEnd(int Channel)
	{
		if (Channel == 0)
		{
			if (AmmoType.GetAmmoAmount() < Default.ReloadCount)
				ReloadCount = AmmoType.GetAmmoAmount();
			else
				ReloadCount = Default.ReloadCount;
			FinishedReloading();
			if (bHasAmmoNeedle)
				ManageAmmoEffect(false);
		}
	}

	simulated function FinishedReloading()
	{
		if ( Role == ROLE_Authority )
			ServerThink();
		else
			ClientThink();
	}

	simulated function EndState()
	{
		if( (ReloadCount == 0) && !Pawn(Owner).Controller.IsA('PlayerController') )
		{
			if (AmmoType.GetAmmoAmount() < Default.ReloadCount)
				ReloadCount = AmmoType.GetAmmoAmount();
			else
				ReloadCount = Default.ReloadCount;
		}
	}
}

state GrenadeThrow
{
	simulated function Fire(float value) {}
	simulated function AltFire(float value) {}
	simulated function ServerForceReload() {}
	simulated function ThrowGrenade(float value) {}
	simulated function bool PutDown(weapon NextWeapon) 
	{ 		
		if (bForcesWeaponChange || NextWeapon.bForcesWeaponChange)
			return Global.PutDown(NextWeapon);
		else
		{
			bChangeWeapon = false;
			return false; 
		}
	}

	event bool CanSwitchWeapon()
	{
		return false;
	}

	simulated function BeginState()
	{
		//Log("GrenadeThrow BeginState");
		bCanSwitchGrenade = false;
		PlayThrowingGrenade();
		// AI Needs
		if ( !Pawn(Owner).Controller.IsA('PlayerController') && !Pawn(Owner).HasAnim('ThrowGrenade') )
			ReleaseGrenade();
	}

	simulated function AnimEnd(int Channel)
	{
		if (Channel == 0)
		{
			if (ROLE == ROLE_Authority)
				ServerThink();
			else
				ClientThink();
		}
	}
}

state Melee
{
	simulated function Fire(float value) {}
	//simulated function AltFire(float value) {}
	simulated function ServerForceReload() {}
	simulated function ThrowGrenade(float value) {}
	simulated function Zoom(bool ZoomIn) {}
	simulated function bool PutDown(weapon NextWeapon) 
	{ 
		if (bForcesWeaponChange || NextWeapon.bForcesWeaponChange)
			return Global.PutDown(NextWeapon);
		else
		{
			bChangeWeapon = false;
			return false; 
		}
	}

	event bool CanSwitchWeapon()
	{
		return false;
	}

	/* jH: this messes up the zoom if zoom if pressed before melee finishes
	simulated function float GetNextZoomFOV()
	{
		return ZoomFOVs[CurrentZoomFOVIndex];
	}
	*/

	simulated function BeginState()
	{
		//Log("Melee BeginState");
		PlayMelee();
	}

	simulated event AnimLoopEnd( float LoopStart )
	{
		//Log("Melee AnimLoopEnd");
		if (bLoopMelee)
			PlayMelee();
	}

	simulated function AnimEnd(int Channel)
	{		
		if (Channel == 0)
		{
			if (ROLE == ROLE_Authority)
				ServerThink();
			else
				ClientThink();
		}
	}
}

/* DownWeapon
Putting down weapon in favor of a new one.  No firing in this state
*/
state DownWeapon
{
	simulated function Fire(float value) {}
	simulated function AltFire(float value) {}
	simulated event ForceReload() {}
	simulated function ServerForceReload() {}
	simulated function ThrowGrenade(float value) {}
	simulated function Zoom(bool ZoomIn) {}

	simulated function bool PutDown(weapon NextWeapon)
	{		
		NewWeapon = NextWeapon;
		bChangeWeapon = true;

		// if trying to put down itself and your a special weapon... force bring up
		if ((self == NextWeapon) && (bForcesWeaponChange))
			BringUp(self);

		return true;
	}

	simulated function BeginState()
	{
		//Log("DownWeapon BeginState "$self$" Time "$Level.TimeSeconds);
		//bChangeWeapon = false;
		PlayAnim('None',,,,1); //clear out any FireAnim
		TweenDown();
		bApplyKick = false;
		SetTimer(2.0, false);
		if ( Owner != None && Pawn(Owner).Controller != None )
		{
			PlayerController(Pawn(Owner).Controller).ResetFOV();
			SetZoom(false);
		}
	}

	simulated function AnimEnd(int Channel)
	{
		if( Channel == 0 )
		{
			if (self == Pawn(Owner).Weapon)
			{
				if ( NewWeapon != None )
					Pawn(Owner).ChangedWeapon();
				EndState();
			}
		}
	}
	
	simulated function Timer()
	{
		if (!IsAnimating())
			AnimEnd(0);
	}	

	simulated function float GetNextZoomFOV()
	{
		return ZoomFOVs[CurrentZoomFOVIndex];
	}

	simulated function EndState()
	{
		//Log("DownWeapon EndState "$self$" Time "$Level.TimeSeconds);
	}
}

/* Active
Bring newly active weapon up.
The weapon will remain in this state while its selection animation is being played (as well as any postselect animation).
While in this state, the weapon cannot be fired.
*/
state Active
{
	simulated function Fire(float value) {}
	simulated function AltFire(float value) {}
	simulated function ServerForceReload() {}
	simulated function ThrowGrenade(float value) {}
	simulated function PlayTurnAnim(float deltaYaw, float deltaPitch) {}
	simulated function Zoom(bool ZoomIn) {}

	simulated function bool PutDown(weapon NextWeapon)
	{
		bChangeWeapon = true;
		NewWeapon = NextWeapon;
		GotoState('DownWeapon');
		return true;
		/*
		// allow weapon disarm if it's during a briefing
		if ( Pawn(Owner) != None && PlayerController(Pawn(Owner).Controller)!= None )
		{
			if (PlayerController(Pawn(Owner).Controller).bBriefing)
			{
				bChangeWeapon = true;
				NewWeapon = NextWeapon;
				GotoState('DownWeapon');
				return true;
			}
		}
		bChangeWeapon = false;
		return false;
		*/
	}

	simulated function float GetNextZoomFOV()
	{
		return ZoomFOVs[CurrentZoomFOVIndex];
	}

	event bool CanSwitchWeapon()
	{
		return true;
	}

	simulated function BeginState()
	{
		//Log("Active BeginState "$self$" Time "$Level.TimeSeconds$" Mesh "$Mesh.Name);
		Instigator = Pawn(Owner);
		bForceFire = false;
		bForceAltFire = false;
		bWeaponUp = false;
		bChangeWeapon = false;
		PlaySelect();
		SetTimer(2.0, false);
	}

	simulated function EndState()
	{
		//Log("Active EndState "$self$" Time "$Level.TimeSeconds$" Mesh "$Mesh.Name);
		bForceFire = false;
		bForceAltFire = false;
		if ( Pawn(Owner) != None && Pawn(Owner).PendingWeapon == self )
			Pawn(Owner).PendingWeapon = None;
	}

	simulated function Timer()
	{
		if (!IsAnimating())
			AnimEnd(0);
	}

	simulated function AnimEnd(int Channel)
	{
		if (Channel == 0)
		{
			if (bHasAmmoNeedle)
				ManageAmmoEffect(true);
			if ( bChangeWeapon )
				GotoState('DownWeapon');
			if ( Owner == None )
			{
				Global.AnimEnd(0);
				GotoState('');
			}
			else
			{
				bWeaponUp = true;
				if (ROLE == ROLE_Authority)
					ServerThink();
				else
					ClientThink();
			}
		}
	}
}

/* PendingClientWeaponSet
Weapon on network client side may be set here by the replicated function ClientWeaponSet(), to wait,
if needed properties have not yet been replicated.  ClientWeaponSet() is called by the server to 
tell the client about potential weapon changes after the player runs over a weapon (the client 
decides whether to actually switch weapons or not.
*/
simulated State PendingClientWeaponSet
{
	simulated function Timer()
	{
		if ( Pawn(Owner) != None )
		{
			ClientWeaponSet(true);
		}
	}

	simulated function BeginState()
	{
		//Log("PendingClientWeaponSet BeginState "$self);
		SetTimer(0.05, true);
	}

	simulated function EndState()
	{
		//Log("PendingClientWeaponSet EndState "$self);
		SetTimer(0.0, false);
	}
}


defaultproperties
{
     PickupAmmoCount=20
     InitAmmoAmount=-1
     bCanThrow=True
     bCanReload=True
     bCanSwitchGrenade=True
     bSaveAsLastWeapon=True
     AutoSwitchPriority=1
     BurstCount=1
     WeaponType=WT_Secondary
     FPGrenadeOffset=(X=57.4,Y=-30,Z=-10.56)
     DisableRecoil=1.5
     XRecoilMultiplier=1
     ZoomDamageMultiplier=1
     OptimalDist=5000
     ValidRange=(Max=7000)
     WeaponEmptySound=Sound'GEN_Sound.Weapons.wep_GEN_empty_01'
     GrenadeEmptySound=Sound'GEN_Sound.Interface.int_GEN_selGrnEmpty_01'
     MessageNoAmmo=" has no ammo."
     NameColor=(B=255,G=255,R=255,A=255)
     DisplayFOV=85
     ZoomFOVs(0)=85
     ZoomFOVs(1)=85
     ZoomFOVs(2)=85
     ZoomFOVs(3)=85
     NumZoomFOVs=1
     MuzzleBone="muzzle"
     ShotDelay=0.1
     PlayIdleTime=15
     LoadAnim="Load"
     FireAnim="Fire"
     HolsterAnim="Holster"
     PlayerSpeedModifier=1
     HudArmsShaderIndex(0)=-1
     HudArmsShaderIndex(1)=-1
     AttachmentClass=Class'Engine.WeaponAttachment'
     ItemName="Weapon"
     DrawType=DT_Mesh
     bReplicateInstigator=True
     bReplicateAnimations=False
     Mass=1
}

