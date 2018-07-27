// ====================================================================
//  Class:  CTGame.Turret
//
//  Base class for Clone Trooper Turrets
// ====================================================================

class Turret extends Pawn
	native
	placeable;

struct TurretInfo
{
	var()	Name			TransitionEvent;
	var()	Material		TransitionMaterial;
	var()	int				TransitionMaterialIndex;
	var()	Sound			TransitionSound;
	var()	class<Emitter>	TransitionEffect;
	var()	vector			TransitionEffectOffset;
};

var		Pawn				Gunner;
var		vector				TargetDirection;
var(Events)	name			ControlEvent;		// event to fire off when anyone takes control of turret
var(Events)	name			ReleaseEvent;		// event to fire off when anyone releases control of turret
var(Events) name			PlayerControlEvent; // event to fire off when player takes control of turret
var		static name			PitchJoint,YawJoint;
var()	int					MinPitch, MaxPitch, MinYaw, MaxYaw;
var()   int					LastNewYaw, LastNewPitch;
var	int						LastPitchAdjustment;
var()	int					Speed;
var		int					Yaw,Pitch;
var		int					TurretHeight;
var		float				FireRate;
var		float				LastFireTime;		// time which the turret last fired - used for shot delay
var		float				LastPitchAdjustTime;
var		static class<Ammunition>	AmmoName;
var		Ammunition			AmmoType;
var()	sound				FireSound;
var		sound				CurrentLoopingFireSound;
var()	float				ArmorAngle;			// What angle does this door absorb damage from
var()	bool				bUnlimitedYaw;		// No angle restrictions on the max, min yaw
var		bool				bPlayedStartSound;	// Have we played the start sound yet?
var		bool				bLoopingFireSound;	
var		bool				bUseSmartShot;

// variables used by automated turrets
var		bool				bResetTurret;		// move the turret back to its original location
var		bool				bInitLocation;
var		bool				bFire;				// true if enemy within FOV
var		bool				bCooldown;			// cooling down
var		bool				bIsAuto;			// is it auto or manned by player
var		bool				SitToUse;			// Whether this is a sit or stand style turret
var		bool				Damagable;			// Whether this Turret takes damage or not
var()	bool				bDeployed;			// Has the turret already deployed?
var()	bool				bPlayerUsable;		// Whether the turret can be mounted by the player
var		Pawn				ReservedBy;
var		float				TurretDeltaTime;	// deltaTime
var		float				MinYawRotation;
var     float				MaxYawRotation;
var()	float				TimeUntilCooldown;	// time before the auto turret cools down
var()	float				CooldownDuration;	// duration of cooldown
var		float				BeginCooldownTime;	// timestamp of when it first starts to cooldown
var		float				BeginFireTime;		// time at which the turret begins to fire - used to cooldown
var()	static float		ShotDelay;			// time in between shots
var		class				FPWeaponClass;		// the first person weapon associated with this turret
var		class <Emitter>		MuzzleEffect;		// third person muzzle effect

var		class<LaserBeam>	LaserBeamClass;
var		Actor				Laser;				// Actor pointer for Laser Beam Mesh
var()	sound				StartRotateSound;	// start sound as turret rotates
var()	sound				LoopRotateSound;	// Loop sound as turret rotates
var()	sound				StopRotateSound;	// stop sound as turret rotates

var()	TurretInfo			OffStateEnemy;
var()	TurretInfo			OffStateFriend;
var()	TurretInfo			OnStateEnemy;
var()	TurretInfo			OnStateFriend;
var()	TurretInfo			CoolStateEnemy;	
var()	TurretInfo			CoolStateFriend;	
var()	TurretInfo			DeadState;		

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

final event ReserveFor( Pawn User )
{
	ReservedBy = User;
}

final event Release()
{
	ReservedBy = None;
}

final event bool IsAvailableFor( Pawn User )
{
	if( User != None && User.Weapon != None && !User.Weapon.CanSwitchWeapon() )
		return false;

   	if( Gunner == None && ( ReservedBy == None || ReservedBy == User ) && !IsA('AutoTurret') )
  		return true;
   	
	return false;
}

function name PawnFireAnim()
{
	if (SitToUse)
		return 'TurretSitFire';
	else
		return 'TurretStandFire';
}

