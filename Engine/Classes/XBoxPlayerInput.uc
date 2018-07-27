class XBoxPlayerInput extends PlayerInput
	config(User)
	transient;

const InputMax = 32768;
const RunThresh = 16384;
const DodgeThresh = 10000;

var() config float  HScale;
var() config float  HExponent;
var() config float  HLookRateMax;
var() config float  VScale;
var() config float  VExponent;
var() config float  VLookRateMax;
var() config bool   bInvertVLook;
var() config bool   bLookSpring;
var() float         HLook;
var() float         VLook;

struct LookPreset
{
    var() localized string  PresetName;
    var() float             HScale;
    var() float             HExponent;
    var() float             VScale;
    var() float             VExponent;
};

const NumPresets=4;
var() config LookPreset     LookPresets[NumPresets];
var() config string         SelectedPresetName;
 
var() float                 VelScale;
var() float                 AccelScale;
var() float                 DampeningFactor;
var() float                 MinAccelComponent;

const MaxFilterEntries=4;
var() float                 ForwardFilter[MaxFilterEntries];
var() float                 StrafeFilter[MaxFilterEntries];

// Postprocess the player's input.
function PlayerInput( float DeltaTime )
{
    local float FOVScale;

    if (bSnapLevel != 0)
        bCenterView = true;
    else if (aBaseZ != 0)
        bCenterView = false;

    if (bInvertVLook)
        aBaseZ *= -1.f;

    FOVScale = DesiredFOV * 0.01; //should be 1/defaultFOV

    // Remap the turn inputs to an exponential curve
    HLook = Remap(aBaseX, HScale, HExponent, HLookRateMax);

    if (bSnapToLevel)
        VLook += (aBaseZ*0.45 - VLook) * DeltaTime * (VLookRateMax/500.0*FOVScale);
    else
        VLook = Remap(aBaseZ, VScale, VExponent, VLookRateMax) * FOVScale;

	// Check for Double click move
	// flag transitions
    if (Abs(aStrafe) > InputMax || Abs(aBaseY) > InputMax) // d-pad inputs are always 2 x inputmax
    {
	    bEdgeForward    = (bWasForward  ^^ (aBaseY  > DodgeThresh));
	    bEdgeBack       = (bWasBack     ^^ (aBaseY  < -DodgeThresh));
	    bEdgeRight      = (bWasRight    ^^ (aStrafe > DodgeThresh));
	    bEdgeLeft       = (bWasLeft     ^^ (aStrafe < -DodgeThresh));
	    bWasForward     = (aBaseY  > DodgeThresh);
	    bWasBack        = (aBaseY  < -DodgeThresh);
	    bWasRight       = (aStrafe > DodgeThresh);
	    bWasLeft        = (aStrafe < -DodgeThresh);
    }
    else // don't allow dodging with analog stick (it sucks)
    {
	    bEdgeForward    = false;
	    bEdgeBack       = false;
	    bEdgeRight      = false;
	    bEdgeLeft       = false;
	    bWasForward     = false;
	    bWasBack        = false;
	    bWasRight       = false;
	    bWasLeft        = false;
    }

    // Map to other input axes
    aForward = aBaseY;
    aTurn    = HLook * FOVScale;
    aLookUp  = VLook;

    if (Abs(aBaseY) > RunThresh || Abs(aStrafe) > RunThresh)
        bRun = 0; 
    else
        bRun = 1; // bRun=1 means walking obviously
    
	// Handle walking.
	HandleWalking();
}

// exp remap + linear remap
static function float Remap(float in, float scale, float exp, float ratemax)
{
    local float out;
    local bool bNeg;

    in /= InputMax;

    if (in < 0)
    {
        bNeg = true;
        in *= -1.f;
    }

    out = (in * scale) + (in**exp);

    if (bNeg)
        out *= -1.f;

    out *= ratemax/(1.f + scale);

    return out;
}


defaultproperties
{
     HExponent=1
     HLookRateMax=1500
     VExponent=1
     VLookRateMax=750
     LookPresets(0)=(PresetName="Linear",HExponent=1,VExponent=1)
     LookPresets(1)=(PresetName="Exponential",HExponent=2,VExponent=2)
     LookPresets(2)=(PresetName="Hybrid",HScale=0.5,HExponent=4,VScale=0.5,VExponent=4)
     LookPresets(3)=(PresetName="Custom",HScale=0.5,HExponent=4,VScale=0.5,VExponent=4)
     SelectedPresetName="Hybrid"
     VelScale=0.0134
     AccelScale=4.655
     DampeningFactor=30
     MinAccelComponent=0.1
}

