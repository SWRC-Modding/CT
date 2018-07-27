class TexPanner2D extends TexModifier
	editinlinenew
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var() float SpeedU;
var() float SpeedV;
var() float OffsetU;
var() float OffsetV;
var() float ScaleU;
var() float ScaleV;
var() float ClampedSizeU;
var() float ClampedSizeV;

var Matrix M;



cpptext
{
	// UTexModifier interface
	virtual FMatrix* GetMatrix(FLOAT TimeSeconds);

}

defaultproperties
{
     SpeedU=0.5
     SpeedV=0.5
     ScaleU=1
     ScaleV=1
     ClampedSizeU=1
     ClampedSizeV=1
}

