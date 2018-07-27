//=============================================================================
// Clone Troooper Game Replication Info
//
// Created: 2003 Jan 21, John Hancock
//=============================================================================

class CTGRI extends GameReplicationInfo;

exec function SetTeamRelationship(int Team1, int Team2, ETeamRelationship relation)
{
	local CTTeamInfo CTTeam;

	CTTeam = CTTeamInfo(GetTeam(Team1));

	if ( CTTeam != None )
		CTTeam.RelationTowards[Team2] = relation;
}

function CreateTeam(int index, class<TeamInfo> teamclass)
{	
	//only create the team if an override doesn't exist
	if (Teams[index] == None)
	{
		Teams[index] = Spawn(teamclass);
		Teams[index].SetTeamIndex(index);
		TeamSize++;
	}
	else
		Log("CTGameInfo::CreateTeam: Using TeamOverride for team#"$index);
}

simulated function int WeaponInventoryGroup(Weapon Weap)
{
	return Weap.InventoryGroup;
}



defaultproperties
{
}

