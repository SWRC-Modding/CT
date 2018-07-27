//=============================================================================
// Ammunition: the base class of weapon ammunition
//
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================

class Ammunition extends Inventory
	abstract
	native
	nativereplication;

var()		int MaxAmmo;						// Max amount of ammo
var travel	int AmmoAmount;						// Amount of Ammo current available
var()		bool DigitalClipDisplay;			// True if clips count is shown as a number
												//  false if it uses a empty/full bar

// Used by Bot AI

var		bool	bTossed;
var		bool	bInstantHit;
var		bool	bIsRecharge;
var		bool	bInfiniteClipsForNonPlayers;
var(AI)	bool	bSquadUsesPlayerClips;

// Damage and Projectile information

var class<Projectile> ProjectileClass;
var class<DamageType> MyDamageType;
var float MaxAutoAimRange; 

// from CTAmmo
enum EAmmunitionType {
	AT_Random,
	AT_Line,
	AT_Circle,
	AT_Ring,
};

var() int				ProjPerFire;
var() float				TracerFreq;				//if tracers, what's the randomized frequency
var() float				Spread;					//angular spread for the firing patterns
var	  float				Offset;					//spatial offset for the firing patterns
var() float				Range;					//distance instant hit weapons travel
var() float				Damage;					//amount of damage to inflict
var() float				DamageRadius;			//damage radius of instant hit weapon
var() float				AreaDamageTimer;		//timer before dealing area damage
var   EAmmunitionType	AmmunitionType;
var	  EStimulusType		FireStimulus;
var() int				AimAdjustment;			//used in auto aim
var() int				ZoomedAimAdjustment;	//used in auto aim
var	  vector			LastHitLocation;
var   rotator			LastHitDir;
var	  Actor				LastHitActor;
var() float				DamageDampening;		//1: ammo damage becomes 0 after one hit.  0: ammo damage doesn't decrease
var() class<Emitter>	TracerEffect;
var	  Emitter			TracerEmitter;
var   Emitter			CurrentHitEffect;

// Hit Effects
var() class<MaterialTypeInfo>	HitEffectsByMaterial;
var() float						HitEffectsProbability;
var() TexPanner2D		PannerCharge;
var() TexPanner2D		PannerClip;
var() TexPanner2D		PannerDigit1;
var() TexPanner2D		PannerDigit10;
var() TexPanner2D		PannerDigit100;

var() bool				bHighPrecision;

// Network replication
//

replication
{
	// Things the server should send to the client.
	reliable if( bNetOwner && bNetDirty && (Role==ROLE_Authority) )
		AmmoAmount;

	//reliable if (Role==ROLE_Authority)
	//	CreateTracerEffect;
}


simulated function bool HasAmmo()
{
	return ( AmmoAmount > 0 );
}

function DeductAmmo()
{
	AmmoAmount -= 1;
}

function DeductAmmoAmount(int amount)
{
	if (amount <= AmmoAmount)
		AmmoAmount -= amount;
	else
		AmmoAmount = 0;
}

function ProcessTraceHit(Weapon W, Actor Other, Vector HitLocation, Vector HitNormal, Vector X, Vector Y, Vector Z)
{
	DeductAmmo();
}

simulated function DisplayDebug(Canvas Canvas, out float YL, out float YPos)
{
	Canvas.DrawText("Ammunition "$GetItemName(string(self))$" amount "$AmmoAmount$" Max "$MaxAmmo);
	YPos += YL;
	Canvas.SetPos(4,YPos);
}
	
function bool HandlePickupQuery( pickup Item )
{
	local bool bReloadWeapon;

	if ( class == item.InventoryType ) 
	{
		if (AmmoAmount==MaxAmmo) 
			return true;
		item.AnnouncePickup(Pawn(Owner));
		if (AmmoAmount == 0)
			bReloadWeapon = true;
		AddAmmo(Ammo(item).AmmoAmount);
		if ( bReloadWeapon && (Pawn(Owner).Weapon != None) && (Pawn(Owner).Weapon.AmmoType == self) )
			Pawn(Owner).Weapon.ServerForceReload();
        item.SetRespawn(); 
		return true;				
	}
	if ( Inventory == None )
		return false;

	return Inventory.HandlePickupQuery(Item);
}

// If we can, add ammo and return true.  
// If we are at max ammo, return false
//
function bool AddAmmo(int AmmoToAdd)
{
	AmmoAmount = Min(MaxAmmo, AmmoAmount+AmmoToAdd);
	AmmoAmount = Max(0, AmmoAmount);
	return true;
}

