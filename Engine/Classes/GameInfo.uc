//=============================================================================
// GameInfo.
//
// The GameInfo defines the game being played: the game rules, scoring, what actors 
// are allowed to exist in this game type, and who may enter the game.  While the 
// GameInfo class is the public interface, much of this functionality is delegated 
// to several classes to allow easy modification of specific game components.  These 
// classes include GameInfo, AccessControl, Mutator, BroadcastHandler, and GameRules.  
// A GameInfo actor is instantiated when the level is initialized for gameplay (in 
// C++ UGameEngine::LoadMap() ).  The class of this GameInfo actor is determined by 
// (in order) either the DefaultGameType if specified in the LevelInfo, or the 
// DefaultGame entry in the game's .ini file (in the Engine.Engine section), unless 
// its a network game in which case the DefaultServerGame entry is used.  
//
//=============================================================================
class GameInfo extends Info
	native;

//-----------------------------------------------------------------------------
// Variables.

var bool				      bRestartLevel;			// Level should be restarted when player dies
var bool				      bPauseable;				// Whether the game is pauseable.
var config bool					bWeaponStay;              // Whether or not weapons stay when picked up.
var	bool				      bCanChangeSkin;			// Allow player to change skins in game.
var bool				      bTeamGame;				// This is a team game.
var	bool					  bGameEnded;				// set when game ends
var	bool					  bOverTime;
var localized bool			  bAlternateMode;
var	bool					  bCanViewOthers;
var bool					  bDelayedStart;
var bool					  bWaitingToStartMatch;
var globalconfig bool		  bChangeLevels;
var		bool				  bAlreadyChanged;
var bool						bLoggingGame;           // Does this gametype log?
var globalconfig bool			bEnableStatLogging;		// If True, games will log
var config bool					bAllowWeaponThrowing;
var config bool					bAllowBehindView;
var globalconfig bool			bAdminCanPause;
var globalconfig bool			bPlayerDeath;
var globalconfig bool			bHideSquadSDIcon;
var bool						bGameRestarted;
var bool						bKeepSamePlayerStart;	// used in post login
var					bool						bDedicatedServer;
var bool						bBlindAI;				// AI cheat variables
var bool						bDeafAI;
var	bool						bBlindEnemies;
var bool						bDeafEnemies;
var globalconfig bool			bLogEvents;
var globalconfig float        GameDifficulty;
var	  globalconfig int		  GoreLevel;				// 0=Normal, increasing values=less gore
var   globalconfig float	  AutoAim;					// How much autoaiming to do (1 = none, 0 = always).
													// (cosine of max error to correct)
var   float					  GameSpeed;				// Scale applied to game rate.
var   float                   StartTime;

var   autoload string		  DefaultPlayerClassName;

// user interface
var   autoload string         ScoreBoardType;           // Type of class<Menu> to use for scoreboards. (gam)
var   autoload string	      BotMenuType;				// Type of bot menu to display.
var   autoload string	      RulesMenuType;			// Type of rules menu to display.
var   autoload string		  SettingsMenuType;			// Type of settings menu to display.
var   autoload string		  GameUMenuType;			// Type of Game dropdown to display.
var   autoload string		  MultiplayerUMenuType;		// Type of Multiplayer dropdown to display.
var   autoload string		  GameOptionsMenuType;		// Type of options dropdown to display.
var	  autoload string		  HUDType;					// HUD class this game uses.
var   config string			  MapListType;				// Maplist this game uses.
var   string			      MapPrefix;				// Prefix characters for names of maps for this game type.
var   string			      BeaconName;				// Identifying string used for finding LAN servers.

var   globalconfig int	      MaxSpectators;			// Maximum number of spectators.
var	  int					  NumSpectators;			// Current number of spectators.
var   globalconfig int		  MaxPlayers; 
var   int					  NumPlayers;				// number of human players
var	  int					  NumWeaponsAllowed;		// number of weapons allowed in each weapon slot
var   int					  NumGrenadesAllowed;		// number of grenades allowed in grenade slot
//--Live
var()   globalconfig int	  NumReservedSlots;         // number of spaces to keep for invited players
var()   int                   NumInvitedPlayers;        // number of invited players connected

var	  int					  NumBots;					// number of non-human players (AI controlled but participating as a player)
var   int					  CurrentID;
var localized string	      DefaultPlayerName;
var localized string	      GameName;
var float					  FearCostFallOff;			// how fast the FearCost in NavigationPoints falls off

var config int                GoalScore;                // what score is needed to end the match
var config int                TimeLimit;                // time limit in minutes

// Message classes.
var class<LocalMessage>		  DeathMessageClass;
var class<GameMessage>		  GameMessageClass;
var	name					  OtherMesgGroup;

//-------------------------------------
// GameInfo components
var autoload string MutatorClass;
var Mutator BaseMutator;				// linked list of Mutators (for modifying actors as they enter the game)
var globalconfig autoload string AccessControlClass;
var AccessControl AccessControl;		// AccessControl controls whether players can enter and/or become admins
var GameRules GameRulesModifiers;		// linked list of modifier classes which affect game rules
var autoload string BroadcastHandlerClass;
var BroadcastHandler BroadcastHandler;	// handles message (text and localized) broadcasts

var class<PlayerController> PlayerControllerClass;	// type of player controller to spawn for players logging in
var autoload string PlayerControllerClassName;

// ReplicationInfo
var() class<GameReplicationInfo> GameReplicationInfoClass;
var GameReplicationInfo GameReplicationInfo;
var bool bWelcomePending;

// Stats - jmw
var GameStats							GameStats;				// Holds the GameStats actor
var globalconfig autoload string		GameStatsClass;			// Type of GameStats actor to spawn

// Voice chat
// ali 05-04-04 Xbox Live Voice Chat
struct VoiceChatterInfo
{
	var controller			Controller;
	var XboxAddr			xbAddr;
	var int					Handle;
	var int					VoiceChannels;
};
var array<VoiceChatterInfo>		VoiceChatters;

// Cheat Protection
var globalconfig autoload string	SecurityClass;

var() String ScreenShotName;
var() String DecoTextName;
var() String Acronym;

// localized PlayInfo descriptions & extra info
var private localized string GIPropsDisplayText[11];
var private localized string GIPropsExtras[2];

var globalconfig float MaxIdleTime;		// maximum time players are allowed to idle before being kicked

var(CustomAnims) array<MeshSetLinkup> CustomMeshSets;

//------------------------------------------------------------------------------
// Engine notifications.

function PreBeginPlay()
{
	StartTime = 0;
	SetGameSpeed(GameSpeed);
	GameReplicationInfo = Spawn(GameReplicationInfoClass);
	
	InitGameReplicationInfo();
	// Create stat logging actor.
    InitLogging();

	Super.PreBeginPlay();
}

event PostLoadBeginPlay()
{
	local int i;
	for( i = 0; i < CustomMeshSets.length; i++ )
	{
		LinkAnimToMesh( CustomMeshSets[i].Set, CustomMeshSets[i].Mesh );
	}
	Super.PostLoadBeginPlay();
}

function UpdatePrecacheMaterials()
{
	PrecacheGameTextures(Level);
}

function UpdatePrecacheStaticMeshes()
{
	PrecacheGameStaticMeshes(Level);
}

static function PrecacheGameTextures(LevelInfo myLevel);
static function PrecacheGameStaticMeshes(LevelInfo myLevel);

function string FindPlayerByID( int PlayerID )
{
    local int i;

    for( i=0; i<GameReplicationInfo.PRIArray.Length; i++ )
    {
        if( GameReplicationInfo.PRIArray[i].PlayerID == PlayerID )
            return GameReplicationInfo.PRIArray[i].GetPlayerName();
    }
    return "";
}

static function bool UseLowGore()
{
	return ( Default.bAlternateMode || (Default.GoreLevel > 0) );
}
		
function PostBeginPlay()
{
	if ( MaxIdleTime > 0 )
		MaxIdleTime = FMax(MaxIdleTime, 20);

	if (GameStats!=None)
	{
		GameStats.NewGame();
		GameStats.ServerInfo();
	}
}

/* Reset() 
reset actor to initial state - used when restarting level without reloading.
*/
function Reset()
{
	Super.Reset();
	bGameEnded = false;
	GameReplicationInfo.bMatchEnded = False;
	bOverTime = false;
	bWaitingToStartMatch = true;
	InitGameReplicationInfo();
}

/* InitLogging()
Set up statistics logging
*/
function InitLogging()
{
	local class <GameStats> MyGameStatsClass;

    if ( !bEnableStatLogging || !bLoggingGame || (Level.NetMode == NM_Standalone) || (Level.NetMode == NM_ListenServer) )
		return;

	MyGameStatsClass=class<GameStats>(DynamicLoadObject(GameStatsClass,class'class'));
    if (MyGameStatsClass!=None)
    {
		GameStats = spawn(MyGameStatsClass);
        if (GameStats==None)
        	log("Could to create Stats Object");
    }
    else
    	log("Error loading GameStats ["$GameStatsClass$"]");
}

