class AdminCommands extends AdminService;

var() config bool bAllowConsoleCommands;

function int ParseId(string Cmd)
{
	local int id;
	id = -1;
	ParseIntParam(Cmd, "id=", id);
	return id;
}

function string ParseReason(string Cmd)
{
	local string Reason;
	ParseStringParam(Cmd, "reason=", Reason);
	return Reason;
}

function bool ExecCmd(string Cmd, optional PlayerController PC)
{
	local PlayerReplicationInfo PRI;
	local string                CommandResult;
	local int                   IntParam;
	local string                StringParam;
	local Controller            C;
	local PlayerController      Player;

	if(ParseCommand(Cmd, "CMD"))
	{
		if(bAllowConsoleCommands || IsLocalPlayer(PC)) // The host can always execute console commands
		{
			StringParam = Cmd;

			if(IsLocalPlayer(PC))
			{
				CommandResult = ConsoleCommand(Cmd);
			}
			else if(ParseCommand(StringParam, "GET") || ParseCommand(StringParam, "SET"))
			{
				if(InStr(Caps(StringParam), "ADMINPASSWORD") == -1) // Security measure
					CommandResult = ConsoleCommand(Cmd);
				else
					CommandResult = "Remote players are not allowed to access the admin password";
			}
			else
			{
				CommandResult = "Remote players are only allowed to use the get and set commands"; // Might still mess things up but at least some access should be provided
			}

			if(Len(CommandResult) > 0)
				CommandFeedback(PC, CommandResult);
		}
		else
		{
			CommandFeedback(PC, "Console commands are not allowed!");
		}

		return true;
	}
	else if(ParseCommand(Cmd, "LISTPLAYERS")) // Mostly useful for the console since ingame you can just look at the scoreboard
	{
		for(C = Level.ControllerList; C != None; C = C.nextController)
		{
			if(PlayerController(C) != None)
			{
				PRI = C.PlayerReplicationInfo;
				CommandResult = "name=" $ PRI.PlayerName $ " id=" $ PRI.PlayerID $ " kills=" $ int(PRI.Score) $ " deaths=" $ int(PRI.Deaths);

				CommandFeedback(PC, CommandResult, PC != None);
			}
		}

		return true;
	}
	else if(ParseCommand(Cmd, "KICK"))
	{
		IntParam = ParseId(Cmd);

		if(IntParam < 0)
			StringParam = ParseToken(Cmd);

		Player = GetPlayer(StringParam, IntParam);

		if(Player != None)
		{
			if(IsLocalPlayer(Player))
				CommandFeedback(PC, "Can't kick host");
			else
				AdminAccessControl(Level.Game.AccessControl).KickPlayerController(Player, ParseReason(Cmd));
		}
		else
		{
			CommandFeedback(PC, "No matching player");
		}

		return true;
	}
	else if(ParseCommand(Cmd, "KICKALL"))
	{
		for(C = Level.ControllerList; C != None; C = C.nextController)
		{
			if(PlayerController(C) != None && C != PC && !IsLocalPlayer(PlayerController(C))) // Don't kick yourself or the host
				AdminAccessControl(Level.Game.AccessControl).KickPlayerController(PlayerController(C));
		}

		return true;
	}
	else if(ParseCommand(Cmd, "BAN"))
	{
		IntParam = ParseId(Cmd);

		if(IntParam < 0)
			StringParam = ParseToken(Cmd);

		Player = GetPlayer(StringParam, IntParam);

		if(Player != None)
		{
			if(IsLocalPlayer(Player))
				CommandFeedback(PC, "Can't ban host");
			else
				AdminAccessControl(Level.Game.AccessControl).BanPlayerController(Player, ParseReason(Cmd));
		}
		else
		{
			CommandFeedback(PC, "No matching player");
		}

		return true;
	}
	else if(ParseCommand(Cmd, "PROMOTE"))
	{
		IntParam = ParseId(Cmd);

		if(IntParam < 0)
			StringParam = ParseToken(Cmd);

		Player = GetPlayer(StringParam, IntParam);

		if(Player != None)
		{
			PRI = Player.PlayerReplicationInfo;

			if(!PRI.bAdmin)
			{
				PRI.bAdmin = true;

				if(PC != None)
					StringParam = PC.PlayerReplicationInfo.PlayerName;
				else
					StringParam = "the server";

				CommandResult = PRI.PlayerName $ " was promoted to admin by " $ StringParam;

				Log(CommandResult);
				Level.Game.Broadcast(self, CommandResult);
				AdminControl.SaveStats(PlayerController(C));
			}
			else
			{
				CommandFeedback(PC, PRI.PlayerName $ " is already an admin");
			}
		}
		else
		{
			CommandFeedback(PC, "No matching player");
		}

		return true;
	}
	else if(ParseCommand(Cmd, "DEMOTE"))
	{
		IntParam = ParseId(Cmd);

		if(IntParam < 0)
			StringParam = ParseToken(Cmd);

		Player = GetPlayer(StringParam, IntParam);

		if(Player != None)
		{
			PRI = Player.PlayerReplicationInfo;

			if(PRI.bAdmin)
			{
				PRI.bAdmin = false;

				if(PC != None)
					StringParam = PC.PlayerReplicationInfo.PlayerName;
				else
					StringParam = "the server";

				CommandResult = PRI.PlayerName $ "'s admin priviledges were revoked by " $ StringParam;

				Log(CommandResult);
				Level.Game.Broadcast(self, CommandResult);
				AdminControl.SaveStats(PlayerController(C));
			}
			else
			{
				CommandFeedback(PC, PRI.PlayerName $ " is not an admin");
			}
		}
		else
		{
			CommandFeedback(PC, "No matching player");
		}

		return true;
	}
	else if(ParseCommand(Cmd, "SWITCHMAP"))
	{
		StringParam = parseToken(Cmd);

		if(Len(StringParam) > 0)
		{
			if(ResolveMapURL(StringParam))
			{
				IntParam = InStr(StringParam, ".ctm");

				if(IntParam == -1)
					Level.Game.Broadcast(self, "Switching map to " $ StringParam);
				else
					Level.Game.Broadcast(self, "Switching map to " $ Left(StringParam, IntParam));

				Level.ServerTravel(StringParam, false);
			}
			else
			{
				CommandFeedback(PC, "No map found matching name");
			}
		}
		else
		{
			CommandFeedback(PC, "Expected map URL");
		}

		return true;
	}
	else if(ParseCommand(Cmd, "NEXTMAP"))
	{
		Level.Game.Broadcast(self, "Switching to next map in rotation");
		Level.Game.GameRulesModifiers = None;
		Level.Game.bGameRestarted = false;
		Level.Game.bChangeLevels = true;
		Level.Game.bAlreadyChanged = false;
		Level.Game.RestartGame();

		return true;
	}
	else if(ParseCommand(Cmd, "RESTARTMAP"))
	{
		Level.Game.Broadcast(self, "Restarting map");
		Level.ServerTravel("?restart", false);

		return true;
	}

	return false;
}

defaultproperties
{
	bAllowConsoleCommands=true
}