function PostBeginPlay()
{
	AmmoType = Spawn(AmmoName,self);
	AmmoType.AmmoAmount = 10000;

	if (NavPts.Length > 0)
		NavPts[0].bDontIdleHere = true;
	if( LaserBeamClass != None )
	{
		Laser = spawn( LaserBeamClass, self );
		AttachToBone( Laser, GetWeaponBoneFor( None ) );
		Laser.SetRelativeLocation( vect(0,0,0) );
		Laser.SetRelativeRotation( rot(0,0,0) );		
		Laser.bHidden = true;		
	}
	super.PostBeginPlay();
}

function Destroyed()
{
	if( Laser != None )
		Laser.Destroy();

	StopFiring();

	Super.Destroyed();
}

function Died(Controller Killer, class<DamageType> damageType, vector HitLocation, optional name BoneName )
{
	super.Died(Killer, damageType, HitLocation, BoneName);
	MakeTransition(DeadState);
}

function FireProjectile()
{
	local vector Start;
	local Rotator Aim;

	Start = GetBoneLocation('muzzle');
	Aim = GetBoneRotation('muzzle');

	AmmoType.DoFire(Start, Aim);
	DoMuzzleEffect('muzzle', Start);

	if( Laser != None )
		Laser.bHidden = false;
}

simulated function DoMuzzleEffect(name MuzzleBone, vector Start)
{
	local Actor Effect;	

	if ( MuzzleEffect != None )
	{
		Effect = Spawn(MuzzleEffect,,, Start);
		AttachToBone(Effect, MuzzleBone);	
		Effect.SetRelativeLocation(vect(0,0,0)); 
		Effect.SetRelativeRotation(rot(0,0,0));
	}
}

simulated function bool CanChangeAnims()
{
	return false;
}

function Fire(optional float F)
{	
	if (bIsAuto)
	{		
		if (bFire && !bCooldown)
		{			
			if ( (TimeUntilCooldown > 0.0) && (Level.TimeSeconds >= (BeginFireTime + TimeUntilCooldown)) )
			{
				bCooldown = true;
				BeginFireTime = -1.0;
				StopFiring();
				return;
			}
			if ( TeamIndex > 2 )
				MakeTransition(OnStateEnemy);
			else
				MakeTransition(OnStateFriend);
			PlayFire();		
		}
		if (bCooldown)
		{			
			if (BeginCooldownTime < 0.0)
			{
				if ( TeamIndex > 2 )
					MakeTransition(CoolStateEnemy);
				else
					MakeTransition(CoolStateFriend);
				BeginCooldownTime = Level.TimeSeconds;
			}
			if ( Level.TimeSeconds >= (BeginCooldownTime + CooldownDuration) )
			{				
				bCooldown = false;
				BeginCooldownTime = -1.0;
			}
		}
	}
	else
		PlayFire();
}

simulated function Name GetMuzzleBone()
{
	return 'muzzle';
}

simulated event vector GetMuzzleLocation()
{
	return GetBoneLocation(GetMuzzleBone());
}

simulated function PlayFire()
{	
	if ( FireSound != None && FireSound.Looping )
	{
		if( !bLoopingFireSound )
			PlayOwnedSound(FireSound);
		bLoopingFireSound = true;
		CurrentLoopingFireSound = FireSound;
	}
	else
		PlayOwnedSound(FireSound);
	PlayAnim('Fire',,,,4);
	FireProjectile();
	LastFireTime = Level.TimeSeconds;
	if (Gunner != None)
		Gunner.PlayAnim(PawnFireAnim(),'Fire',,,3);
}

function AltFire(optional float F)
{
	Fire();
}

simulated function StopFiring()
{
	if( Laser != None && !Laser.bHidden )
	{
		Laser.bHidden = true;
		if (AmmoType != None)
		{
			if( AmmoType.CurrentHitEffect != None )
			{
				AmmoType.CurrentHitEffect.FadeOut();
				AmmoType.CurrentHitEffect = None;
			}
			if (AmmoType.TracerEmitter != None)
			{
				AmmoType.TracerEmitter.FadeOut();
				AmmoType.TracerEmitter = None;
			}
		}
	}

	bFire = false;

	if (IsSoundActive(LoopRotateSound))
		StopTurretSound();
	if (IsSoundActive(FireSound))
	{		
		StopSound(self, CurrentLoopingFireSound, 0);
	}
	bLoopingFireSound = false;
}

function vector GetAttachPosition()
{
	return GetBoneLocation('pawnAttach');
}

