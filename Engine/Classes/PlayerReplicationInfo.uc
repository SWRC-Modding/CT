//=============================================================================
// PlayerReplicationInfo.
//=============================================================================
class PlayerReplicationInfo extends ReplicationInfo
	native nativereplication;

var float				Score;			// Player's current score.
var float				Deaths;			// Number of player's deaths.

var Volume				PlayerVolume;
var ZoneInfo            PlayerZone;
var int					NumLives;

var string				PlayerName;		// Player name, or blank if none.
var string				CharacterName, OldCharacterName;
var string				OldName, PreviousName;		// Temporary value.
var int					PlayerID;		// Unique id number.
var TeamInfo			Team;			// Player Team
var int					TeamID;			// Player position in team.
var class<VoicePack>	VoiceType;
var bool				bAdmin;				// Player logged in as Administrator
var bool				bIsSpectator;
var bool				bDedicatedServerSpectator;
var bool				bOnlySpectator;
var bool				bWaitingPlayer;
var bool				bReadyToPlay;
var bool				bBot;
var bool				bWelcomed;			// set after welcome message broadcast (not replicated)

// Time elapsed.
var int					StartTime;

var localized String	StringDead;
var localized String    StringSpectating;
var localized String	StringUnknown;

var int					GoalsScored;		// not replicated - used on server side only
var int					Kills;				// not replicated

var byte				Ping;
var bool				bReceivedPing;			

//--Live
var bool bIsGuest;
var bool bHasVoice;
var String xuid;
var String Gamertag;
var int Skill;

replication
{
	// Things the server should send to the client.
	reliable if ( bNetDirty && (Role == Role_Authority) )
		xuid, Gamertag, bIsGuest, bHasVoice, Skill,
		Score, Deaths, PlayerVolume, PlayerZone,
		PlayerName, Team, TeamID, VoiceType, bAdmin, 
		bIsSpectator, bOnlySpectator, bWaitingPlayer, bReadyToPlay, CharacterName;

	reliable if ( bNetDirty && (Role == Role_Authority) && (!bNetOwner || bDemoRecording) )
		Ping;

	reliable if ( bNetInitial && (Role == Role_Authority) )
		StartTime, bBot, bDedicatedServerSpectator;

	//reliable if ( Role < ROLE_Authority )
	//	ChangeHasVoice( bool VoiceOn );
}

function PostBeginPlay()
{
	if ( Role < ROLE_Authority )
		return;
    if (AIController(Owner) != None)
        bBot = true;
	StartTime = Level.Game.GameReplicationInfo.ElapsedTime;
	Timer();
	SetTimer(1.5 + FRand(), true);
}

//function ChangeHasVoice( bool VoiceOn )
//{
//	bHasVoice = VoiceOn;
//}

simulated function PostNetBeginPlay()
{
	local GameReplicationInfo GRI;
	
	ForEach DynamicActors(class'GameReplicationInfo',GRI)
	{
		if ( !bDedicatedServerSpectator )
			GRI.AddPRI(self);
		break;
	}
}

simulated function Destroyed()
{
	local GameReplicationInfo GRI;
	
	ForEach DynamicActors(class'GameReplicationInfo',GRI)
        GRI.RemovePRI(self);
        
    Super.Destroyed();
}
	
//Return the GamerTag if logged into XBox Live and the PlayerName otherwise
simulated event String GetPlayerName()
{
	if (ConsoleCommand("XLIVE GETAUTHSTATE") == "ONLINE" && !Level.IsSystemLink() && !Level.IsSplitScreen())
	{
		return GamerTag;
	}
	return PlayerName;
}

function SetCharacterName(string S)
{
	CharacterName = S;
}

/* Reset() 
reset actor to initial state - used when restarting level without reloading.
*/
function Reset()
{
	Super.Reset();
	Score = 0;
	Deaths = 0;
	bReadyToPlay = false;
	NumLives = 0;	
}

simulated function string GetHumanReadableName()
{
	return GetPlayerName();
}

simulated function string GetLocationName()
{
    if( ( PlayerVolume == None ) && ( PlayerZone == None ) )
    {
    	if ( (Owner != None) && Controller(Owner).IsInState('Dead') )
        	return StringDead;
        else
        return StringSpectating;
    }
    
	if( ( PlayerVolume != None ) && ( PlayerVolume.LocationName != class'Volume'.Default.LocationName ) )
		return PlayerVolume.LocationName;
	else if( PlayerZone != None && ( PlayerZone.LocationName != "" )  )
		return PlayerZone.LocationName;
    else if ( Level.Title != Level.Default.Title )
		return Level.Title;
	else
        return StringUnknown;
}

simulated function material GetPortrait();
event UpdateCharacter();

function UpdatePlayerLocation()
{
    local Volume V, Best;
    local Pawn P;
    local Controller C;

    C = Controller(Owner);

    if( C != None )
        P = C.Pawn;
    
    if( P == None )
		{
        PlayerVolume = None;
        PlayerZone = None;
        return;
    }
    
    if ( PlayerZone != P.Region.Zone )
		PlayerZone = P.Region.Zone;

    foreach P.TouchingActors( class'Volume', V )
    {
        if( V.LocationName == "") 
            continue;
        
        if( (Best != None) && (V.LocationPriority <= Best.LocationPriority) )
            continue;
            
        if( V.Encompasses(P) )
            Best = V;
		}
    if ( PlayerVolume != Best )
		PlayerVolume = Best;
}

/* DisplayDebug()
list important controller attributes on canvas
*/
simulated function DisplayDebug(Canvas Canvas, out float YL, out float YPos)
{
	if ( Team != None )
		Canvas.DrawText("     PlayerName "$PlayerName$" Team "$Team.GetHumanReadableName());
	else
		Canvas.DrawText("     PlayerName "$PlayerName$" NO Team");
}
 					
event ClientNameChange()
{
    local PlayerController PC;

	ForEach DynamicActors(class'PlayerController', PC)
		PC.ReceiveLocalizedMessage( class'GameMessage', 2, self );          
}

function Timer()
{
    local Controller C;

	UpdatePlayerLocation();
	SetTimer(1.5 + FRand(), true);
	if( FRand() < 0.65 )
		return;

	if( !bBot )
	{
	    C = Controller(Owner);
		if ( !bReceivedPing )
			Ping = Min(int(0.25 * float(C.ConsoleCommand("GETPING"))),255);
	}
}

function SetPlayerName(string S)
{
	OldName = PlayerName;
	PlayerName = S;
}

function SetWaitingPlayer(bool B)
{
	bIsSpectator = B;	
	bWaitingPlayer = B;
}


defaultproperties
{
     StringDead="Dead"
     StringSpectating="Spectating"
     StringUnknown="Unknown"
     bIsGuest=True
     Skill=50
     NetUpdateFrequency=5
}

