//=============================================================================
// Clone Troooper Player Controller
//
// Created: 2003 Jan 21, John Hancock
//=============================================================================

class CTPlayer extends PlayerController config(User)
	native;

var float			TargetLastUpdated;		// Time that the next weapon update
var vector			HitNormal;				// Cached HitNormal
var vector			HitLocation;				// Cached HitLocation
var float			DistToTarget;			// The distance to the target
var int				FriendliesKilled;
var int				FriendlyFireInstances;
var int				TimeOfLastFriendlyFire;
var float			BeganFiringTime;
var() float			PickupTargetDistance;
var() class<Emitter> WeaponRolloverEffectClass;	// Effect to get played when the player looks at a pickup
var() class<Emitter> AmmoRolloverEffectClass;	// Effect to get played when the player looks at an ammo pickup

native function AssignSquadMembers();
native event ETargetType SelectTargetType();

replication
{
	reliable if( Role<ROLE_Authority )
		ServerRevive;

	//reliable if( Role==ROLE_Authority )
	//	ClientRevive;
}

function PreBeginPlay()
{
	local PlayerStart Start;

	super.PreBeginPlay();
	if ( (Level != None) && (Level.Game != None) )
		Start = PlayerStart(Level.Game.FindPlayerStart(self));

	if (Start != None)
		bShouldTravelInv = !Start.bClearAllWeapons;
}

/*event PostNetBeginPlay()
{
	Super.PostNetBeginPlay();
	if ( myHUD.GameTypeMaterial == None )
	{
		// Only load subtitles in single player mode
		Helmet.LoadSubtitles();
	}
}*/

event PostLoadBeginPlay()
{
	Super.PostLoadBeginPlay();
	if ( myHUD.GameTypeMaterial == None )
	{
		// Only load subtitles in single player mode
		Helmet.LoadSubtitles();
	}
}

//for tracking instances of player friendly fire

function Killed(Pawn Victim, class<DamageType> damageType)
{
	super.Killed(Victim, damageType);
	//ignore kills in the heat of battle, they might be accidental
	//also ignore if we're the victim
	if (Pawn != None && Pawn != Victim && Pawn.IsAlly(Victim) && !Victim.bIgnoreFriendlyFire && Pawn.Squad != None && !Pawn.Squad.bInBattle)
	{
		++FriendliesKilled;
		if (Level.TimeSeconds - TimeOfLastFriendlyFire > 0.5)
			RegisterFriendlyFireInstance(Victim);
	}
}

function StartHandSignal()
{
	HandSignalTargetType = TargetType;
	HandSignalTarget = Target;
	HandSignalTargetMarker = TargetMarker;
	SwitchWeapon(11);
}

function FriendlyFireCallback(Pawn Ally, class<DamageType> DamageType, int ShotCount)
{
	//Shot count is the number of shots in a row we've plugged the Ally with
	//once we've gotten to 10 shots, it should be pretty clear that the
	//player is intentionally shooting us...
	if (!Ally.bIgnoreFriendlyFire && (ShotCount == 10 || ShotCount == 20))
	{
		if (Level.TimeSeconds - TimeOfLastFriendlyFire > 0.5)
		{
			RegisterFriendlyFireInstance(Ally);
			//Log("Player friendly fire instances "$FriendlyFireInstances);
		}
	}
}


function RegisterFriendlyFireInstance(Pawn Victim)
{
	++FriendlyFireInstances;
	TimeOfLastFriendlyFire = Level.TimeSeconds;
	if (FriendlyFireInstances > 3)
	{
		if (Pawn.Squad != None)
		{
			Pawn.Squad.PlayerWentBerserk(Pawn);
			Pawn.SetTeamIndex(9); //berserk team -- we should probably have an enum for this
			Pawn.bCannotDie = false; //now we can die because no one will heal us
		}
	}
}


function SpawnDefaultHUD()
{
    // note that myHUD is a variable defined in PlayerController
	if (myHUD == None)
		myHUD = Spawn(class'CTHUD', self);
	if( FrameFX == None )
	    FrameFX = new class'FrameFX';
	if( Helmet == None )
	    Helmet = new class'Helmet';
    Helmet.FrameFX = FrameFX;
}