function vector GetCameraLocation()
{
	return GetBoneLocation('camera');
}

function rotator GetCameraRotation()
{
	return GetBoneRotation('camera');
}

function ChangeAngle( float NewYaw, float NewPitch, float DeltaTime )
{
	local rotator NewPitchRotation, NewYawRotation;
	local float SpeedMax;

	SpeedMax = Speed * DeltaTime;
	NewYaw = FClamp( NewYaw, -SpeedMax, SpeedMax );
	NewPitch = FClamp( NewPitch, -SpeedMax, SpeedMax );

	if ( bFire && ((abs(NewYaw) > 100) || (abs(NewPitch) > 100)) )
	{
		if (!bPlayedStartSound)
		{
			PlayOwnedSound(StartRotateSound);
			bPlayedStartSound = true;
		}
		else if (!IsSoundActive(LoopRotateSound))
			PlayOwnedSound(LoopRotateSound);
	}
	// eventhough player is turning the turret, sometimes NewYaw/NewPitch = 0 because it doesn't update the input fast enough
	// check for a couple of consecutive 0's before stopping - ugh.
	else if (IsSoundActive(LoopRotateSound) && (LastNewYaw == 0) && (LastNewPitch == 0) )
		StopTurretSound();

	LastNewYaw = NewYaw;
	LastNewPitch = NewPitch;
	Yaw += NewYaw;
	Pitch += NewPitch;	
	
	bFire = true;
	if (!bUnlimitedYaw)
	{
		if( Yaw > MaxYaw )
		{
			Yaw = MaxYaw;			
			StopFiring();
		}
		else if( Yaw < MinYaw )
		{
			Yaw = MinYaw;			
			StopFiring();
		}
	}

	if( Pitch > MaxPitch )
	{
		Pitch = MaxPitch;		
		StopFiring();
	}
	else if( Pitch < MinPitch )
	{
		Pitch = MinPitch;		
		StopFiring();
	}

	Yaw = Yaw & 65535;
	if (Yaw > 32768)
		Yaw -= 65536;

	Pitch = Pitch & 65535;
	if (Pitch > 32768)
		Pitch -= 65536;

	//Yaw = Yaw % 65535;
	//Pitch = Pitch % 65535;

	NewYawRotation.Yaw = Yaw;
	NewPitchRotation.Pitch = Pitch;

	/*
	if( NewYawRotation.Yaw < 0 )
		NewYawRotation.Yaw += 65535;

	if( NewPitchRotation.Pitch < 0 )
		NewPitchRotation.Pitch += 65535;
	*/

	NewPitchRotation.Yaw = NewYawRotation.Yaw;

	SetBoneRotation(YawJoint,NewYawRotation,SPACE_Mesh);	
	SetBoneRotation(PitchJoint,NewPitchRotation,SPACE_Mesh);	
}

// Rail movement	
function ChangeLocation( int StrafeDelta );

function UsedBy( Pawn User )
{
	if( User == Gunner )
		LeaveTurret();
    else if( IsAvailableFor( User ) )
		ManTurret(User);		
}

simulated function PlayerManTurret()
{	
	Gunner.CreateInventory( ""$FPWeaponClass );
	PlayerController(Controller).SwitchWeapon(0);
	//Gunner.ChangedWeapon();
	PlayerController(Controller).GotoState('PlayerManningTurret');	
	if (PlayerController(Controller).ViewTarget == Gunner)
		bHidden = true;		
}


