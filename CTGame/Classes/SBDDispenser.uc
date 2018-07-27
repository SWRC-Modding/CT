//=============================================================================
// SuperBattleDroid Dispensor
//
// Created: 2004, February 25, Jenny Huang
//=============================================================================

class SBDDispenser extends DroidDispenser
	placeable;



defaultproperties
{
     NumDispensers=4
     DamagedInfo=(TransitionEffect=Class'CTEffects.Dispenser_Explosion')
     AttachBones(0)="droidAttach1"
     AttachBones(1)="droidAttach2"
     AttachBones(2)="droidAttach3"
     AttachBones(3)="droidAttach4"
     DispenseAnims(0)="Dispense1"
     DispenseAnims(1)="Dispense2"
     DispenseAnims(2)="Dispense3"
     DispenseAnims(3)="Dispense4"
     Mesh=SkeletalMesh'Dispensers.SBDDispenser'
     CollisionRadius=200
     CollisionHeight=125
}

