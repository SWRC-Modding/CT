class TexOscillator extends TexModifier
	editinlinenew
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

enum ETexOscillationType
{
	OT_Pan,
	OT_Stretch,
	OT_StretchRepeat,
	OT_Jitter
};

var() Float UOscillationRate;
var() Float VOscillationRate;
var() Float UOscillationPhase;
var() Float VOscillationPhase;
var() Float UOscillationAmplitude;
var() Float VOscillationAmplitude;
var() ETexOscillationType UOscillationType;
var() ETexOscillationType VOscillationType;
var() float UOffset;
var() float VOffset;

var Matrix M;

// current state for OT_Jitter.
var float LastSu;
var float LastSv;
var float CurrentUJitter;
var float CurrentVJitter;



cpptext
{
	// UTexModifier interface
	virtual FMatrix* GetMatrix(FLOAT TimeSeconds);
	// UObject interface
	virtual void PostEditChange();

}

defaultproperties
{
     UOscillationRate=1
     VOscillationRate=1
     UOscillationAmplitude=0.1
     VOscillationAmplitude=0.1
}

