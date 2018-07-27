//=============================================================================
// Clone Troooper Audio Parameters
//
// Created: 2003 April 2, John Hancock
//=============================================================================

class PawnAudioTable extends Object
	native;

//
// Add SquadOnly cues after PAE_SquadOnly. All cues before PAE_SquadOnly
// are assumed to be used for everyone
// Don't move cues around -- breaks DamageType hookups
enum EPawnAudioEvent
{
	PAE_Invalid,	//the invalid cue
	PAE_DieQuiet,
	PAE_DieFall,
	PAE_DieExplosion,
	PAE_DieLargeArms,
	PAE_DieSmallArms,
	PAE_DieElectricity,
	PAE_DieFire,
	PAE_HurtFall,
	PAE_HurtExplosion,
	PAE_HurtLargeArms,
	PAE_HurtMelee,
	PAE_HurtSmallArms,
	PAE_HurtElectricity,
	PAE_HurtFire,
	PAE_SpottedEnemy,
	PAE_KilledExcited,
	PAE_KilledCalm,
	PAE_HeardSomething,
	PAE_SearchAbort,
	PAE_ThrowGrenade,
	PAE_SeeGrenade,
	PAE_DropToOrange,

	PAE_LandGrunt,
	PAE_JumpGrunt,
	PAE_MeleeSwing,
	PAE_MeleeBerserk,
	PAE_Reloading,
	PAE_ScavDroidAttached,
	PAE_RetreatSolo,
	PAE_RetreatHealthy,
	PAE_RequestSupport,
	PAE_Taunt,
	PAE_OrderAttack,	

	//SquadOnly cues below this
	PAE_SquadOnly,	

	PAE_IncapacitatedGroan,

	PAE_SpottedDroid,
	PAE_SpottedElite,
	PAE_SpottedDrone,
	PAE_SpottedWarrior,
	PAE_SpottedMerc,
	PAE_SpottedSlaver,
	PAE_SpottedSBD,
	PAE_SpottedSpider,
	PAE_SpottedScavenger,
	PAE_SpottedDroideka,
	PAE_SpottedBeast,
	PAE_SpottedGuard,
	PAE_SpottedHAM,

	PAE_LongBattle,
	PAE_LongBattleResponse,
	PAE_BattleLull,
	PAE_JetpackExplosion,
	PAE_BehindYou,
	PAE_MoveOut,

	PAE_FriendlyFireHealth,
	PAE_FriendlyFireInjured,	//CUTME?
	PAE_CheckCorpse,
	PAE_CheckCorpseFire, 

	PAE_KilledGeo,
	PAE_KilledTran,
	PAE_KilledDroid,
	PAE_KilledGuard,
	PAE_SquadLeaderDown,
	PAE_AllyDown,			//CUTME
	PAE_Delta07Down,
	PAE_Delta40Down,
	PAE_Delta62Down,

	PAE_DownedByGeo,
	PAE_DownedByTran,
	PAE_DownedByDroid,
	PAE_DownedByPlayer,
	PAE_DownedByTurret,
	PAE_DownedByGrenade,
	PAE_DownedByTrap,

	PAE_AssistingAlly,
	PAE_Assisting07,
	PAE_Assisting40,
	PAE_Assisting62,
	PAE_AssistingLeader,
	PAE_Revived,
	PAE_RevivedYetAgain,
	PAE_Healed,
	PAE_LeaderTooFar,
	PAE_LeaderTooFarCombat,
	PAE_RevivingInterrupted,
	PAE_HealthAcknowledge,
	PAE_GoodShotLeader,
	PAE_GoodThrowLeader,
	PAE_GoodMeleeLeader,
	PAE_AllyKilledEnemy,
	PAE_TakingCoverHealthy,
	PAE_TakingCoverIdle,
	PAE_RefuseOrder,
	PAE_RefuseCantGoThere,
	PAE_CantReviveNow,

	PAE_PlayerNotMoving,
	PAE_PlayerShootingGhosts,
	PAE_PlayerSchizo,
	PAE_PlayerKilledSelf,
	PAE_PlayerOutOfMyWay,
	PAE_PlayerInLineOfFire,
	PAE_PlayerHostile,
	PAE_MuzzleDiscipline,
	