function Possess(Pawn APawn)
{
	super.Possess(APawn);
	if (APawn.Squad == None) //we shouldn't add squad members if we already had a squad
		AssignSquadMembers();
	APawn.bCanPickupInventory = true;
	if (APawn.bIncapacitated)
		GotoState('CtrlIncapacitated');
}

simulated function PawnIncapacitated(Pawn P)
{
	Super.PawnIncapacitated( P );
}

event ServerRevive(Pawn Other)
{
	//Log("CTPawn::ServerRevive: "$Other);
	//ClientRevive(Other); //let the clients know
	Other.Revive();  //revive the guy on the server
}

exec function ToggleHeadlamp()
{
	local byte NumModes;
	local byte NewMode;
	
	NewMode = Pawn.CurrentUserVisionMode + 1;
	
	NumModes = ArrayCount(Pawn.VisionPawnUser);
	if( NewMode >= NumModes )
		NewMode = 0;
	
	if ( VisorModeDefault == 0 )
	{
		if ( NewMode == 0 )
			NewMode = 1;	// Skip over "normal" mode
	}
	else if ( VisorModeDefault == 1 )
	{
		if ( NewMode == 1 )
			NewMode = 2;	// Skip over tactical mode
	}
	
	GotoVisionMode( NewMode );
}

exec function GotoVisionMode( byte NewMode )
{
	local byte NumModes;
	//local ActivateItem A;

	if( Pawn.Flashlight == None )
		return;

	if ( NewMode == Pawn.CurrentUserVisionMode )
		return;

	// Cycle through the vision modes
	// Done here because only players have vision modes
	NumModes = ArrayCount(Pawn.VisionPawnUser);
	
	if ( NewMode < 0 || NewMode >= NumModes )
		return;
	
	/*
	if (Pawn.CurrentUserVisionMode == 1)
	{
		//right now, only ActivateItems have a definition for the function
		ForEach DynamicActors(class'ActivateItem', A)
			A.TacticalModeEnd(self);
	}
	*/

	Pawn.CurrentUserVisionMode = NewMode;

	/*
	if (Pawn.CurrentUserVisionMode == 1)
	{
		ForEach DynamicActors(class'ActivateItem', A)
			A.TacticalModeBegin(self);
	}
	*/

	if( Pawn.VisionPawnUser[Pawn.CurrentUserVisionMode].FlashlightOn )
		Pawn.SetFlashlight( true );
	else
		Pawn.SetFlashlight( false );
}

simulated event NoHelmetInit()
{
	Super.NoHelmetInit();
	
	if ( Helmet != None )
		Helmet.bDoInitSequence = False;
}

exec function ToggleCautious()
{
	if (Pawn.Squad != None)
		CTSquad(Pawn.Squad).ToggleCautious();
}

exec function SetStanceOffensive()
{
	//make sure we're the squad leader
	if (Pawn.Squad != None && Pawn.Squad.SquadLeader == Pawn && Pawn.Squad.CountAliveMembers() != 0)
	{
		Pawn.Squad.SetStance(class'StanceSearchAndDestroy');
		HandSignalTargetType = TGT_SquadOffensiveStance;
		SwitchWeapon(11);
		Helmet.SquadCommandSelected( 0 );
	}


}

exec function SetStanceDefensive()
{
	if (Pawn.Squad != None && Pawn.Squad.SquadLeader == Pawn && Pawn.Squad.CountAliveMembers() != 0)
	{
		Pawn.Squad.SetStance(class'StanceFormUp');
		HandSignalTargetType = TGT_SquadDefensiveStance;
		SwitchWeapon(11);
		Helmet.SquadCommandSelected( 1 );
	}
}

exec function SetStanceHold()
{
	if (Pawn.Squad != None && Pawn.Squad.SquadLeader == Pawn)
		Pawn.Squad.SetStance(class'StanceHold');
}


exec function CancelAllMarkers()
{
	if (Pawn.Squad != None && Pawn.Squad.SquadLeader == Pawn && Pawn.Squad.CountAliveMembers() != 0)
	{
		CTSquad(Pawn.Squad).CancelAllMarkers(Pawn);
		HandSignalTargetType = TGT_CancelAllMarkers;
		SwitchWeapon(11);
		Helmet.SquadCommandSelected( 3 );
	}		
}

