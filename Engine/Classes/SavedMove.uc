//=============================================================================
// SavedMove is used during network play to buffer recent client moves,
// for use when the server modifies the clients actual position, etc.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class SavedMove extends Info;

// also stores info in Acceleration attribute
var SavedMove NextMove;		// Next move in linked list.
var float TimeStamp;		// Time of this move.
var float Delta;			// Distance moved.
var bool	bRun;
var bool	bDuck;
var bool	bPressedJump;
var bool	bDoubleJump;
var EDoubleClickDir DoubleClickMove;	// Double click info.
var EPhysics SavedPhysics;
var vector StartLocation, StartRelativeLocation, StartVelocity, StartFloor, SavedLocation, SavedVelocity, SavedRelativeLocation;
var Actor StartBase, EndBase;

final function Clear()
{
	TimeStamp = 0;
	Delta = 0;
	DoubleClickMove = DCLICK_None;
	Acceleration = vect(0,0,0);
	StartVelocity = vect(0,0,0);
	bRun = false;
	bDuck = false;
	bPressedJump = false;
	bDoubleJump = false;
}

final function PostUpdate(PlayerController P)
{
	bDoubleJump = P.bDoubleJump || bDoubleJump;
	if ( P.Pawn != None )
	{
		SavedLocation = P.Pawn.Location;
		SavedVelocity = P.Pawn.Velocity;
		EndBase = P.Pawn.Base;
		if ( (EndBase != None) && !EndBase.bWorldGeometry )
			SavedRelativeLocation = P.Pawn.Location - EndBase.Location;
	}
	SetRotation(P.Rotation);
}

final function bool IsJumpMove()
{
  return ( bPressedJump || bDoubleJump 
		|| ((DoubleClickMove != DCLICK_None) && (DoubleClickMove != DCLICK_Active) && (DoubleClickMove != DCLICK_Done)) );
}

function vector GetStartLocation()
{
	if ( Mover(StartBase) != None )
		return StartBase.Location + StartRelativeLocation;
	
	return StartLocation;

}
final function SetInitialPosition(Pawn P)
{
	SavedPhysics = P.Physics;
	StartLocation = P.Location;
	StartVelocity = P.Velocity;
	StartBase = P.Base;
	StartFloor = P.Floor;
	if ( (StartBase != None) && !StartBase.bWorldGeometry )
		StartRelativeLocation = P.Location - StartBase.Location;
}

final function SetMoveFor(PlayerController P, float DeltaTime, vector NewAccel, EDoubleClickDir InDoubleClick)
{
	Delta = DeltaTime;
	if ( VSize(NewAccel) > 3072 )
		NewAccel = 3072 * Normal(NewAccel);
	if ( P.Pawn != None )
		SetInitialPosition(P.Pawn);
	Acceleration = NewAccel;
		DoubleClickMove = InDoubleClick;
	bRun = (P.bRun > 0);
	bDuck = (P.bDuck > 0);
	bPressedJump = P.bPressedJump;
	bDoubleJump = P.bDoubleJump;
	TimeStamp = Level.TimeSeconds;
}


defaultproperties
{
}

