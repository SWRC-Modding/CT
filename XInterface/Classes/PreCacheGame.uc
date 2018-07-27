class PreCacheGame extends GameInfo
    native;

// Force precaching of a few resources that will most likely be needed so that
// they're in the LIN.

var() Array <String> PackageList;
var() Array <Object> Cache;

const PRECACHE_LEVEL = 1;

event InitGame( string Options, out string Error )
{
    Super.InitGame( Options, Error );
    
    log( "Enabling gamepad input merging (in menu subsystem).", 'Log' );
    /*
    Level.bMergeGamepadInput = true;
	*/
}

simulated function PostBeginPlay()
{
	/*
    local Array <xUtil.GameTypeRecord> GameTypeRecords;
    local Array <xUtil.MapRecord> MapRecords;
    local Array <xUtil.PlayerRecord> PlayerRecords;
    local Array <xUtil.MutatorRecord> MutatorRecords;
    local int i;

    if( PRECACHE_LEVEL > 0 )
    {
        log( "Precaching menu resources...", 'Init' );

        class'xUtil'.static.GetPlayerList( PlayerRecords );
        for( i = 0; i < PlayerRecords.Length; i++ )
        {
            PlayerRecords[i].Portrait = Material(DynamicLoadObject(PlayerRecords[i].PortraitName, class'Material'));

            if( PRECACHE_LEVEL > 1 )
                PreLoad( PlayerRecords[i].Portrait );
        }

        class'xUtil'.static.GetGameTypeList( GameTypeRecords );
        for( i = 0; i < GameTypeRecords.Length; i++ )
        {
            assert( GameTypeRecords[i].Screenshot != None );

            if( PRECACHE_LEVEL > 1 )
                PreLoad( GameTypeRecords[i].Screenshot );
        }
        
        class'xUtil'.static.GetMapList( MapRecords );
        for( i = 0; i < MapRecords.Length; i++ )
        {
            assert( MapRecords[i].Screenshot != None );

            if( PRECACHE_LEVEL > 1 )
                PreLoad( MapRecords[i].Screenshot );
        }

        class'xUtil'.static.GetMutatorList( MutatorRecords );
        for( i = 0; i < MutatorRecords.Length; i++ )
        {
            MutatorRecords[i].IconMaterial = Material(DynamicLoadObject(MutatorRecords[i].IconMaterialName, class'Material'));

            if( PRECACHE_LEVEL > 1 )
                PreLoad( MutatorRecords[i].IconMaterial );
        }

        CachePackageList();
    }
    else
    {
        Cache[0] = DynamicLoadObject("XInterfaceCommon.MenuPreBeginMain", class'Class');
    }
	*/
}

simulated native function CachePackageList();


defaultproperties
{
     PackageList(0)="XInterface.u"
     PackageList(1)="XInterfaceCommon.u"
     PackageList(2)="XInterfaceLive.u"
     PackageList(3)="XInterfaceSP.u"
     PackageList(4)="XInterfaceMP.u"
     PackageList(5)="XInterfaceSettings.u"
}