exec function CancelAllOrders()
{
	if (Pawn.Squad != None && Pawn.Squad.SquadLeader == Pawn)
		CTSquad(Pawn.Squad).CancelAllOrders(Pawn);
}

exec function SquadEngage()
{
	if (Pawn.Squad != None && Pawn.Squad.SquadLeader == Pawn && Pawn.Squad.CountAliveMembers() != 0)
	{
		CTSquad(Pawn.Squad).EngageLocation(HitLocation);
		HandSignalTargetType = TGT_SquadEngage;
		SwitchWeapon(11);
		Helmet.SquadCommandSelected( 2 );
	}		
}


exec function SquadRecall()
{
	if (Pawn.Squad != None && Pawn.Squad.SquadLeader == Pawn)
	{
		CTSquad(Pawn.Squad).Recall(Pawn);
	}

	//Helmet.ShowSquadCommandPrompt(false);
}

state Briefing
{
	ignores SetStanceOffensive, SetStanceDefensive, SetStanceHold,
		CancelAllMarkers, CancelAllOrders, SquadEngage, SquadRecall,
		StopUse, NextWeapon, PrevWeapon, ToggleHeadlamp, SwitchGrenade;
		

	exec function Use()
	{
		local string strCurrentMap;
		local int CurrentLevelIndex;
		local string strNextMap;

		// If we're in briefing mode, and we're in a "briefing level" 
		// and we've seen it before, and Longo hates me, 
		// then we go to the next level -- otherwise, we stay here and just 
		// do ServerUse.

		if (Level.NetMode == NM_Standalone)
		{

			GetCurrentMapName(strCurrentMap);
			if ( -1 != InStr(Caps(strCurrentMap),"BRIEFING") )
			{
				// We're in a briefing: are we already past this?
				CurrentLevelIndex = GetLevelIndex( strCurrentMap );
				if (CurrentLevelIndex < LevelProgress && CurrentLevelIndex != -1)
				{
					// Figure out what the next level is and load it
					GetNextLevel( CurrentLevelIndex, strNextMap );
					if (-1 != GetLevelIndex(strNextMap) )
					{
						if ( (Pawn != None) && (Pawn.Squad != None) )
							Pawn.SaveSquadTravelInfo();

						if ((Level != None) && (Level.Game != None))
						{
							Level.Game.SendPlayer(self, strNextMap);
						}
						return;
					}
				}
			}
		}
	}

	function BeginState()
	{
		Super.BeginState();
		Helmet.ShowSquadCommandPrompt( False );
	}
		
}

// while possessed, you can't give orders
state PlayerPossessed
{
	ignores SelectTargetType;

	exec function SetStanceOffensive() {}
	exec function SetStanceDefensive() {}
	exec function SetStanceHold() {}
	exec function CancelAllMarkers() {}
	exec function CancelAllOrders() {}
	exec function SquadEngage() {}
	exec function SquadRecall()	{}
	exec function AltFire( optional float F )
	{
		Fire(F); //make melee do the same thing as firing when scav is on us
	}
}

