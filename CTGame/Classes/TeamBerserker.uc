//=============================================================================
// Clone Trooper Team Info class
// 
// Created: 2003 Jan 21, John Hancock
//
// I dislike having these specific classes. This ought to be a temporary solution.
// Team relationships should be mostly data-driven/tweakable
//=============================================================================


class TeamBerserker extends CTTeamInfo;

function PreBeginPlay()
{
	local int i;
	for (i=1; i < MAXTEAMS; ++i){
		RelationTowards[i] = TEAM_Enemy;
	}
}

function SetTeamIndex(int index)
{
	TeamIndex = index;
}




defaultproperties
{
     TeamName="Berserker"
}

