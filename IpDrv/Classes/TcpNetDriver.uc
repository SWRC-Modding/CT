//=============================================================================
// TcpNetDriver.
//=============================================================================
class TcpNetDriver extends NetDriver;

defaultproperties
{
     LogMaxConnPerIPPerMin=True
     MaxConnPerIPPerMinute=5
     LanServerMaxTickRate=35
     NetServerMaxTickRate=20
     ServerTravelPause=4
     SpawnPrioritySeconds=1
     RelevantTimeout=5
     KeepAliveTime=0.2
     InitialConnectTimeout=150
     ConnectionTimeout=120
}