state UsingItem
{
	ignores SelectTargetType, NextWeapon, PrevWeapon, ForceReload,ThrowGrenade, Fire, AltFire, StopFire;

	exec function SetStanceOffensive() {}
	exec function SetStanceDefensive() {}
	exec function SetStanceHold() {}
	exec function CancelAllMarkers() {}
	exec function CancelAllOrders() {}
	exec function SquadEngage() {}
	exec function SquadRecall()	{}
	exec function SwitchWeapon(byte F){}

	function PlayerMove(float DeltaTime)
	{
		local vector ToLoc;
		local vector ToTarget;
		local vector NewAccel;
		local vector EyePos;
		local float Dist;
		local float Speed;
		local float RotSpeed;
		local vector DesiredLocation;
		local rotator DesiredRotation;
		local rotator DeltaRot;
		local float PlayerOffset;

		Speed = 450.0;

		PlayerOffset = 80.0;
		DesiredLocation = Pawn.Location;

		if (Target.IsA('ActivateItem'))
		{
			PlayerOffset = ActivateItem(Target).PlayerOffset;
			if (ActivateItem(Target).bLockPlayerLocation)
			{
				DesiredLocation = Target.Location + PlayerOffset * Vector(Target.Rotation);
				//could use the z location of the anchor navpt - Nav.CollisionHeight + Pawn.CollisionHeight
				DesiredLocation.Z = Pawn.Location.Z; //don't change Pawn's Z offset
			}

		}

		ToLoc = DesiredLocation - Pawn.Location;
		Dist = VSize(ToLoc);
		if (Dist > (Speed * DeltaTime))
		{
			NewAccel = 15.0 * ToLoc - Pawn.Velocity;
			NewAccel.Z = 0;
			//Pawn.SetLocation(Pawn.Location + Normal(ToLoc) * Speed * DeltaTime);
		}
		else
		{
			if (Dist > 1.0)
				Pawn.SetLocation(DesiredLocation);
			Pawn.Velocity = vect(0,0,0);
			Pawn.Acceleration = vect(0,0,0);
			NewAccel = vect(0,0,0);
		}

		EyePos = Pawn.Location + Pawn.EyePosition();
		ToTarget = Target.Location - EyePos;
		ToTarget.Z += Target.HUDMarkerIconZOffset;
		DesiredRotation = Rotator(ToTarget);

		RotSpeed = 45000.0f * DeltaTime;

		DeltaRot = DesiredRotation - Rotation;

		//Log("Time: "$Level.TimeSeconds$" DesLoc: "$DesiredLocation$" PawnLoc: "$Pawn.Location$" EyeHeight: "$Pawn.EyeHeight$" EyePos: "$EyePos$" Pitch: "$DeltaRot.Pitch);
		//Log("        WalkBob: "$Pawn.WalkBob$" ShakeOffset: "$ShakeOffset);
		DeltaRot.Yaw = DeltaRot.Yaw & 65535;
		if (DeltaRot.Yaw > 32768)
			DeltaRot.Yaw -= 65536;

		DeltaRot.Pitch = DeltaRot.Pitch & 65535;
		if (DeltaRot.Pitch > 32768)
			DeltaRot.Pitch -= 65536;

		DeltaRot.Yaw = FClamp(DeltaRot.Yaw, -RotSpeed, RotSpeed );
		DeltaRot.Pitch = FClamp(DeltaRot.Pitch, -RotSpeed, RotSpeed );

		//Log("Clamped Pitch: "$DeltaRot.Pitch);

		SetRotation(Rotation + DeltaRot);

		ViewShake(DeltaTime);
		ViewFlash(DeltaTime);

		if ( Role < ROLE_Authority ) // then save this move and replicate it
			ReplicateMove(DeltaTime, NewAccel, DCLICK_None, DeltaRot );
		else
			ProcessMove(DeltaTime, NewAccel, DCLICK_None, DeltaRot );
	}
	function BeginState()
	{
		//Pawn.bUpdateEyeheight = false;
		HealingTime = 0;
		if (TargetType == TGT_FriendToHeal)
		{
			if ( Pawn.Weapon.WeaponType != WT_Heal )
				ForceWeaponSwitch(10);
		}
		else if (TargetType == TGT_Panel)
		{
			if (Pawn.Weapon.WeaponType != WT_Marker )
				ForceWeaponSwitch(11);
		}
		/*
		if (Target.NavPts.Length > 0)
		{
			Pawn.SetLocation(Target.NavPts[0].Location);
			Pawn.SetRotation(Target.NavPts[0].Rotation);
		}
		*/
	}
	function EndState()
	{
		Pawn.bUpdateEyeheight = true;
		ReturnToWeapon();
	}
	exec function StopUse()
	{
		if (Pawn.Weapon.WeaponType == WT_Heal)
			Pawn.Weapon.StopFire(0);
		GotoState('PlayerWalking');
		Global.StopUse();
	}

	event PlayerTick( float	DeltaTime )
	{
		local ActivateItem TargetActivate;

		assert(bUse);

		if (Pawn !=	None &&	bUse)
		{
			if (TargetType == TGT_FriendToHeal)
			{
				// switch weapon to	the	bacta healer
				if ( (HealingTime == 0)	&& (Pawn.Weapon.WeaponType != WT_Heal) )
					ForceWeaponSwitch(10);
				if ( (Pawn.Weapon != None) && (Pawn.Weapon.WeaponType == WT_Heal))
				{
					if (Pawn.Weapon.IsInState('Idle'))
					{
						HealingTime = 0;
						Pawn.Weapon.StopFire(0);
						Pawn.Fire(1);
						Pawn.Weapon.CreateMuzzleEffect();
					}
					else if (Pawn.Weapon.IsInState('NormalFire'))
					{
						HealingTime	+= DeltaTime;
						//Log("Healing Time: "$HealingTime);
					}
					Pawn(Target).bBeingRevivedByPlayer = true;
				}

				if (HealingTime	> TotalTimeToRevive){ //the	amount of time it takes	to revive a	guy
					Pawn.Weapon.StopFire(0);
					Pawn.SwitchToLastWeapon();
					ServerRevive(Pawn(Target));
					//need to display the effect
					HealingTime	= 0;
					Pawn(Target).bBeingRevivedByPlayer = false;
					StopUse();
				}
			}
			else if	( (TargetType == TGT_Panel))
			{
				TargetActivate = ActivateItem(Target);

				if (!TargetActivate.bIsUsable)
				{
					StopUse();
				}
				else if (Pawn.Weapon.WeaponType == WT_Marker)
				{
					//Log("UsingItem:: Weapon Type Marker");
					//if (!ActivateItem(Target).bActivating)
					//	Pawn.Weapon.StopFire(0);
					if	(Pawn.Weapon.IsInState('Idle'))
						Pawn.Weapon.Fire(0);
				}
				else
				{
					//Log("Starting Hand Signal");
					StartHandSignal();
				}
			}
		}
		TargetDuration += DeltaTime;	
		Super.PlayerTick( DeltaTime );
	}
}

