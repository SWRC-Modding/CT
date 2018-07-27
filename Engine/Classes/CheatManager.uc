//=============================================================================
// CheatManager
// Object within playercontroller that manages "cheat" commands
// only spawned in single player mode
//=============================================================================

class CheatManager extends Object within PlayerController
	native;

var rotator LockedRotation;

/* Used for correlating game situation with log file
*/

exec function ReviewJumpSpots(name TestLabel)
{	
	if ( TestLabel == 'Transloc' )
		TestLabel = 'Begin';
	else if ( TestLabel == 'Jump' )
		TestLabel = 'Finished';
	else if ( TestLabel == 'Combo' )
		TestLabel = 'FinishedJumping';
	else if ( TestLabel == 'LowGrav' )
		TestLabel = 'FinishedComboJumping';
	log("TestLabel is "$TestLabel);
	Level.Game.ReviewJumpSpots(TestLabel);
}

exec function ListDynamicActors()
{
	local Actor A;
	local int i;
	
	ForEach DynamicActors(class'Actor',A)
	{
		i++;
		log(i@A);
	}
	log("Num dynamic actors: "$i);
}

exec function FreezeFrame(float delay)
{
	Level.Game.SetPause(true,outer);
	Level.PauseDelay = Level.TimeSeconds + delay;
}

exec function WriteToLog()
{
	log("NOW!");
}

exec function SetFlash(float F)
{
	FlashScale.X = F;
}

exec function SetFogR(float F)
{
	FlashFog.X = F;
}

exec function SetFogG(float F)
{
	FlashFog.Y = F;
}

exec function SetFogB(float F)
{
	FlashFog.Z = F;
}

exec function KillViewedActor()
{
	if ( ViewTarget != None )
	{
		if ( (Pawn(ViewTarget) != None) && (Pawn(ViewTarget).Controller != None) )
			Pawn(ViewTarget).Controller.Destroy();	
		ViewTarget.Destroy();
		SetViewTarget(None);
	}
}

/* LogScriptedSequences()
Toggles logging of scripted sequences on and off
*/
exec function LogScriptedSequences()
{
	local AIScript S;

	ForEach AllActors(class'AIScript',S)
		S.bLoggingEnabled = !S.bLoggingEnabled;
}

exec function SetCoords( float x, float y, float z, optional float yaw, optional float pitch )
{
	if ( Pawn != None )
	{
		Pawn.SetLocation(MakeVec(x,y,z));
		Pawn.SetRotation(MakeRot(pitch*182,yaw*182,0));
		SetRotation(Pawn.Rotation);
	}
}

/* Teleport()
Teleport to surface player is looking at
*/
exec function Teleport()
{
	local actor HitActor;
	local vector HitNormal, HitLocation;

	HitActor = Trace(HitLocation, HitNormal, ViewTarget.Location + 10000 * vector(Rotation),ViewTarget.Location, true);
	if ( HitActor == None )
		HitLocation = ViewTarget.Location + 10000 * vector(Rotation);
	else
		HitLocation = HitLocation + ViewTarget.CollisionRadius * HitNormal;

	ViewTarget.SetLocation(HitLocation);
}

/* 
Scale the player's size to be F * default size
*/
exec function ChangeSize( float F )
{
	if ( Pawn.SetCollisionSize(Pawn.Default.CollisionRadius * F,Pawn.Default.CollisionHeight * F) )
	{
		Pawn.SetDrawScale(F);
		Pawn.SetLocation(Pawn.Location);
	}
}

exec function LockCamera()
{
	local vector LockedLocation;
	local rotator LockedRot;
	local actor LockedActor;

	if ( !bCameraPositionLocked )
	{
		PlayerCalcView(LockedActor,LockedLocation,LockedRot);
		Outer.SetLocation(LockedLocation);
		LockedRotation = LockedRot;
		SetViewTarget(outer);
	}
	else
		SetViewTarget(Pawn);

	bCameraPositionLocked = !bCameraPositionLocked;
	bBehindView = bCameraPositionLocked;
	bFreeCamera = false;
}

exec function SetCameraDist( float F )
{
	CameraDist = FMax(F,2);
}

/* Stop interpolation
*/
exec function EndPath()
{
}

/* 
Camera and pawn aren't rotated together in behindview when bFreeCamera is true
*/
exec function FreeCamera( bool B )
{
	bFreeCamera = B;
	bBehindView = B;
}

