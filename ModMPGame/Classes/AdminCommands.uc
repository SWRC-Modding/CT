class AdminCommands extends AdminService;

var() config bool bAllowConsoleCommands;

function bool ExecCmd(String Cmd, optional PlayerController PC){
	local PlayerReplicationInfo PRI;
	local String CommandResult;

	if(ParseCommand(Cmd, "CMD")){
		if(PC == None || bAllowConsoleCommands || IsLocalPlayer(PC)){ // The host can always execute console commands
			CommandResult = ConsoleCommand(Cmd);

			if(CommandResult != ""){
				Log(CommandResult);

				if(PC != None)
					PC.ClientMessage(CommandResult);
			}
		}else if(PC != None){
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
