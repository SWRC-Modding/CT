class AdminCommands extends AdminService;

var() config bool bAllowConsoleCommands;

function bool ExecCmd(String Cmd, optional PlayerController PC){
	local PlayerReplicationInfo PRI;
	local String CommandResult;
	local int IntParam;
	local Controller C;

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
	}else if(ParseCommand(Cmd, "LISTPLAYERS")){ // Mostly useful for the console since ingame you can just look at the scoreboard
		for(C = Level.ControllerList; C != None; C = C.nextController){
			if(PlayerController(C) != None){
				PRI = C.PlayerReplicationInfo;
				CommandResult = PRI.PlayerName $ " (" $ PRI.PlayerID $ ")";

				if(PC != None)
					PC.ClientMessage(CommandResult);

				Log(CommandResult);
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "KICK")){
		Level.Game.Kick(Cmd);

		return true;
	}else if(ParseCommand(Cmd, "KICKID")){
		IntParam = int(Cmd);

		for(C = Level.ControllerList; C != None; C = C.nextController){
			if(PlayerController(C) != None){
				PRI = C.PlayerReplicationInfo;

				if(PRI.PlayerID == IntParam){
					Level.Game.Kick(PRI.PlayerName);

					break;
				}
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "KICKSCORE")){
		IntParam = int(Cmd);

		for(C = Level.ControllerList; C != None; C = C.nextController){
			if(PlayerController(C) != None){
				PRI = C.PlayerReplicationInfo;

				if(PRI.Score == IntParam)
					Level.Game.Kick(PRI.PlayerName);
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "BAN")){
		Level.Game.KickBan(Cmd);

		return true;
	}else if(ParseCommand(Cmd, "BANID")){
		IntParam = int(Cmd);

		for(C = Level.ControllerList; C != None; C = C.nextController){
			if(PlayerController(C) != None){
				PRI = C.PlayerReplicationInfo;

				if(PRI.PlayerID == IntParam){
					Level.Game.KickBan(PRI.PlayerName);

					break;
				}
			}
		}

		return true;
	}else if (ParseCommand(Cmd, "PROMOTE")){
		for(C = Level.ControllerList; C != None; C = C.nextController){
			PRI = C.PlayerReplicationInfo;

			if(PRI.PlayerName ~= Cmd){
				PRI.bAdmin = true;

				break;
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "DEMOTE")){
		for(C = Level.ControllerList; C != None; C = C.nextController){
			PRI = C.PlayerReplicationInfo;

			if(PRI.PlayerName ~= Cmd){
				PRI.bAdmin = false;

				break;
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "SWITCHMAP")){
		Level.ServerTravel(Cmd, false);

		return true;
	}else if(ParseCommand(Cmd, "RESTARTMAP")){
		Level.ServerTravel("?restart", false);

		return true;
	}

	return false;
}

defaultproperties
{
	bAllowConsoleCommands=true
}
