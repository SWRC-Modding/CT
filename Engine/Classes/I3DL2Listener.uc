//=============================================================================
// I3DL2Listener: Base class for I3DL2 room effects.
//=============================================================================

class I3DL2Listener extends Object
	abstract
	editinlinenew
	native;

var()			float		EnvironmentDiffusion;
var()			int			Room; 
var()			int			RoomHF; 
var()			float		DecayTime;
var()			int			Reflections;
var()			float		ReflectionsDelay;
var()			int			Reverb;
var()			float		ReverbDelay;
var()			float		RoomRolloffFactor;
var()			float		HFReference;

var(I3DL2ListenerXBox)		float		Density;              // [0.0, 100.0]     default: 100.0 %

var(I3DL2ListenerEAX30)		float		EnvironmentSize;
var(I3DL2ListenerEAX30)		int			RoomLF;
var(I3DL2ListenerEAX30)		float		DecayHFRatio;
var(I3DL2ListenerEAX30)		float		DecayLFRatio;
var(I3DL2ListenerEAX30)		vector		ReflectionsPan;
var(I3DL2ListenerEAX30)		vector		ReverbPan;
var(I3DL2ListenerEAX30)		float		EchoTime;
var(I3DL2ListenerEAX30)		float		EchoDepth;
var(I3DL2ListenerEAX30)		float		ModulationTime;
var(I3DL2ListenerEAX30)		float		ModulationDepth;
var(I3DL2ListenerEAX30)		float		AirAbsorptionHF;
var(I3DL2ListenerEAX30)		float		LFReference;

var(I3DL2ListenerEAX30)		bool		bDecayTimeScale;
var(I3DL2ListenerEAX30)		bool		bReflectionsScale;
var(I3DL2ListenerEAX30)		bool		bReflectionsDelayScale;
var(I3DL2ListenerEAX30)		bool		bReverbScale;
var(I3DL2ListenerEAX30)		bool		bReverbDelayScale;
var(I3DL2ListenerEAX30)		bool		bEchoTimeScale;
var(I3DL2ListenerEAX30)		bool		bModulationTimeScale;
var(I3DL2ListenerEAX30)		bool		bDecayHFLimit;

var	transient	int			Environment;
var transient	int			Updated;


defaultproperties
{
     EnvironmentDiffusion=1
     Room=-10000
     DecayTime=1.49
     Reflections=-2602
     ReflectionsDelay=0.007
     Reverb=200
     ReverbDelay=0.011
     HFReference=5000
     Density=100
     EnvironmentSize=7.5
     DecayHFRatio=0.83
     DecayLFRatio=1
     EchoTime=0.25
     ModulationTime=0.25
     AirAbsorptionHF=-5
     LFReference=250
     bDecayTimeScale=True
     bReflectionsScale=True
     bReflectionsDelayScale=True
     bReverbScale=True
     bReverbDelayScale=True
     bEchoTimeScale=True
     bDecayHFLimit=True
}

