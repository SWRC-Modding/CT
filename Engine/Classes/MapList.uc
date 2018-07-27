//=============================================================================
// MapList.
//
// contains a list of maps to cycle through
//
//=============================================================================
class MapList extends Info
	abstract;

var(Maps) config array<string> Maps;
var config int MapNum;


// For debugging
function PrintMapsList()
{
	local int i;

	for ( i=0; i<Maps.Length; i++ )
	{
		Log(" ***** [" $ string(i) $ "] " $ Maps[i]);
	}
}

function string GetNextMap()
{
	local string CurrentMap;
	local int i;

	PrintMapsList();
	CurrentMap = GetURLMap();

	if ( CurrentMap != "" && MapNum > 0 )
	{
		//// i am changing this to NOT use the map fn extension.
		//if ( Right(CurrentMap,4) ~= ".ctm" )
		//{
		//	CurrentMap = Left(CurrentMap, Len(CurrentMap) -4);
		//}

		// remove any args from currentmap
		i = InStr( CurrentMap, "?" );
		if (i >= 0)
			CurrentMap = Left( CurrentMap, i-1 );

		for ( i=0; i<Maps.Length; i++ )
		{
			if ( CurrentMap ~= Maps[i] )
			{
				MapNum = i;
				break;
			}
		}
	}

	// search vs. w/ or w/out .unr extension

	MapNum++;
	if ( MapNum > Maps.Length - 1 )
		MapNum = 0;
	if ( Maps[MapNum] == "" )
		MapNum = 0;

	Log ("Switching map, of type:"@Level.Game.MapListType);
	SaveConfig( Level.Game.MapListType );
	return Maps[MapNum];
}

defaultproperties
{
}