function ManTurret( Pawn User )
{	
	local vector TurretBoneOffset;

	if (SquadMarker != None && SquadMarker.bMarkerActive && SquadMarker.Participants[0].Member != User)
		SquadMarker.CancelMarker(None);

	Gunner = User;
	Controller = User.Controller;
	TeamIndex = Gunner.TeamIndex;
	Instigator = User;
	bResetTurret = false;	
	bValidEnemy = true;

	TurretBoneOffset.Z = Gunner.CollisionHeight;

	Gunner.bPhysicsAnimUpdate = false;
	if( Gunner.IsHumanControlled() )
		SetCollision(false, false, false);
	else
		Gunner.SetCollision(true, false, false);
	Gunner.bCollideWorld = false;	
	Gunner.SetPhysics(PHYS_RootMotion);		

	Gunner.Velocity = vect(0,0,0);
	Gunner.Acceleration = vect(0,0,0);	
	AttachToBone(Gunner,'pawnAttach');
	Gunner.SetRelativeLocation(TurretBoneOffset); 
	Gunner.SetRelativeRotation(rot(0,0,0));

	// Have to clear out all animation channels to avoid any wierdness
	Gunner.PlayAnim( '', , , , 1 );
	Gunner.PlayAnim( '', , , , 2 );
	Gunner.PlayAnim( '', , , , 3 );

	if (Gunner.Weapon != None)
		Gunner.Weapon.ThirdPersonActor.bHidden = true;

	//Play your standing on turret animation
	if( SitToUse )
		Gunner.CurrentIdleState = AS_SitTurret;		
	else
		Gunner.CurrentIdleState = AS_StandTurret;		
	
	Gunner.ChangeAnimation();

	Controller.Machine = self;	
	if( Controller.IsA( 'PlayerController' ) )
	{
		TriggerEvent( PlayerControlEvent, Self, Instigator );
		PlayerManTurret();
	}
	else
		GotoState('AutoTurret');

	TriggerEvent( ControlEvent, Self, Instigator );	
}

function LeaveTurret()
{
	local Inventory inv;
	local rotator NewRotation;	

	Gunner.bCollideWorld = true;
	Gunner.SetCollision(true, true, true);	
	Gunner.bPhysicsAnimUpdate = true;	
	Gunner.SetPhysics(PHYS_Walking);

	Gunner.CurrentIdleState = AS_Alert;
	Gunner.ChangeAnimation();

	NewRotation = Gunner.Rotation;
	NewRotation.Pitch = 0;
	NewRotation.Roll = 0;
	Gunner.SetRotation( NewRotation );

	if( Controller.IsA( 'PlayerController' ) )
	{
		SetCollision(true, true, true);
		if (Gunner != None && Gunner.Weapon != None && Gunner.Weapon.ThirdPersonActor != None)
			StopSound(Gunner.Weapon.ThirdPersonActor, FireSound, 0.1);
		PlayerController(Controller).SetViewTarget(Gunner);	
		Gunner.SetLocation( NavPts[0].Location );
		Controller.GotoState('PlayerWalking');
		Gunner.SwitchToLastWeapon();
		Gunner.ChangedWeapon();
		inv = Gunner.FindInventoryType(FPWeaponClass);
		Gunner.DeleteInventory(inv);

		if (PlayerController(Controller).ViewTarget == Gunner)
			bHidden = false;
	}

	if (Gunner.Weapon != None)
		Gunner.Weapon.ThirdPersonActor.bHidden = false;

	bIsAuto=false;
	StopFiring();
	GotoState('Resetting');
	Gunner = None;
	Controller = None;
	TeamIndex = 0;
	TriggerEvent( ReleaseEvent, Self, Instigator );
}

// called by automated turrets
function DetectTarget( float DeltaTime )
{	
	local vector DesiredDirection;
	local vector FocusLocation;
	local rotator DesiredRotation;	
	local rotator PitchJointRotation, YawJointRotation;
	local rotator Temp;
	local rotator PitchRot;
	local float PitchDesired;
	local float Speed;

	YawJointRotation = GetBoneRotation( YawJoint );
	PitchJointRotation = GetBoneRotation( PitchJoint );
	FocusLocation = Controller.GetFocusLocation();
	if (bUseSmartShot && Controller.Target != None && Controller.bSmartShot && Controller.bEnemyIsPawn)
	{
		FocusLocation.Z += Abs(-1.0f + 2.0f * Frac(0.5f * Level.TimeSeconds)) * Controller.Enemy.CollisionHeight;
	}
	DesiredDirection = FocusLocation - GetBoneLocation( YawJoint );

	Temp = rotator( DesiredDirection << YawJointRotation );
	DesiredRotation.Yaw = Temp.Yaw;

	//DesiredDirection = FocusLocation - GetBoneLocation( PitchJoint );
	DesiredDirection = FocusLocation - GetMuzzleLocation();
	
	//Adjust pitch every so often based on gravity, but only bother if there's an enemy
	if (Controller.Enemy == None)
	{
		LastPitchAdjustment = 0;
		DesiredDirection.Z = 0;
	}
	else if (Level.TimeSeconds - LastPitchAdjustTime > 0.5)
	{
		LastPitchAdjustTime = Level.TimeSeconds;
		if (AmmoType.ProjectileClass != None && AmmoType.ProjectileClass.default.Physics == PHYS_Falling)
		{
			Temp = rotator(DesiredDirection);
			Speed = (AmmoType.ProjectileClass.default.MaxSpeed + AmmoType.ProjectileClass.default.Speed)/2.0;

			PitchDesired = CalculateAimPitch(Speed, DesiredDirection.Z, VSize2D(DesiredDirection), CurrentPhysicsVolume.Gravity.Z);
			LastPitchAdjustment = PitchDesired - Temp.Pitch;
			//Log("Turret Pitch Desired: "$PitchDesired$" Adjustment: "$LastPitchAdjustment);
		}
	}
	PitchRot.Pitch = LastPitchAdjustment;
	//if (LastPitchAdjustTime == Level.TimeSeconds)
	//	Log("DesiredDirection before pitch adjustment: "$DesiredDirection);
	DesiredDirection = DesiredDirection << PitchRot;
	//if (LastPitchAdjustTime == Level.TimeSeconds)
	//	Log("DesiredDirection after pitch adjustment: "$DesiredDirection);
	Temp = rotator( DesiredDirection << PitchJointRotation );
	DesiredRotation.Pitch = Temp.Pitch;
	ChangeAngle( DesiredRotation.Yaw, DesiredRotation.Pitch, DeltaTime );	

	// find out when the turret is firing for the first time after each cool down
	if ( bFire && !bCooldown && (BeginFireTime < 0.0) && (BeginCooldownTime < 0.0) )
		BeginFireTime = Level.TimeSeconds;
}

