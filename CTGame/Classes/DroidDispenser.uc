//=============================================================================
// Droid Dispensor
//
// Created: 2004, February 10, Jenny Huang
//=============================================================================

class DroidDispenser extends PawnFactory
	showcategories(Collision)
	notplaceable;

struct TransitionInfo
{
	var()	static class<Emitter>	TransitionEffect;
	var()	static Vector			TransitionEffectOffset;	
	var()	static Sound			TransitionSound;
	var()	static Material			TransitionMaterial;
	var()	Name					TransitionEvent;
	var()	static Name				TransitionAnim;
	var()	static bool				bAnimLoop;
};

var()	float				MaxHealth;				// Max health
var		float				Health;					// current health
var()	float				WarmUpTime;				// time between offline and online as the dispense blinks on
var		bool				bInitDispense;			// initial dispense
var		bool				bWarmUp;				// is it in the warmup stage between offline and online?
var		bool				bBeginDispense;			// begin to dispense
var		bool				bSlightlyDamaged;		// has it reached the stage of being slightly damaged? makeTransition once
var		bool				bHeavilyDamaged;		// has it reached the stage of being heavily damaged? makeTransition once
var()	bool				DestroyBlockersOnDeath; // destroy blocking volumes on death rather than Destroyed()
var		bool				bCheckExhausted;		// if there's an ExhaustedEvent, check during tick of damage state
var		bool				bDispensingDroid;		// are we in the middle of dispensing a droid?
var		int					NumDispensers;			// number of dispensers
var		int					OrigMaxAlive;			// original number of maxAlive
var()	int					NumInitDroids;			// number of initial droids
var()	TransitionInfo		WarmUpInfo;				// effects, sound, material, and events upon entering the offline's warmup state
var()	TransitionInfo		OnlineInfo;				// effects, sound, material, and events upon entering the online state
var()	TransitionInfo		SlightlyDamagedInfo;	// effects, sound, material, and events when health is less than 2/3 - remains in the online state
var()	TransitionInfo		HeavilyDamagedInfo;		// effects, sound, material, and events when health is less than 1/3 - remaind in the online state
var()	TransitionInfo		ShutdownInfo;			// effects, sound, material, and events upon entering the shutdown state
var()	TransitionInfo		DamagedInfo;			// effects, sound, material, and events upon entering the damaged state
var()	Sound				DispenseSound;			// sound to play each time a droid gets dispensed
var()	Name				IntroAnim;				// anim to play as the dispense first appears in the room
var()	Name				OnlineTag;				// tag to turn it online
var		Emitter				CurrentEffect;			// current effect on the dispenser if it wasn't already auto-destroyed
var()	staticmesh			BlockerMesh;			// Mesh to use for blockers
var()	array<name>			BlockerBones;			// Bones at which to spawn blockers
var		array<actor>		Blockers;				// Additional actors to handle oddly shaped protrusions for legs and doors and whatnot
var		array<name>			AttachBones;
var		array<name>			DispenseAnims;


// only SPM_Cyclic & SPM_Random are supported
function PostBeginPlay()
{
	local int i;

	super.PostBeginPlay();

	if ( (SpawnMode == SPM_FromFactory) || (SpawnMode == SPM_NearPlayer) )
		SpawnMode = SPM_Cyclic;
	SetCollision(true, true, true);
	PlayAnim('ActionBreathe');
	OrigMaxAlive = MaxAlive;
	Health = MaxHealth;

	if( BlockerMesh != None )
	{
		for( i = 0; i < BlockerBones.length; i++ )
		{
			Blockers[i] = Spawn( class'Accessory', , , GetBoneLocation(BlockerBones[i]), GetBoneRotation(BlockerBones[i]) );
			Blockers[i].SetStaticMesh( BlockerMesh ) ;
			AttachToBone( Blockers[i], BlockerBones[i] );
            Blockers[i].SetCollision( true, true, true );
			Blockers[i].bBlockNonZeroExtentTraces = true;
		}
	}

	//Log("Blockers Length "$Blockers.length);
}

function DestroyBlockers()
{
	local int i;

	for( i = 0; i < Blockers.length; i++ )
	{
		if( Blockers[i] != None )
			Blockers[i].Destroy();        
	}
}

function Destroyed()
{
	DestroyBlockers();
	Super.Destroyed();
}

