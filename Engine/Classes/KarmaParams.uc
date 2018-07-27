//=============================================================================
// The Karma physics parameters class.
// This provides 'extra' parameters needed by Karma physics to the Actor class.
// Need one of these (or a subclass) to set Physics to PHYS_Karma.
// (see Actor.uc)
// NB: All parameters are in KARMA scale!
//=============================================================================

class KarmaParams extends KarmaParamsCollision
	editinlinenew
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

// Used internally for Karma stuff - DO NOT CHANGE!
var transient const int		KAng3;
var transient const int		KTriList;
var transient const float   KLastVel;

var()    float   KMass;						// Mass used for Karma physics
var()    float   KLinearDamping;			// Linear velocity damping (drag)
var()    float   KAngularDamping;			// Angular velocity damping (drag)

var()	 float   KBuoyancy;					// Applies in water volumes. 0 = no buoyancy. 1 = neutrally buoyant

var()    bool    KStartEnabled;				// Start simulating body as soon as PHYS_Karma starts
var()    vector  KStartLinVel;				// Initial linear velocity for actor
var()    vector  KStartAngVel;              // Initial angular velocity for actor

var()	 bool	 bKNonSphericalInertia;		// Simulate body without using sphericalised inertia tensor

var()	 float   KActorGravScale;		    // Scale how gravity affects this actor.

var()	 float   KVelDropBelowThreshold;    // Threshold that when actor drops below, KVelDropBelow event is triggered.

// NB - the below settings only apply to PHYS_Karma (not PHYS_KarmaRagDoll)
var()	 bool    bHighDetailOnly;			// Only turn on karma physics for this actor if the level PhysicsDetailLevel is PDL_High
var      bool    bClientOnly;				// Only turn on karma physics for this actor on the client (not server).

var()	 bool	 bKStayUpright;				// Stop this object from being able to rotate (using Angular3 constraint)
var()	 bool	 bKAllowRotate;				// Allow this object to rotate about a vertical axis. Ignored unless KStayUpright == true.
var		 bool	 bDestroyOnSimError;		// If there is a problem with the physics, destroy, or leave around to be fixed (eg. by network).

var()	 float   StayUprightStiffness;
var()	 float   StayUprightDamping;

// Whether to do a 'safe time' check to avoid this actor passing through other things
// Auto means it will only do it when it thinks it needs to (ie moving fast).
var()	enum ESafeTimeMode
{
	KST_None,
	KST_Auto,
	KST_Always
} SafeTimeMode;

// default is sphere with mass 1 and radius 1


cpptext
{
#ifdef WITH_KARMA
    void PostEditChange();
#endif

}

defaultproperties
{
     KMass=1
     KLinearDamping=0.2
     KAngularDamping=0.2
     KActorGravScale=1
     KVelDropBelowThreshold=1e+006
     bHighDetailOnly=True
     bClientOnly=True
     bDestroyOnSimError=True
     StayUprightStiffness=50
}

