//=============================================================================
// Info, the root of all information holding classes.
//=============================================================================
class Info extends Actor
	abstract
	hidecategories(Movement,Collision,Lighting,LightColor,Karma,Force)
	native;

//------------------------------------------------------------------------------
// Structs for reporting server state data

struct native export KeyValuePair
{
	var() string Key;
	var() string Value;
};

struct native export PlayerResponseLine
{
	var() int PlayerNum;
	var() int PlayerID;
	var() string PlayerName;
	var() int Ping;
	var() int Score;
	var() int StatsID;
	var() array<KeyValuePair> PlayerInfo;

};

struct native export ServerResponseLine
{
	var() int ServerID;
	var() string IP;
	var() int Port;
	var() int QueryPort;
	var() string ServerName;
	var() string MapName;
	var() string GameType;
	var() int CurrentPlayers;
	var() int MaxPlayers;
	var() int Ping;
	
	var() array<KeyValuePair> ServerInfo;
	var() array<PlayerResponseLine> PlayerInfo;
};

struct MeshSetLinkup
{
	var() mesh Mesh;	
	var() MeshAnimation Set;
};


defaultproperties
{
     bHidden=True
     bSkipActorPropertyReplication=True
     bOnlyDirtyReplication=True
     RemoteRole=ROLE_None
     NetUpdateFrequency=10
}

