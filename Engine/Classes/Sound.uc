class Sound extends SoundBase
    native
	hidecategories(Object)
    noexport;

var(Sound) AudioParams Params;
var(Sound) RumbleParams Rumble;

var(Sound) bool Looping;
var bool Streaming; //not editable
var(Sound) ESoundBattleContext BattleContext;
var(Sound) byte MinCampaign;
var(Sound) byte MaxCampaign;

var(Sound) float Likelihood;
var(Sound) BOOL  XBoxMemoryResident;

var native const byte Data[24]; // sizeof (FSoundData) :(
var native const Name FileType;
//var (Sound) native const String FileName;
var native const int OriginalSize;
var native const float Duration;
var native const int	Handle;
var native const INT	XBBankHandle;
var native const Sound Parent;


defaultproperties
{
     MaxCampaign=255
     Likelihood=1
}

