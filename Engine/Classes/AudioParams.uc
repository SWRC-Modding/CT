//=============================================================================
// Clone Troooper Audio Parameters
//
// Created: 2003 April 2, John Hancock
//=============================================================================

class AudioParams extends SoundBase
	hidecategories(Advanced, Display, Events, Object)
	native;

enum EAudioGroup
{
	AG_Music,
	AG_Voice,
	AG_SFX
};

var (Sound) EAudioGroup		Category;
var (Sound)	bool			bDisableVoiceCues;
var (Sound) name			ActorTag;

var (Sound) float			MaxVolume;
var (Sound) float			MinVolume;
var (Sound) float			MaxPitch;
var (Sound) float			MinPitch;
var (Sound) float			MaxVolumeXBox;
var (Sound) float			MinVolumeXBox;
var (Sound) float			MaxPitchXBox;
var (Sound) float			MinPitchXBox;

var (Sound) bool			NonPositional; //whether the sound is positional
var (Sound) bool			NonPositionalXBoxOverride; //whether the XB sound is positional

var (Sound) float			Radius;
var (Sound) float			RadiusXBox;


var (Sound)	bool			bDisable3DOcclusion;
var (Sound)	bool			bDisable3DFalloff;
var (Sound)	bool			bDisable3DOcclusionXBox;
var (Sound)	bool			bDisable3DFalloffXBox;

var (Sound) float			XBoxRolloffFactor;
var (Sound) bool			XBox6dbBoost;


defaultproperties
{
     MaxVolume=1
     MinVolume=1
     MaxPitch=1
     MinPitch=1
     MaxVolumeXBox=-1
     MinVolumeXBox=-1
     MaxPitchXBox=-1
     MinPitchXBox=-1
     Radius=2000
     RadiusXBox=-1
     XBoxRolloffFactor=1
}

