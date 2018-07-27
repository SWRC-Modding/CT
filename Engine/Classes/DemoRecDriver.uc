//=============================================================================
// DemoRecDriver.
//=============================================================================
class DemoRecDriver extends NetDriver;

defaultproperties
{
     DemoSpectatorClass="Engine.DemoRecSpectator"
     LanServerMaxTickRate=30
     NetServerMaxTickRate=30
     MaxClientRate=25000
     ServerTravelPause=4
     SpawnPrioritySeconds=1
     RelevantTimeout=5
     KeepAliveTime=1
     InitialConnectTimeout=500
     ConnectionTimeout=15
}

