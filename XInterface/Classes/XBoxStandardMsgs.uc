class XboxStandardMsgs extends Menu;

// NOTE: Most, if not all of the Live errors are in XboxLive.int!

// ALSO NOTE: Don't refer to the xbox buttons (ie: press A to ...).

// xbox standard msgs
const Surprise_MU_Removal = 0; // C13-1
const MU_Write_Warning = 1; // C13-2
const MU_Read_Warning = 2; // C13-3
const Hard_Disk_Write_Warning = 3; // C13-4
const Content_Protection_Fault = 4; // C13-5
const Press_START = 5; // C13-6
const Lost_Controller = 6; // C13-7
const Low_Storage = 7; // C13-8 - pressing B should launch dashboard memory section
const Bad_Game_Disc = 8; // C13-9
const Lost_Controller_No_Pause = 9; // C13-7
const System_Link_Disconnect = 10; // C10-7
const Lost_Controller_Dedicated = 11; // C13-7

//enum EXboxLiveMsg
const XONLINE_E_CANNOT_ACCESS_SERVICE = 0;
const XONLINE_E_LOGON_UPDATE_REQUIRED = 1;
const XONLINE_E_LOGON_SERVERS_TOO_BUSY = 2;
const XONLINE_E_DUPLICATE_LOGON_DETECTED = 3;
const XONLINE_E_LOGON_USER_ACCOUNT_REQUIRES_MANAGEMENT = 4;
const XONLINE_S_LOGON_USER_HAS_MESSAGE = 5;
const XONLINE_E_MATCH_INVALID_SESSION_ID = 6;
const Bad_Network_Connection = 7;
const Not_Enough_Openings = 8;
const Modified_Corrupt_Package = 9;
const XONLINE_E_LOGON_NO_NETWORK_CONNECTION = 10;
const XONLINE_E_LOGON_KICKED_BY_DUPLICATE_LOGON = 11;
const Misc_Online_Failure = 12;
const XONLINE_E_LOGON_CONNECTION_LOST = 13;
const XONLINE_E_LOGON_INVALID_USER = 14;
const XONLINE_E_LOGON_SERVICE_NOT_AUTHORIZED = 15;
const XONLINE_E_LOGON_SERVICE_TEMPORARILY_UNAVAILABLE = 16;
const Misc_Online_Service_Error = 17;
const Accept_Invite = 18;

var() localized string ErrorMsg[14];
var() localized string LiveError[19];


defaultproperties
{
     ErrorMsg(0)="The Xbox memory unit %s was removed during use. %s failed."
     ErrorMsg(1)="Saving to memory unit %s. Please do not remove memory units or disconnect any controllers."
     ErrorMsg(2)="Loading %s. Please don't remove memory units or disconnect any controllers."
     ErrorMsg(3)="Saving %s. Please don't turn off your Xbox console."
     ErrorMsg(4)="Unable to load %s."
     ErrorMsg(5)="Please press START to begin."
     ErrorMsg(6)="Please reconnect the controller to port %d and press START to continue."
     ErrorMsg(7)="Your Xbox doesn't have enough free blocks to save games. You need to free %d more blocks. Press A to continue without saving or B to free more blocks."
     ErrorMsg(8)="There's a problem with the disc you're using. It may be dirty or damaged."
     ErrorMsg(9)="Please reconnect the controller to port %d."
     ErrorMsg(10)="There's no connection available, please check your cable."
     ErrorMsg(11)="Please reconnect the controller to port %d (Server is still running)."
     ErrorMsg(12)="There is a maximum of %d profiles."
     ErrorMsg(13)="There is a maximum of %d saved games."
     LiveError(0)="Your Xbox console cannot connect to Xbox Live."
     LiveError(1)="A required update is available for the Xbox Live service. You cannot connect to Xbox Live until the update is installed."
     LiveError(2)="The Xbox Live service is very busy."
     LiveError(3)="You were signed out of Xbox Live because another person signed on using your account."
     LiveError(4)="You have an important message from Xbox Live."
     LiveError(5)="You have a new Xbox Live message."
     LiveError(6)="This game session is no longer available."
     LiveError(7)="Network conditions may affect this game."
     LiveError(8)="The game does not have enough openings for all the players on your console."
     LiveError(9)="The %s is damaged and cannot be used."
     LiveError(10)="There's no connection available, please check your cable."
     LiveError(11)="You have been signed out because your account signed in on another Xbox."
     LiveError(12)="Xbox Live connection was lost. Must re-login."
     LiveError(13)="Network Connection was lost."
     LiveError(14)="Invalid user detected."
     LiveError(15)="Access to service %d is denied."
     LiveError(16)="Service %d is temporarily unavailable."
     LiveError(17)="Error %d signing onto service %d"
     LiveError(18)="You have a pending game invitation. Accept Invitation?"
}