// The player wants to fire.
exec function Fire( optional float F )
{
	BeganFiringTime = Level.TimeSeconds;
	//Pawn.PlayAnim(CTWeapon(Pawn.Weapon).PawnFireAnim(), 'spine2');
	Super.Fire();
}

// The player wants to use.
exec function Use()
{	
	local ActivateItem TargetActivate;
	local bool ShouldUse;
	//local actor A;
	
	bUse = true;
	ShouldUse = true;

	if ( Helmet != None )
		Helmet.ShowSquadCommandPrompt( True );

	if( Pawn == None )
		return;

	switch (TargetType)
	{
	case TGT_MarkerCancel:
	case TGT_Marker:
		if (TargetMarker != None && Pawn.Squad != None && Pawn.Squad.SquadLeader == Pawn)
		{
			if (TargetMarker.ExecuteMarker(Pawn))
			{
				StartHandSignal();
				//TargetLastUpdated = 0.0; //force an update of the TargetType next frame
			}
			ShouldUse = false;
		}
		break;
	case TGT_Enemy:
		if (Target != None && Target.IsA('Pawn') && Pawn.Squad != None && Pawn.Squad.SquadLeader == Pawn)
		{				
			CTSquad(Pawn.Squad).EngageTarget(Pawn(Target));		
		}
		ShouldUse = false;
		break;
	case TGT_EngagedEnemy:
		if (Pawn.Squad != None && Pawn.Squad.SquadLeader == Pawn)
			CTSquad(Pawn.Squad).EngageTarget(None);
		break;
	case TGT_MountTurret:
		if (Target != None && Target.IsA('Turret'))
			Target.UsedBy(Pawn);
		ShouldUse = false;
		break;
	case TGT_FriendToHeal:		
		ShouldUse = false;
		GotoState('UsingItem');
		break;
	case TGT_Panel:
		TargetActivate = ActivateItem(Target);
		if ( (TargetActivate.ActivateItemType == AIT_BactaDispenser) && (Pawn.Health >= Pawn.MaxHealth) )
			break;
		if (!TargetActivate.bActivating)
		{
			TargetActivate.bPlayerActivated = true;
			TargetActivate.bUsed = true;
			TriggerEvent(TargetActivate.EventUnderway, TargetActivate, Pawn );
			//TargetActivate.StartSetup();
			if ( (TargetActivate != None) && TargetActivate.bIsUsable )
			{
				StartHandSignal();
				GotoState('UsingItem');
			}	
		}
		ShouldUse = false;
		break;
	case TGT_DemolitionReady:
		if (Pawn.Squad != None && Pawn.Squad.CountAliveMembers() > 0)
			Pawn.PlayOwnedCue(PAE_DemolitionExplode, 2.0f);
		StartHandSignal();
		break;
	}
	if (ShouldUse)
	{
		Super.Use();
	}
}

