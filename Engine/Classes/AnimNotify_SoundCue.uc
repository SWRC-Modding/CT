class AnimNotify_SoundCue extends AnimNotify
	dependsOn(PawnAudioTable)
	native;

var() PawnAudioTable.EPawnAudioEvent Cue;
var() float MinInterval;
var	name EnumName;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)



cpptext
{
	// AnimNotify interface.
	virtual void Notify( UMeshInstance *Instance, AActor *Owner );
	virtual void PostEditChange();
	virtual void PostLoad();

}

defaultproperties
{
     MinInterval=-1
}

