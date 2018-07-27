class WeaponAttachment extends InventoryAttachment
	native
	nativereplication;

var		byte	FlashCount;			// incremented on the server and replicated to client when time to draw muzzle flash
var		byte	PrivateFlashCount;  // when different from FlashCount, client will draw muzzle flash for current frame
var		byte	ThrowCount;			// incremented on the server and replicated to client when time to throw
var		byte	PrivateThrowCount;	// when different from ThrowCount, time for the client to play the throw anim
var		byte	ReloadCount;		// incremented on the server and replicated to client when time to reload
var		byte	PrivateReloadCount;	// when different from ReloadCount, time for the client to play the reload anim
var		byte	MeleeCount;			// incremented on the server and replicated to client when time to melee
var		byte	PrivateMeleeCount;	// when different from MeleeCount, time for the client to play the melee attack anim
var		byte	HolsterCount;		// incremented on the server and replicated to client when time to holster weapon
var		byte	PrivateHolsterCount;// when different from HolsterCount, time for the client to play the holster weapon anim
var		byte	LoadCount;			// incremented on the server and replicated to client when time to load
var		byte	PrivateLoadCount;	// when different from LoadCount, time for the client to play the bring up weapon anim
var		byte	StopFireCount;		// incremented on the server and replicated to client when time to stop firing
var		byte	PrivateStopFireCount;//when different from StopFireCount, time for the client to stop playing the firing anim

var		bool	bChangeHolsterCount;  // tick

var Emitter MuzzleFlash;
var class<Emitter> MuzzleClass;
var vector MuzzleOffset;
var rotator MuzzleRotationOffset;
var DynamicLight MuzzleLight;
var vector LightOffset;

//var vector HitLoc;						// Used to say where a trace hit occured
var vector EffectLocationOffset[2];		// Used for 1st person vs 3rd person Effects

var bool bPlayingFireSound;				// are we playing the fire sound?
var	bool bAutoFire;						// set to tell us if the weapon is a rapid fire type aka automatic
var	bool bNeedToBreathePawn;			// true upon creation until we have told our owner how to breathe while carrying us
var Sound LastLoopingSound;				// the fire sound we are currently playing

//animations for the 3rd person pawns to play
var(Animation) name MPPawnBreatheAnim;
var(Animation) name MPPawnFireAnim;
var(Animation) name MPPawnHolsterAnim;
var(Animation) name MPPawnLoadAnim;
var(Animation) name MPPawnIdleAnim;
var(Animation) name MPPawnMeleeAnim;
var(Animation) name MPPawnThrowAnim;

var class<Projectile> ProjectileClass;

replication
{
	// Things the server should send to the client.
	reliable if( bNetDirty && !bNetOwner && (Role==ROLE_Authority) )
		FlashCount, ThrowCount, ReloadCount, MeleeCount, HolsterCount, LoadCount, StopFireCount, bPlayingFireSound, LastLoopingSound;
	
	// Things the server should send to the client.
//	unreliable if( bNetDirty && !bNetOwner && (Role==ROLE_Authority) )
//		bAutoFire; //EffectLocationOffset, HitLoc;

	// functions called by client on server
	reliable if ( Role<ROLE_Authority )
		IncrementHolsterCount;

	reliable if ( !bNetOwner && bNetInitial && (Role==ROLE_Authority))
		ProjectileClass;

}

function PostBeginPlay()
{
	Super.PostBeginPlay();

	if (( Pawn(Owner) != None) && ( Pawn(Owner).Weapon != None )&& ( Pawn(Owner).Weapon.AmmoType != None )
		&& ( Pawn(Owner).Weapon.AmmoType.bInstantHit == true ) && ( Pawn(Owner).Weapon.AmmoType.TracerEffect == None ))			
		ProjectileClass       = Pawn(Owner).Weapon.AmmoType.ProjectileClass;	
}

simulated event Tick(float Delta)
{
	local Pawn PawnOwner;

	Super.Tick(Delta);
	
	if( bNeedToBreathePawn && Owner != None )
	{
		PawnOwner = Pawn(Owner);
		if( PawnOwner.IsHumanControlled() )		
			PawnOwner.PlayBreathe(MPPawnBreatheAnim);

		bNeedToBreathePawn = false;
	}

	// This delay is needed to prevent a bad network order of operations failure that results in the no-shooting bug
	if ( bChangeHolsterCount )
	{
		bChangeHolsterCount = false;
		IncrementHolsterCount();
	}
}