// Anim testing functions.
exec function PlayAnim( name Seq )
{
	Pawn.PlayAnim( Seq );
}

exec function LoopAnim( name Seq )
{
	Pawn.LoopAnim( Seq );
}

exec function StopAnim()
{
	Pawn.StopAnimating();
}

exec function SetMesh( string MeshName )
{
	local mesh NewMesh;
	NewMesh = Mesh( DynamicLoadObject( MeshName, class'Mesh', true ) );
	if( NewMesh == none )
		log("Cannot load mesh " $MeshName);
	else
		Pawn.LinkMesh( NewMesh );
}


exec function CauseEvent( optional name EventName )
{
	if( EventName == '' )
		Target.Trigger( Pawn, Pawn );
	else
		TriggerEvent( EventName, Pawn, Pawn);
}

exec function Trig( optional name EventName )
{
	if( EventName == '' )
		Target.Trigger( Pawn, Pawn );
	else
		TriggerEvent( EventName, Pawn, Pawn );
}

exec function HOG()
{
	Target.TakeDamage( 1000000, Pawn, Target.Location, vect(0,0,0), class'Crushed' );
}
	
exec function Fly()
{
	if ( Pawn == None )
		return;
	ClientMessage("You feel much lighter");
	Pawn.SetCollision(true, true , true);
	Pawn.bCollideWorld = true;
	bCheatFlying = true;
	Outer.GotoState('PlayerFlying');
}

exec function Walk()
{	
	if ( Pawn != None )
	{
		bCheatFlying = false;
		Pawn.SetCollision(true, true , true);
		Pawn.SetPhysics(PHYS_Walking);
		Pawn.bCollideWorld = true;
		Pawn.bHidden = false;
		Pawn.Visibility = Pawn.Default.Visibility;
		ClientReStart(Pawn);
	}
}

exec function Ghost()
{
	if( Pawn != None && !Pawn.IsA('Vehicle') )
	{
		ClientMessage("You feel ethereal");
		Pawn.SetCollision(false, false, false);
		Pawn.bCollideWorld = false;
		bCheatFlying = true;
		Outer.GotoState('PlayerFlying');
		Pawn.bHidden = true;
		Pawn.Visibility = 0;		
	}
	else
		Log("Can't Ghost In Vehicles");
}

exec function Fierfek()
{
	AllAmmo();
}

function AllAmmo()
{
	local Inventory Inv;

	for( Inv=Pawn.Inventory; Inv!=None; Inv=Inv.Inventory ) 
		if (Ammunition(Inv)!=None) 
		{
			Ammunition(Inv).AmmoAmount  = Ammunition(Inv).MaxAmmo;
			//Ammunition(Inv).MaxAmmo  = 999;				
		}
}	

exec function Invisible(bool B)
{
	Pawn.bHidden = B;

	if (B)
		Pawn.Visibility = 0;
	else
		Pawn.Visibility = Pawn.Default.Visibility;
}
	
exec function TheMatulaakLives()
{
	God();
}

function God()
{
	if ( bGodMode )
	{
		bGodMode = false;
		ClientMessage("God mode off");
		return;
	}

	bGodMode = true; 
	ClientMessage("God Mode on");
}

exec function Lamasu()
{
	AllLevels();
}

function AllLevels()
{
	SetLevelProgress( "EPILOGUE" );
	SaveConfig();
	PropagateSettings();	
}

exec function Darman()
{
	SkipLevel();
}

function SkipLevel()
{
	local int CurrentLevelIndex;
	local String strNextMap;
	local String strCurrentMap;
	
	GetCurrentMapName( strCurrentMap );			
	CurrentLevelIndex = GetLevelIndex( strCurrentMap );
	if ( CurrentLevelIndex != -1 )
	{
		// Figure out what the next level is and load it
		GetNextLevel( CurrentLevelIndex, strNextMap );
		if ( -1 != GetLevelIndex( strNextMap ) )
		{
			if ( ( Pawn != None ) && ( Pawn.Squad != None ) )
				Pawn.SaveSquadTravelInfo();

			if ( ( Level != None ) && ( Level.Game != None ) )
			{
				Level.Game.SendPlayer( Outer, strNextMap );
			}
		}
	}
}

