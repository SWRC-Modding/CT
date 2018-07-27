class MotionBlur extends CameraEffect
	native
	noexport
	editinlinenew
	collapsecategories;

var() byte		BlurAlpha;

var const int	RenderTargets[2];
var const float	LastFrameTime;


defaultproperties
{
     BlurAlpha=32
}

