//=============================================================================
// InternetInfo: Parent class for Internet connection classes
//=============================================================================
class InternetInfo extends Info
	native
	transient;

function int GetBeaconCount()
{
    return (0);
}

function string GetBeaconAddress( int i );
function string GetBeaconText( int i );


defaultproperties
{
}

