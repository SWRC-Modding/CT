//=============================================================================
// The Basic constraint class.
//=============================================================================

class KConstraint extends KActor
    abstract
	placeable
	native;

#exec Texture Import File=Textures\S_KConstraint.pcx Name=S_KConstraint Mips=Off MASKED=1

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

// Used internally for Karma stuff - DO NOT CHANGE!
var transient const int KConstraintData;

// Actors joined effected by this constraint (could be NULL for 'World')
var(KarmaConstraint) edfindable Actor KConstraintActor1;
var(KarmaConstraint) edfindable Actor KConstraintActor2;

// If an KConstraintActor is a skeletal thing, you can specify which bone inside it
// to attach the constraint to. If left blank (the default) it picks the nearest bone.
var(KarmaConstraint) name KConstraintBone1;
var(KarmaConstraint) name KConstraintBone2;

// Disable collision between joined
var(KarmaConstraint) const bool bKDisableCollision;

// Constraint position/orientation, as defined in each body's local reference frame
// These are in KARMA scale!

// Body1 ref frame
var vector KPos1;
var vector KPriAxis1;
var vector KSecAxis1;

// Body2 ref frame
var vector KPos2;
var vector KPriAxis2;
var vector KSecAxis2;

// Force constraint to re-calculate its position/axis in local ref frames
// Usually true for constraints saved out of UnrealEd, false for everything else
var const bool bKForceFrameUpdate;

// [see KForceExceed below]
var(KarmaConstraint) float KForceThreshold;


// This function is used to re-sync constraint parameters (eg. stiffness) with Karma.
// Call when you change a parameter to get it to actually take effect.
native function KUpdateConstraintParams();

native final function KGetConstraintForce(out vector Force);
native final function KGetConstraintTorque(out vector Torque);

// Event triggered when magnitude of constraint (linear) force exceeds KForceThreshold
event KForceExceed(float forceMag);

// Default behaviour when triggered is to wake up the physics.
function Trigger( actor Other, pawn EventInstigator )
{
	if( Physics != PHYS_Karma )
		SetPhysics( PHYS_Karma );

	KWake();
}



cpptext
{
#ifdef WITH_KARMA
    virtual MdtConstraintID getKConstraint() const;
    virtual McdModelID getKModel() const;

	virtual void physKarma(FLOAT DeltaTime);

	virtual void PostEditChange();
	virtual void PostEditMove();

    virtual void KUpdateConstraintParams();
	
	virtual void CheckForErrors(); // used for checking that this constraint is valid buring map build
	virtual void RenderEditorSelected(FLevelSceneNode* SceneNode,FRenderInterface* RI, FDynamicActor* FDA);
	virtual UBOOL Tick( FLOAT DeltaTime, enum ELevelTick TickType );

	virtual void preKarmaStep(FLOAT DeltaTime) {};
	virtual void postKarmaStep() {};
#endif

}

defaultproperties
{
     bKDisableCollision=True
     KPriAxis1=(X=1)
     KSecAxis1=(Y=1)
     KPriAxis2=(X=1)
     KSecAxis2=(Y=1)
     DrawType=DT_Sprite
     bHidden=True
     bCriticalAttachment=True
     bWorldGeometry=False
     bCollideActors=False
     bBlockActors=False
     bBlockPlayers=False
     bProjTarget=False
     bBlockKarma=False
     Texture=Texture'Engine.S_KConstraint'
}

