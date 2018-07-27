// ====================================================================
//  Class:  WarfareGame.StationaryWeapons
//  Parent: Engine.Actor
//
 
// ====================================================================

class StationaryWeapons extends Pickup
		Abstract
		Native
		NativeReplication;

var()	int  TeamIndex;			// The team that owns this weapon
var()   bool bIgnoreTeammates;	// This weapon will ignore teammates
var()	StaticMesh Meshes[2];	// What meshes to use

var 	byte bActive;			// Is this weapon active
var	byte bLastActive;		// What was the last bActive 

replication
{
	reliable if (ROLE==ROLE_Authority)
		bActive, TeamIndex;
}

event PostBeginPlay()
{
	SetTeam(TeamIndex);
	Super.PostBeginPlay();
}	

function SetTeam(int NewTeamIndex)
{
	SetStaticMesh(Meshes[NewTeamIndex]);
	TeamIndex = NewTeamIndex;
}	

// Activated and Deactivated are called when bActive changes state on replication

simulated event Activated();
simulated event Deactivated();

auto state fucked
{
}

function Activate()			// Should be subclassed
{
	bActive = 1;
	if ( (Level.NetMode==NM_ListenServer) || (Level.NetMode==NM_Standalone) )
		Activated();
	
}

function DeActivate()		// Should be subclassed
{
	bActive = 0;
	if ( (Level.NetMode==NM_ListenServer) || (LEvel.NetMode==NM_Standalone) )
		Deactivated();	
}

function Explode(vector HitLocation, vector HitNormal)
{
	Destroy();
}

function float ReduceDamage(float Damage, class <DamageType> DamageType)
{
	return Damage;
}

function float TakeDamage(float Damage, Pawn EventInstigator, vector HitLocation, vector Momentum, class <DamageType> DamageType, optional Name BoneName)
{
	Explode(HitLocation, vect(0,0,0));
	return Damage;
}



defaultproperties
{
     bIgnoreTeammates=True
     bOnlyReplicateHidden=False
     DrawType=DT_StaticMesh
     bProjTarget=True
     RemoteRole=ROLE_SimulatedProxy
}

