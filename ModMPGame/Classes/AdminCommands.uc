class AdminCommands extends AdminService;

var() config bool bAllowConsoleCommands;

function bool ExecCmd(PlayerController Player, String Cmd){
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
	}

	return false;
}

defaultproperties
{
	bAllowConsoleCommands=true
}
