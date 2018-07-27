//=============================================================================
// Clone Trooper Team Info class
// 
// Created: 2004 Jan 30, Chris Lassonde
//=============================================================================

class CTTeamInfo extends TeamInfo
	native;

var() ETeamRelationship RelationTowards[MAXTEAMS];
var int KillsBy[MAXTEAMS];

simulated function ETeamRelationship GetRelationTowards (int TeamID)
{
	log(">> RelationTowards from"@TeamIndex@"to"@TeamID);
	return RelationTowards[TeamID]; 
}

simulated function bool IsEnemy(int TeamID)
{
	return (RelationTowards[TeamID] == TEAM_Enemy);
}

simulated function bool IsNotEnemy(int TeamID)
{
	return (RelationTowards[TeamID] != TEAM_Enemy);
}

simulated function bool IsAlly(int TeamID)
{
	return (RelationTowards[TeamID] == TEAM_Ally);
}

simulated function bool IsNotAlly(int TeamID)
{
	return (RelationTowards[TeamID] != TEAM_Ally);
}

simulated function bool IsNeutral(int TeamID)
{
	return (RelationTowards[TeamID] == TEAM_Neutral);
}

function ScoreKillsBy(int TeamID)
{
	if (TeamID < MAXTEAMS)
		KillsBy[TeamID]++;
}

function SetRelationship(int index, ETeamRelationship relation)
{
	RelationTowards[index] = relation;
}

function SetTeamIndex(int index)
{	
	RelationTowards[index] = TEAM_Ally;
	super.SetTeamIndex(index);	
}


defaultproperties
{
     RelationTowards(0)=TEAM_Neutral
}

