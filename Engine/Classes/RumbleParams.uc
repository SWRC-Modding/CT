//=============================================================================
// Clone Troooper Rumble Parameters
//
// Created: 2003 April 2, Xristos Gripeos
//=============================================================================

class RumbleParams extends SoundBase
	hidecategories(Advanced, Display, Events, Object)
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)


// public
var(Sound) float IntensityLeft;
var(Sound) float IntensityRight;

var(Sound) float OverrideRadius;

var(Sound) float TimeFadeIn;
var(Sound) float TimeHold;
var(Sound) float TimeFadeOut;

var(Sound) float	CamShakeRollMag;
var(Sound) vector	CamShakeOffsetMag;
var(Sound) float	CamShakeRollRate;
var(Sound) vector	CamShakeOffsetRate;

// private



// defaults


cpptext
{
	virtual void PostEditChange();

	inline void CalcIntensities(FLOAT TotalTime, FLOAT& Left, FLOAT& Right )
	{
		//if( TotalTime > (TimeFadeIn + TimeHold + TimeFadeOut) )
		//{ // done
			Left = 0;
			Right = 0;
		//}
		if(TotalTime < TimeFadeIn)
		{ // fade in
			float time = (TotalTime / TimeFadeIn);
			Left  = IntensityLeft * time;
			Right = IntensityRight * time;
		}
		else if( TotalTime > (TimeFadeIn + TimeHold) )
		{// fade out
			float time = (TotalTime - (TimeFadeIn + TimeHold)) / TimeFadeOut;
			Left  = IntensityLeft  - (IntensityLeft  * time);
			Right = IntensityRight - (IntensityRight * time);
		}
		else
		{ // hold
			Left  = IntensityLeft; 
			Right = IntensityRight;
		}
	};


}

defaultproperties
{
     OverrideRadius=-1
}