function float GetDamageRadius()
{
	if ( ProjectileClass != None )
		return ProjectileClass.Default.DamageRadius;

	return 0;
}

// Added for HUD purposes
// return the current ammount of ammo
simulated function float GetAmmoAmount()
{
	return AmmoAmount;
}

// return the max amount of ammo
simulated function float GetMaxAmmo()
{
	return MaxAmmo;
}

function Timer()
{
	CauseAreaDamage();
}

function CauseAreaDamage()
{
	if( LastHitActor != None )
	{
		LastHitActor.TakeDamage( Damage, Pawn(Owner), LastHitLocation, Vector(LastHitDir) * MyDamageType.default.KDamageImpulse, MyDamageType );
		HurtRadius( Damage, DamageRadius, MyDamageType, MyDamageType.default.KDamageImpulse, LastHitLocation, LastHitActor );
		LastHitActor = None;
	}
	else
	{
		HurtRadius( Damage, DamageRadius, MyDamageType, MyDamageType.default.KDamageImpulse, LastHitLocation );
	}
}

function bool CreateProximityProjectile(Vector Start, rotator Dir, Pawn PawnOwner)
{
	local Actor Other;
	local vector TraceEnd, HitLocation, HitNormal, PawnLocation;
	local EMaterialType HitMaterialType;
	local name HitBone;

	if (!PawnOwner.IsHumanControlled() || PawnOwner.Controller.IsInState('PlayerManningTurret'))
		return false;

	PawnLocation = PawnOwner.Location + PawnOwner.EyePosition();
	TraceEnd = Start;
	Other = Trace(HitLocation,HitNormal,TraceEnd,PawnLocation,true,,,HitBone, HitMaterialType);
	if (Other != None)
	{
		if (bTossed)
			Spawn(ProjectileClass,,, PawnLocation,Dir);
		else
		{
			HitLocation = PawnLocation + Vector(PawnOwner.Controller.Rotation) * 50;
			Other.TakeDamage( Damage, PawnOwner, HitLocation, Vector(Dir) * MyDamageType.default.KDamageImpulse, MyDamageType, HitBone );
			HurtRadius( Damage, DamageRadius, MyDamageType, MyDamageType.default.KDamageImpulse, HitLocation, Other );
			PawnOwner.PostStimulusAtLocation(ST_WeaponHitDefault,HitLocation);
			if (None != HitEffectsByMaterial && frand() <= HitEffectsProbability )
				HitEffectsByMaterial.Static.SpawnEffectsFor(self, HitMaterialType, HitLocation, HitNormal);
		}
		return true;
	}
	return false;
}

