class AdminCommands extends AdminService;

var() config bool bAllowConsoleCommands;

function bool ExecCmd(PlayerController PC, String Cmd){
	local PlayerReplicationInfo PRI;
	local String CommandResult;

	if(ParseCommand(Cmd, "CMD")){
		if(bAllowConsoleCommands || Viewport(PC.Player) != None){ // The host can always execute console commands
			Log(PC.PlayerReplicationInfo.PlayerName $ " is executing a console command (" $ Cmd $ ")");

			CommandResult = ConsoleCommand(Cmd);

			if(CommandResult != ""){
				Log(CommandResult);
				PC.ClientMessage(CommandResult);
			}
		}else{
			PC.ClientMessage("Console commands are not allowed!");
		}

		return true;
	}else if(ParseCommand(Cmd, "KICK")){
		Level.Game.AccessControl.Kick(Cmd);

		return true;
	}else if(ParseCommand(Cmd, "BAN")){
		Level.Game.AccessControl.KickBan(Cmd);

		return true;
	}else if (ParseCommand(Cmd, "PROMOTE")){
		foreach DynamicActors(class'PlayerReplicationInfo', PRI){
			if(PRI.PlayerName ~= Cmd){
				PRI.bAdmin = true;

				break;
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "DEMOTE")){
		foreach DynamicActors(class'PlayerReplicationInfo', PRI){
			if(PRI.PlayerName ~= Cmd){
				PRI.bAdmin = false;

				break;
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "SERVERTRAVEL")){
		Level.ServerTravel(Cmd, false);

		return true;
	}

	return false;
}

defaultproperties
{
	bAllowConsoleCommands=true
}
