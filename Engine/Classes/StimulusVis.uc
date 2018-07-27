//=============================================================================
// Actor: The base class of all actors.
// Actor is the base class of all gameplay objects.  
// A large number of properties, behaviors and interfaces are implemented in Actor, including:
//
// -	Display 
// -	Animation
// -	Physics and world interaction
// -	Making sounds
// -	Networking properties
// -	Actor creation and destruction
// -	Triggering and timers
// -	Actor iterator functions
// -	Message broadcasting
//
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class StimulusVis extends Actor
	native;

var float			Range;
var Actor			Instigator;
var Actor			Subject;
var EStimulusType	StimType;
var color			DrawColor;


defaultproperties
{
     DrawType=DT_None
     LifeSpan=3
}

