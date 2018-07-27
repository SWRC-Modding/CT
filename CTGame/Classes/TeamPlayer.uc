//=============================================================================
// Clone Trooper Team Info class
// 
// Created: 2003 Jan 21, John Hancock
//
// I dislike having these specific classes. This ought to be a temporary solution.
// Team relationships should be mostly data-driven/tweakable
//=============================================================================

//this is a team that is friendly to the player
class TeamPlayer extends CTTeamInfo;




defaultproperties
{
     RelationTowards(1)=TEAM_Ally
     RelationTowards(2)=TEAM_Ally
     RelationTowards(3)=TEAM_Ally
     TeamName="Player-Friendly"
     TeamIndex=1
}