function Timer()
{
	//local NavigationPoint N;
	local int i;

    // If we are a server, broadcast a welcome message.
    if( bWelcomePending )
    {
		bWelcomePending = false;
		if ( Level.NetMode != NM_Standalone )
		{
			for ( i=0; i<GameReplicationInfo.PRIArray.Length; i++ )
				if ( (GameReplicationInfo.PRIArray[i] != None)
					&& !GameReplicationInfo.PRIArray[i].bWelcomed )
				{
					GameReplicationInfo.PRIArray[i].bWelcomed = true;
					if ( !GameReplicationInfo.PRIArray[i].bOnlySpectator )
						BroadcastLocalizedMessage(GameMessageClass, 1, GameReplicationInfo.PRIArray[i]);
					else
						BroadcastLocalizedMessage(GameMessageClass, 16, GameReplicationInfo.PRIArray[i]);
				}
		}
	}

	BroadcastHandler.UpdateSentText();
    //for ( N=Level.NavigationPointList; N!=None; N=N.NextNavigationPoint )
	//	N.FearCost *= FearCostFallOff;
}

// Called when game shutsdown.
event GameEnding()
{
	EndLogging("serverquit");
}

/* KickIdler() called if
		if ( (Pawn != None) || (PlayerReplicationInfo.bOnlySpectator && (ViewTarget != self))
			|| (Level.Pauser != None) || Level.Game.bWaitingToStartMatch || Level.Game.bGameEnded )
		{
			LastActiveTime = Level.TimeSeconds;
		}
		else if ( (Level.Game.MaxIdleTime > 0) && (Level.TimeSeconds - LastActiveTime > Level.Game.MaxIdleTime) )
			KickIdler(self);
*/
event KickIdler(PlayerController PC)
{
	log("Kicking idle player "$PC.PlayerReplicationInfo.PlayerName);

	//TODO CL: Kick a player
	//AccessControl.DefaultKickReason = AccessControl.IdleKickReason;
	//AccessControl.KickPlayer(PC);
	//AccessControl.DefaultKickReason = AccessControl.Default.DefaultKickReason;
}

//------------------------------------------------------------------------------
// Replication

function InitGameReplicationInfo()
{
	GameReplicationInfo.bTeamGame = bTeamGame;
	GameReplicationInfo.GameName = GameName;
	GameReplicationInfo.GameClass = string(Class);    
	//epic bcl -- mark the game joinable and invitable
	GameReplicationInfo.bJoinable = true;
	// TODO CL: Scion merge
	//GameReplicationInfo.bInvitable = bRegisterGameOnline; // bRegisterGameOnline reflects whether or not this is an Xbox Live hosted program.

	//epic pv - dedicated server support
	GameReplicationInfo.bDedicatedServer = bDedicatedServer;

	//scion rwd - set the gri on the level info
	Level.GRI = GameReplicationInfo;
}

native function string GetNetworkNumber();

//------------------------------------------------------------------------------
// Server/Game Querying.

function GetServerInfo( out ServerResponseLine ServerState )
{
	ServerState.ServerName		= GameReplicationInfo.ServerName;
	ServerState.MapName			= Left(string(Level), InStr(string(Level), "."));
	ServerState.GameType		= Mid( string(Class), InStr(string(Class), ".")+1);
	ServerState.CurrentPlayers	= GetNumPlayers();
	ServerState.MaxPlayers		= MaxPlayers;
	ServerState.IP				= ""; // filled in at the other end.
	ServerState.Port			= GetServerPort();

	ServerState.ServerInfo.Length = 0;
	ServerState.PlayerInfo.Length = 0;
}

event int GetFriendlyFirePercent()
{
	return 0;
}

function int GetNumPlayers()
{
	return NumPlayers;
}

function GetServerDetails( out ServerResponseLine ServerState )
{
	local int i;
	local Mutator M;
	local GameRules G;

	i = ServerState.ServerInfo.Length;

	// servermode
	ServerState.ServerInfo.Length = i+1;
	ServerState.ServerInfo[i].Key = "servermode";
	if( Level.NetMode==NM_ListenServer )
		ServerState.ServerInfo[i++].Value = "non-dedicated";
    else
		ServerState.ServerInfo[i++].Value = "dedicated";

	// adminname
	ServerState.ServerInfo.Length = i+1;
	ServerState.ServerInfo[i].Key = "adminname";
	ServerState.ServerInfo[i++].Value = GameReplicationInfo.AdminName;
	
	// adminemail
	ServerState.ServerInfo.Length = i+1;
	ServerState.ServerInfo[i].Key = "adminemail";
	ServerState.ServerInfo[i++].Value = GameReplicationInfo.AdminEmail;

	// adminemail
	ServerState.ServerInfo.Length = i+1;
	ServerState.ServerInfo[i].Key = "ServerVersion";
	ServerState.ServerInfo[i++].Value = level.EngineVersion;

	// has password
	if( AccessControl.RequiresPassword() )
		{
		ServerState.ServerInfo.Length = i+1;
		ServerState.ServerInfo[i].Key = "password";
		ServerState.ServerInfo[i++].Value = "true";
	}

	// has stats enabled
	ServerState.ServerInfo.Length = i+1;
	ServerState.ServerInfo[i].Key = "gamestats";
	if( GameStats!=None )
		ServerState.ServerInfo[i++].Value = "true";
	else
		ServerState.ServerInfo[i++].Value = "false";

	// game speed
	if( GameSpeed != 1.0 )
			{
		ServerState.ServerInfo.Length = i+1;
		ServerState.ServerInfo[i].Key = "gamespeed";
		ServerState.ServerInfo[i++].Value = string( int(GameSpeed*100)/100.0 );
	}

	// Ask the mutators if they have anything to add.
	for (M = BaseMutator.NextMutator; M != None; M = M.NextMutator)
		M.GetServerDetails(ServerState);

	// Ask the gamerules if they have anything to add.
	for ( G=GameRulesModifiers; G!=None; G=G.NextGameRules )
		G.GetServerDetails(ServerState);
}
			
function GetServerPlayers( out ServerResponseLine ServerState )
{
    local Mutator M;
	local Controller C;
	local PlayerReplicationInfo PRI;
	local int i;

	i = ServerState.PlayerInfo.Length;

	for( C=Level.ControllerList;C!=None;C=C.NextController )
        {
			PRI = C.PlayerReplicationInfo;
			if( (PRI != None) && !PRI.bBot && MessagingSpectator(C) == None )
            {
			ServerState.PlayerInfo.Length = i+1;
			ServerState.PlayerInfo[i].PlayerNum  = C.PlayerNum;		
			ServerState.PlayerInfo[i].PlayerName = PRI.GetPlayerName();
			ServerState.PlayerInfo[i].Score		 = PRI.Score;			
			ServerState.PlayerInfo[i].Ping		 = 4 * PRI.Ping;
			i++;
		}
	}

	// Ask the mutators if they have anything to add.
	for (M = BaseMutator.NextMutator; M != None; M = M.NextMutator)
		M.GetServerPlayers(ServerState);
}

//------------------------------------------------------------------------------
// Misc.

// Return the server's port number.
event int GetServerPort()
{
	local string S;
	local int i;

	// Figure out the server's port.
	S = Level.GetAddressURL();
	i = InStr( S, ":" );
	assert(i>=0);
	return int(Mid(S,i+1));
}

function bool SetPause( BOOL bPause, PlayerController P )
{
    if( bPauseable || (bAdminCanPause && (P.IsA('Admin') || P.PlayerReplicationInfo.bAdmin)) || Level.Netmode==NM_Standalone || Level.IsSplitScreen() )
	{
		if( bPause )
		{
			Level.Pauser=P.PlayerReplicationInfo;
		}
		else
			Level.Pauser=None;
		
		return True;
	}
	else return False;
}

//------------------------------------------------------------------------------
// Voice chat.
// ali 05-04-04 Xbox Live Voice Chat
function ChangeVoiceChatter( Controller Client, XboxAddr xbAddr, int Handle, int VoiceChannels, bool Add )
{
	local int Index;
	local PlayerController P;
		
	if( Add )
	{
		Index = VoiceChatters.length;
		VoiceChatters.Insert(Index,1);
		VoiceChatters[Index].Controller		= Client;
		VoiceChatters[Index].xbAddr			= xbAddr;
		VoiceChatters[Index].Handle			= Handle;
		VoiceChatters[Index].VoiceChannels	= VoiceChannels;
	}
	else
	{
		for( Index=0; Index<VoiceChatters.Length; Index++ )
		{
			if( (VoiceChatters[Index].xbAddr.ina == xbAddr.ina) && (VoiceChatters[Index].Handle == Handle) )
				VoiceChatters.Remove(Index,1);
		}
	}
	
	foreach DynamicActors( class'PlayerController', P )
	{
		if( P != Client )
		{
			P.ClientChangeVoiceChatter( xbAddr, Handle, VoiceChannels, Add );
		}
	}
}