function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
{
	local vector FacingDir;

	FacingDir = vector( GetBoneRotation( YawJoint ) );
	if( ( FacingDir Dot Normal(-Momentum) ) > ArmorAngle )
		Damage *= ( 1.0 - ArmorFactor );

	// REVISIT: We don't have the support code for Clone's going into incapacitated from Turrets
	// so the last term here makes sure that squadmates don't take damage while on turrets
	if ( Gunner != None && Gunner.IsHumanControlled() )
	{		
		Gunner.TakeDamage( CalculateDamageFrom(DamageType, Damage), instigatedBy, hitlocation, momentum, damageType );
		if (Gunner.Health <= 0)
		{
			GotoState('Resetting');
			Gunner.Health = 0.5;
		}
	}
	else if( Damagable )
		return Super.TakeDamage( Damage, instigatedBy, hitlocation, momentum, damageType, BoneName );
	
	return 0;
}

// This function is used by the editor to indicate to the search function
// that an event is important to this guy
function bool ContainsPartialEvent(string StartOfEventName)
{
	local string EventString;
	EventString = string(ControlEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}

	EventString = string(ReleaseEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}

	return super.ContainsPartialEvent(StartOfEventName);
}


function bool ResetTurret(float DeltaTime)
{
	local vector DesiredDirection;
	local rotator YawJointRotation, PitchJointRotation;
	local rotator DesiredRotation, Temp;

	// we put the check first to avoid any unnecessary work while waiting for other parts of the turret to reset
	if( abs( Pitch ) < 10 && abs( Yaw ) < 10 )
		return true;

	YawJointRotation = GetBoneRotation( YawJoint );
	PitchJointRotation = GetBoneRotation( PitchJoint );
	
	DesiredDirection = vector(Rotation);

	Temp = rotator( DesiredDirection << YawJointRotation );
	DesiredRotation.Yaw = Temp.Yaw;

	Temp = rotator( DesiredDirection << PitchJointRotation );
	DesiredRotation.Pitch = Temp.Pitch;	
	
	ChangeAngle( DesiredRotation.Yaw, DesiredRotation.Pitch, DeltaTime );	

	return false;	
}

simulated function StopTurretSound()
{
	StopSound(self, LoopRotateSound, 0);
	PlayOwnedSound(StopRotateSound);
	bPlayedStartSound = false;
}

function MakeTransition(TurretInfo TurretState)
{	
	if (TurretState.TransitionMaterial != None)
	{
		CopyMaterialsToSkins();
		Skins[TurretState.TransitionMaterialIndex] = TurretState.TransitionMaterial;
	}
	if (TurretState.TransitionSound != None)
		PlaySound(TurretState.TransitionSound);
	if ( TurretState.TransitionEffect != None )
		Spawn(TurretState.TransitionEffect,,,self.Location+TurretState.TransitionEffectOffset);
	if ( TurretState.TransitionEvent != 'None' )
		TriggerEvent( TurretState.TransitionEvent, Self, None );
}

