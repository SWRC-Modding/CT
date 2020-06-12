This is a mod for Star Wars Republic Commando which extends the multiplayer part of the game. It is a server mod and therefore it is not necessary for clients to have it installed.
It allows players to enter commands via the ingame chat. Every chat message starting with a '/' is treated as a command which is executed server side.
All of the mod's functionality is implemented within _services_. Each service exists independently from the others and implements specific functionality. Which commands are available depends on the currently active services.
Thanks to the modular design, adding a new service is just a matter of creating a new subclass of _AdminService_ and adding it to the list in _ModMPGame.ini_. Existing services can be removed from that list if they are not wanted.
A service implementer can decide whether everybody can execute commands or only admins. The host of the server is always an admin by default.

## Installation
1. Copy the content of the mod's _GameData_ directory into the one from your SWRC installation.
2. Edit System.ini: Find the two occurences of 'ServerActors=...' (they're not next to each other!) under _Engine.GameEngine_ and replace the '=' with '+='. Comment out or remove the one with _IpDrv.UdpBeacon_ and add a new entry: 'ServerActors+=ModMPGame.AdminControl'

## Default services

### AdminAuthentication
The AdminAuthentication service implements the _login_ and _logout_ commands. Without it players are not able to log in as administrators and must be manually promoted by the host. It should always be present for dedicated servers.
The login password is specified in the configuration file.

|command             |description  |
|--------------------|-------------|
|login _&lt;password>_|allows a player to log in as a server admin|
|logout              |removes the admin permissions|

### AdminCommands
This service adds support for basic admin commands like kicking or banning players.

|command                   |description  |
|--------------------------|-------------|
|cmd _&lt;console command>_ |executes a console command on the server|
|kick _&lt;player name>_    |kicks the player with the specified name from the current session|
|ban _&lt;player name>_     |bans the ip address of the specified player|
|promote _&lt;player name>_ |gives a player admin rights|
|demote _&lt;player name>_  |removes a player's admin rights|
|servertravel _&lt;map and options>_  |switches the server to the specified map with optional parameters|

### BotSupport
As the name already implies, this service adds support for bots. They are treated like human players and show up on the scoreboard. In order for the bots to be able to walk around the map there need to be path nodes. This is not a problem when playing on custom maps since they can easily be added by the author but stock maps shouldn't be modified or else players who don't have the modified versions are unable to join. The _BotSupport_ service provides functionality to import or place path nodes during gameplay so that modifying stock maps is not necessary. There are a few ways to add the paths:
1. Copy a stock map, add AI paths and export them using the _ExportPaths_ commandlet with ucc: (`> ucc ModMPGame.ExportPaths <map name>`)
2. Place path nodes manually using  commands and once you're finished export them with the _ExportPaths_ command.

Paths are stored as _.ctp_ files in the _GameData\\Maps\\Paths_ directory. If _bAutoImportPaths_ is set to true in the configuration file, the paths will be automatically imported on level startup.

|command              |description  |
|---------------------|-------------|
|addbot               |adds a new bot to the game|
|removebot            |removes a bot from the game|
|putpathnode          |spawns a PathNode at the player's current position|
|putcoverpoint        |spawns a CoverPoint at the player's current position|
|putpatrolpoint       |spawns a PatrolPoint at the player's current position|
|removenavigationpoint|removes the navigation point at the player's current position|
|importpaths          |imports the paths for the map from a _.ctp_ file if it exists|
|exportpaths          |exports all current paths to a _.ctp_ file that can later be imported|
|buildpaths           |calculates connections between navigation points|
|clearpaths           |clears paths but leaves navigation points intact|
|enableautobuildpaths |enables automatic rebuilding of paths whenever a new navigation point is placed (SLOW!!!)|
|disableautobuildpaths|opposite of _enableautobuildpaths_|

The following commands are only available for the host of a non-dedicated server:

|command             |description  |
|--------------------|-------------|
|showpaths           |draws the navigation points and paths similar to UnrealEd's _View Paths_ option|
|hidepaths           |the opposite of _showpaths_|

## For Modders
All services inherit from _AdminService_. It contains some convenience functions for command and parameter parsing but also the _ExecCmd_ function. The service implementation can override it to add custom commands that are specific to that service. The commands are automatically dispatched by the _AdminControl_ and _ExecCmd_ returns a bool value specifying whether the entered command was recognized or not.
Checking for a command is done using the _ParseCommand_ function. It checks if the input string starts with the specified command and returns true if that is the case. It also removes the parsed command from the input so that it is easier to get the commands arguments if there are any.
_ParseCommand_ can be used in an if else chain to cover all available commands:
```cpp
function bool ExecCmd(String Cmd, optional PlayerController PC){ // PC can be 'None' if the command is executed via the server console
	if(ParseCommand(Cmd, "FIRSTCMD")){
		// Cmd now contains the rest of the input without the leading "FIRSTCMD"

		/* ... */

		// The command was recognized so we return true regardless of whether it was successful or not
		return true;
	}else if(ParseCommand(Cmd, "SECONDCMD")){
		/* ... */
		return true;
	}

	return Super.ExecCmd(Player, Cmd);
}
```
