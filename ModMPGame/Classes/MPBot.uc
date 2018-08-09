class MPBot extends CTBot;

var int Team;
var int ChosenSkin;

//Prevents crash when game ends
function GameHasEnded(){}
function ClientGameEnded(){}

defaultproperties
{
	RequiredStates(0)=Class'CTGame.StateMercAttack'
	RequiredStates(1)=Class'CTGame.StateInvestigate'
	RequiredStates(2)=Class'CTGame.StateIdle'
	RequiredStates(3)=Class'CTGame.StatePatrol'
	bCanGesture=false
	NetPriority=3
}