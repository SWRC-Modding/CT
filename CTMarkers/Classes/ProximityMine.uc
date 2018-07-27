// ====================================================================
//  Class:  CTGame.ProximityMine
//
// Created by 2004, February 29, John Hancock
// Begun as duplicate of SetTrap. Kept SetTrap as
// separate in case we wanted to go back or use both
// ====================================================================

class ProximityMine extends ExplosiveCharge
	;

var		int					TeamIndex;
var()	static float		TriggerRadius;
var		float				NextCheckTime;
var		static StaticMesh	DetonationPreviewMesh;
var		Actor				DetonationPreviewActor;
var()	static float		AlertTimeLimit;
var		float				AlertBeginTime;
var(Sound)	static Sound		ActivateSound;
var(Sound)	static Sound		AlertSound;

function PostBeginPlay()
{
	local float scale;
	local vector loc;
	
	if (DetonationPreviewMesh != None && TriggerRadius > 1)
	{
		//Log("Created Bomb DetonationPreviewActor");
		loc = Location;
		loc.Z -= 90.0f;
		DetonationPreviewActor = Spawn(class'IntangibleActor',,,loc,Rotation);
		DetonationPreviewActor.SetStaticMesh(DetonationPreviewMesh);
		scale = TriggerRadius/512.0;
		DetonationPreviewActor.SetDrawScale(scale);
		DetonationPreviewActor.bHidden = true;
	}

	super.PostBeginPlay();
}

function InternalSetup()
{
	super.InternalSetup();
	if (User != None)
		TeamIndex = User.Pawn.GetTeamIndex();
}

event Destroyed()
{
	if (DetonationPreviewActor != None)
		DetonationPreviewActor.Destroy();
	super.Destroyed();
}

simulated function TacticalModeBegin( PlayerController Player )
{
	if (bActivated && DetonationPreviewActor != None)
		DetonationPreviewActor.bHidden = false;
}

simulated function TacticalModeEnd( PlayerController Player )
{
	if (Player.Target == self)
		return;
	if (DetonationPreviewActor != None)
		DetonationPreviewActor.bHidden = true;
}

simulated function NotifyTargeted( PlayerController Player )
{
	if (PropToDestroy == None || PropToDestroy.LifeSpan > 0 || PropToDestroy.bDeleteMe)
	{
		Destroy(); //destroy ourselves
		bIsUsable = false;
	}

	if (bActivated && DetonationPreviewActor != None)
		DetonationPreviewActor.bHidden = false;
	//TriggerEvent(EventTargeted, self, Player.Pawn);
}

simulated function NotifyUnTargeted( PlayerController Player )
{
	if (DetonationPreviewActor != None)
		DetonationPreviewActor.bHidden = true;
}


function Activate(Pawn Instigator)
{
	User = Instigator.Controller;
	TeamIndex = Instigator.GetTeamIndex();
	GotoState('Armed');
}

function bool DetectNearbyPawn()
{
	local Controller C;
	local TeamInfo BombTeam;

	BombTeam = Level.GRI.GetTeam(TeamIndex);
	for (C=Level.ControllerList; C!= None; C=C.NextController)
	{
		if (C.Pawn != None && !C.Pawn.bIsCrouched && !C.Pawn.IsDeadOrIncapacitated() && 
			BombTeam.IsEnemy(C.Pawn.GetTeamIndex()) &&
			VDistSq(C.Pawn.Location, Location) < Square(TriggerRadius))
		{
			return true;
		}
	}
	return false;
}


auto state() Unplaced
{
	function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
	{	
		//we're not really here, so any damage our collision model takes should
		//get passed on to the prop
		if (PropToDestroy != None)
			PropToDestroy.TakeDamage(Damage, instigatedBy, hitlocation, momentum, damageType, BoneName);
		if (PropToDestroy == None || PropToDestroy.bDeleteMe || PropToDestroy.IsInState('Exploding'))
		{
			Destroy();			
		}

		return Damage;
	}
}


state() Placed
{
	function BeginState()
	{
		Super.BeginState();
		bTriggeredByDamage=true;
	}
	function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
	{	
		if (bTriggeredByDamage)
		{
			GotoState('Explode'); //bypass Armed state, and go directly to explode
			return Damage;
		}
		else
			return 0;
	}
	//REVISIT JAH: we could just have a timer here that automatically goes
	//to armed...
}

state() Armed
{
	function Timer()
	{
		PlaySound(ActivateSound);
		GotoState('ProximityDetect');
	}
}

state() ProximityDetect
{
	function BeginState()
	{
	}

	function Tick(float deltaTime)
	{
		Global.Tick(deltaTime);

		if (TriggerRadius > 1 && Level.TimeSeconds > NextCheckTime)
		{
			NextCheckTime = Level.TimeSeconds + 0.25; //check every 1/4 second
			if (DetectNearbyPawn())
			{
				PlaySound(AlertSound);
				PostStimulusToWorld(ST_MineChirp);
				GotoState('ProximityAlert');
			}
		}
	}

}

state() ProximityAlert
{
	function BeginState()
	{
		AlertBeginTime = Level.TimeSeconds;
	}

	function Tick(float deltaTime)
	{
		Global.Tick(deltaTime);

		if (TriggerRadius > 1 && Level.TimeSeconds > NextCheckTime)
		{
			NextCheckTime = Level.TimeSeconds + 0.2; //check every .2 second
			if (DetectNearbyPawn())
			{
				PlaySound(AlertSound);
				//Log("Alert: Detected Pawn "$Level.TimeSeconds);
				if (Level.TimeSeconds >= AlertBeginTime + AlertTimeLimit)
					GotoState('Explode');
			}
			else
				GotoState('ProximityDetect');
		}
	}
}


defaultproperties
{
     TeamIndex=1
     TriggerRadius=512
     DetonationPreviewMesh=StaticMesh'MarkerIcons.SetTrap.DtatonRadius'
     AlertTimeLimit=1.5
     ActivateSound=Sound'GEN_Sound.Interface.int_GEN_proxMineFull_01'
     AlertSound=Sound'GEN_Sound.Interface.int_GEN_proxMineChirp_01'
     ChargeUnplaced=(Material=HardwareShader'MarkerIconTextures.Clone.MarkerDynamic_DemolitionChargeHolo')
     ChargePlaced=(TransitionSound=Sound'GEN_Sound.Impacts_Explos.imp_GEN_proxMineSet_01',Material=Shader'MarkerIconTextures.SetTrap.ProximityMine_Inactive',Duration=1)
     ChargeArmed=(TransitionSound=Sound'GEN_Sound.Interface.int_GEN_ProxMineArm_01',Material=Shader'MarkerIconTextures.SetTrap.ProximityMine_Active',Duration=3)
     ChargeExplode=(TransitionEffect=Class'CTEffects.Explosion_Sparks',TransitionSound=Sound'GEN_Sound.Impacts_Explos.exp_GEN_proxMine_01',MyDamageType=Class'CTGame.CTDamageExplosion',Damage=256,DamageRadius=256)
     ExplosionStimulus=ST_ProxMineExplode
     bExplodeOnTimer=True
     ActivateItemType=AIT_ProximityMine
     bTriggeredByDamage=False
     bSavesProgress=True
     AttachItemClass=Class'CTGame.DoorChargeObject'
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     StaticMesh=StaticMesh'MarkerIcons.SetTrap.ProximityMine'
     bDrawHUDMarkerIcon=False
     NavPtLocs(0)=(Offset=(X=108),Yaw=32768)
}

