// ====================================================================
//  Class:  CTGame.Prop
//  Parent: Engine.Ammunition
//
//  Base class for Clone Trooper Turrets
// ====================================================================

class Prop extends Actor
	native
	placeable
	hidecategories(Force,Karma);

struct TransitionInfo
{
	var()	Name				Event;						// The event to trigger on entering a state
	var()	class<Emitter>		TransitionEffect;			// The effect to spawn on entering the state
	var()	Vector				TransitionEffectOffset;		// The offset of the transition effect
	var()	Sound				TransitionSound;			// The sound to play on entering the state	
	var()	Material			Material;					// The material to use in this state
	var()	StaticMesh			Mesh;						// The static mesh to use in this state
	var()	vector				ImpulseDir;					// Direction of Karma Impulse
	var()	float				ImpulseMagnitude;			// Magnitude of Karma Impulse
	var()	class<DamageType>	ImpulseDamageType;			// Type of Karma Impulse
	var()	bool				bDisableCollision;			// Whether to use disable collision in this state
	var()	bool				bAttachEffect;				// Whethter to attach effect to prop	
	var()	bool				bPreserveEffect;			// Keep Effect alive after state change	
	var()	EStimulusType		TransitionStimulus;
	var()	float				StateLifeSpan;				// The amount of time before the object disappears once it hits destroyed state	
	var()	float				StateLifeSpanMax;
	var		Emitter				SpawnedEmitter;				// The emitter that was spawned for this state		
};

var()	int					Health;				// The total health of the object
var()	float				DamagedRatio;		// The point at which the prop will switch from healthy to damaged
												// a value of 0 will skip the Damaged state
var		int					InitialHealth;

var()	float				ExplosionRadius;			// The radius of the explosion when transitioning into the destroyed state
var()	float				ExplosionDamage;			// The explosion damage when transitioning into the destroyed state
var()	class<DamageType>   ExplosionDamageType;		// The type of damage for the explosion
var()	class<StatusEffect> ExplosionStatusEffect;		// The status effect to spawn as part of the explosion
var()	float				StatusEffectMaxDuration;	// The max duration for the status effect
var()	float				StatusEffectDamagePerSec;	// How much damage per second to do from the effect
var()	EStimulusType		ExplosionAIStimulus;		// Stimulus on explosion
var()	class<Actor>		DeathSpawnActorClass;		// A class of actor to spawn on prop destruction
var()	TransitionInfo		DamagedState;
var()	TransitionInfo		DestroyedState;

var()	bool				bDisplayStateMessages;



function PreBeginPlay()
{
	InitialHealth = Health;
	super.PreBeginPlay();
}

function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
{
	local int ActualDamage;
	ActualDamage = CalculateDamageFrom(damageType, Damage);
	Instigator = instigatedBy;
	return ActualDamage;
}

function vector GetEffectLocation (Vector Offset)
{
	local vector X, Y, Z, Out;
	GetAxes(Rotation, X, Y, Z);
	Out = Location + X * Offset.X + Y * Offset.Y + Z * Offset.Z;
	return out;
}

function bool OkayToDestroyAnimProp()
{
	return true;
}

function Destroyed()
{
	local Actor NewActor;
	local Pawn NewPawn;
	local Controller NewController;
	local rotator NewRotation;

	if( DeathSpawnActorClass != None )
	{
		SetCollision(false,false,false);
		NewActor = Spawn( DeathSpawnActorClass );

		if( NewActor != None && NewActor.IsA('Pawn') ) 
		{
			NewPawn = Pawn(NewActor);

			NewRotation = NewPawn.Rotation;
			NewRotation.Pitch = 0;
			NewRotation.Roll = 0;
			NewPawn.SetRotation( NewRotation );

			if( NewPawn.ControllerClass != None && NewPawn.Controller == None )
				NewController = spawn( NewPawn.ControllerClass );
			if( NewController != None )		
				NewController.Possess( NewPawn );

			if( NewPawn.Controller != None )
				NewPawn.AddDefaultInventory();
		}
	}

	if( DamagedState.SpawnedEmitter != None )
		DamagedState.SpawnedEmitter.FadeOut();
	if( DestroyedState.SpawnedEmitter != None )	
		DestroyedState.SpawnedEmitter.FadeOut();

	Super.Destroyed();
}

