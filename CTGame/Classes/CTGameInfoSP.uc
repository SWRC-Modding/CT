//=============================================================================
// Clone Troooper Game Info for Single-Player campaign
//
// Created: 2003 Jan 21, John Hancock
//=============================================================================
class CTGameInfoSP extends CTGameInfo;

function PostBeginPlay()
{
	local int i;

	//Log("CTGameInfoSP::PostBeginPlay -- spawning teams");
	GameReplicationInfo.CreateTeam(0, class'TeamNeutral');
	for (i=1; i <= 2; ++i){ //teams 1 to 3 are player friendly
		GameReplicationInfo.CreateTeam(i, class'TeamPlayer');
	}
	GameReplicationInfo.CreateTeam(3, class'TeamWookiee');
	GameReplicationInfo.CreateTeam(4, class'TeamAttachedScav');
	GameReplicationInfo.CreateTeam(5, class'TeamHostile');	
	for (i=6; i <= 8; ++i){ //teams 6 to 8 are hostile to every other team
		GameReplicationInfo.CreateTeam(i, class'TeamMP');
	}
	GameReplicationInfo.CreateTeam(9, class'TeamBerserker');
	//Log("CTGameInfoSP::PostBeginPlay -- finished spawning teams");
	super.PostBeginPlay();
}

function byte PickTeam(byte num, Controller C)
{
	if( C != None )
	{
		if ( C.IsA('PlayerController') )
			return 1;
		else if ( C.Pawn != None )
			return C.Pawn.TeamIndex;
	}
	
	// Otherwise just put em on team 1
	return 1;	
}

/* Change teams to team
*/
function bool ChangeTeam(Controller Other, int N, bool bNewTeam, optional bool bSwitch)
{
	local TeamInfo NewTeam;
	
	NewTeam = GameReplicationInfo.GetTeam(PickTeam(N,Other));
	
	NewTeam.AddToTeam(Other);

	return true;
}

function RestartPlayer( Controller aPlayer )	
{
	if (!aPlayer.RestartGame())
		super.RestartPlayer(aPlayer);
}

function ScoreKill(Controller Killer, Controller Victim)
{
	if (Killer != None)
		Victim.Pawn.GetTeam().ScoreKillsBy(Killer.Pawn.GetTeamIndex());
}

// In single player if the player comes over with equipment then don't
// destroy their inventory
function AcceptInventory(pawn PlayerPawn)
{
	// player has inventory carried over from the previous sublevel so don't destroy
	if (PlayerPawn.Weapon != None)
		return;
	else
		super.AcceptInventory(PlayerPawn);
}

function int GetNumAllowedInGroup(Weapon Weap)
{
	if (Weap.WeaponType == WT_Thrown)
		return NumGrenadesAllowed;
	else
		return NumWeaponsAllowed;
}

function bool WeaponIsTossable(Weapon Weap)
{
	if ( Weap.WeaponType == WT_Secondary && Weap.bCanThrow )
		return true;
	else
		return false;
}



defaultproperties
{
     DefaultPlayerClassName="CTCharacters.PlayerCommando"
     NumWeaponsAllowed=1
     NumGrenadesAllowed=3
     CustomMeshSets(0)=(Mesh=SkeletalMesh'Clone.CloneCommando',Set=MeshAnimation'Clone.mDemolitionSet')
     CustomMeshSets(1)=(Mesh=SkeletalMesh'Clone.CloneCommando',Set=MeshAnimation'Clone.mDoorBreachSet')
     CustomMeshSets(2)=(Mesh=SkeletalMesh'Clone.CloneCommando',Set=MeshAnimation'Clone.mHackUnlockConSet')
     CustomMeshSets(3)=(Mesh=SkeletalMesh'Clone.CloneCommando',Set=MeshAnimation'Clone.mHackUnlockDoorSet')
     CustomMeshSets(4)=(Mesh=SkeletalMesh'Clone.CloneCommando',Set=MeshAnimation'Clone.mProximityMineSet')
     CustomMeshSets(5)=(Mesh=SkeletalMesh'Clone.CloneCommando',Set=MeshAnimation'Clone.mTrapSet')
}

