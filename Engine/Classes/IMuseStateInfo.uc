//=============================================================================
// IMuseStateInfofa: Stores info about the state of IMuse
//=============================================================================

class IMuseStateInfo extends Object
	dependsOn(Actor)
	native;

var bool mbPlayerDown;
var bool mbBattleEngineEnabled;

var Sound mCurrentRoom;
var Sound mCurrentOverlay;
var Sound mCurrentEpisode;

var int mNextOverlayTime;
var int mMinOverlayTime;
var int mMaxOverlayTime;
var int mOverlayStartupDelay;

var EngineBattleVolume CurrentBattleZone;

var int   mNextBattleUpdateTime;

var Array<Pawn> mBattleBaddies;

var Sound mPlayerDownLoopID;
var Sound mActiveBattleCueID;

var Sound CurrentBattleTrio;
var Sound CurrentOverlayMultiSound;
var EngineBattleVolume BattleZone;


defaultproperties
{
     mbBattleEngineEnabled=True
     mMinOverlayTime=10
     mMaxOverlayTime=20
     mOverlayStartupDelay=5
}

