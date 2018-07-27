//=============================================================================
// This is the full set of Karma parameters, including inertia tensor and 
// centre-of-mass position, which are normally stored with the StaticMesh.
// This gives you a chance to overrids these values.
// NB: All parameters are in KARMA scale!
//=============================================================================

class KarmaParamsRBFull extends KarmaParams
	editinlinenew
	native;

// Inertia tensor of object assuming a mass of 1 - symmetric so just store 6 elements:
// (0 1 2)
// (1 3 4)
// (2 4 5)
// This will be scaled by the mass of the object to work out its actual inertia tensor.
var()    float   KInertiaTensor[6];
var()    vector  KCOMOffset;         // Position of centre of mass in body ref. frame

// default is sphere with radius 1

defaultproperties
{
     KInertiaTensor(0)=0.4
     KInertiaTensor(3)=0.4
     KInertiaTensor(5)=0.4
}

