 //=============================================================================
// TeamInfo.
//=============================================================================
class TeamInfo extends ReplicationInfo
	native
	nativereplication;

var Color HudTeamColor;
var localized string TeamName;
var int Size; //number of players on this team in the level
var float Score;
var() int TeamIndex;
var color TeamColor[2];
var Actor HomeBase;			// key objective associated with this team
var() class<Pawn> DefaultPlayerClass;
var localized string ColorNames[2];
var Material TeamIcon;

replication
{
	// Variables the server should send to the client.
	reliable if( bNetDirty && (Role==ROLE_Authority) )
		Score, HomeBase, TeamIndex;
	reliable if ( bNetInitial && (Role==ROLE_Authority) )
		TeamIcon;
}

simulated event Color GetTeamColor()
{
	if(TeamIndex == 0 || TeamIndex == 1)
		return TeamColor[TeamIndex];
	return TeamColor[0]; 
}

function bool BelongsOnTeam(class<Pawn> PawnClass)
{
	return true;
}

simulated function UpdatePrecacheMaterials()
{
	Level.AddPrecacheMaterial(TeamIcon);
}

simulated function string GetHumanReadableName()
{
	return TeamName;
}

function bool AddToTeam( Controller Other )
{
	local Controller P;
	local bool bSuccess;
	
	// make sure loadout works for this team
	if ( Other == None )		
		return false;

	if (MessagingSpectator(Other) != None)
		return false;

	// This is setting the player to the team
	Size++;
	Other.PlayerReplicationInfo.Team = self;

	bSuccess = false;
	if ( Other.IsA('PlayerController') )
		Other.PlayerReplicationInfo.TeamID = 0;
	else
		Other.PlayerReplicationInfo.TeamID = 1;

	while ( !bSuccess )
	{
		bSuccess = true;
		for ( P=Level.ControllerList; P!=None; P=P.nextController )
            if ( P.bIsPlayer && (P != Other) 
				&& (P.PlayerReplicationInfo.Team == Other.PlayerReplicationInfo.Team) 
				&& (P.PlayerReplicationInfo.TeamId == Other.PlayerReplicationInfo.TeamId) )
				bSuccess = false;
		if ( !bSuccess )
			Other.PlayerReplicationInfo.TeamID = Other.PlayerReplicationInfo.TeamID + 1;
	}
	return true;
}

function RemoveFromTeam(Controller Other)
{
	Size--;
}

function ScoreKillsBy(int TeamID)
{
}

simulated function ETeamRelationship GetRelationTowards (int TeamID)
{
	log(">> RelationTowards from"@TeamIndex@"to"@TeamID);
	if ( TeamID == TeamIndex )
		return TEAM_Ally;
	
	return TEAM_Enemy;
}

simulated function bool IsEnemy(int TeamID)
{
	return (TeamID != TeamIndex);
}

simulated function bool IsNotEnemy(int TeamID)
{
	return (TeamID == TeamIndex);
}

simulated function bool IsAlly(int TeamID)
{
	return (TeamID == TeamIndex);
}

simulated function bool IsNotAlly(int TeamID)
{
	return (TeamID != TeamIndex);
}

simulated function bool IsNeutral(int TeamID)
{
	return false;
}

function SetTeamIndex(int index)
{	
	TeamIndex = index;
}


defaultproperties
{
     TeamName="Team"
     TeamColor(0)=(B=186,G=93,R=93,A=255)
     TeamColor(1)=(B=72,G=75,R=240,A=255)
     ColorNames(0)="Blue"
     ColorNames(1)="Red"
}

