//=============================================================================
// Clone Trooper Team Info class
// 
// Created: 2003 Jan 21, John Hancock
//
// I dislike having these specific classes. This ought to be a temporary solution.
// Team relationships should be mostly data-driven/tweakable
//=============================================================================

//this is a team that is friendly to the player
class TeamHostile extends CTTeamInfo;




defaultproperties
{
     RelationTowards(4)=TEAM_Ally
     RelationTowards(5)=TEAM_Ally
     TeamName="Player-Hostile"
}

