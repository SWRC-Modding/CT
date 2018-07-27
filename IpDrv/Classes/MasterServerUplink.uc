/////////////////////////
// MasterServerUplink
//
// completely revamped to use only gamespy connectivity as the master server concept.  No more md5 stuff.

class MasterServerUplink extends MasterServerLink
    config
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var GameInfo.ServerResponseLine ServerState;
var MasterServerGameStats GameStats;

// config
var globalconfig int CheckCDKey;		//gdr REMOVE this before shipping
var globalconfig int AdvertiseServer;	//0 = No, 1 = LAN only, 2 = Internet and LAN

// sorry, no code for you!
native function Reconnect();

native function GameSpyRegister();
native function GameSpyUnRegister();
native function GameSpyThink(float Delta);

event BeginPlay()
{
//gdr Removed everything but uplinking to GameSpy.  It is my understanding that
// we do not want to link to the Unreal master server at all.

	// all this really does is turn on some stuff for gamespy now.
	Reconnect();

	if(AdvertiseServer > 0)
	{
		GameSpyRegister();
	}
	// If not using gamespy, just forget it, kill me now!
	else
	{
		Destroy();
	}
}

// Called when the connection to the master server fails or doesn't connect.
event ConnectionFailed( bool bShouldReconnect )
{
	Log("Master server connection failed");
}

// Called when we should refresh the game state
event Refresh()
{
	Level.Game.GetServerInfo(ServerState);
	Level.Game.GetServerDetails(ServerState);
	Level.Game.GetServerPlayers(ServerState);
}

// Call to log a stat line
native function bool LogStatLine( string StatLine );

// Handle disconnection.
simulated function Tick( float Delta )
{
	Super.Tick(Delta);
	GameSpyThink(Delta);
}



cpptext
{
	void Destroy();
	void PostScriptDestroyed();
	UBOOL Poll( INT WaitTime );

}

defaultproperties
{
     AdvertiseServer=2
}