function bool GenerateActor()
{	
	local int NumInWorld;
	local bool bSpawnOK;
	local Actor Other;

	if ( !bInitDispense && NumInitDroids > 0 )
	{
		MaxAlive = NumInitDroids;
		if ( SpawnedActors.Length >= MaxAlive )
		{
			MaxAlive = OrigMaxAlive;
			bInitDispense = true;
		}
	}

	RemoveDeadActor();

	if( SpawnedActors.Length < MaxAlive && ( RemainingItems > 0 || bGenerateForever ) )
	{
		if( MaxInWorld > 0 )
		{
			NumInWorld = 0;
			ForEach DynamicActors( InternalPrototype, Other )
			{
				if( !Other.IsA('Pawn') || Pawn(Other).Health > 0 )
					++NumInWorld;
			}

			if( NumInWorld >= MaxInWorld )
				return false;
		}

		if (DispenseSound != None)
			PlaySound(DispenseSound);
		SpawnedActors.Insert(0,1);	
		SpawnedActors[0] = spawn(InternalPrototype);
		if (SpawnedActors[0] != None)
		{
			bDispensingDroid = true;
			PostStimulusToWorld(ST_FootstepLoud);
			bSpawnOK = SetupNewActor(SpawnedActors[0], SpawnPoints[NextSpawnPoint]);
			SpawnedActors[0].LoopAnim('DispBreathe',,,,4);
			if( bSpawnOK && !bGenerateForever )
				RemainingItems--;		
			DetermineNextSpawnPoint(bSpawnOK);
			Dispense(SpawnedActors[0]);
			if ( bStartTimerAfterDeath && SpawnedActors.Length >= MaxAlive )
			{
				MaxAlive = OrigMaxAlive;
				bInitDispense = true;
				GotoState('Waiting');
			}
			return true;
		}
	}
	else if ( RemainingItems == 0 )
		GotoState('Shutdown');
	return false;
}

function bool SetupNewActor(Actor NewActor, SpawnPoint SpawnPt)
{
	super.SetupNewActor(NewActor, SpawnPt);
	Pawn(NewActor).SetShields( 0 );
	Pawn(NewActor).TeamIndex = 0;
	if( Pawn(NewActor).Controller != None )
		CTBot( Pawn(NewActor).Controller ).ForceState(class'StateDispensing',true);;
	return true;
}

function Dispense(Actor NewActor)
{
	local vector BoneOffset;
	local int i;

	NewActor.SetPhysics(PHYS_RootMotion);
	Pawn(NewActor).bPhysicsAnimUpdate = false;
	NewActor.SetCollision(false, false, false);
	NewActor.bCollideWorld = false;	

	//NewActor.Velocity = vect(0,0,0);
	//NewActor.Acceleration = vect(0,0,0);	

	if ( NextSpawnPoint < 0 )
		i = 0;
	else if ( NextSpawnPoint >= NumDispensers )
		i = NumDispensers - 1;
	else
		i = NextSpawnPoint;
	AttachToBone(NewActor, AttachBones[i]);
	PlayAnim(DispenseAnims[i]);

	BoneOffset.Z = NewActor.CollisionHeight;

	NewActor.SetRelativeLocation(BoneOffset); 
	NewActor.SetRelativeRotation(rot(0,0,0));
}

event Bump( Actor Other )
{
	if ( (Pawn(Other) != None) && (IsPlayingAnim('entranceSky') || IsPlayingAnim('EntranceSky704') || IsPlayingAnim('EntranceSky384')) )
		Other.TakeDamage( Pawn(Other).MaxHealth, None, Location, vect(0,0,0), class'Crushed' );
	super.Bump(Other);
}

function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
{
	local int actualDamage;	
	local float SlightlyDamagedThreshold, HeavilyDamagedThreshold;

	SlightlyDamagedThreshold = MaxHealth*0.66;
	HeavilyDamagedThreshold = MaxHealth*0.33;

	if ( DamageType == None )
	{
		if ( InstigatedBy != None )
		warn("No DamageType for damage by "$instigatedby$" with weapon "$InstigatedBy.Weapon);
		DamageType = class'DamageType';
	}

	// Check to make sure we're actually taking damage
	actualDamage = CalculateDamageFrom(DamageType, Damage);
	if (actualDamage <= 0)
		return 0;

	Health -= actualDamage;
	if ( !bSlightlyDamaged && Health <= SlightlyDamagedThreshold && Health > HeavilyDamagedThreshold )
	{
		MakeTransition(SlightlyDamagedInfo);
		bSlightlyDamaged = true;
	}
	else if ( !bHeavilyDamaged && Health <= HeavilyDamagedThreshold && Health > 0 )
	{
		MakeTransition(HeavilyDamagedInfo);
		bHeavilyDamaged = true;
	}
	else if (Health <= 0)
	{
		DestroyBlockers();
		GotoState('Damaged');
	}

	return actualDamage;
}