function CreateProjectile(vector Start, rotator Dir)
{
	local Actor Other;	
	local vector TraceEnd, HitLocation, HitNormal;
	local name HitBone;
	local float ActualDamage;
	local EMaterialType HitMaterialType;		
	local Weapon PawnWeapon;
	local Pawn PawnOwner;

	PawnOwner = Pawn(Owner);
	if (PawnOwner == None)
		return;

	// Test to see if we're up against an actor
	if ( CreateProximityProjectile(Start, Dir, PawnOwner) )
		return;

	if( bInstantHit )
	{
		ActualDamage = Damage;

		PawnWeapon = PawnOwner.Weapon;
		// ZoomDamageMultiplier
		if ( PawnWeapon != None && PawnWeapon.bWeaponZoom && PawnWeapon.ZoomDamageMultiplier != 1.0 )
			ActualDamage *= PawnWeapon.ZoomDamageMultiplier;

		HitMaterialType = MT_None;
        TraceEnd = Start + (Vector(Dir) * Range);

		if ( DamageRadius > 0 )
		{
			Other = Trace(HitLocation,HitNormal,TraceEnd,Start,true,,,HitBone, HitMaterialType);			

			//// if didn't hit anything, just set a bogus hit location in front of direction of fire
			if (Other == None)
				HitLocation = TraceEnd;

			LastHitActor = Other;
			LastHitLocation = HitLocation;
			LastHitDir = Dir;

			if ( AreaDamageTimer > 0 )
				SetTimer(AreaDamageTimer, false);
			else
				CauseAreaDamage();

			if (None != HitEffectsByMaterial && frand() <= HitEffectsProbability )
				HitEffectsByMaterial.Static.SpawnEffectsFor(self, HitMaterialType, HitLocation, HitNormal);
		}
		else
		{
			if (DamageDampening == 1)
			{
				Other = Trace(HitLocation,HitNormal,TraceEnd,Start,true,,,HitBone,HitMaterialType);
				if ( Other != None )
				{
					if( ActualDamage > 0 )
					{
						Other.TakeDamage( ActualDamage, PawnOwner, HitLocation, Vector(Dir) * MyDamageType.default.KDamageImpulse, MyDamageType, HitBone );
						Owner.PostStimulusAtLocation(ST_WeaponHitDefault,HitLocation);

						if (None != HitEffectsByMaterial && frand() <= HitEffectsProbability)
							HitEffectsByMaterial.Static.SpawnEffectsFor(self, HitMaterialType, HitLocation, HitNormal);
					}
				}
				// if didn't hit anything, just set a bogus hit location in front of direction of fire
				else
					HitLocation = TraceEnd;
			}
			else
			{
				ForEach TraceActors( class'Actor',Other,HitLocation,HitNormal,TraceEnd,Start,,HitBone,HitMaterialType )
				{
					if (None != HitEffectsByMaterial && frand() <= HitEffectsProbability)
						HitEffectsByMaterial.Static.SpawnEffectsFor(self, HitMaterialType, HitLocation, HitNormal);

					Owner.PostStimulusAtLocation(ST_WeaponHitDefault,HitLocation);

					if (Other != None && ActualDamage > 0)
						Other.TakeDamage( ActualDamage, PawnOwner, HitLocation, Vector(Dir) * MyDamageType.default.KDamageImpulse, MyDamageType, HitBone );					

					ActualDamage *= (1-DamageDampening);

					if( Other == Level || Other.IsA('StaticMesh') || Other.IsA('Prop') || Other.IsA('KActor') || Other.IsA('StaticMeshActor') || Other.IsA('Mover') )
						break;
					else
						Other = None;
				}

				// if didn't hit anything, just set a bogus hit location in front of direction of fire
				if (Other == None)
					HitLocation = TraceEnd;
			}			
		}					

		if ( TracerEffect != None )
		{
			// these are real tracers, replicate them
			CreateTracerFire(Start, Dir, true);
			CreateTracerEffect(Start, Dir, HitLocation);
		}
	}
	else
		Spawn(ProjectileClass,,, Start,Dir);
}

//tracers
simulated function CreateTracerFire(vector Start, rotator Dir, bool bReplicate)
{
	local int NumFired;
	local float	Freq;
	local Weapon PawnWeapon;
	local Pawn PawnOwner;
	local Actor aProjectile;
	
	if (TracerFreq > 0)
	{
		PawnOwner = Pawn(Owner);
		if (PawnOwner == None)
			return;

		PawnWeapon = PawnOwner.Weapon;
		if ( PawnWeapon == None )
			return;

		NumFired = PawnWeapon.FireCount;
		Freq = TracerFreq*FRand();		//  spawn a trace every few shots (randomize)
		PawnWeapon.FireCount++;
		if( PawnWeapon.FireCount >= Freq )
		{
			aProjectile = Spawn(ProjectileClass,,, Start,Dir);
			
			// Don't replicate tracers that we are told not to
			if ( !bReplicate && aProjectile != None )
				aProjectile.RemoteRole = ROLE_None;

			PawnWeapon.FireCount = 0;
		}
	}
}

simulated function ResizeEmitter( Emitter TEmitter, vector Start, vector HitLocation)
{
	local float TraceDist;
	local int i;

	TraceDist = VSize(HitLocation-Start);
	for (i = 0; i < TEmitter.Emitters.Length; i++)
	{
		// sprite
		if( !VIsZero( TEmitter.Emitters[i].StartLocationOffset ) )
		{
			TEmitter.Emitters[i].StartSizeRange.Y.Min = TraceDist/2;
			TEmitter.Emitters[i].StartSizeRange.Y.Max = TraceDist/2;
			TEmitter.Emitters[i].StartLocationOffset.X = TraceDist/2;
		}
		// particle box
		else
			TEmitter.Emitters[i].StartLocationRange.X.Max = TraceDist;
	}
}


simulated function CreateTracerEffect(vector Start, rotator Dir, vector HitLocation)
{
	local Controller P;	

//	if ( (Level.NetMode != NM_DedicatedServer) && (TracerEffect != None) )
	if (TracerEffect != None)
	{
		if (Level.NetMode == NM_StandAlone || !bHighPrecision)
		{
			TracerEmitter = Spawn(TracerEffect,,, Start,Dir);
			ResizeEmitter( TracerEmitter, Start, HitLocation );
		}
		else
		{
			// in network play, call TracerProxy, which is a replicated function of playercontroller
			// so that each client creates it's own tracer emitter.
			for ( P=Level.ControllerList; P!=None; P=P.NextController )
			{
				if ( P.IsA('PlayerController') )
					PlayerController(P).TracerProxy(Start, HitLocation, TracerEffect);
			}
		}
	}
}