simulated function Adjust(int Person, vector Adjustment)
{
	EffectLocationOffset[Person] = EffectLocationOffset[Person]+Adjustment;

	if (Person==0)
		log("#### New 3rd Person Offset: "$EffectLocationOffset[Person]);
	else
		log("#### New 1st Person Offset: "$EffectLocationOffset[Person]); 
}

simulated event PostNetBeginPlay()
{
	Super.PostNetBeginPlay();

	if (Owner == None)
		return;

	if (Level.NetMode == NM_Standalone && !Owner.IsA('MPPawn'))
	{
		MPPawnBreatheAnim='';
		MPPawnFireAnim='';
		MPPawnHolsterAnim='';
		MPPawnLoadAnim='';
		MPPawnIdleAnim='';
		MPPawnMeleeAnim='';
		MPPawnThrowAnim='';
	}	
}

simulated function SpawnAttachmentMuzzleFlash()
{
	local name BoneName;	
	local int i;

	if (Instigator == None && Pawn(Owner) != None)
		Instigator = Pawn(Owner);
	if (Instigator == None)
		return;

	MuzzleFlash = spawn(MuzzleClass,Owner);
	BoneName = GetAttachmentBoneName();
	if (BoneName == '')
	{
		MuzzleFlash.SetLocation(Instigator.Location);
		MuzzleFlash.SetBase(Instigator);
	}
	else
		Instigator.AttachToBone(MuzzleFlash,BoneName);

	MuzzleFlash.SetRelativeRotation(MuzzleRotationOffset);
	MuzzleFlash.SetRelativeLocation(MuzzleOffset);

	MuzzleFlash.AutoDestroy = false;
	MuzzleFlash.LightType = LT_Steady;		
	for (i = 0; i < MuzzleFlash.Emitters.Length; i++)
	{				
		MuzzleFlash.Emitters[i].TriggerDisabled = false;
		MuzzleFlash.Emitters[i].ResetOnTrigger = true;
		MuzzleFlash.Emitters[i].AutoDestroy = false;
	}	
}

simulated function name GetAttachmentBoneName()
{
	return Instigator.GetWeaponBoneFor(Instigator.Weapon);
}

simulated event AttachMuzzleFlash()
{
}

simulated event ThirdPersonEffects()
{
	local Weapon InstigatorWeapon;
	local Actor aProjectile;

	if ( (Level != None) && (Level.NetMode == NM_DedicatedServer) )
		return;
	if ( Instigator == None )
		return;

	InstigatorWeapon = Instigator.Weapon;
	
	if (PrivateFlashCount != FlashCount) 
	{
		PrivateFlashCount = FlashCount;

		if( MuzzleLight != None )
		{			
			MuzzleLight.LightType = LT_Steady;			
			SetTimer( 0.1, false );
		}

		if (MuzzleFlash == None)
		{
			if (MuzzleClass != None)
				SpawnAttachmentMuzzleFlash();
		}
		else //if (InstigatorWeapon != None)
		{
			MuzzleFlash.bHidden = false;

			if (InstigatorWeapon != None && InstigatorWeapon.bLoopMuzzleFX)
			{				
				MuzzleFlash.SetRelativeRotation(MuzzleRotationOffset);
				MuzzleFlash.SetRelativeLocation(MuzzleOffset);
			}
			// reset the particles
			else
			{
				MuzzleFlash.LightType = LT_Steady;
				MuzzleFlash.Trigger( self, None );
			}
		}
		// have pawn play firing anim
		Instigator.PawnPlayFiring(MPPawnFireAnim, bAutoFire);

		//		
		if ( ProjectileClass != None && !Instigator.IsLocallyControlled() && Level.NetMode != NM_Standalone )
		{
			if ( MuzzleFlash != None )
			{
				aProjectile = Spawn(ProjectileClass,,,MuzzleFlash.Location, MuzzleFlash.Rotation);
				if ( aProjectile != None )
					aProjectile.RemoteRole = ROLE_None;
			}
		}

	}
	else if (MuzzleFlash != None)
		MuzzleFlash.bHidden = true;


	// have pawn play throw anim
	if (PrivateThrowCount != ThrowCount) 
	{
		PrivateThrowCount = ThrowCount;
		Instigator.PlayAnim( MPPawnThrowAnim, 'spine1', , , 3 ); //play the throw anim on the pawn
	}

	// have pawn play reload anim
	if (PrivateReloadCount != ReloadCount) 
	{
		PrivateReloadCount = ReloadCount;

		if (IsA('RocketLauncherAttachment'))
			Instigator.PlayAnim('RocketReload', 'spine1', , , 3);
		else
			Instigator.PlayAnim('Reload', 'spine1', , , 3);//play the reload anim on the pawn
	}

	// have pawn play melee attack anim
	if (PrivateMeleeCount != MeleeCount) 
	{
		PrivateMeleeCount = MeleeCount;
		Instigator.PlayAnim( MPPawnMeleeAnim, 'spine1', , , 3 );//play the melee attack anim on the pawn
	}

	// have pawn play holster anim
	if (PrivateHolsterCount != HolsterCount) 
	{
		PrivateHolsterCount = HolsterCount;
		Instigator.PlayAnim( MPPawnHolsterAnim, 'spine1', , , 3 );//play the holster anim on the pawn
	}

	// have pawn play bring up weapon anim
	if (PrivateLoadCount != LoadCount) 
	{
		PrivateLoadCount = LoadCount;
		Instigator.PlayAnim( MPPawnLoadAnim, 'spine1', , , 3 );//play the bring up weapon anim on the pawn
		Instigator.PlayBreathe(MPPawnBreatheAnim);
	}
}

