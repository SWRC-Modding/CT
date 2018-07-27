class SoundMultiple extends Sound
    native
	hidecategories(Object)
    noexport;

enum ESoundSelectionStyle
{
	SS_Random,
	SS_InOrder,
};

var(Sound) native array<Sound> Sounds;
var(Special) Sound FirstSound;
var(Special) Sound BattleWonMusic;
var(Special) Sound BattleLostMusic;
var(Sound) native ESoundSelectionStyle SelectionStyle;
var const transient bool bFirstTime;
var const transient bool bCheckedRestrictions;
var const transient bool bAccurateTotalLikelihood;
var const transient Sound RenderedSound;
var const transient native float TotalLikelihood;
var const transient native int WhichSound;


defaultproperties
{
}

