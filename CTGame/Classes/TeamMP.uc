//=============================================================================
// Clone Trooper Team Info class
// 
// Created: 2003 Jan 21, John Hancock
//
// This is the default multiplayer team class
//=============================================================================

//this is a team that is friendly to the player
class TeamMP extends CTTeamInfo;
var() class<Pawn> AllowedTeamMembers[32];

//var TeamAI AI;

//SetTeamIndex will make us allies to ourselves
function bool AddToTeam( Controller Other )
{
	local bool bResult;
	
	bResult = Super.AddToTeam(Other);

	//if ( bResult && (Other.PawnClass != None) && !BelongsOnTeam(Other.PawnClass) )
	//{
	Other.PawnClass = DefaultPlayerClass;
	//}

	return bResult;
}

/* BelongsOnTeam()
returns true if PawnClass is allowed to be on this team
*/
function bool BelongsOnTeam(class<Pawn> PawnClass)
{
	return false;
	
	/*
	local int i;

	for ( i=0; i<ArrayCount(AllowedTeamMembers); i++ )
		if ( PawnClass == AllowedTeamMembers[i] )
			return true;

	return false;
	*/
}


function RemoveFromTeam(Controller Other)
{
	Super.RemoveFromTeam(Other);
	//if ( AI != None )
	//	AI.RemoveFromTeam(Other);
	/*
	for ( i=0; i<Roster.Length; i++ )
	FIXME- clear bTaken for the roster entry
	*/	
}


defaultproperties
{
     RelationTowards(8)=TEAM_Ally
}

