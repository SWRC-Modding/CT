class AdminControl extends Actor config(ModMPGame);

var() config array<String> ServiceClasses;

var array<AdminService> Services;

function PostBeginPlay(){
	local int i;
	local Class<AdminService> ServiceClass;
	local AdminService Service;

	Level.Game.BroadcastHandlerClass = "ModMPGame.AdminControlBroadcastHandler";

	if(Level.Game.BroadcastHandler != None){ // If this is being called after the LevelInfo has been set up, we have to replace the broadcast handler
		Level.Game.BroadcastHandler.Destroy();
		Level.Game.BroadcastHandler = Spawn(Class'AdminControlBroadcastHandler');
	}

	for(i = 0; i < ServiceClasses.Length; ++i){
		ServiceClass = Class<AdminService>(DynamicLoadObject(ServiceClasses[i], Class'Class'));

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

function ExecCmd(PlayerController PC, String Cmd){
	local int i;
	local bool RecognizedCmd;

	if(Viewport(PC.Player) != None) // The host is always an admin and doesn't need to log in
		PC.PlayerReplicationInfo.bAdmin = true;

	if(PC.PlayerReplicationInfo.bAdmin){
		for(i = 0; i < Services.Length; ++i)
			RecognizedCmd = Services[i].ExecCmd(PC, Cmd) || RecognizedCmd;
	}else{ // Player is not logged in, so only forward the command to services that have bRequiresAdminPermissions set to false
		for(i = 0; i < Services.Length; ++i){
			if(!Services[i].bRequiresAdminPermissions)
				RecognizedCmd = Services[i].ExecCmd(PC, Cmd) || RecognizedCmd;
		}
	}

	if(RecognizedCmd){
		/*
		 * Commands might change the value of config properties which are then saved.
		 * Ideally SaveConfig would be called at the end of the game but I haven't found a way to do this
 		* (overriding 'Destroyed' doesn't work since it is never called on this actor).
		 */
		SaveConfig();

		for(i = 0; i < Services.Length; ++i)
			Services[i].SaveConfig();
	}else{
		if(PC.PlayerReplicationInfo.bAdmin)
			PC.ClientMessage("Unrecognized command");
		else
			PC.ClientMessage("Unrecognized command or missing permissions");
	}
}

defaultproperties
{
	ServiceClasses(0)="ModMPGame.AdminAuthentication"
	ServiceClasses(1)="ModMPGame.AdminCommands"
	ServiceClasses(2)="ModMPGame.BotSupport"
}