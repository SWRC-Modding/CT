// ====================================================================
//  Class:  CTGame.MoverDB
//
// Created 2004 February 23, John Hancock
// A superclass for all door breach doors
// ====================================================================

class MoverDB extends Mover
	placeable;

function FinishedClosing()
{
	super.FinishedClosing();
	StayOpenTime = default.StayOpenTime;
}


defaultproperties
{
     MoveTimes(0)=0.5
     StayOpenTime=0.2
     bIsLeader=True
     DelayTime=0.2
     bAutoDoor=True
     DrawType=DT_StaticMesh
     InitialState="Locked"
}