exec function SloMo( float T )
{
	Level.Game.SetGameSpeed(T);
	Level.Game.SaveConfig(); 
	Level.Game.GameReplicationInfo.SaveConfig();
}

exec function SetJumpZ( float F )
{
	Pawn.JumpZ = F;
}

exec function SetGravity( float F )
{
	CurrentPhysicsVolume.Gravity.Z = F;
}

exec function SetSpeed( float F )
{
	Pawn.GroundSpeed = Pawn.Default.GroundSpeed * f;
	Pawn.WaterSpeed = Pawn.Default.WaterSpeed * f;
}

exec function KillAll(class<actor> aClass)
{
	local Actor A;

	if ( ClassIsChildOf(aClass, class'AIController') )
	{
		Level.Game.KillBots(Level.Game.NumBots);
		return;
	}
	if ( ClassIsChildOf(aClass, class'Pawn') )
	{
		KillAllPawns(class<Pawn>(aClass));
		return;
	}
	ForEach DynamicActors(class 'Actor', A)
		if ( ClassIsChildOf(A.class, aClass) )
			A.Destroy();
}

// Kill non-player pawns and their controllers
function KillAllPawns(class<Pawn> aClass)
{
	local Pawn P;
	
	Level.Game.KillBots(Level.Game.NumBots);
	ForEach DynamicActors(class'Pawn', P)
		if ( ClassIsChildOf(P.Class, aClass)
			&& !P.IsPlayerPawn() )
		{
			if ( P.Controller != None )
				P.Controller.Destroy();
			P.Destroy();
		}
}

exec function KillPawns()
{
	KillAllPawns(class'Pawn');
}

exec function SmiteEvil()
{
	local Pawn P;

	Log("Smiting Evil");

	ForEach DynamicActors(class'Pawn', P)
		if (!P.IsPlayerPawn() && (P.TeamIndex==5 || P.TeamIndex==4 || P.TeamIndex==8))
		{
			if (P.Controller != None)
				P.Controller.Destroy();
			P.Destroy();
		}
}

/* Avatar()
Possess a pawn of the requested class
*/
exec function Avatar( string ClassName )
{
	local class<actor> NewClass;
	local Pawn P;
		
	NewClass = class<actor>( DynamicLoadObject( ClassName, class'Class' ) );
	if( NewClass!=None )
	{
		Foreach DynamicActors(class'Pawn',P)
		{
			if ( (P.Class == NewClass) && (P != Pawn) )
			{
				if ( Pawn.Controller != None )
					Pawn.Controller.PawnDied(Pawn);
				Possess(P);
				break;
			}
		}
	}
}

exec function Summon( string ClassName )
{
	local class<actor> NewClass;
	local vector SpawnLoc;

	log( "Fabricate " $ ClassName );
	NewClass = class<actor>( DynamicLoadObject( ClassName, class'Class' ) );
	if( NewClass!=None )
	{
		if ( Pawn != None )
			SpawnLoc = Pawn.Location;
		else
			SpawnLoc = Location;
		Spawn( NewClass,,,SpawnLoc + 72 * Vector(Rotation) + vect(0,0,1) * 15 );
	}
}

exec function PlayersOnly()
{
	Level.bPlayersOnly = !Level.bPlayersOnly;
}

exec function CheatView( class<actor> aClass, optional bool bQuiet )
{
	ViewClass(aClass,bQuiet,true);
}

exec function BlindAI(bool bBlind)
{
	Level.Game.bBlindAI = bBlind;
}

exec function DeafAI(bool bDeaf)
{
	Level.Game.bDeafAI = bDeaf;
}

exec function BlindEnemies(bool bBlind)
{
	Level.Game.bBlindEnemies = bBlind;
}

exec function DeafEnemies(bool bDeaf)
{
	Level.Game.bDeafEnemies = bDeaf;
}

// ***********************************************************
// Navigation Aids (for testing)

// remember spot for path testing (display path using ShowDebug)
exec function RememberSpot()
{
	if ( Pawn != None )
		Destination = Pawn.Location;
	else
		Destination = Location;
}

// ***********************************************************
// Changing viewtarget

exec function ViewSelf(optional bool bQuiet)
{
	bBehindView = false;
	bViewBot = false;
	if ( Pawn != None )
		SetViewTarget(Pawn);
	else
		SetViewtarget(outer);
	if (!bQuiet )
		ClientMessage(OwnCamera, 'Event');
	FixFOV();
}