// add/remove voice chatters based on team.
function UpdateTeamVoiceChatters()
{
	local int Index;
	local PlayerController P;
	local PlayerController P2;

	foreach DynamicActors( class'PlayerController', P )
	{
		for( Index=0; Index<VoiceChatters.Length; Index++ )
		{
			// turn on / off voice chatters based on teams.
			P2 = PlayerController( VoiceChatters[Index].Controller );
			
			if (P2 == P)
			{
				// myself.  don't turn on or off.
			}
			else if( P2 != None &&
				P2.PlayerReplicationInfo != None &&
				P2.PlayerReplicationInfo.Team == P.PlayerReplicationInfo.Team )
			{
				Log(P.GamerTag@" Turning ON voice for"@P2.GamerTag);

				P.ClientChangeVoiceChatter( VoiceChatters[Index].xbAddr, VoiceChatters[Index].Handle, VoiceChatters[Index].VoiceChannels, true );
			}
			else
			{
				Log(P.GamerTag@" Turning OFF voice for"@P2.GamerTag);

				P.ClientChangeVoiceChatter( VoiceChatters[Index].xbAddr, VoiceChatters[Index].Handle, 0, false );
			}
		}
		
	}
}

//------------------------------------------------------------------------------
// Game parameters.

//
// Set gameplay speed.
//
function SetGameSpeed( Float T )
{
	local float OldSpeed;

	OldSpeed = GameSpeed;
	GameSpeed = FMax(T, 0.1);
	Level.TimeDilation = GameSpeed;
	if ( GameSpeed != OldSpeed )
    {
		Default.GameSpeed = GameSpeed;
		class'GameInfo'.static.StaticSaveConfig();
	}
	SetTimer(Level.TimeDilation, true);
}

//
// Called after setting low or high detail mode.
//
event DetailChange()
{
	local actor A;
	local zoneinfo Z;

    if( Level.DetailMode == DM_Low )
	{
		foreach DynamicActors(class'Actor', A)
		{
            if( (A.bHighDetail || A.bSuperHighDetail) && !A.bGameRelevant )
                A.Destroy();
        }
    }
    else if( Level.DetailMode == DM_High )
    {
        foreach DynamicActors(class'Actor', A)
        {
            if( A.bSuperHighDetail && !A.bGameRelevant )
				A.Destroy();
		}
	}
	foreach AllActors(class'ZoneInfo', Z)
		Z.LinkToSkybox();
}

//------------------------------------------------------------------------------
// Player start functions

//
// Grab the next option from a string.
//
//--Live I need this static so I can use it elsewhere
static function bool GrabOption( out string Options, out string Result )
{
	if( Left(Options,1)=="?" )
	{
		// Get result.
		Result = Mid(Options,1);
		if( InStr(Result,"?")>=0 )
			Result = Left( Result, InStr(Result,"?") );

		// Update options.
		Options = Mid(Options,1);
		if( InStr(Options,"?")>=0 )
			Options = Mid( Options, InStr(Options,"?") );
		else
			Options = "";

		return true;
	}
	else return false;
}

//
// Break up a key=value pair into its key and value.
//
//--Live I need this static so I can use it elsewhere
static function GetKeyValue( string Pair, out string Key, out string Value )
{
	if( InStr(Pair,"=")>=0 )
	{
		Key   = Left(Pair,InStr(Pair,"="));
		Value = Mid(Pair,InStr(Pair,"=")+1);
	}
	else
	{
		Key   = Pair;
		Value = "";
	}
}

/* ParseOption()
 Find an option in the options string and return it.
*/
//--Live I need this static so I can use it elsewhere
static function string ParseOption( string Options, string InKey )
{
	local string Pair, Key, Value;
	while( GrabOption( Options, Pair ) )
	{
		GetKeyValue( Pair, Key, Value );
		if( Key ~= InKey )
			return Value;
	}
	return "";
}

//
// HasOption - return true if the option is specified on the command line.
//
//--Live I need this static so I can use it elsewhere
static function bool HasOption( string Options, string InKey )
{
    local string Pair, Key, Value;
    while( GrabOption( Options, Pair ) )
    {
        GetKeyValue( Pair, Key, Value );
        if( Key ~= InKey )
            return true;
    }
    return false;
}

//--Live - Added from UC.  Not sure why it is simulated
static simulated function String ParseToken(out String Str)
{
	local String Ret;
	local int len;

	Ret = "";
	len = 0;

	// Skip spaces and tabs.
	while( Left(Str,1)==" " || Asc(Left(Str,1))==9 )
		Str = Mid(Str, 1);

	if( Asc(Left(Str,1)) == 34 )
	{
		// Get quoted String.
		Str = Mid(Str, 1);
		while( Str!="" && Asc(Left(Str,1))!=34 )
		{
			Ret = Ret $ Mid(Str,0,1);
			Str = Mid(Str, 1);
		}
		if( Asc(Left(Str,1))==34 )
			Str = Mid(Str, 1);
	}
	else
	{
		// Get unquoted String.
		for( len=0; (Str!="" && Left(Str,1)!=" " && Asc(Left(Str,1))!=9); Str = Mid(Str, 1) )
			Ret = Ret $ Mid(Str,0,1);
	}

	return Ret;
}

/* Initialize the game.
 The GameInfo's InitGame() function is called before any other scripts (including 
 PreBeginPlay() ), and is used by the GameInfo to initialize parameters and spawn 
 its helper classes.
 Warning: this is called before actors' PreBeginPlay.
*/
event InitGame( string Options, out string Error )
{
	local string InOpt, LeftOpt;
	local int pos;
	local class<AccessControl> ACClass;
	local class<GameRules> GRClass;
	local class<BroadcastHandler> BHClass;

	log( "InitGame:" @ Options );

    MaxPlayers       = Clamp(GetIntOption( Options, "MaxPlayers", MaxPlayers ),0,32);
    MaxSpectators    = Clamp(GetIntOption( Options, "MaxSpectators", MaxSpectators ),0,32);
    GameDifficulty   = FMax(0,GetIntOption(Options, "Difficulty", GameDifficulty));
	
	//--Live
	//IsSystemLink not working because options have not been setup on gameinfo yet
	if ( HasOption( Options, "LAN") || Level.IsSplitScreen() )
		NumReservedSlots = 0;
	else
		NumReservedSlots = Clamp(GetIntOption( Options, "ReservedSlots", NumReservedSlots ),0,32);

	InOpt = ParseOption( Options, "GameSpeed");
	if( InOpt != "" )
	{
		log("GameSpeed"@InOpt);
		SetGameSpeed(float(InOpt));
	}

    AddMutator(MutatorClass); 
	
	BHClass = class<BroadcastHandler>(DynamicLoadObject(BroadcastHandlerClass,Class'Class'));
	BroadcastHandler = spawn(BHClass);

	InOpt = ParseOption( Options, "AccessControl");
	if( InOpt != "" )
		ACClass = class<AccessControl>(DynamicLoadObject(InOpt, class'Class'));
    if ( ACClass == None )
	{
		ACClass = class<AccessControl>(DynamicLoadObject(AccessControlClass, class'Class'));
		if (ACClass == None)
			ACClass = class'Engine.AccessControl';
	}

	LeftOpt = ParseOption( Options, "AdminName" );
	InOpt = ParseOption( Options, "AdminPassword");
	if( LeftOpt!="" && InOpt!="" )
		ACClass.default.bDontAddDefaultAdmin = true;

	AccessControl = Spawn(ACClass);
//FIXME_MERGE	if (AccessControl != None && LeftOpt!="" && InOpt!="" )
//        AccessControl.SetAdminFromURL(LeftOpt, InOpt);

	InOpt = ParseOption( Options, "GameRules");
	if ( InOpt != "" )
	{
		log("Game Rules"@InOpt);
		while ( InOpt != "" )
		{
			pos = InStr(InOpt,",");
			if ( pos > 0 )
			{
				LeftOpt = Left(InOpt, pos);
				InOpt = Right(InOpt, Len(InOpt) - pos - 1);
			}
			else
			{
				LeftOpt = InOpt;
				InOpt = "";
			}
			log("Add game rules "$LeftOpt);
			GRClass = class<GameRules>(DynamicLoadObject(LeftOpt, class'Class'));
			if ( GRClass != None )
			{
				if ( GameRulesModifiers == None )
					GameRulesModifiers = Spawn(GRClass);
				else	
					GameRulesModifiers.AddGameRules(Spawn(GRClass));
			}
		}
	}

	log("Base Mutator is "$BaseMutator);
    
	InOpt = ParseOption( Options, "Mutator");
	if ( InOpt != "" )
	{
		log("Mutators"@InOpt);
		while ( InOpt != "" )
		{
			pos = InStr(InOpt,",");
			if ( pos > 0 )
			{
				LeftOpt = Left(InOpt, pos);
				InOpt = Right(InOpt, Len(InOpt) - pos - 1);
			}
			else
			{
				LeftOpt = InOpt;
				InOpt = "";
			}
			log("Add mutator "$LeftOpt);
            AddMutator(LeftOpt, true); 
		}
	}

	InOpt = ParseOption( Options, "GamePassword");
	if( InOpt != "" )
	{
		AccessControl.SetGamePassWord(InOpt);
		log( "GamePassword" @ InOpt );
	}

    InOpt = ParseOption( Options,"AllowThrowing");
    if ( InOpt != "" )
    	bAllowWeaponThrowing = bool (InOpt);

	InOpt = ParseOption( Options,"AllowBehindview");
    if ( InOpt != "" )
    	bAllowBehindview = bool ( InOpt);

	InOpt = ParseOption(Options, "GameStats");
	if ( InOpt != "")
		bEnableStatLogging = bool(InOpt);
	else
		bEnableStatLogging = false;
	
	log("GameInfo::InitGame : bEnableStatLogging"@bEnableStatLogging);
	 
//	if( HasOption(Options, "DemoRec") )
//		Log( Level.ConsoleCommand("demorec"@ParseOption(Options, "DemoRec")) );
	
}