// will cause client side throwing animation
simulated function IncrementThrowCount()
{
	ThrowCount++;
	if (Level.NetMode != NM_ListenServer && !Level.IsSplitScreen())
		PrivateThrowCount = ThrowCount;
}

// will cause client side weapon firing effects
simulated function IncrementFlashCount(bool bLoopingFire)
{
	bAutoFire = bLoopingFire;
	FlashCount++;
	if (Level.NetMode != NM_ListenServer && Level.NetMode != NM_Standalone && !Level.IsSplitScreen())
		PrivateFlashCount = FlashCount;
}

// will cause client side weapon reload animation
simulated function IncrementReloadCount()
{
	ReloadCount++;
	if (Level.NetMode != NM_ListenServer && !Level.IsSplitScreen())
		PrivateReloadCount = ReloadCount;
}

// will cause client side weapon melee attack animation
simulated function IncrementMeleeCount()
{
	MeleeCount++;
	if (Level.NetMode != NM_ListenServer && !Level.IsSplitScreen())
		PrivateMeleeCount = MeleeCount;
}

// will cause client side weapon holster animation
// this function is different from the rest b/c the calling code runs client-side only
// so, this function has to be replicated to the server, who in the case of a listen server, needs to go ahead and do the FX
function IncrementHolsterCount()
{

	HolsterCount++;
	if (Level.NetMode != NM_ListenServer && !Level.IsSplitScreen())
		PrivateHolsterCount = HolsterCount;
	else if (Instigator != None && !Instigator.IsLocallyControlled() || Level.IsSplitScreen())//or splitscreen!
		ThirdPersonEffects();
}

// will cause client side bring up weapon animation
simulated function IncrementLoadCount()
{
	LoadCount++;
	if (Level.NetMode != NM_ListenServer && !Level.IsSplitScreen())
		PrivateLoadCount = LoadCount;
}

// used to cause clients to stop playing looping fire animations
simulated function IncrementStopFireCount()
{
	StopFireCount++;
	PrivateStopFireCount = StopFireCount;
}

simulated function StopMuzzleFlash()
{
	if (MuzzleFlash != None)
	{
		MuzzleFlash.LightType = LT_None;
		//MuzzleFlash.FadeOut();
		//MuzzleFlash = None;

		MuzzleFlash.bHidden = true;
	}
}

simulated function Timer()
{	
	if( MuzzleLight != None )
		MuzzleLight.LightType = LT_None;	
}

simulated event Destroyed()
{
	if (MuzzleFlash!=None)	
		MuzzleFlash.Destroy();	
	
	if (MuzzleLight!=None)	
		MuzzleLight.Destroy();	
	
	Super.Destroyed();
}


defaultproperties
{
     bNeedToBreathePawn=True
     MPPawnBreatheAnim="DC17mActionBreathe"
     MPPawnFireAnim="DC17mFire"
     MPPawnHolsterAnim="DC17Holster"
     MPPawnLoadAnim="DC17Load"
     MPPawnIdleAnim="DC17Idle"
     MPPawnMeleeAnim="DC17mMeleeAttack"
     MPPawnThrowAnim="DC17mThrowGrenade"
     bActorShadows=True
     bReplicateInstigator=True
}

