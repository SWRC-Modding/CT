class SoundStreamed extends Sound
    native
	hidecategories(Object)
    noexport;

var(Sound)	native String Filename;
var(Music)	float FadeInTime;
var(Music)	float FadeOutTime;
var(Music)	Actor.EMusicFadeType FadeInType;
var(Music)	Actor.EMusicFadeType FadeOutType;

var editconst float OverrideFadeInTime;
var editconst float OverrideFadeOutTime;
var editconst Actor.EMusicFadeType OverrideFadeInType;
var editconst Actor.EMusicFadeType OverrideFadeOutType;
var editconst bool DoOverrideFades;


defaultproperties
{
}

