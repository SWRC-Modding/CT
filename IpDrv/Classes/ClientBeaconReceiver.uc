//=============================================================================
// ClientBeaconReceiver: Receives LAN beacons from servers.
//=============================================================================
class ClientBeaconReceiver extends UdpBeacon
	transient;

var struct BeaconInfo
{
	var IpAddr      Addr;
	var float       Time;
	var string      Text;
//--Live
    var string      RemoteSessionID;
    var string      RemoteExchangeKey;
    var string      RemoteHostAddr;
//Live--
} Beacons[32];

function int GetBeaconCount()
{
    return (ArrayCount (Beacons));
}

function string GetBeaconAddress( int i )
{
    local string s;
    //--Live
	s = IpAddrToString(Beacons[i].Addr)
      $ "?XSessionID="   $ Beacons[i].RemoteSessionID 
      $ "?XExchangeKey=" $ Beacons[i].RemoteExchangeKey
      $ "?XHostAddr="    $ Beacons[i].RemoteHostAddr;

    return s;
}

function string GetBeaconText(int i)
{
	return Beacons[i].Text;
}

function BeginPlay()
{
	if( BindPort( BeaconPort, true ) > 0 )
		SetTimer( 1.0, true );
	else
		warn( "ClientBeaconReceiver failed: Beacon port in use." );
}

function Timer()
{
	local int i, j;

    // Remove any stale beacons and compress the list

    j = 0;

	for (i = 0; i < ArrayCount(Beacons); i++)
    {
		if (Beacons[i].Addr.Addr == 0)
            continue;
        
        if (Level.TimeSeconds - Beacons[i].Time >= BeaconTimeout)
            continue;

        if (i != j)
    		Beacons[j] = Beacons[i];

        j++;
    }

	while (j < ArrayCount(Beacons))
    {
		Beacons[j].Addr.Addr = 0;
        Beacons[j].Text = "";
        j++;
    }

    BroadcastQuery();
}

function BroadcastQuery ()
{
    local IpAddr Addr;

	Addr.Addr = BroadcastAddr;
	Addr.Port = ServerBeaconPort;

	SendText( Addr, "REPORT" );	
}

event ReceivedText (IpAddr Addr, string Text)
{
	local int i, n;
    local String Product;
//--Live
    local String RemoteSessionID;
    local String RemoteExchangeKey;
    local String RemoteHostAddr;
    local String BeaconText;
//Live--

	n = Len (BeaconProduct);

    Product = Left (Text, n + 1);

	if (!(Product ~= (BeaconProduct$" ")))
	{
		log(Product@"!="@BeaconProduct$" ");
        return;
	}

	Text = Mid (Text, n + 1);

	Addr.Port = int (Text);

//--Live
    // eat leading spaces
    if( InStr(Text,"?")>=0 )
    {
        Text = Mid(Text,InStr(Text,"?"));
    }

    RemoteSessionID   = class'GameInfo'.static.ParseOption ( Text, "XSessionID");
    RemoteExchangeKey = class'GameInfo'.static.ParseOption ( Text, "XExchangeKey" );
    RemoteHostAddr    = class'GameInfo'.static.ParseOption ( Text, "XHostAddr" );
    BeaconText        = class'GameInfo'.static.ParseOption ( Text, "BeaconText" );
//Live--

	//gdr Check the added xbox info to make sure these are identical
	for (i = 0; i < ArrayCount(Beacons); i++)
		if (Beacons[i].Addr == Addr && Beacons[i].RemoteSessionID == RemoteSessionID
            && Beacons[i].RemoteExchangeKey == RemoteExchangeKey && Beacons[i].RemoteHostAddr == RemoteHostAddr)
			break;

	if (i == ArrayCount(Beacons))
	    for (i = 0; i< ArrayCount(Beacons); i++)
			if (Beacons[i].Addr.Addr == 0)
				break;

	if (i == ArrayCount(Beacons))
		return;

	Beacons[i].Addr = Addr;
	Beacons[i].Time = Level.TimeSeconds;
	Beacons[i].Text = BeaconText;
    Beacons[i].RemoteSessionID = RemoteSessionID;
    Beacons[i].RemoteExchangeKey = RemoteExchangeKey;
    Beacons[i].RemoteHostAddr = RemoteHostAddr;
//	log("Added Beacon #"$i@"to list");
}


defaultproperties
{
}

