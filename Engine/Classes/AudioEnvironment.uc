//=============================================================================
// AudioEnvironment, a class for defining audio properties for a zone or area
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class AudioEnvironment extends Object
	hidecategories(Advanced, CollisionAdvanced, DisplayAdvanced, Marker, MovementAdvanced, Display, Events, Object)
	native
	abstract;
	//editinlinenew;

var(Sound) editinline I3DL2Listener	Effect;
var(Sound) editinline I3DL2Listener	EffectXBox;
var(Sound) Sound PreAmbient;
var(Sound) Sound RandomOneShot;
var(Sound) float ROSradiusMin;
var(Sound) float ROSradiusMax;
var(Sound) float ROSintervalMin;
var(Sound) float ROSintervalMax;

var(Music) Sound iMuseState;
var(Music) Sound iMuseOverlays;
var(Music) int OverlayStartupDelay;
var(Music) int OverlayMinSpacing;
var(Music) int OverlayMaxSpacing;
var(Music) bool bDoMusicOnceOnly;
var const transient bool bMusicUsedUp;

// When an actor enters this environment.
//event ActorEntered( actor Other );

// When an actor leaves this environment.
//event ActorLeaving( actor Other );


defaultproperties
{
     ROSintervalMin=30
     ROSintervalMax=60
     OverlayStartupDelay=15
     OverlayMinSpacing=30
     OverlayMaxSpacing=60
     bDoMusicOnceOnly=True
}

