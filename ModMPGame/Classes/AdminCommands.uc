class AdminCommands extends AdminService;

var() config bool bAllowConsoleCommands;

function bool ExecCmd(PlayerController Player, String Cmd){
	local Controller C;
	local PlayerController PC;

	if(ParseCommand(Cmd, "CMD") && bAllowConsoleCommands){
		Log(Player.PlayerReplicationInfo.PlayerName $ " is executing a console command (" $ Cmd $ ")");
		ConsoleCommand(Cmd);

		return true;
	}else if(ParseCommand(Cmd, "KICK")){
		Level.Game.AccessControl.Kick(Cmd);

		return true;
	}else if(ParseCommand(Cmd, "BAN")){
		Level.Game.AccessControl.KickBan(Cmd);

		return true;
	}else if (ParseCommand(Cmd, "PROMOTE")){
		for(C = Level.ControllerList; C != None; C = C.NextController){
			PC = PlayerController(C);

			if(PC != None && PC.PlayerReplicationInfo.PlayerName ~= Cmd){
				PC.PlayerReplicationInfo.bAdmin = true;

				break;
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "DEMOTE")){
		for(C = Level.ControllerList; C != None; C = C.NextController){
			PC = PlayerController(C);

			if(PC != None && PC.PlayerReplicationInfo.PlayerName ~= Cmd){
				PC.PlayerReplicationInfo.bAdmin = false;

				break;
			}
		}

		return true;
	}

	return false;
}

defaultproperties
{
	bAllowConsoleCommands=true
}