function vector GetOffsetOrigin(rotator Dir, vector Origin, float OffsetX, float OffsetY, float OffsetZ)
{
	local Vector Start, X,Y,Z;

	GetAxes(Dir,X,Y,Z);
	Start = Origin + X * OffsetX + Y * OffsetY + Z * OffsetZ;
	return Start;
}

function DoFire(vector Start, rotator Aim)
{	
    local vector X;
	local rotator R;
	local float OffsetX, OffsetY;
	local float mult;
    local int p;
    local int SpawnCount;
    local float theta;
	local vector Origin;
	
	SpawnCount = Max(1, ProjPerFire);

	switch (AmmunitionType)
	{
	case AT_Random:
		X = Vector(Aim);
		CreateProjectile(Start, Aim);
		for (p = 1; p < SpawnCount; p++)
		{
			offsetX = (FRand()-0.5);
			offsetY = (FRand()-0.5);
			R.Yaw = Spread * offsetX;
			R.Pitch = Spread * offsetY;
			R.Roll = Spread * (FRand()-0.5);
			Origin = Start;
			if (Offset > 0)
				Origin = GetOffsetOrigin(Aim, Start, 0.0, offsetX * Offset, offsetY * Offset);			 
			CreateProjectile(Origin, Rotator(X >> R));
		}
		break;
	case AT_Line:
		for (p = 0; p < SpawnCount; p++)
		{
			mult = p - float(SpawnCount - 1)/2.0;
			theta = Spread * PI/32768*mult;
			X.X = Cos(theta);
			X.Y = Sin(theta);
			X.Z = 0.0;
			Origin = Start;
			if (Offset > 0)
				Origin = GetOffsetOrigin(Aim, Start, 0.0, mult * Offset, 0.0);
			CreateProjectile(Origin, Rotator(X >> Aim));
		}
		break;
	case AT_Circle:
		X = Vector(Aim);
		CreateProjectile(Start, Aim);
		for (p = 1; p < SpawnCount; p++)
		{
			theta = (p - 1) * 2 * PI / (SpawnCount - 1);
			offsetX = Cos(theta);
			offsetY = Sin(theta);
			R.Yaw = Spread * OffsetX;
			R.Pitch = Spread * OffsetY;
			Origin = Start;
			if (Offset > 0)
				Origin = GetOffsetOrigin(Aim, Start, 0.0, offsetX * Offset, offsetY * Offset);
			CreateProjectile(Origin, Rotator(X) + R);			
		}
		break;
	case AT_Ring:
		X=Vector(Aim);
		for (p = 0; p < SpawnCount; p++)
		{
			theta = p * 2 * PI / (SpawnCount);
			OffsetX = Cos(theta);
			OffsetY = Sin(theta);
			R.Yaw = Spread * OffsetX;
			R.Pitch = Spread * OffsetY;
			Origin = Start;
			if (Offset > 0)
				Origin = GetOffsetOrigin(Aim, Start, 0.0, OffsetX * Offset, OffsetY * Offset);
			
			CreateProjectile(Origin, Rotator(X) + R);			
		}
		break;
	default:
		CreateProjectile(Start, Aim);
	}
	if (Owner != None)
		Owner.PostStimulusToWorld(FireStimulus);
	DeductAmmo();
}


defaultproperties
{
     bInfiniteClipsForNonPlayers=True
     MyDamageType=Class'Engine.DamageType'
     MaxAutoAimRange=5000
     ProjPerFire=1
     Range=100000
     FireStimulus=ST_WeaponFireDefault
     AimAdjustment=255
     DamageDampening=1
     HitEffectsProbability=1
     PannerCharge=TexPanner2D'HudArmsTextures.HudArmsDC17m.DC17mChargeCount_Pan'
     PannerClip=TexPanner2D'HudArmsTextures.HudArmsDC17m.DC17mClipCount_Pan'
     PannerDigit1=TexPanner2D'HudArmsTextures.HudArmsDC17m.DC17mDigit1'
     PannerDigit10=TexPanner2D'HudArmsTextures.HudArmsDC17m.DC17mDigit10'
     PannerDigit100=TexPanner2D'HudArmsTextures.HudArmsDC17m.DC17mDigit100'
     bHighPrecision=True
}

