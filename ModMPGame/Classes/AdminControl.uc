class AdminControl extends Actor native config(ModMPGame);

var() config array<String> ServiceClasses;
var array<AdminService>    Services;

var() config string EventLogFile;
var() config bool   AppendEventLog;
var() config bool   EventLogTimestamp;

native final function EventLog(coerce string Msg, name Event);

function PostBeginPlay(){
	local int i;
	local Class<AdminService> ServiceClass;
	local AdminService Service;

	Level.Game.bAdminCanPause = false;
	Level.Game.BroadcastHandlerClass = "ModMPGame.AdminControlBroadcastHandler";
	Level.Game.AccessControlClass = "ModMPGame.AdminAccessControl";

	if(Level.Game.BroadcastHandler != None){
		Level.Game.BroadcastHandler.Destroy();
		Level.Game.BroadcastHandler = Spawn(class'AdminControlBroadcastHandler');
	}

	if(Level.Game.AccessControl != None){
		Level.Game.AccessControl.Destroy();
		Level.Game.AccessControl = Spawn(class'AdminAccessControl');
	}

	for(i = 0; i < ServiceClasses.Length; ++i){
		ServiceClass = Class<AdminService>(DynamicLoadObject(ServiceClasses[i], class'Class'));

		if(ServiceClass == None){
			Warn("'" $ ServiceClasses[i] $ "' is not a subclass of AdminService");

			continue;
		}

		if(!Level.Game.IsA(ServiceClass.default.RelevantGameInfoClass.Name))
			continue; // Service is not relevant for current game mode so don't spawn it

		Log("Spawning actor for admin service class '" $ ServiceClass $ "'");
		Service = Spawn(ServiceClass);

		if(Service == None){
			Warn("Unable to spawn admin service '" $ ServiceClass $ "'");

			continue;
		}

		Services[Services.Length] = Service;
	}
}

event bool ExecCmd(String Cmd, optional PlayerController PC){
	local int i;
	local bool RecognizedCmd;
	local string CommandSource;

	if(Left(Cmd, 5) ~= "XLIVE" || Left(Cmd, 7) ~= "GETPING")
		return false;

	if(PC != None){
		if(Viewport(PC.Player) != None) // The host is always an admin and doesn't need to log in
			PC.PlayerReplicationInfo.bAdmin = true;

		CommandSource = PC.PlayerReplicationInfo.PlayerName;
	}else{
		CommandSource = "ServerConsole";
	}

	EventLog("(" $ CommandSource $ "): " $ Cmd, 'Command');

	if((PC == None || PC.PlayerReplicationInfo.bAdmin) && Cmd ~= "SAVECONFIG"){
		for(i = 0; i < Services.Length; ++i)
			Services[i].SaveConfig();

		SaveConfig();

		RecognizedCmd = true;
	}else{
		for(i = 0; i < Services.Length; ++i){
			if(PC == None || PC.PlayerReplicationInfo.bAdmin || !Services[i].bRequiresAdminPermissions){
				if(Services[i].ExecCmd(Cmd, PC))
					RecognizedCmd = true;
			}
		}
	}

	if(PC != None && !RecognizedCmd){
		if(PC.PlayerReplicationInfo.bAdmin)
			PC.ClientMessage("Unrecognized command");
		else
			PC.ClientMessage("Unrecognized command or missing permissions");
	}else{
		SaveConfig();
	}

	return RecognizedCmd;
}

cpptext
{
	// Overrides
	virtual void Spawned();
	virtual void Destroy();

	void EventLog(const TCHAR* Msg, FName Event);
}

defaultproperties
{
	bHidden=true
	EventLogFile="../Save/ServerEvents.log"
	AppendEventLog=true
	EventLogTimestamp=true
	ServiceClasses(0)="ModMPGame.AdminAuthentication"
	ServiceClasses(1)="ModMPGame.AdminCommands"
	ServiceClasses(2)="ModMPGame.BotSupport"
}
