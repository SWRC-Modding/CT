class FadeColor extends ConstantMaterial
	native
	editinlinenew;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

enum EColorFadeType
{
	FC_Linear,
	FC_Sinusoidal,
};

var() Color Color1;
var() Color Color2;
var() float FadePeriod;
var() float FadePhase;
var() EColorFadeType ColorFadeType;



cpptext
{
	//
	// UConstantMaterial interface
	//
	virtual FColor GetColor(FLOAT TimeSeconds);

}

defaultproperties
{
}