exec function ViewPlayer( string S )
{
	local Controller P;

	for ( P=Level.ControllerList; P!=None; P= P.NextController )
		if ( P.bIsPlayer && (P.PlayerReplicationInfo.PlayerName ~= S) )
			break;

	if ( P.Pawn != None )
	{
		ClientMessage(ViewingFrom@P.PlayerReplicationInfo.PlayerName, 'Event');
		SetViewTarget(P.Pawn);
	}

	bBehindView = ( ViewTarget != Pawn );
	if ( bBehindView )
		ViewTarget.BecomeViewTarget();
}

exec function ViewActor( name ActorName)
{
	local Actor A;

	ForEach AllActors(class'Actor', A)
		if ( A.Name == ActorName )
		{
			SetViewTarget(A);
			bBehindView = true;
			return;
		}
}
/*
// TODO: CL: This function should not be here
exec function ViewFlag()
{
	local Controller C;

	For ( C=Level.ControllerList; C!=None; C=C.NextController )
		if ( C.IsA('AIController') && (C.PlayerReplicationInfo != None) && (C.PlayerReplicationInfo.HasFlag != None) )
		{
			SetViewTarget(C.Pawn);
			return;
		}
}
*/
		
exec function ViewBot()
{
	local actor first;
	local bool bFound;
	local Controller C;

	bViewBot = true;
	For ( C=Level.ControllerList; C!=None; C=C.NextController )
		if ( C.IsA('AIController') && (C.Pawn != None) )
	{
		if ( bFound || (first == None) )
		{
			first = C.Pawn;
			if ( bFound )
				break;
		}
		if ( C.Pawn == ViewTarget ) 
			bFound = true;
	}  

	if ( first != None )
	{
		SetViewTarget(first);
		bBehindView = true;
		ViewTarget.BecomeViewTarget();
		FixFOV();
	}
	else
		ViewSelf(true);
}

exec function ViewClass( class<actor> aClass, optional bool bQuiet, optional bool bCheat )
{
	local actor other, first;
	local bool bFound;

	if ( !bCheat && (Level.Game != None) && !Level.Game.bCanViewOthers )
		return;

	first = None;

	ForEach AllActors( aClass, other )
	{
		if ( bFound || (first == None) )
		{
			first = other;
			if ( bFound )
				break;
		}
		if ( other == ViewTarget ) 
			bFound = true;
	}  

	if ( first != None )
	{
		if ( !bQuiet )
		{
			if ( Pawn(first) != None )
				ClientMessage(ViewingFrom@First.GetHumanReadableName(), 'Event');
			else
				ClientMessage(ViewingFrom@first, 'Event');
		}
		SetViewTarget(first);
		bBehindView = ( ViewTarget != outer );

		if ( bBehindView )
			ViewTarget.BecomeViewTarget();

		FixFOV();
	}
	else
		ViewSelf(bQuiet);
}

exec function Loaded()
{
	if( Level.Netmode!=NM_Standalone )
		return;

    AllWeapons();
    AllAmmo();
}

exec function AllWeapons() 
{
	if( (Level.Netmode!=NM_Standalone) || (Pawn == None) )
		return;
	
	Pawn.GiveWeapon( "CTInventory.DC17mBlaster" );
	Pawn.GiveWeapon( "CTInventory.DC17mSniper" );
	Pawn.GiveWeapon( "CTInventory.ThermalDetonator" );
	Pawn.GiveWeapon( "CTInventory.EMPGrenade" );
	Pawn.GiveWeapon( "CTInventory.SonicDetonator" );
	Pawn.GiveWeapon( "CTInventory.FlashBang" );
	Pawn.GiveWeapon( "CTInventory.Shotgun" );
	Pawn.GiveWeapon( "CTInventory.DC17mAntiArmor" );	
	Pawn.GiveWeapon( "CTInventory.Bowcaster" );
	Pawn.GiveWeapon( "CTInventory.DC15s" );
	Pawn.GiveWeapon( "CTInventory.SMG" );
	Pawn.GiveWeapon( "CTInventory.ConcussionRifle" );
	Pawn.GiveWeapon( "CTInventory.EliteBeam" );
	Pawn.GiveWeapon( "CTInventory.RocketLauncher" );	
}


defaultproperties
{
}

