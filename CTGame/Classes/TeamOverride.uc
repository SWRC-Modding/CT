//=============================================================================
// Clone Trooper Team Info class
// 
// Created: 2003 Jan 21, John Hancock
//
// I dislike having these specific classes. This ought to be a temporary solution.
// Team relationships should be mostly data-driven/tweakable
//=============================================================================


class TeamOverride extends CTTeamInfo
	placeable;

function PostBeginPlay()
{
	Level.Game.GameReplicationInfo.UseTeam(self); //if an override is placed, use it instead
}




defaultproperties
{
}