function MakeTransition(TransitionInfo Info)
{
	if (Info.TransitionSound != None)
		PlaySound(Info.TransitionSound);
	if (Info.TransitionEffect != None)
	{
		if (CurrentEffect != None)
			CurrentEffect.Destroy();
		CurrentEffect = Spawn(Info.TransitionEffect,,,self.Location + Info.TransitionEffectOffset);
		if (CurrentEffect != None)
			CurrentEffect.SetBase(self);
	}
	if ( Info.TransitionEvent != 'None' )
		TriggerEvent( Info.TransitionEvent, Self, None );
	if (Info.TransitionMaterial != None)
	{
		CopyMaterialsToSkins();
		Skins[0] = Info.TransitionMaterial;
	}
	if (HasAnim(Info.TransitionAnim))
	{
		if (Info.bAnimLoop)
			LoopAnim(Info.TransitionAnim);
		else
			PlayAnim(Info.TransitionAnim);
	}
}

function bool IsExhausted()
{
	if (IsInState('Damaged'))
		return true;
	return Super.IsExhausted();
}

// since we're playing one dispense anim at a time, SpawnedActors[0] gives us our new droid
event ReleaseDroid()
{
	if (SpawnedActors[0] != None)
	{
		bWorldGeometry = false;
		DetachFromBone(SpawnedActors[0]);
		SpawnedActors[0].GotoState('Dispense');
		TriggerEvent( SpawnEvent, self, None );
		bDispensingDroid = false;
		bWorldGeometry = true;
	}
}

auto state Offline
{
	function AnimEnd(int Channel)
	{
		if (!bBeginDispense)
			return;

		if( SpawnMode == SPM_FromFactory || SpawnPoints.length > 0 )
		{
			if (OnlineTag == 'None')
			{
				if (WarmUpTime > 0)
				{
					bWarmUp = true;
					MakeTransition(WarmUpInfo);
					SetTimer(WarmUpTime, false);
				}
				else
					GotoState('Online');        
			}
			else
			{
				Tag = OnlineTag;
				MakeTransition(WarmUpInfo);				// will we need this?
				GotoState('Wait');
			}
		}
		else
			Log( "Factory has no Spawn Points" );
	}

	function Trigger( actor Other, pawn EventInstigator )
	{
		bBeginDispense = true;
		if (HasAnim(IntroAnim))
			PlayAnim(IntroAnim);
		else
			PlayAnim('ActionBreathe');
	}

	function Tick(float DeltaTime)
	{
		if (bWarmUp)
		{
			if (WarmUpInfo.TransitionSound != None)
				PlaySound(WarmUpInfo.TransitionSound);
		}
	}

	function Timer()
	{
		bWarmUp = false;
		GotoState('Online');
	}
}

state Wait
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		GotoState('Online');
	}
}

state Online
{
	function BeginState()
	{
		super.BeginState();
		if (!bAdditionalTarget)
			AddToTargetList();
		MakeTransition(OnlineInfo);
	}

	function Timer()
	{
		SetFactoryTimer();
		bWorldGeometry = false;
		if( !GenerateActor() && RemainingItems == 0 && !bGenerateForever  )		
			TriggerEvent( ExhaustedEvent, self, None );
		bWorldGeometry = true;
	}
}

state Shutdown
{
	function BeginState()
	{
		Log("No more droids left!");
		MakeTransition(ShutdownInfo);
	}
}

state Damaged
{
	function BeginState()
	{
		//Log("Boom!");
		bValidEnemy=false;
		MakeTransition(DamagedInfo);
		if (SquadMarker != None && 
			SquadMarker.Anchor != None && SquadMarker.Anchor.IsA('ActivateItem'))
			SquadMarker.Anchor.GotoState('Disabled');
		if (bAdditionalTarget)
			RemoveFromTargetList();
		if ( bDispensingDroid && SpawnedActors[0] != None )
			SpawnedActors[0].TakeDamage( 100000, None, Location, vect(0,0,0), class'CTDamageExplosion' );
	}

	function Tick(float deltaTime)
	{	
		if( bCheckExhausted )
		{			
			RemoveDeadActor();
			if ( SpawnedActors.Length == 0 )	
			{
				TriggerEvent( ExhaustedEvent, self, None );
				bCheckExhausted = false;
			}
		}
	}
}


defaultproperties
{
     MaxHealth=5000
     bCheckExhausted=True
     NumDispensers=1
     AttachBones(0)="droidAttach1"
     AttachBones(1)="droidAttach2"
     AttachBones(2)="droidAttach3"
     AttachBones(3)="droidAttach4"
     DispenseAnims(0)="Dispense1"
     DispenseAnims(1)="Dispense2"
     DispenseAnims(2)="Dispense3"
     DispenseAnims(3)="Dispense4"
     SpawnInterval=3
     SpawnMode=SPM_Cyclic
     Physics=PHYS_RootMotion
     DrawType=DT_Mesh
     bHidden=False
     bWorldGeometry=True
     bAlignBottom=True
     bCollideActors=True
     bCollideWorld=True
     bBlockActors=True
     bBlockPlayers=True
     bProjTarget=True
     bBlockKarma=True
     bPathColliding=True
     bAdditionalTarget=True
     bValidEnemy=True
}