function AddMutator(string mutname, optional bool bUserAdded)
{
    local class<Mutator> mutClass;
    local Mutator mut;

    mutClass = class<Mutator>(DynamicLoadObject(mutname, class'Class'));
    if (mutClass == None)
        return;
	
	if ( (mutClass.Default.GroupName != "") && (BaseMutator != None) )
	{
		// make sure no mutators with same groupname
		for ( mut=BaseMutator; mut!=None; mut=mut.NextMutator )
			if ( mut.GroupName == mutClass.Default.GroupName )
				return;
	}
	 
    mut = Spawn(mutClass);
	// mc, beware of mut being none
	if (mut == None)
		return;

	// Meant to verify if this mutator was from Command Line parameters or added from other Actors
	mut.bUserAdded = bUserAdded;

    if (BaseMutator == None)
        BaseMutator = mut;
    else
        BaseMutator.AddMutator(mut);
}

//
// Return beacon text for serverbeacon.
//
event string GetBeaconText()
{
    local String BeaconText;
    local String MapName;
    local PlayerController PC;
    local String HostName;

    HostName = Level.ComputerName;

    foreach DynamicActors( class'PlayerController', PC )
    {
        if( (NetConnection(PC.Player) == None) && (PC.PlayerReplicationInfo != None) )
        {
            HostName = PC.PlayerReplicationInfo.GetPlayerName();
            break;
        }
    }

    MapName = String(Level);
    MapName = Left( MapName, InStr(MapName, ".") );
    
    BeaconText = 
        "###"						@
        "\"" $ HostName		$ "\""	@
        "\"" $ Class		$ "\""	@
        "\"" $ Acronym		$ "\""	@
        "\"" $ MapName		$ "\""	@
        "\"" $ Level.Title  $ "\""	@
        NumPlayers					@
        MaxPlayers					@
        "###";

    return( BeaconText );        
}

/* ProcessServerTravel()
 Optional handling of ServerTravel for network games.
*/
function ProcessServerTravel( string URL, bool bItems )
{
	local playercontroller P, LocalPlayer;
	local string ClientURL, S;

	ClientURL = ExpandRelativeURL(URL);
	ClientURL = ClientURL $ ConsoleCommand("XLIVE CLIENT_TRAVEL");

    // Pass it along
    BaseMutator.ServerTraveling(ClientURL,bItems);

	EndLogging("mapchange");

	log("ProcessServerTravel setting ClientTravel:"@ClientURL);

	// Notify clients we're switching level and give them time to receive.
	// We call PreClientTravel directly on any local PlayerPawns (ie listen server)
	log("ProcessServerTravel:"@ClientURL);
	foreach DynamicActors( class'PlayerController', P )
		if( NetConnection( P.Player)!=None )
		{
			S = ClientURL $ "?Team="$P.PlayerReplicationInfo.Team.TeamIndex;
			log( "Sending" @ P.PlayerReplicationInfo.PlayerName @ "to" @ S );
			P.ClientTravel( S, TRAVEL_Relative, bItems );
//			P.ClientTravel( ClientURL, TRAVEL_Relative, bItems );
		}
		else
		{	
			LocalPlayer = P;
			P.PreClientTravel();
		}

	if ( (Level.NetMode == NM_ListenServer) && (LocalPlayer != None) )
        Level.NextURL = Level.NextURL
					 $"?Team="$LocalPlayer.GetDefaultURL("Team")
					 $"?Name="$LocalPlayer.GetDefaultURL("Name")
                     $"?Class="$LocalPlayer.GetDefaultURL("Class")
                     $"?Character="$LocalPlayer.GetDefaultURL("Character"); 

	// Switch immediately if not networking.
	if( Level.NetMode!=NM_DedicatedServer && Level.NetMode!=NM_ListenServer )
		Level.NextSwitchCountdown = 0.0;
}

//Live
native final function bool PreServerTravel();

//
// Accept or reject a player on the server.
// Fails login if you set the Error to a non-empty string.
//
event PreLogin
(
	string Options,
	string Address,
	out string Error,
	out string FailCode
)
{
	local bool bSpectator;

    bSpectator = ( ParseOption( Options, "SpectatorOnly" ) ~= "true" );
	AccessControl.PreLogin(Options, Address, Error, FailCode, bSpectator);
}

function int GetIntOption( string Options, string ParseString, int CurrentValue)
{
	local string InOpt;

	InOpt = ParseOption( Options, ParseString );
	if ( InOpt != "" )
	{
		log(ParseString@InOpt);
		return int(InOpt);
	}	
	return CurrentValue;
}

function bool BecomeSpectator(PlayerController P);

function bool AllowBecomeSpectator(PlayerController P)
{
	return false;
}

function bool AllowBecomeActivePlayer(PlayerController P)
{
	return true;
}

//--Live - migrated these from UC
event bool AtCapacity( bool bSpectator, bool bInvited )
{
	local int PublicSlotsFilled, PublicSlotsAvailable;
	local int PrivateSlotsFilled, PrivateSlotsAvailable;
	//local string logMessage;

	if( Level.NetMode == NM_Standalone )
		return false;

	if( bSpectator )
		return( (NumSpectators >= MaxSpectators) && ((Level.NetMode != NM_ListenServer) || (NumPlayers > 0)) );

	PrivateSlotsFilled = Min( NumInvitedPlayers, NumReservedSlots );
	PrivateSlotsAvailable = NumReservedSlots - PrivateSlotsFilled;

	PublicSlotsFilled = NumPlayers - PrivateSlotsFilled;
	PublicSlotsAvailable = MaxPlayers - (NumReservedSlots + PublicSlotsFilled);

//gdr Cutting out some spam since the number of players issue in test seems to have gone away
/*
	LogMessage = ">> AtCapacity(bSpectator=" $ bSpectator $ ", bInvited=" $ bInvited $ ")";
	log(logMessage);

	LogMessage = ">> PrivateSlotsFilled = " $ PrivateSlotsFilled $ ", PrivateSlotsAvailable = " $ PrivateSlotsAvailable;
	log(logMessage);

	LogMessage = ">> PublicSlotsFilled  = " $ PublicSlotsFilled $ ", PublicSlotsAvailable  = " $ PublicSlotsAvailable;
	log(logMessage);
*/

	if( bInvited )
		return( (PrivateSlotsAvailable <= 0) && (PublicSlotsAvailable <= 0) );
	else
		return( PublicSlotsAvailable <= 0 );
}

function UpdateCapacity()
{
	local String S;

	if( GameReplicationInfo == None )
		return;

	GameReplicationInfo.bJoinable = !AtCapacity(false,false);
	GameReplicationInfo.bInvitable = !AtCapacity(false,true);

	S = "Capacity updated:";

	if( GameReplicationInfo.bJoinable )
		S = S @ "joinable,";
	else
		S = S @ "not joinable,";

	if( GameReplicationInfo.bInvitable )
		S = S @ "invitable.";
	else
		S = S @ "not invitable.";

	// Log( S );
}
//and removed this one
/*
function bool AtCapacity(bool bSpectator)
{
	if ( Level.NetMode == NM_Standalone )
		return false;

	if ( bSpectator )
		return ( (NumSpectators >= MaxSpectators)
			&& ((Level.NetMode != NM_ListenServer) || (NumPlayers > 0)) );
	else
		return ( (MaxPlayers>0) && (NumPlayers>=MaxPlayers) );
}
*/

