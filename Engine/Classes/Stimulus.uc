//=============================================================================
// Clone Troooper Stimulus base class
//
// Created: 2003 April 2, John Hancock
//=============================================================================

class Stimulus extends Object
	native;

const SF_ProcessImmediately = 0x0001;
const SF_PassToSquad = 0x0002;
const SF_IgnoreIfNoInstigator = 0x0004; //ignore if the instigator is dead
const SF_Audible = 0x0008; //this is an audible stimulus, and therefore competes w/ ambient noise

//const SF_WakeUp = 0x0004;	//if the AI is asleep, wake them up
//const SF_ForceDecisionNow = 0x0008;

//enum EStimulusType is defined in Actor.uc because of parsing order problem
//EStimulusType is what is used to generate the stimulus. It corresponds to a set
//of parameters including the StimulusGroup, volume, and range. The parameters
//are independent of the location and instigator of the event. Parameters
//are not used for some 


//This will get copied...
var Actor.EStimulusType StimType;
var Vector Location;
var Pawn Instigator;		//the root cause of the stimulus (always a Pawn)
var Actor Subject;			//the subject of the stimulus or message (could be None)
var int	TeamIndex;			//the team index of the Instigator (in case it gets deleted)
var int RefCount; //whenever a bot receives a stimulus, it should increment the refCount
var int Responses; //how many bots have responded to the stimulus
var float Time;

//perhaps we should cache the relative volume here... if we are copying this
//per character anyhow, we might as well save that info

native final function Actor.EStimulusGroup Group(); //returns the group
//native final function float RelativeVolume(Vector Loc);


defaultproperties
{
}

