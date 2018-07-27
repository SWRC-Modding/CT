//=============================================================================
// Clone Trooper Team Info class
// 
// Created: 2003 Jan 21, John Hancock
//
// I dislike having these specific classes. This ought to be a temporary solution.
// Team relationships should be mostly data-driven/tweakable
//=============================================================================


class TeamNeutral extends CTTeamInfo //maybe just have this extend ReplicationInfo?
	placeable;

function PreBeginPlay()
{
	local int i;
	for (i=0; i < MAXTEAMS; ++i){
		RelationTowards[i] = TEAM_Neutral;
	}
}

function SetTeamIndex(int index)
{
	//Only allow 0
}




defaultproperties
{
     TeamName="Neutral"
}