//
// Log a player in.
// Fails login if you set the Error string.
// PreLogin is called before Login, but significant game time may pass before
// Login is called, especially if content is downloaded.
//
event PlayerController Login
(
	string Portal,
	string Options,
	out string Error
)
{
	local NavigationPoint StartSpot;
	local PlayerController NewPlayer;
	local Pawn      TestPawn;
    local string          InName, InAdminName, InPassword, InChecksum, InClass; 
	local byte            InTeam;
    local bool bSpectator, bAdmin, bInvited;
    local class<Security> MySecurityClass;
local bool					bDedicatedServerSpectator;
    bSpectator = ( ParseOption( Options, "SpectatorOnly" ) ~= "true" );
//FIXME_MERGE	bAdmin = AccessControl.CheckOptionsAdmin(Options);

    bInvited = bAdmin || bool(ParseOption( Options, "WasInvited" ) );

	// Make sure there is capacity except for admins. (This might have changed since the PreLogin call).
	if ( !bAdmin && AtCapacity(bSpectator,bInvited) )
	{
		Error = "__SERVERFULL__"; //GameMessageClass.Default.MaxedOutMessage;
		return None;
	}

    	// If admin, force spectate mode if the server already full of reg. players
	if ( bAdmin && AtCapacity(bSpectator, bInvited)) //epic bcl --gam's AtCapacity fix
		bSpectator = true;
	
	//epic pv - dedicated server support
	bDedicatedServer = ( ParseOption( Options, "DedicatedServer" ) ~= "true" );
	if ( bDedicatedServer )
	{
		bAdmin = true;
		bDedicatedServerSpectator = true;		
		if (GameReplicationInfo != None)
			GameReplicationInfo.bDedicatedServer = true;
		bSpectator = true;
	}

	BaseMutator.ModifyLogin(Portal, Options);

	// Get URL options.
	InName     = Left(ParseOption ( Options, "Name"), 20);
	InTeam     = GetIntOption( Options, "Team", 255 ); // default to "no team"
	InAdminName= ParseOption ( Options, "AdminName");
	InPassword = ParseOption ( Options, "Password" );
	InChecksum = ParseOption ( Options, "Checksum" );

	//epic pv - dedicated server support
	if (bDedicatedServerSpectator)
		InName = "DedicatedServer";
	
	// Pick a team (if need teams)
	InTeam = PickTeam(InTeam,None);
		 
	// Find a start spot.
	StartSpot = FindPlayerStart( None, InTeam, Portal );

	if( StartSpot == None )
	{
		Error = GameMessageClass.Default.FailedPlaceMessage;
		return None;
	}

	if ( PlayerControllerClass == None )
		PlayerControllerClass = class<PlayerController>(DynamicLoadObject(PlayerControllerClassName, class'Class'));

	NewPlayer = spawn(PlayerControllerClass,,,StartSpot.Location,StartSpot.Rotation);

	// Handle spawn failure.
	if( NewPlayer == None )
	{
		log("Couldn't spawn player controller of class "$PlayerControllerClass);
		Error = GameMessageClass.Default.FailedSpawnMessage;
		return None;
	}

	NewPlayer.StartSpot = StartSpot;

	// Init player's replication info
	NewPlayer.GameReplicationInfo = GameReplicationInfo;
	NewPlayer.GotoState('Spectating');

	// Apply security to this controller
	
	MySecurityClass=class<Security>(DynamicLoadObject(SecurityClass,class'class'));
	if (MySecurityClass!=None)
	{
		NewPlayer.PlayerSecurity = spawn(MySecurityClass,NewPlayer);
		if (NewPlayer.PlayerSecurity==None)
			log("Could not spawn security for player "$NewPlayer,'Security');
	}	
	else
	    log("Unknown security class ["$SecurityClass$"] -- System is no secure.",'Security');

	// Init player's name
	if( InName=="" )
		InName=DefaultPlayerName;
	if( Level.NetMode!=NM_Standalone || NewPlayer.PlayerReplicationInfo.PlayerName==DefaultPlayerName )
		ChangeName( NewPlayer, InName, false );

	if ( bSpectator || NewPlayer.PlayerReplicationInfo.bOnlySpectator )
	{
		NewPlayer.PlayerReplicationInfo.bDedicatedServerSpectator = bDedicatedServerSpectator;
		NewPlayer.PlayerReplicationInfo.bOnlySpectator = true;
		NewPlayer.PlayerReplicationInfo.bIsSpectator = true;		
		NumSpectators++;
		return NewPlayer;
	}
	 
	 newPlayer.StartSpot = StartSpot;
	
    // Init player's administrative privileges and log it
/*FIXME_MERGE    if (AccessControl.AdminLogin(NewPlayer, InAdminName, InPassword))
    {
		AccessControl.AdminEntered(NewPlayer, InAdminName);
    }
*/
	// Set the player's ID.
	NewPlayer.PlayerReplicationInfo.PlayerID = CurrentID++;

	InClass = ParseOption( Options, "Class" );//MHi - getting command line and/or .ini opinion
    if (InClass == "")
        InClass = DefaultPlayerClassName;

    NewPlayer.SetPawnClass(InClass);
	 if (NewPlayer.PawnClass == None)
		 Warn("Failed to assign pawn class to new player!");
	
	 NumPlayers++;
    
	//epic bcl -- UC1 -------------------------
	// Count the player as invited if they were invited
	// gam ---
	newPlayer.bWasInvited = bInvited;
	// --- gam

	if( newPlayer.bWasInvited )
	{
		log( "NumInvitedPlayers++" );
		NumInvitedPlayers++;
	}
	// --- gam
	//epic bcl -- UC1 -------------------------

	//epic bcl -- put the Live status on the url.
	if (HasOption(Options, "XboxLiveClient"))
		NewPlayer.UpdateURL("XboxLiveClient", ParseOption(Options, "XboxLiveClient"), true);
	

    bWelcomePending = true;

	// TODO: check something less totally lame.
	if (IsOnConsole() && int(ConsoleCommand("NUMVIEWPORTS")) > 1)
	{
		NewPlayer.GotoState('ChooseProfile');
		return NewPlayer;
	}
	else if ( bTeamGame && Level.NetMode != NM_Standalone)
	{
		NewPlayer.GotoState('ChooseTeam');
		return NewPlayer;
	}

	 // Change player's team.
	 // don't worry bout this til below where we ask them to choose a team
	 else if ( !ChangeTeam(newPlayer, InTeam, false) )
	 {
		 Error = GameMessageClass.Default.FailedTeamMessage;
		 return None;
	 }
	 
	// if delayed start, don't give a pawn to the player yet
	// Normal for multiplayer games
	if ( bDelayedStart )
	{
		NewPlayer.GotoState('PlayerWaiting');
		return NewPlayer;	
	}

	// Try to match up to existing unoccupied player in level,
	// for savegames and coop level switching.
	ForEach DynamicActors(class'Pawn', TestPawn )
	{
		if ( (TestPawn!=None) && (PlayerController(TestPawn.Controller)!=None) && (PlayerController(TestPawn.Controller).Player==None) && (TestPawn.Health > 0)
			&&  (TestPawn.OwnerName~=InName) )
		{
			NewPlayer.Destroy();
			TestPawn.SetRotation(TestPawn.Controller.Rotation);
			TestPawn.PlayWaiting();
			return PlayerController(TestPawn.Controller);
		}
	}
	return newPlayer;
}	

/* StartMatch()
Start the game - inform all actors that the match is starting, and spawn player pawns
*/
function StartMatch()
{	
	local Controller P;
	local Actor A; 

	if (GameStats!=None)
		GameStats.StartGame();

	// tell all actors the game is starting
	ForEach AllActors(class'Actor', A)
		A.MatchStarting();

	// start human players first
	for ( P = Level.ControllerList; P!=None; P=P.nextController )
		if ( P.IsA('PlayerController') && (P.Pawn == None) )
		{
            if ( bGameEnded ) 
                return; // telefrag ended the game with ridiculous frag limit
            else if ( PlayerController(P).CanRestartPlayer()  ) 
				RestartPlayer(P);
		}

	// start AI players
	for ( P = Level.ControllerList; P!=None; P=P.nextController )
		if ( P.bIsPlayer && !P.IsA('PlayerController') )
        {
			if ( Level.NetMode == NM_Standalone )
			RestartPlayer(P);
        	else
				P.GotoState('Dead','MPStart');
		}

	bWaitingToStartMatch = false;
	GameReplicationInfo.bMatchHasBegun = true;
}

