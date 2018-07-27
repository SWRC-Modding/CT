class SoundBanter extends SoundBase
    native
	dependsOn(PawnAudioTable)
	hidecategories(Object);

struct BanterLine {
	var() class<Pawn> PawnClass;
	var() Sound	Sound;
};

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var(Sound) class<Pawn> PawnClass;
var name EnumName;
var(Sound) PawnAudioTable.EPawnAudioEvent Cue;
var(Sound) ESoundBattleContext BattleContext;
var(Sound) array<BanterLine> Lines;
var(Sound) float Likelihood;
var bool Used;	//whether it has been played



cpptext
{
	// AnimNotify interface.
	virtual void PostEditChange();
	virtual void PostLoad();

}

defaultproperties
{
     Likelihood=1
}