exec function StopUse()
{
	//Log("CTPlayer::StopUse");
	Super.StopUse();
	SetOkToSwitchWeapon(true);
	if ( Helmet != None )
		Helmet.ShowSquadCommandPrompt( False );
}

/* AdjustAim()
AIController version does adjustment for non-controlled pawns. 
PlayerController version does the adjustment for player aiming help.
Only adjusts aiming at pawns
allows more error in Z direction (full as defined by AutoAim - only half that difference for XY)
*/
//this is our current target location based on our controller rotation and the muzzle location
simulated function vector TargetLocation(Ammunition FiredAmmunition)
{
	local vector FireDir, AimSpot, HitNormal, HitLocation, EyeLoc;
	local Actor HitActor;

	FireDir = vector(Rotation);
	//Log("First-person fire direction is: " $FireDir);
	EyeLoc = Pawn.Location + Pawn.EyePosition();
	AimSpot = EyeLoc + 10000 * FireDir; //ammo should return TraceLength (Max Range)
	HitActor = Trace(HitLocation, HitNormal, AimSpot, EyeLoc, true);
	if (HitActor == None)
		return AimSpot;
	return HitLocation;
}

/* cg removed cause.. it does nothing. 
simulated function rotator AdjustAim(Ammunition FiredAmmunition, vector projStart, float aimerror)
{
	//performs no auto-aiming, but instead adjusts aim to hit reticle
	local vector target;

	Log("cg -> ********** CTPlayer::AdjustAim() : Called with aim error "$aimerror$"! ******************");

	target = TargetLocation(FiredAmmunition);
	target.Z += aimerror;

	return Rotator(target - projStart);
}
*/ 

function ReturnToWeapon()
{
	HealingTime = 0;
	if ( (Pawn.Weapon != None) && (Pawn.Weapon.WeaponType == WT_Marker  || Pawn.Weapon.WeaponType == WT_Heal) )
	{
		Pawn.SwitchToLastWeapon();
	}
}

event PlayerTick( float DeltaTime )
{
	//local Weapon PawnWeapon;
	local Actor OldTarget;
	local ETargetType OldTargetType;
	
	// track if the target changes
	OldTarget = Target;
	OldTargetType = TargetType;

	if (Pawn != None)
	{
		if (Level.TimeSeconds > Pawn.NextFootstepTime && VSizeSq(Pawn.Velocity) > 1 && Pawn.Visibility != 0)
		{
			if (Pawn.bIsCrouched || Pawn.bIsWalking)
			{
				if(Pawn.WalkStimulus != ST_Invalid)
					Pawn.PostStimulusToWorld(Pawn.WalkStimulus);
			}
			else if (Pawn.RunStimulus != ST_Invalid)
				Pawn.PostStimulusToWorld(Pawn.RunStimulus);
			//only send out footsteps about every 1/4 second
			Pawn.NextFootstepTime = Level.TimeSeconds + 0.25f;
		}

		if( TargetLastUpdated + 0.1 <= Level.TimeSeconds )
		{
			TargetType = SelectTargetType();
			TargetLastUpdated = Level.TimeSeconds;
			ReturnToWeapon();
		}

		// Track how long the target has been targeted
		if( OldTarget != Target || OldTargetType != TargetType )
		{
			if (TargetType == TGT_Enemy && Target.IsA('Pawn'))
			{
				SetEnemy(Pawn(Target));
			}
			if (Enemy != None && !Enemy.IsValidEnemy())
				SetEnemy(None);
			if (OldTarget != None)
			{
				OldTarget.NotifyUnTargeted(self);
				if (OldTarget.IsA('Pawn'))
					Pawn(OldTarget).bBeingRevivedByPlayer = false;
			}

			if( Target != None )
				Target.NotifyTargeted(self);

			if (OldTargetMarker != None)
				OldTargetMarker.NotifyUnTargeted(self);

			if (TargetMarker != None)
				TargetMarker.NotifyTargeted(self);

			OldTargetMarker = TargetMarker;
			
			TargetDuration = 0;
			OldTarget = Target;
			OldTargetType = TargetType;
			// Switch off the bacta healer or marker weapon
			ReturnToWeapon();
		}
		else
		{
			//Let the marker know it was targeted regardless to allow it to update its holograms
			if (TargetMarker != None)
				TargetMarker.UpdateIcons(self);

			TargetDuration += DeltaTime;	
		}

		//check for player shooting at nothing
		if (bFire != 0 && Pawn.Squad != None && TargetType == TGT_Default)
		{
			//if it has been several seconds since battle,
			//and the player is firing at nothing for at least 1 second
			if (!Pawn.Squad.bInBattle
				&& (Level.TimeSeconds - Pawn.Squad.WhenBattleEnded) > 6.0
				&& (Level.TimeSeconds - BeganFiringTime) > 0.75)
				Pawn.Squad.PlayerShootingGhosts(Pawn);
		}
	}

	Super.PlayerTick( DeltaTime );
}

