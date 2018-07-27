//=============================================================================
// DamageType, the base class of all damagetypes.
// this and its subclasses are never spawned, just used as information holders
//=============================================================================
class DamageType extends Object
	native
	abstract
	dependsOn(PawnAudioTable);

// Description of a type of damage.
var() localized string		DeathString;	 				// String to describe death by this type of damage	
var() localized string		MaleSuicide;

var() bool					bBypassesShields;				// This damage goes straight to health
var() bool					bKillsNotIncapacitates;			// Whether this damage will kill rather than incapacitate all characters
var() bool					bKillsNotIncapacitatesPlayer;	// Whether this damage will kill rather than incapacitate the player
var() bool					KApplyImpulseToRoot;			// Apply the Damage to the root
var() bool					DisallowDeathAnim;				// Don't play a death anim when hurt by this damage type
var() bool					CannotBeAbsorbed;				// Bone modifiers, variances, etc. have no effect on this damage
var() bool					NoDeathEffect;					// Don't spawn a death effect
var() bool					bCountFriendlyFire;
var() float					RagdollOnDeathProbability;		// Chance Actor should try to go to ragdoll immediately rather than playing an animation

var() float					FlashScale;						// For flashing victim's screen

var() float					KDamageImpulse;					// Magnitude of impulse applied to KActor due to this damage type.
var() float					KDeathUpKick;					// Amount of upwards kick ragdolls get when they die

var(Sound) PawnAudioTable.EPawnAudioEvent	HurtAudioEvent;
var(Sound) PawnAudioTable.EPawnAudioEvent	DeathAudioEvent;
var(Sound) PawnAudioTable.EPawnAudioEvent	AllyDownedByEvent;
var(Sound) PawnAudioTable.EPawnAudioEvent	LeaderKilledOtherEvent;
var(Sound) PawnAudioTable.EPawnAudioEvent	FriendlyFireEvent;

var() array<Material>		VisorHitDecals;
var() class<Emitter>		VisorHitEffect;
var() float					VisorHitScaleMin;
var() float					VisorHitScaleMax;
var(Sound) Sound			VisorHitSound;
var() float					VisorHitDelayNeed;

static function IncrementKills(Controller Killer);

static function string DeathMessage(PlayerReplicationInfo Killer, PlayerReplicationInfo Victim)
{
	return Default.DeathString;
}

static function string SuicideMessage(PlayerReplicationInfo Victim)
{
	/* // TODO: CL: Not likely to have female actors, but left in for now.
	if ( Victim.bIsFemale )
		return Default.FemaleSuicide;
	else
	*/
		return Default.MaleSuicide;
}


defaultproperties
{
     DeathString="%o was killed by %k."
     MaleSuicide="%o killed himself."
     bCountFriendlyFire=True
     RagdollOnDeathProbability=0.5
     FlashScale=0.3
     HurtAudioEvent=PAE_HurtSmallArms
     DeathAudioEvent=PAE_DieSmallArms
     LeaderKilledOtherEvent=PAE_GoodShotLeader
     FriendlyFireEvent=PAE_FriendlyFireHealth
     VisorHitScaleMin=0.9
     VisorHitScaleMax=1.25
     VisorHitDelayNeed=5
}

