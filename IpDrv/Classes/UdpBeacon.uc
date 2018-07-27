//=============================================================================
// UdpBeacon: Base class of beacon sender and receiver.
//=============================================================================
class UdpBeacon extends UdpLink
	config
	transient;

var() globalconfig bool       DoBeacon;
var() globalconfig int        ServerBeaconPort;		// Listen port
var() globalconfig int        BeaconPort;			// Reply port
var() globalconfig float      BeaconTimeout;
var() globalconfig string     BeaconProduct;

var int	UdpServerQueryPort;
var int boundport;

//--Live
var IpAddr                    BroadcastReplyAddr;

function BeginPlay()
{
	boundport = BindPort(ServerBeaconPort, True);
	if ( boundport == 0 )
	{
		log( "UdpBeacon failed to bind a port." );
		return;
	}

	BroadcastReplyAddr.Addr = BroadcastAddr;
	BroadcastReplyAddr.Port = BeaconPort;

	// BroadcastBeacon(Addr); // Initial notification.
}

function BroadcastBeacon(IpAddr Addr)
{
    local string response;

    response = BeaconProduct @ Mid(Level.GetAddressURL(),InStr(Level.GetAddressURL(),":")+1)
        $ "?XSessionID=" $ LocalXSessionID
        $ "?XExchangeKey=" $ LocalXExchangeKey
        $ "?XHostAddr=" $ LocalXHostAddr
        $ "?BeaconText=" $ Level.Game.GetBeaconText();

	SendText( BroadcastReplyAddr, response  );
//	Log( "UdpBeacon: sending reply ["$response$"] to "$IpAddrToString(BroadcastReplyAddr) );
}

function BroadcastBeaconQuery(IpAddr Addr)
{
	SendText( BroadcastReplyAddr, BeaconProduct @ UdpServerQueryPort );
//	Log( "UdpBeacon: sending query reply to "$IpAddrToString(BroadcastReplyAddr) );
}

event ReceivedText( IpAddr Addr, string Text )
{
	//Don't respond to systemlink beacons if we are hosting a game on XBox Live.
    if(Level != None && !Level.IsSystemLink())
    {
        Destroy();
        return;
    }

	if( Text == "REPORT" )
		BroadcastBeacon(Addr);

	if( Text == "REPORTQUERY" )
		BroadcastBeaconQuery(Addr);
}

function Destroyed()
{
	Super.Destroyed();
	//Log("ServerBeacon Destroyed");
}


defaultproperties
{
     ServerBeaconPort=8777
     BeaconPort=9777
     BeaconTimeout=5
     BeaconProduct="ut"
}