	//Stance Cues
	PAE_AcknowledgeOrder,
	PAE_FormUp,
	PAE_EngageTarget,
	PAE_SecurePosition,
	PAE_SearchAndDestroy,
	PAE_Recall,
	PAE_StanceHold,
	PAE_EngageTargetComplete,
	PAE_SecurePositionComplete,

	PAE_FriendlyFireMelee,
	PAE_MineSpotted,

	//Marker Cues
	PAE_SetTrapInitiate,
	PAE_SetTrapConfirm,
	PAE_SetTrapCancel,
	PAE_SetTrapCancelConfirm,
	PAE_SetTrapCover,
	PAE_SetTrapProgress,
	PAE_SetTrapComplete,
	PAE_SetTrapExplode,
	PAE_SnipeInitiate,
	PAE_SnipeConfirm,
	PAE_SnipeReady,
	PAE_SnipeCancel,
	PAE_SnipeCancelConfirm,
	PAE_SnipeComplete,
	PAE_DisarmTrapInitiate,
	PAE_DisarmTrapConfirm,
	PAE_DisarmTrapCancel,
	PAE_DisarmTrapCancelConfirm,
	PAE_DisarmTrapCover,
	PAE_DisarmTrapProgress,
	PAE_DisarmTrapComplete,
	PAE_BreachInitiate,
	PAE_BreachConfirm,
	PAE_BreachCancel,
	PAE_BreachCancelConfirm,
	PAE_BreachGo,
	PAE_BreachSetCharge,
	PAE_BreachClear,
	PAE_BreachGrenade,
	PAE_BreachEnter,
	PAE_HackDoorInitiate,
	PAE_HackDoorConfirm,
	PAE_HackDoorCancel,
	PAE_HackDoorCancelConfirm,
	PAE_HackDoorProgress,
	PAE_HackDoorComplete,
	PAE_TurretInitiate,
	PAE_TurretConfirm,
	PAE_TurretCancel,
	PAE_TurretCancelConfirm,
	PAE_TurretReady,
	PAE_TurretComplete,
	PAE_HackTerminalInitiate,
	PAE_HackTerminalConfirm,
	PAE_HackTerminalCancel,
	PAE_HackTerminalCancelConfirm,
	PAE_HackTerminalProgress,
	PAE_HackTerminalComplete,
	PAE_DemolitionInitiate,
	PAE_DemolitionConfirm,
	PAE_DemolitionCancel,
	PAE_DemolitionCancelConfirm,
	PAE_DemolitionCover,
	PAE_DemolitionProgress,
	PAE_DemolitionComplete,
	PAE_DemolitionExplode,
	PAE_BactaInitiate,
	PAE_BactaCancel,
	PAE_BactaCancelConfirm,
	PAE_BactaJackIn,
	PAE_BactaNotNeeded,
	PAE_AntiArmorInitiate,
	PAE_AntiArmorConfirm,
	PAE_AntiArmorReady,
	PAE_AntiArmorCancel,
	PAE_AntiArmorCancelConfirm,
	PAE_AntiArmorComplete,
	PAE_GrenadeInitiate,
	PAE_GrenadeConfirm,
	PAE_GrenadeReady,
	PAE_GrenadeCancel,
	PAE_GrenadeCancelConfirm,
	PAE_GrenadeComplete,
	PAE_ReviveInitiate,
	PAE_ReviveInitiateSelf,
	PAE_RevivePlayerInitiate, //this should really have been called RevivePlayerConfirm
	PAE_ReviveCancel,
	PAE_ReviveCancelConfirm,
};


var(Sound) Array<Sound> Table;

//var int Flags;

static final native function Sound GetSound(EPawnAudioEvent Event /*, optional float MinimumTriggerInterval*/);


//static final native function PlaySound(EPawnAudioEvent Event, Pawn MyPawn, optional float MinimumTriggerInterval)


defaultproperties
{
}

