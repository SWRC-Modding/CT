class AdminControl extends Actor native config(ModMPGame);

var() config bool          bAdminsCanPause;
var() config array<String> ServiceClasses;

var array<AdminService> Services;

function PostBeginPlay(){
	local int i;
	local Class<AdminService> ServiceClass;
	local AdminService Service;

	Level.Game.bAdminCanPause = bAdminsCanPause;
	Level.Game.BroadcastHandlerClass = "ModMPGame.AdminControlBroadcastHandler";

	if(Level.Game.BroadcastHandler != None){ // If this is called after the LevelInfo has been set up, we have to replace the broadcast handler
		Level.Game.BroadcastHandler.Destroy();
		Level.Game.BroadcastHandler = Spawn(class'AdminControlBroadcastHandler');
	}

	for(i = 0; i < ServiceClasses.Length; ++i){
		ServiceClass = Class<AdminService>(DynamicLoadObject(ServiceClasses[i], class'Class'));

		if(ServiceClass == None){
			Warn("'" $ ServiceClasses[i] $ "' is not a subclass of AdminService");

			continue;
		}

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

	if(PC != None && Viewport(PC.Player) != None) // The host is always an admin and doesn't need to log in
		PC.PlayerReplicationInfo.bAdmin = true;

	for(i = 0; i < Services.Length; ++i){
		if(PC == None || PC.PlayerReplicationInfo.bAdmin || !Services[i].bRequiresAdminPermissions){
			if(Services[i].ExecCmd(Cmd, PC)){
				Services[i].SaveConfig();
				RecognizedCmd = true;
			}
		}
	}

	if(PC != None && !RecognizedCmd){
		if(PC.PlayerReplicationInfo.bAdmin)
			PC.ClientMessage("Unrecognized command");
		else
			PC.ClientMessage("Unrecognized command or missing permissions");
	}

	return RecognizedCmd;
}

cpptext
{
	// Overrides
	virtual void Spawned();
	virtual void Destroy();
}

defaultproperties
{
	ServiceClasses(0)="ModMPGame.AdminAuthentication"
	ServiceClasses(1)="ModMPGame.AdminCommands"
	ServiceClasses(2)="ModMPGame.BotSupport"
}