state CtrlIncapacitated
{
	ignores SelectTargetType, ToggleHeadlamp, SwitchGrenade;
	exec function SetStanceOffensive() {}
	exec function SetStanceDefensive() {}
	exec function SetStanceHold() {}
	exec function CancelAllMarkers() {}
	exec function CancelAllOrders() {}
	exec function SquadEngage() {}
	exec function SquadRecall()	{}
}

exec function ShowActorObjectiveTracker( Actor a )
{
	Helmet.ShowActorObjectiveTracker( a );
}

exec function ShowPositionObjectiveTracker( Vector position )
{
	Helmet.ShowPositionObjectiveTracker( position );
}

exec function HideObjectiveTracker()
{
	Helmet.HideObjectiveTracker();
}

exec function ShowPrompt( string PromptText, Array<string> PromptButtonFuncs, float PromptTime, bool bFlash, String ACTION_Name )
{
	// NOTE for single player, we just use the text passed in and don't try to localize it
	//		(since we know this was initiated on the client).
	Helmet.GenericPromptText = PromptText;
	Helmet.GenericPromptButtonFuncs = PromptButtonFuncs;
	Helmet.GenericPromptTime = PromptTime;
	Helmet.GenericPromptFlash = bFlash;
}

exec function BlinkHUDSquadCommandPrompt( float BlinkTime )
{
	Helmet.bBlinkSquadCommandPrompt = True;
	Helmet.SquadCommandPromptBlinkTime = BlinkTime;
	Helmet.ElapsedSquadCommandPromptBlinkTime = 0.0;
}

exec function BlinkHUDSquad( float BlinkTime )
{
	Helmet.bBlinkSquad = True;
	Helmet.SquadBlinkTime = BlinkTime;
	Helmet.ElapsedSquadBlinkTime = 0.0;
}

exec function BlinkHUDGrenade( float BlinkTime )
{
	Helmet.bBlinkGrenade = True;
	Helmet.GrenadeBlinkTime = BlinkTime;
	Helmet.ElapsedGrenadeBlinkTime = 0.0;
}

exec function BlinkHUDHealth( float BlinkTime )
{
	Helmet.bBlinkHealth = True;
	Helmet.HealthBlinkTime = BlinkTime;
	Helmet.ElapsedHealthBlinkTime = 0.0;
}

exec function BlinkHUDShields( float BlinkTime )
{
	Helmet.bBlinkShields = True;
	Helmet.ShieldsBlinkTime = BlinkTime;
	Helmet.ElapsedShieldsBlinkTime = 0.0;
}

simulated event PotentialSubtitledSoundPlayed(String SoundName, float Duration, int Priority)
{
	Super.PotentialSubtitledSoundPlayed(SoundName, Duration, Priority);
	
	if ( myHUD.bShowSubtitles && !IsPaused() )
	{
		Helmet.PotentialSubtitledSoundPlayed(SoundName, Duration, Priority);
	}
}

simulated event bool InTacticalVisionMode()
{
	return Helmet.bInTacticalVisionMode;
}

simulated event CheckpointSaveStarted()
{
	Helmet.CheckpointSaveStarted();
}

//------------------------------


defaultproperties
{
     PickupTargetDistance=700
     WeaponRolloverEffectClass=Class'CTEffects.HoloHUD_RollOver_Weapon'
     AmmoRolloverEffectClass=Class'CTEffects.HoloHUD_RollOver_Ammo'
     PlayerReplicationInfoClass=Class'CTGame.CTPRI'
}

