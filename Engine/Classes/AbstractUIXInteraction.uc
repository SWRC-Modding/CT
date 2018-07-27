class AbstractUIXInteraction
	extends Interaction
	native;
/* epic  ===============================================
 * Author: brian
 * =====================================================
 * This is the interface for the UIX (XboxLive) interaction
 * ===================================================== 
 */
//`include(..\Core\Classes\Avaris.huc)

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// ----- Initialization and finalization routines -----
// Interaction interface: Called when interaction is instantiated 
function Initialize() {
	super.Initialize();
	InitUIX();
} // Initialize

// Interaction interface: Called when the interaction is destroyed
// tears down everything
function Destroyed();

// implemented natively in subclasses --
// initializes all logon and session information
function InitUIX(); 

// ----- Sign-on/Sign-off functions ------
// Signing on is asynchronous; this function begins the process
function StartSignIn(optional bool bSilient);
// returns true if asynch sign on is done AND have not signed out
function bool SignedIn() { return false; }
// Signing out starts with this function
function StartSignOut();

// returns the gamer tag for a given gamepad. Will be "GUEST" for guests
function string getGamerTag(int gamePadID);

// ----- Session management -----
// Interaction interface: Registers the current game online (here it calls register session)
function RegisterOnline(GameInfo gi)
{
	RegisterSession();
} // RegisterOnline

function RegisterSession();
function UpdateSession();
function EndSession();

// ----- Matchmaking Functions -----
function QuickMatch();
function OptiMatch();

// ----- Friend/Player management -----
function ShowFriends(int gamePadID);

// AbstractUIXInteraction


cpptext
{
	virtual void EndSession() {}

}

defaultproperties
{
}