// Ask whether an event belongs to this actor; used in the editor
function bool ContainsPartialEvent(string StartOfEventName)
{
	local string EventString;
	EventString = string(DamagedState.Event);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}
	EventString = string(DestroyedState.Event);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}
	return super.ContainsPartialEvent(StartOfEventName);
}


auto state() Invulnerable
{
	function BeginState()
	{
		if( Health > 0 )
			GotoState('Damagable');
		else if( bDisplayStateMessages )
		{
			Level.Game.Broadcast(self, ""$self$" Is Invulnerable" );
		}
	}
}

state() Damagable
{
	function BeginState()
	{
		if( bDisplayStateMessages )		
			Level.Game.Broadcast(self, ""$self$" Is Damagable" );		
	}

	function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
	{		
		local int ActualDamage;
		ActualDamage = CalculateDamageFrom(damageType, Damage);		

		Instigator = instigatedBy;
		Health -= ActualDamage;
		if( DamagedRatio == 0.0 && Health <= 0 )
			GotoState('PropDestroyed');
		else if( Health <= (InitialHealth * DamagedRatio) )
			GotoState('PropDamaged');

		return ActualDamage;
	}
}

state() PropDamaged
{
	function BeginState()
	{
		if( bDisplayStateMessages )		
			Level.Game.Broadcast(self, ""$self$" has been Damaged" );		

		TriggerEvent( DamagedState.Event, Self, None );

		if (DamagedState.TransitionStimulus != ST_Invalid)
			PostStimulusToWorld(DamagedState.TransitionStimulus);

		if( DamagedState.TransitionEffect != None )
		{
			if( DamagedState.bAttachEffect )
			{				
				DamagedState.SpawnedEmitter = spawn( DamagedState.TransitionEffect, self, , Location + DestroyedState.TransitionEffectOffset );								
				DamagedState.SpawnedEmitter.SetBase( self );				
				DamagedState.SpawnedEmitter.SetRelativeLocation( DamagedState.TransitionEffectOffset );
				DamagedState.SpawnedEmitter.SetRelativeRotation( rot(0,0,0) );				
			}
			else
				DamagedState.SpawnedEmitter = spawn( DamagedState.TransitionEffect, , , GetEffectLocation( DamagedState.TransitionEffectOffset ) );
		}

		if( DamagedState.TransitionSound != None )
			PlaySound(DamagedState.TransitionSound);

		if( DamagedState.Material == None && ( DamagedState.Mesh == None && !IsA('AnimProp') ) )
		{
			Destroy();
		}
		else
		{
			if( DamagedState.bDisableCollision )
				SetCollision( bCollideActors, !DamagedState.bDisableCollision, !DamagedState.bDisableCollision );

			if( DamagedState.Mesh != None )
				SetStaticMesh(DamagedState.Mesh);
			if( DamagedState.Material != None )
			{
				CopyMaterialsToSkins();
				Skins[0] = DamagedState.Material;
			}
		}

		if( DamagedState.StateLifeSpan > 0 )
			SetTimer( RandRange( DamagedState.StateLifeSpan, DamagedState.StateLifeSpan + DamagedState.StateLifeSpanMax ), false );
	}

	function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
	{		
		local int ActualDamage;
		ActualDamage = CalculateDamageFrom(damageType, Damage);	

		Instigator = instigatedBy;
		Health -= ActualDamage;
		if( Health <= 0 )
			GotoState('PropDestroyed');

		return ActualDamage;
	}

	function Timer()
	{
		TakeDamage( Health, none, Location, vect(0,0,0), class'DamageType' );		
	}

	function EndState()
	{ 
		if( !DamagedState.bPreserveEffect && DamagedState.SpawnedEmitter != None )
			DamagedState.SpawnedEmitter.FadeOut();
	}
}

