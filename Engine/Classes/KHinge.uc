//=============================================================================
// The Hinge joint class.
//=============================================================================

#exec Texture Import File=Textures\S_KHinge.pcx Name=S_KHinge Mips=Off MASKED=1


class KHinge extends KConstraint
    native
    placeable;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

// Spatial light effect to use.
var(KarmaConstraint) enum EHingeType
{
	HT_Normal,
	HT_Springy,
	HT_Motor,
    HT_Controlled
} KHingeType;


// SPRINGY - around hinge axis, default position being KDesiredAngle (below)
var(KarmaConstraint) float KStiffness;
var(KarmaConstraint) float KDamping;

// MOTOR - tries to achieve angular velocity
var(KarmaConstraint) float KDesiredAngVel; // 65535 = 1 rotation per second
var(KarmaConstraint) float KMaxTorque;

// CONTROLLED - achieve a certain angle
// Uses AngularVelocity and MaxForce from above.
// Within 'ProportionalGap' of DesiredAngle, 
var(KarmaConstraint) float KDesiredAngle; // 65535 = 360 degrees
var(KarmaConstraint) float KProportionalGap; // 65535 = 360 degrees

// This is the alternative 'desired' angle, and the bool that indicates whether to use it.
// See ToggleDesired and ControlDesired below.
var(KarmaConstraint) float KAltDesiredAngle; // 65535 = 360 degrees
var					 bool  KUseAltDesired;

// output - current angular position of joint // 65535 = 360 degrees
var const float KCurrentAngle;

// In this state nothing will happen if this hinge is triggered or untriggered.
auto state Default
{
ignores Trigger, Untrigger;

}

// In this state, Trigger will cause the hinge type to change to HT_Motor.
// Another trigger will toggle it to HT_Controlled, and it will try and maintain its current angle.
state() ToggleMotor
{
ignores Untrigger;
	function Trigger( actor Other, pawn EventInstigator )
	{
		//Log("ToggleMotor - Trigger");
		if(KHingeType == HT_Motor)
		{
			KDesiredAngle = KCurrentAngle;
			KUseAltDesired = False;
			KHingeType = HT_Controlled;
		}
		else
			KHingeType = HT_Motor;

		KUpdateConstraintParams();
		KConstraintActor1.KWake(); // force re-enable of simulation on this actor.
	}

Begin:
	KHingeType = HT_Controlled;
	KUseAltDesired = False;
	KUpdateConstraintParams();
}

// In this state, Trigger will turn motor on.
// Untrigger will turn toggle it to HT_Controlled, and it will try and maintain its current angle.
state() ControlMotor
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		//Log("ControlMotor - Trigger");
		if(KHingeType != HT_Motor)
		{
			KHingeType = HT_Motor;
			KUpdateConstraintParams();
			KConstraintActor1.KWake();
		}
	}

	function Untrigger( actor Other, pawn EventInstigator )
	{
		//Log("ControlMotor - Untrigger");
		if(KHingeType == HT_Motor)
		{
			KDesiredAngle = KCurrentAngle;
			KUseAltDesired = False;
			KHingeType = HT_Controlled;
			KUpdateConstraintParams();
			KConstraintActor1.KWake();
		}
	}

Begin:
	KHingeType = HT_Controlled;
	KUseAltDesired = False;
	KUpdateConstraintParams();
}

// In this state a trigger will toggle the hinge between using KDesiredAngle and KAltDesiredAngle.
// It will use whatever the current KHingeType is to achieve this, so this is only useful with HT_Controlled and HT_Springy.
state() ToggleDesired
{
ignores Untrigger;

	function Trigger( actor Other, pawn EventInstigator )
	{
		//Log("ToggleDesired - Trigger");
		if(KUseAltDesired)
			KUseAltDesired = False;
		else
			KUseAltDesired = True;
		//Log("UseAlt"$KUseAltDesired);
		KUpdateConstraintParams();
		KConstraintActor1.KWake();
	}
}

// In this state, trigger will cause the hinge to use KAltDesiredAngle, untrigger will caus it to use KAltDesiredAngle
state() ControlDesired
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		//Log("ControlDesired - Trigger");
		KUseAltDesired = True;
		//Log("UseAlt"$KUseAltDesired);
		KUpdateConstraintParams();
		KConstraintActor1.KWake();
	}

	function Untrigger( actor Other, pawn EventInstigator )
	{
		//Log("ControlDesired - Untrigger");
		KUseAltDesired = False;
		//Log("UseAlt"$KUseAltDesired);
		KUpdateConstraintParams();
		KConstraintActor1.KWake();
	}
}



cpptext
{
#ifdef WITH_KARMA
    virtual void KUpdateConstraintParams();
	virtual void preKarmaStep(FLOAT DeltaTime);
#endif

}

defaultproperties
{
     KStiffness=50
     KProportionalGap=8200
     Texture=Texture'Engine.S_KHinge'
     bDirectional=True
}

