//=============================================================================
// ReplicationInfo.
//=============================================================================
class ReplicationInfo extends Info
	abstract
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)



cpptext
{
	INT* GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );

}

defaultproperties
{
     bAlwaysRelevant=True
     RemoteRole=ROLE_SimulatedProxy
}