//
// Restart a player.
//
function RestartPlayer( Controller aPlayer )	
{
	local NavigationPoint startSpot;
	local int TeamNum;
	local class<Pawn> DefaultPlayerClass;

	if( bRestartLevel && Level.NetMode!=NM_DedicatedServer && Level.NetMode!=NM_ListenServer )
		return;

	if ( (aPlayer.PlayerReplicationInfo == None) || (aPlayer.PlayerReplicationInfo.Team == None) )
		TeamNum = 255;
	else
		TeamNum = aPlayer.PlayerReplicationInfo.Team.TeamIndex;

	startSpot = FindPlayerStart(aPlayer, TeamNum);
	if( startSpot == None )
	{
		log(" Player start not found!!!");
		return;
	}	
	
	if (aPlayer.PreviousPawnClass!=None && aPlayer.PawnClass != aPlayer.PreviousPawnClass)
		BaseMutator.PlayerChangedClass(aPlayer);			

	if( aPlayer.Pawn==None && aPlayer.PawnClass != None )
		aPlayer.Pawn = Spawn(aPlayer.PawnClass,,,StartSpot.Location,StartSpot.Rotation);
	
	if( aPlayer.Pawn==None )
	{
	    DefaultPlayerClass = GetDefaultPlayerClass(aPlayer);
		aPlayer.Pawn = Spawn(DefaultPlayerClass,,,StartSpot.Location,StartSpot.Rotation);
	}
	if( aPlayer.Pawn == None )
	{
		log("Couldn't spawn player of type "$aPlayer.PawnClass$" at "$StartSpot);
		aPlayer.GotoState('Dead');
		return;
	}
    aPlayer.Pawn.Anchor = startSpot;	
	aPlayer.Pawn.LastStartSpot = PlayerStart(startSpot);
	aPlayer.Pawn.LastStartTime = Level.TimeSeconds;
	aPlayer.PreviousPawnClass = aPlayer.Pawn.Class;

	aPlayer.Possess(aPlayer.Pawn);
	aPlayer.PawnClass = aPlayer.Pawn.Class;

    aPlayer.Pawn.PlayTeleportEffect(true, true);
	aPlayer.ClientSetRotation(aPlayer.Pawn.Rotation);
		
	SetPlayerDefaults(aPlayer.Pawn);
	
	TriggerEvent( StartSpot.Event, StartSpot, aPlayer.Pawn);
}

function class<Pawn> GetDefaultPlayerClass(Controller C)
{
    local PlayerController PC;
    local String PawnClassName;
    local class<Pawn> PawnClass;
	
    PC = PlayerController( C );

    if( PC != None )
{
        PawnClassName = PC.GetDefaultURL( "Class" );
        PawnClass = class<Pawn>( DynamicLoadObject( PawnClassName, class'Class') );

        if( PawnClass != None )
            return( PawnClass );
}

    return( class<Pawn>( DynamicLoadObject( DefaultPlayerClassName, class'Class' ) ) );
}

//
// Called after a successful login. This is the first place
// it is safe to call replicated functions on the PlayerPawn.
//
event PostLogin( PlayerController NewPlayer )
{
    local class<HUD> HudClass;
    local class<Scoreboard> ScoreboardClass;
    //local String SongName;

	//--Live
	UpdateCapacity();

    // Log player's login.
	if (GameStats!=None)
	{
		GameStats.ConnectEvent(NewPlayer.PlayerReplicationInfo);
		GameStats.GameEvent("NameChange",NewPlayer.PlayerReplicationInfo.playername,NewPlayer.PlayerReplicationInfo);		
	}

	if ( !bDelayedStart )
	{
		// start match, or let player enter, immediately
		bRestartLevel = false;	// let player spawn once in levels that must be restarted after every death
		bKeepSamePlayerStart = true;
		if ( bWaitingToStartMatch )
			StartMatch();
		else
			RestartPlayer(newPlayer);
		bKeepSamePlayerStart = false;
		bRestartLevel = Default.bRestartLevel;
	}

	// Start player's music.
	/*
    SongName = Level.Song;
    if( SongName != "" && SongName != "None" )
        NewPlayer.ClientSetMusic( SongName, MTRAN_Fade );
	*/

	// tell client what hud and scoreboard to use

    if( HUDType != "" )
        HudClass = class<HUD>(DynamicLoadObject(HUDType, class'Class'));

    if( ScoreBoardType != "" )
        ScoreboardClass = class<Scoreboard>(DynamicLoadObject(ScoreBoardType, class'Class'));
    NewPlayer.ClientSetHUD( HudClass, ScoreboardClass );

	if ( NewPlayer.Pawn != None )
		NewPlayer.Pawn.ClientSetRotation(NewPlayer.Pawn.Rotation);

	//--Live
	if( NewPlayer.PlayerReplicationInfo == None )
		log( "Can't set gamertag/xuid yet -- no PRI!!!", 'Error' );
	else
	{
		NewPlayer.PlayerReplicationInfo.Gamertag	= NewPlayer.Gamertag;
		NewPlayer.PlayerReplicationInfo.xuid		= NewPlayer.xuid;
		NewPlayer.PlayerReplicationInfo.Skill		= NewPlayer.Skill;
		NewPlayer.PlayerReplicationInfo.bIsGuest	= NewPlayer.bIsGuest;
	}

//Live
	UpdateCapacity();
}

//
// Player exits.
//
function Logout( Controller Exiting )
{
	local bool bMessage;
	local int Index;

	bMessage = true;
	if ( PlayerController(Exiting) != None )
	{
//FIXME_MERGE		if ( AccessControl.AdminLogout( PlayerController(Exiting) ) )
//			AccessControl.AdminExited( PlayerController(Exiting) );

        if ( PlayerController(Exiting).PlayerReplicationInfo.bOnlySpectator )
		{
			bMessage = false;
			NumSpectators--;
		}
		else
        {
			NumPlayers--;
			//--Live
			if( PlayerController(Exiting).bWasInvited )
			{
				log( "NumInvitedPlayers--" );
				NumInvitedPlayers--;
			}
        }
			
		if( Level.NetMode != NM_Standalone )
		{
			for( Index=0; Index<VoiceChatters.Length; Index++ )
			{
				if( VoiceChatters[Index].Controller == Exiting )
				{
					ChangeVoiceChatter( PlayerController(Exiting), VoiceChatters[Index].xbAddr, VoiceChatters[Index].Handle, 0, false );
				}
			}
		}

		if( Level.NetMode == NM_Client )
		{
			PlayerController(Exiting).ClientLeaveVoiceChat();
		}
	}
	if( bMessage && (Level.NetMode==NM_DedicatedServer || Level.NetMode==NM_ListenServer) )
		BroadcastLocalizedMessage(GameMessageClass, 4, Exiting.PlayerReplicationInfo);

	if ( GameStats!=None)
		GameStats.DisconnectEvent(Exiting.PlayerReplicationInfo);

	//--Live
	UpdateCapacity();
}

//
// Examine the passed player's inventory, and accept or discard each item.
// AcceptInventory needs to gracefully handle the case of some inventory
// being accepted but other inventory not being accepted (such as the default
// weapon).  There are several things that can go wrong: A weapon's
// AmmoType not being accepted but the weapon being accepted -- the weapon
// should be killed off. Or the player's selected inventory item, active
// weapon, etc. not being accepted, leaving the player weaponless or leaving
// the HUD inventory rendering messed up (AcceptInventory should pick another
// applicable weapon/item as current).
//
event AcceptInventory(pawn PlayerPawn);

function AddGameSpecificInventory(Pawn p)
{
    local Weapon newWeapon;
    local class<Weapon> WeapClass;
    local Inventory Inv;
	
    // Spawn default weapon.
    WeapClass = BaseMutator.GetDefaultWeapon();
    if( (WeapClass!=None) && (p.FindInventoryType(WeapClass)==None) )
    {
        newWeapon = Spawn(WeapClass,,,p.Location);
        if( newWeapon != None )
        {
            Inv = None;
            // search pawn's inventory for a bCanThrowWeapon==false, if we find one, don't call Bringup
            for( Inv=Inventory; Inv!=None; Inv=Inv.Inventory )
            {
                if ( Inv.IsA('Weapon') && Weapon(Inv).bCanThrow==false )
                    break;
            }

            newWeapon.GiveTo(p);
            newWeapon.bCanThrow = false; // don't allow default weapon to be thrown out
        }
    }	
}

