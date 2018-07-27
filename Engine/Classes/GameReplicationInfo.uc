//=============================================================================
// GameReplicationInfo.
//=============================================================================
class GameReplicationInfo extends ReplicationInfo
	native
	nativereplication;

var string GameName;						// Assigned by GameInfo.
var string GameClass;						// Assigned by GameInfo.
var bool bTeamGame;							// Assigned by GameInfo.
var bool bStopCountDown;
var bool bMatchHasBegun;
var bool bMatchEnded;
var bool bTeamSymbolsUpdated;

//--Live
var bool bJoinable;                         // joinable by anybody
var bool bInvitable;                        // joinable by invite

var int  RemainingTime, ElapsedTime, RemainingMinute;
var float SecondCount;
var int GoalScore;
var int TimeLimit;

//epic pv - dedicated server support
var bool bDedicatedServer;

// Team Information
var TeamInfo Teams[MAXTEAMS];
var int TeamSize;

var() globalconfig string ServerName;		// Name of the server, i.e.: Bob's Server.
var() globalconfig string ShortName;		// Abbreviated name of server, i.e.: B's Serv (stupid example)
var() globalconfig string AdminName;		// Name of the server admin.
var() globalconfig string AdminEmail;		// Email address of the server admin.
var() globalconfig int	  ServerRegion;		// Region of the game server.

var() globalconfig string MOTDLine1;		// Message
var() globalconfig string MOTDLine2;		// Of
var() globalconfig string MOTDLine3;		// The
var() globalconfig string MOTDLine4;		// Day

var Actor Winner;			// set by gameinfo when game ends

var() array<PlayerReplicationInfo> PRIArray;

var vector FlagPos;	// replicated 2D position of one object

//gdr removed bInvitble, bJoinable from replication block since they were not
//being replication in the native rep code.  Sorted entries and adjusted native 
//rep code to use the same order.
// O_o  reinserted bInvitable/bJoinable.  necessary for xlive TCR stuff (join friend's
//      game even when he/she is not serving).  NativeRep adjusted accordingly
replication
{
	reliable if ( bNetDirty && (Role == ROLE_Authority) )
		bMatchEnded,
		bMatchHasBegun,
		bStopCountDown,
		FlagPos, 		
		RemainingMinute,
		Teams,
		Winner,
		bJoinable,
		bInvitable;


	reliable if ( bNetInitial && (Role==ROLE_Authority) )
		bTeamGame, 
		AdminEmail, 
		AdminName,
		ElapsedTime,
		GameClass, 
		GameName, 
		GoalScore, 
		MOTDLine1, 
		MOTDLine2, 
		MOTDLine3, 
		MOTDLine4, 
		RemainingTime, 
		ServerName, 
		ServerRegion, 
		ShortName, 
		TimeLimit;

}

simulated function int WeaponInventoryGroup(Weapon Weap);

simulated function PostNetBeginPlay()
{
	local PlayerReplicationInfo PRI;
	
	ForEach DynamicActors(class'PlayerReplicationInfo',PRI)
		AddPRI(PRI);
	if ( Level.NetMode == NM_Client )
		TeamSymbolNotify();
}

simulated function TeamSymbolNotify()
{
	local Actor A;

	if ( (Teams[0] == None) || (Teams[1] == None)
		|| (Teams[0].TeamIcon == None) || (Teams[1].TeamIcon == None) )
		return;
	bTeamSymbolsUpdated = true;
	ForEach AllActors(class'Actor', A)
		A.SetGRI(self);
}

simulated function PostBeginPlay()
{
	if( Level.NetMode == NM_Client )
	{
		// clear variables so we don't display our own values if the server has them left blank 
		ServerName = "";
		AdminName = "";
		AdminEmail = "";
		MOTDLine1 = "";
		MOTDLine2 = "";
		MOTDLine3 = "";
		MOTDLine4 = "";
	}

	SecondCount = Level.TimeSeconds;
	SetTimer(1, true);
}

/* Reset() 
reset actor to initial state - used when restarting level without reloading.
*/
function Reset()
{
	Super.Reset();
	Winner = None;
}

simulated function Timer()
{
	if ( Level.NetMode == NM_Client )
	{
		if (Level.TimeSeconds - SecondCount >= Level.TimeDilation)
		{
			ElapsedTime++;
			if ( RemainingMinute != 0 )
			{
				RemainingTime = RemainingMinute;
				RemainingMinute = 0;
			}
			if ( (RemainingTime > 0) && !bStopCountDown )
				RemainingTime--;
			SecondCount += Level.TimeDilation;
		}
		if ( !bTeamSymbolsUpdated )
			TeamSymbolNotify();
	}
}

simulated function AddPRI(PlayerReplicationInfo PRI)
{
	if ( !PRI.bDedicatedServerSpectator )
		PRIArray[PRIArray.Length] = PRI;
}

simulated function RemovePRI(PlayerReplicationInfo PRI)
{
    local int i;

    for (i=0; i<PRIArray.Length; i++)
    {
        if (PRIArray[i] == PRI)
            break;
    }

    if (i == PRIArray.Length)
    {
        log("GameReplicationInfo::RemovePRI() pri="$PRI$" not found.", 'Error');
        return;
    }

    PRIArray.Remove(i,1);
	}

simulated function GetPRIArray(out array<PlayerReplicationInfo> pris)
{
    local int i;
    local int num;

    pris.Remove(0, pris.Length);
    for (i=0; i<PRIArray.Length; i++)
    {
		if ((PRIArray[i] != None) && ( !PRIArray[i].bDedicatedServerSpectator))
            pris[num++] = PRIArray[i];
    }
}

function CreateTeam(int index, class<TeamInfo> teamclass);
exec function SetTeamRelationship(int Team1, int Team2, ETeamRelationship relation);

simulated event int GetNumberofTeams()
{
	return TeamSize;
}

simulated function TeamInfo GetTeam(int index)
{	
	if ( TeamSize > 0 )
		return Teams[index];
	
	return None;
}

function UseTeam(TeamInfo team)
{
	Teams[team.TeamIndex] = team;
	TeamSize++;
}



defaultproperties
{
     bStopCountDown=True
     ServerName="Test Server"
     ShortName="RC Server"
}