state () Disabled
{
	function BeginState()
	{
		bPlayerUsable=false;
		bStasis=false;
		bValidEnemy=false;
		Enable('Tick');
		StopFiring();									
		TeamIndex = 0;
		if (SquadMarker != None && SquadMarker.bMarkerActive)
			SquadMarker.GotoState('Disabled');
	}
	function Tick(float DeltaTime)
	{
		if (ResetTurret(DeltaTime))
		{
			bResetTurret = true;
			if (Gunner != None)
			{
				if (Gunner.Controller.bIsPlayer)
					LeaveTurret();
			}
		}
	}
}

state Resetting
{
	function BeginState()
	{
		//Log("Turret::Resetting::BeginState");
		bStasis = false;
		bValidEnemy = false;
		StopFiring();
		Enable('Tick');
	}

	function Tick(float DeltaTime)
	{
		if( ResetTurret( DeltaTime ) )
		{
			GotoState('Inactive');
		}
	}

	function EndState()
	{
		Release();
		bResetTurret = true;
	}
}

auto state Inactive
{	
	simulated function BeginState()
	{
		Disable('Tick');
		if ( TeamIndex > 2 )
			MakeTransition(OffStateEnemy);
		else
			MakeTransition(OffStateFriend);
		if (IsSoundActive(LoopRotateSound))
			StopTurretSound();

		bStasis = true;
	}
}

state AutoTurret
{
	simulated function BeginState()
	{			
		if ( TeamIndex > 2 )
			MakeTransition(OnStateEnemy);
		else
			MakeTransition(OnStateFriend);
		BeginFireTime = -1.0f;
		BeginCooldownTime = -1.0f;
		bIsAuto = true;	
		bResetTurret = false;
		bStasis = false;
		Enable('Tick');
	}

	function Tick(float DeltaTime)
	{
		if( Gunner == None || Gunner.bDeleteMe || Gunner.IsDead() ||
			Controller == None || Controller.bDeleteMe )
		{
			Gunner = None;
			Controller = None;			
			StopFiring();									
			GotoState('Resetting');
			TeamIndex = 0;
			return;
		}
		else if (Gunner != None && Controller.Machine == None) //shouldn't happen, but does on rare occasions
		{
			Log("Turret logic broken... unmanning turret");
			LeaveTurret();
			return;
		}

		TurretDeltaTime = deltaTime;
		if (Controller.Focus != Controller.Pawn)
			DetectTarget( DeltaTime ); //looks at focus location
		if( Controller.Enemy == None || !Controller.Enemy.bValidEnemy || Controller.Enemy.bDeleteMe )
			StopFiring();

		if( bFire && bIsAuto && ((Level.TimeSeconds - LastFireTime) > ShotDelay) )
			Fire();
	}

	simulated function EndState()
	{		
		bIsAuto = false;		
	}
}

function AddDefaultInventory()
{
    SpawnAccessories();
}



cpptext
{
	virtual ANavigationPoint* GetAnchor();
	virtual bool CanAimAt(const FVector& AimLoc);
	virtual UBOOL ShouldTrace(AActor *SourceActor, DWORD TraceFlags);

}

defaultproperties
{
     PitchJoint="pivotPitch"
     YawJoint="pivotYaw"
     MinPitch=-8194
     MaxPitch=8194
     MinYaw=-8194
     MaxYaw=8194
     Speed=10000
     FireSound=Sound'GEN_Sound.Weapons.wep_RAS_turretFire_01'
     bUseSmartShot=True
     bDeployed=True
     bPlayerUsable=True
     ShotDelay=0.5
     StartRotateSound=Sound'GEN_Sound.Weapons.wep_GEN_turretAim_st_01'
     LoopRotateSound=Sound'GEN_Sound.Weapons.wep_GEN_turretAim_lp_01'
     StopRotateSound=Sound'GEN_Sound.Weapons.wep_GEN_turretAim_nd_01'
     bDisallowPainAnims=True
     bNeverMeleeMe=True
     bNoDefaultInventory=True
     bCanBeBaseForPawns=True
     Health=50
     DamageMultipliers(0)=(BoneName="Head",Multiplier=2)
     MinImpactInterval=-1
     ControllerClass=None
     Physics=PHYS_None
     bDrawHUDMarkerIcon=True
     bMovable=False
     bCollideWorld=False
     bPathColliding=True
}