//
// Spawn any default inventory for the player.
//
function AddDefaultInventory( pawn PlayerPawn )
{
	local Weapon newWeapon;
	local class<Weapon> WeapClass;

	// Spawn default weapon.
	WeapClass = BaseMutator.GetDefaultWeapon();
	if( (WeapClass!=None) && (PlayerPawn.FindInventoryType(WeapClass)==None) )
	{
		newWeapon = Spawn(WeapClass,,,PlayerPawn.Location);
		if( newWeapon != None )
		{					
			newWeapon.GiveTo(PlayerPawn);			

            //newWeapon.BringUp();
			newWeapon.bCanThrow = false; // don't allow default weapon to be thrown out
		}
	}
}

/* SetPlayerDefaults()
 first make sure pawn properties are back to default, then give mutators an opportunity
 to modify them
*/
function SetPlayerDefaults(Pawn PlayerPawn)
{
	PlayerPawn.AirControl = PlayerPawn.Default.AirControl;
    PlayerPawn.GroundSpeed = PlayerPawn.Default.GroundSpeed;
    PlayerPawn.WaterSpeed = PlayerPawn.Default.WaterSpeed;
    PlayerPawn.AirSpeed = PlayerPawn.Default.AirSpeed;
    PlayerPawn.Acceleration = PlayerPawn.Default.Acceleration;
    PlayerPawn.JumpZ = PlayerPawn.Default.JumpZ;
	BaseMutator.ModifyPlayer(PlayerPawn);
}


function NotifyKilled(Controller Killer, Controller Killed, Pawn KilledPawn, class<DamageType> damageType )
{
	local Controller C;

	for ( C=Level.ControllerList; C!=None; C=C.nextController )
		C.NotifyKilled(Killer, Killed, KilledPawn, damageType);
}


function KillEvent(string Killtype, PlayerReplicationInfo Killer, PlayerReplicationInfo Victim, class<DamageType> Damage)
{
	if ( GameStats != None )
		GameStats.KillEvent(KillType, Killer, Victim, Damage);
}

function Killed( Controller Killer, Controller Killed, Pawn KilledPawn, class<DamageType> damageType )
{
    if ( (Killed != None) && Killed.bIsPlayer )
	{
		Killed.PlayerReplicationInfo.Deaths += 1;
		BroadcastDeathMessage(Killer, Killed, damageType);

		if ( (Killer == Killed) || (Killer == None) )
		{
			if ( Killer == None )
			KillEvent("K", None, Killed.PlayerReplicationInfo, DamageType);	//"Kill"
		else
			KillEvent("K", Killer.PlayerReplicationInfo, Killed.PlayerReplicationInfo, DamageType);	//"Kill"
		}
		else
		{
		if ( bTeamGame && (Killer.PlayerReplicationInfo != None)
			&& (Killer.PlayerReplicationInfo.Team == Killed.PlayerReplicationInfo.Team) )
			KillEvent("TK", Killer.PlayerReplicationInfo, Killed.PlayerReplicationInfo, DamageType);	//"Teamkill"
		else
				KillEvent("K", Killer.PlayerReplicationInfo, Killed.PlayerReplicationInfo, DamageType);	//"Kill"
		}
	}
    if ( Killed != None )
		ScoreKill(Killer, Killed);
	DiscardInventory(KilledPawn);
    NotifyKilled(Killer,Killed,KilledPawn, damageType);
}

function bool PreventDeath(Pawn Killed, Controller Killer, class<DamageType> damageType, vector HitLocation)
{
	if ( GameRulesModifiers == None )
		return false;
	return GameRulesModifiers.PreventDeath(Killed,Killer, damageType,HitLocation);
}

function bool PreventSever(Pawn Killed,  Name boneName, int Damage, class<DamageType> DamageType)
{
    if ( GameRulesModifiers == None )
        return false;
    return GameRulesModifiers.PreventSever(Killed, boneName, Damage, DamageType);
}

function BroadcastDeathMessage(Controller Killer, Controller Other, class<DamageType> damageType)
{
	if ( (Killer == Other) || (Killer == None) )
        BroadcastLocalized(self,DeathMessageClass, 1, None, Other.PlayerReplicationInfo, damageType);
	else 
        BroadcastLocalized(self,DeathMessageClass, 0, Killer.PlayerReplicationInfo, Other.PlayerReplicationInfo, damageType);
}


// %k = Owner's PlayerName (Killer)
// %o = Other's PlayerName (Victim)
// %w = Owner's Weapon ItemName
static native function string ParseKillMessage( string KillerName, string VictimName, string DeathMessage );

function Kick( string S )
{
	AccessControl.Kick(S);
}

function KickBan( string S )
{
	AccessControl.KickBan(S);
}

function bool IsOnTeam(Controller Other, int TeamNum)
{
    if ( bTeamGame && (Other != None) && Other.bIsPlayer
		&& (Other.PlayerReplicationInfo.Team != None)
		&& (Other.PlayerReplicationInfo.Team.TeamIndex == TeamNum) )
		return true;
	return false;
}

//-------------------------------------------------------------------------------------
// Level gameplay modification.

//
// Return whether Viewer is allowed to spectate from the
// point of view of ViewTarget.
//
function bool CanSpectate( PlayerController Viewer, bool bOnlySpectator, actor ViewTarget )
{
	return true;
}

/* Use reduce damage for teamplay modifications, etc.
*/
function float ReduceDamage( float Damage, pawn injured, pawn instigatedBy, vector HitLocation, out vector Momentum, class<DamageType> DamageType )
{
	local float OriginalDamage;

	OriginalDamage = Damage;

	if( injured.CurrentPhysicsVolume.bNeutralZone )
		Damage = 0;
	else if ( injured.InGodMode() ) // God mode
		return 0;

	if ( GameRulesModifiers != None )
		return GameRulesModifiers.NetDamage( OriginalDamage, Damage,injured,instigatedBy,HitLocation,Momentum,DamageType );

	return Damage;
}

//
// Return whether an item should respawn.
//
function bool ShouldRespawn( Pickup Other )
{
	if( Level.NetMode == NM_StandAlone )
		return false;

	return Other.ReSpawnTime!=0.0;
}

/* Called when pawn has a chance to pick Item up (i.e. when 
   the pawn touches a weapon pickup). Should return true if 
   he wants to pick it up, false if he does not want it.
*/
function bool PickupQuery( Pawn Other, Pickup item )
{
	local byte bAllowPickup;	

	if ( (GameRulesModifiers != None) && GameRulesModifiers.OverridePickupQuery(Other, item, bAllowPickup) )
		return (bAllowPickup == 1);

	if ( Other.Inventory == None )
	{
		log("GameInfo::PickupQuery: Pawn:"$Other@"has no inventory");
		return true;
	}
	else
		return !Other.Inventory.HandlePickupQuery(Item);
}
		
/* Discard a player's inventory after he dies.
*/
function DiscardInventory( Pawn Other )
{
	Other.Weapon = None;
	Other.SelectedItem = None;
    while ( Other.Inventory != None )
        Other.Inventory.Destroy();
}

/* Try to change a player's name.
*/	
function ChangeName( Controller Other, coerce string S, bool bNameChange )
{
	local Controller C;
	
	if( S == "" )
		return;
		
	Other.PlayerReplicationInfo.SetPlayerName(S);
    // notify local players
    if ( bNameChange )
		for ( C=Level.ControllerList; C!=None; C=C.NextController )
			if ( (PlayerController(C) != None) && (Viewport(PlayerController(C).Player) != None) )
				PlayerController(C).ReceiveLocalizedMessage( class'GameMessage', 2, Other.PlayerReplicationInfo );          
}

/* Return whether a team change is allowed.
*/
function bool ChangeTeam(Controller Other, int N, bool bNewTeam, optional bool bSwitch)
{
	return true;
}

/* Return a picked team number if none was specified
*/
function byte PickTeam(byte Current, Controller C)
{
	return Current;
}

/* Send a player to a URL.
*/
function SendPlayer( PlayerController aPlayer, string URL )
{
	aPlayer.ClientTravel( URL, TRAVEL_Relative, true );
}

/* Restart the game.
*/
function RestartGame()
{
	local string NextMap;
    local MapList MyList;

	if ( (GameRulesModifiers != None) && GameRulesModifiers.HandleRestartGame() )
		return;

	if ( bGameRestarted )
		return;
    bGameRestarted = true;

	// these server travels should all be relative to the current URL
	if ( bChangeLevels && !bAlreadyChanged && (MapListType != "") )
	{
		// open a the nextmap actor for this game type and get the next map
		bAlreadyChanged = true;
        MyList = GetMapList(MapListType);
		if (MyList != None)
		{
			NextMap = MyList.GetNextMap();
			MyList.Destroy();
		}
		if ( NextMap == "" )
		{
			NextMap = GetMapName(MapPrefix, NextMap,1);
		}

		if ( NextMap != "" )
		{
			Level.ServerTravel(NextMap, false);
			return;
		}
	}

	Level.ServerTravel( "?Restart", false );
}

