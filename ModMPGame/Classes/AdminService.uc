class AdminService extends Actor abstract native config(ModMPGame);

// Service will only be spawned if the current game mode is an object of this class or one of its subclasses
var() static config class<GameInfo> RelevantGameInfoClass;

var() bool bRequiresAdminPermissions; // Commands are only forwarded to the service if the issuer is logged in as an administrator

var AdminControl AdminControl;
var AdminService nextAdminService;

native static final function bool ParseCommand(out string Stream, string Match);
native static final function string ParseToken(out string Stream);

native static final function bool ParseIntParam(string Stream, string Match, out int Value);
native static final function bool ParseFloatParam(string Stream, string Match, out float Value);
native static final function bool ParseStringParam(string Stream, string Match, out string Value);

// Can be overridden to check for custom commands
native function bool ExecCmd(String Cmd, optional PlayerController PC);

native final function EventLog(coerce string Msg);

static final function bool IsLocalPlayer(PlayerController PC)
{
	return PC == None || Viewport(PC.Player) != None;
}

event CommandFeedback(PlayerController PC, string Msg, optional bool DontWriteToLog)
{
	if(PC != None)
		PC.ClientMessage(Msg);

	if(!DontWriteToLog)
		EventLog(Msg);
}

function bool ResolveMapURL(out string MapURL)
{
	local string MapName;
	local string FirstMap;
	local string CurrentMap;
	local int    Idx;

	Idx = InStr(MapURL, "?");

	if(Idx < 0)
		MapName = MapURL;
	else
		MapName = Left(MapURL, Idx);

	if(Len(MapName) == 0)
	{
		// Empty URL or URL without map name was provided so use the current map name
		MapURL = MapURL $ AdminControl.GetMapFileName();
		return true;
	}

	if(!(Right(MapName, 4) ~= ".ctm"))
		MapName = MapName $ ".ctm";

	FirstMap   = GetMapName("", "", 0);
	CurrentMap = FirstMap;

	if(Len(FirstMap) == 0)
		return false;

	do
	{
		if(CurrentMap ~= MapName || CurrentMap ~= (Level.Game.MapPrefix $ "_" $ MapName))
		{
			MapURL = CurrentMap $ Mid(MapURL, Idx);
			return true;
		}

		CurrentMap = GetMapName("", CurrentMap, 1); // Get the next map after the current one
	}
	until(CurrentMap ~= FirstMap);

	return false;
}

function PlayerController GetPlayer(string PlayerName, int PlayerID)
{
	local PlayerController PC;
	local Controller       C;

	if(PlayerID < 0 && Len(PlayerName) == 0)
		return None;

	for(C = Level.ControllerList; C != None; C = C.nextController)
	{
		PC = PlayerController(C);

		if(PC != None && (PC.PlayerReplicationInfo.PlayerID == PlayerID || PC.PlayerReplicationInfo.PlayerName == PlayerName))
			return PC;
	}

	return None;
}

cpptext
{
	// AAdminService interface
	virtual bool ExecCmd(const TCHAR* Cmd, class APlayerController* PC = NULL){ return false; }

	void EventLog(const TCHAR* Msg);

	/*
	 * Admin services should prefer this function over 'ParseCommand' as it can also collect the command strings to display as help text
	 */
	bool CheckCommand(const TCHAR** Stream, const TCHAR* Match);
}

defaultproperties
{
	bHidden=true
	RelevantGameInfoClass=class'GameInfo'
	bRequiresAdminPermissions=true
}
