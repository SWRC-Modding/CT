// ====================================================================
//	File:	CTPawn.uc
//  Class:  CTPawn
//  Parent: Engine.Pawn
//
//  Base class for all pawns in the game
// ====================================================================

class CTPawn extends Pawn native
	dependsOn(IdleInfo);

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var() class<IdleInfo> IdleInfoClass;
var() autoload string IdleInfoClassName;			// Deprecated

var() class<FootstepInfo> FootstepInfoClass;	

simulated event PostNetBeginPlay()
{
	Super.PostNetBeginPlay();
	if( IdleInfoClassName != "" )
		IdleInfoClass = class<IdleInfo>(DynamicLoadObject(IdleInfoClassName, class'Class'));
}




cpptext
{
	virtual void PlayFootstepSound(BYTE eMatType, BYTE TypeOfWalk, USound *pDefaultSound);
	virtual void PlayOwnedFootstepSound(BYTE eMatType, BYTE TypeOfWalk, USound *pDefaultSound);
protected:
	USound      *DetermineFootstepSound(BYTE eMatType, BYTE TypeOfWalk, USound *pDefaultSound) const;

public:

}

defaultproperties
{
     DamageMultipliers(0)=(BoneName="Head",Multiplier=2)
}