function MapList GetMapList(string MapListType)
{
local class<MapList> MapListClass;

	if (MapListType != "")
	{
        MapListClass = class<MapList>(DynamicLoadObject(MapListType, class'Class'));
		if (MapListClass != None)
			return Spawn(MapListClass);
	}
	return None;
}

//==========================================================================
// Message broadcasting functions (handled by the BroadCastHandler)

event Broadcast( Actor Sender, coerce string Msg, optional name Type )
{
	BroadcastHandler.Broadcast(Sender,Msg,Type);
}

function BroadcastTeam( Controller Sender, coerce string Msg, optional name Type )
{
	BroadcastHandler.BroadcastTeam(Sender,Msg,Type);
}

/*
 Broadcast a localized message to all players.
 Most message deal with 0 to 2 related PRIs.
 The LocalMessage class defines how the PRI's and optional actor are used.
*/
event BroadcastLocalized( actor Sender, class<LocalMessage> Message, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject )
{
	BroadcastHandler.AllowBroadcastLocalized(Sender,Message,Switch,RelatedPRI_1,RelatedPRI_2,OptionalObject);
}

//==========================================================================
	
function bool CheckEndGame(PlayerReplicationInfo Winner, string Reason)
{
	local Controller P;

	if ( (GameRulesModifiers != None) && !GameRulesModifiers.CheckEndGame(Winner, Reason) )
		return false;

	// all player cameras focus on winner or final scene (picked by gamerules)
	for ( P=Level.ControllerList; P!=None; P=P.NextController )
	{
		P.ClientGameEnded();
        P.GameHasEnded();
	}	
	return true;
}

/* End of game.
*/
function EndGame( PlayerReplicationInfo Winner, string Reason )
{
	// don't end game if not really ready
	if ( !CheckEndGame(Winner, Reason) )
	{
		bOverTime = true;
		return;
	}

	bGameEnded = true;
	GameReplicationInfo.bMatchEnded = true;
	TriggerEvent('EndGame', self, None);
	EndLogging(Reason);
}

function EndLogging(string Reason)
{

	if (GameStats == None)
		return;
		
	GameStats.EndGame(Reason);
	GameStats.Destroy();
	GameStats = None;
}

/* Return the 'best' player start for this player to start from.
 */
function NavigationPoint FindPlayerStart( Controller Player, optional byte InTeam, optional string incomingName )
{
	local NavigationPoint N, BestStart;
	local Teleporter Tel;
	local float BestRating, NewRating;
	local byte Team;

	// always pick StartSpot at start of match
    if ( (Player != None) && (Player.StartSpot != None) && (Level.NetMode == NM_Standalone)
		&& (bKeepSamePlayerStart || bWaitingToStartMatch || ((Player.PlayerReplicationInfo != None) && Player.PlayerReplicationInfo.bWaitingPlayer))  )
	{
		return Player.StartSpot;
	}	

	if ( GameRulesModifiers != None )
	{
		N = GameRulesModifiers.FindPlayerStart(Player,InTeam,incomingName);
		if ( N != None )
		    return N;
	}

	// if incoming start is specified, then just use it
	if( incomingName!="" )
		foreach AllActors( class 'Teleporter', Tel )
			if( string(Tel.Tag)~=incomingName )
				return Tel;

	// use InTeam if player doesn't have a team yet
	if ( (Player != None) && (Player.PlayerReplicationInfo != None) )
	{
		if ( Player.PlayerReplicationInfo.Team != None )
			Team = Player.PlayerReplicationInfo.Team.TeamIndex;
		else
			Team = 0;
	}
	else
		Team = InTeam;

	for ( N=Level.NavigationPointList; N!=None; N=N.NextNavigationPoint )
	{
		NewRating = RatePlayerStart(N,InTeam,Player);
		if ( NewRating > BestRating )
		{
			BestRating = NewRating;
			BestStart = N;
		}
	}
	
	if ( BestStart == None )
	{
		log("Warning - PATHS NOT DEFINED or NO PLAYERSTART");			
		foreach AllActors( class 'NavigationPoint', N )
		{
			NewRating = RatePlayerStart(N,0,Player);
			if ( NewRating > BestRating )
			{
				BestRating = NewRating;
				BestStart = N;	
			}
		}
	}

	return BestStart;
}

/* Rate whether player should choose this NavigationPoint as its start
default implementation is for single player game
*/
function float RatePlayerStart(NavigationPoint N, byte Team, Controller Player)
{
	local PlayerStart P;

	P = PlayerStart(N);
	if ( P != None )
	{
		if ( P.bSinglePlayerStart )
		{
			if ( P.bEnabled )
				return 1000;
			return 20;
		}
		return 10;
	}
	return 0;
}

function ScoreObjective(PlayerReplicationInfo Scorer, Int Score)
{
	if ( Scorer != None )
	{
		Scorer.Score += Score;
        /*
		if ( Scorer.Team != None )
			Scorer.Team.Score += Score;
        */
	}
	if ( GameRulesModifiers != None )
		GameRulesModifiers.ScoreObjective(Scorer,Score);

	CheckScore(Scorer);
}

/* CheckScore()
see if this score means the game ends
*/
function CheckScore(PlayerReplicationInfo Scorer)
{
	if ( (GameRulesModifiers != None) && GameRulesModifiers.CheckScore(Scorer) )
		return;
}
	
function ScoreEvent(PlayerReplicationInfo Who, float Points, string Desc)
{
	if (GameStats!=None)
		GameStats.ScoreEvent(Who,Points,Desc);
}

function TeamScoreEvent(int Team, float Points, string Desc)
{
	if ( GameStats != None )
		GameStats.TeamScoreEvent(Team, Points, Desc);
}

function ScoreKill(Controller Killer, Controller Other)
{
	if( (killer == Other) || (killer == None) )
	{
    	if (Other!=None)
        {
		Other.PlayerReplicationInfo.Score -= 1;
			ScoreEvent(Other.PlayerReplicationInfo,-1,"self_frag");
        }
	}
	else if ( killer.PlayerReplicationInfo != None )
	{
		Killer.PlayerReplicationInfo.Score += 1;
		Killer.PlayerReplicationInfo.Kills++;
		ScoreEvent(Killer.PlayerReplicationInfo,1,"frag");
	}

	if ( GameRulesModifiers != None )
		GameRulesModifiers.ScoreKill(Killer, Other);

    if ( Killer != None )
		CheckScore(Killer.PlayerReplicationInfo);
}

function bool TooManyBots(Controller botToRemove)
{
	return false;
}

static function string FindTeamDesignation(GameReplicationInfo GRI, actor A)	// Should be subclassed in various team games
{
	return "";
}

// - Parse out % vars for various messages

static function string ParseMessageString(Mutator BaseMutator, Controller Who, String Message)
{
	return Message;
}

function ReviewJumpSpots(name TestLabel);

function TeamInfo OtherTeam(TeamInfo Requester)
{
	return None;
}

exec function KillBots(int num);

exec function AdminSay(string Msg)
{
	local controller C;

	for( C=Level.ControllerList; C!=None; C=C.nextController )
		if( C.IsA('PlayerController') )
		{
			PlayerController(C).ClearProgressMessages();
			PlayerController(C).SetProgressTime(6);
			PlayerController(C).SetProgressMessage(0, Msg, class'Canvas'.Static.MakeColor(255,255,255));
		}
}

function int GetNumAllowedInGroup(Weapon Weap);
function bool WeaponIsTossable(Weapon Weap);


defaultproperties
{
     bRestartLevel=True
     bPauseable=True
     bCanChangeSkin=True
     bCanViewOthers=True
     bChangeLevels=True
     bAllowWeaponThrowing=True
     bAdminCanPause=True
     bHideSquadSDIcon=True
     GameDifficulty=1
     AutoAim=0.93
     GameSpeed=1
     HUDType="Engine.HUD"
     MaxSpectators=2
     MaxPlayers=16
     DefaultPlayerName="Player"
     GameName="Game"
     FearCostFallOff=0.95
     DeathMessageClass=Class'Engine.LocalMessage'
     GameMessageClass=Class'Engine.GameMessage'
     MutatorClass="Engine.Mutator"
     AccessControlClass="Engine.AccessControl"
     BroadcastHandlerClass="Engine.BroadcastHandler"
     PlayerControllerClassName="Engine.PlayerController"
     GameReplicationInfoClass=Class'Engine.GameReplicationInfo'
     GameStatsClass="Engine.GameStats"
     SecurityClass="Engine.Security"
     Acronym="???"
}