//NathanM: Note that this does not mean destroyed (i.e. deleted) in the classic Unreal sense.
//Destroyed was the name the LDs seemed most comfortable with.
state() PropDestroyed 
{
	function BeginState()
	{
		if( bDisplayStateMessages )		
			Level.Game.Broadcast(self, ""$self$" has been Destroyed" );		

		bValidEnemy=false;

		if( Event != '' )
			TriggerEvent( Event, Self, None );
		else //Trigger Event will check if DestroyedEvent is not ""
			TriggerEvent( DestroyedState.Event, Self, None );

		if (DestroyedState.TransitionStimulus != ST_Invalid)
			PostStimulusToWorld(DestroyedState.TransitionStimulus);

		if( DestroyedState.StateLifeSpan > 0 )
  			LifeSpan = RandRange( DestroyedState.StateLifeSpan, DestroyedState.StateLifeSpan + DestroyedState.StateLifeSpanMax );

		if( DestroyedState.TransitionEffect != None )
		{
			if( DestroyedState.bAttachEffect )
			{				
				DestroyedState.SpawnedEmitter = spawn( DestroyedState.TransitionEffect,self, , Location + DestroyedState.TransitionEffectOffset );				
				DestroyedState.SpawnedEmitter.SetBase( self );
				DestroyedState.SpawnedEmitter.SetRelativeLocation( DestroyedState.TransitionEffectOffset );
				DestroyedState.SpawnedEmitter.SetRelativeRotation( rot(0,0,0) );				
			}
			else
				DestroyedState.SpawnedEmitter = spawn( DestroyedState.TransitionEffect, , , GetEffectLocation( DestroyedState.TransitionEffectOffset ) );
		}

		if( DestroyedState.TransitionSound != None )
			PlaySound(DestroyedState.TransitionSound);

		if( ( ExplosionRadius > 0 && ExplosionDamage > 0 ) || ExplosionStatusEffect != None )
		{
			GotoState( 'Exploding' );
		}
		else if( DestroyedState.Material == None && DestroyedState.Mesh == None && OkayToDestroyAnimProp() )
		{
			Destroy();
		}
		else
		{
			if( DestroyedState.bDisableCollision )
				SetCollision( bCollideActors, !DestroyedState.bDisableCollision, !DestroyedState.bDisableCollision );

			if( DestroyedState.Mesh != None )
			{
				SetStaticMesh(DestroyedState.Mesh);
			}
			if( DestroyedState.Material != None )
			{
				CopyMaterialsToSkins();
				Skins[0] = DestroyedState.Material;
			}

			if( DestroyedState.StateLifeSpan > 0 )
				SetTimer( RandRange( DestroyedState.StateLifeSpan, DestroyedState.StateLifeSpan + DestroyedState.StateLifeSpanMax ), false );
		}

				//If this was being used as a hot spot for a marker
		//and the marker isn't currently active, make it obsolete
		if (SquadMarker != None && !SquadMarker.bMarkerActive)
		{
			SquadMarker.MakeObsolete();
		}		
	}

	function Timer()
	{
		Destroy();
	}

	function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
	{
		return 0;
	}
}

state Exploding
{
	function Timer()
	{
		if( ExplosionDamageType != None )
			HurtRadius( ExplosionDamage, ExplosionRadius, ExplosionDamageType, ExplosionDamageType.default.KDamageImpulse, Location );
		else
			HurtRadius( ExplosionDamage, ExplosionRadius, ExplosionDamageType, 0, Location );

		if (ExplosionAIStimulus != ST_Invalid)
			PostStimulusToWorld(ExplosionAIStimulus);

		if( ExplosionStatusEffect != None )
			ExplosionStatusEffect.static.CreateStatusEffects( self, StatusEffectMaxDuration, ExplosionRadius, StatusEffectDamagePerSec );

		if( DestroyedState.Material == None && DestroyedState.Mesh == None && OkayToDestroyAnimProp() )
		{
			Destroy();
		}
		else
		{
			if( DestroyedState.bDisableCollision )
				SetCollision( bCollideActors, !DestroyedState.bDisableCollision, !DestroyedState.bDisableCollision );

			if( DestroyedState.Mesh != None )
			{
				SetStaticMesh(DestroyedState.Mesh);
			}
			if( DestroyedState.Material != None )
			{
				CopyMaterialsToSkins();
				Skins[0] = DestroyedState.Material;
			}
		}
	}

	function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
	{
		return 0;
	}

	function BeginState()
	{
        SetTimer( RandRange( 0.1, 0.2 ), false );
	}
}


defaultproperties
{
     DrawType=DT_StaticMesh
     bLightingVisibility=False
     bWorldGeometry=True
     bUseHWOcclusionTests=True
     bCollideActors=True
     bBlockActors=True
     bBlockPlayers=True
     bProjTarget=True
     bBlockKarma=True
     CollisionRadius=1
     CollisionHeight=1
     bEdShouldSnap=True
     bPathColliding=True
}

