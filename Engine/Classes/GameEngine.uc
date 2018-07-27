//=============================================================================
// GameEngine: The game subsystem.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class GameEngine extends Engine
	native
	transient;


struct noexport URL
{
	var string			Protocol;
	var string			Host;
	var int				Port;
	var string			Map;
	var array<string>	Op;
	var string			Portal;
	var bool			Valid;
};

var config autoload array<string>	ServerActors;
var config array<string>			ServerPackages;

var Level				GLevel;
var Level				GEntry;
var PendingLevel		GPendingLevel;

 // Added by Demiurge (LevelLoading)
var PlayerController	LevelLoadingController;
var float				LoadProgressFraction;		// Current progress fraction to show.

var URL						LastURL;
var array<PackageCheckInfo>	PackageValidation;	// The Database of allowed MD5s
var Object					MD5Package;

var config bool				bUseXInterface;			// Whether or not to use the XInterface menuing system
var config autoload String		MainMenuClass;			// Menu that appears when you first start
var config autoload String		InitialMenuClass;		// The initial menu that should appear
var config autoload String		ControllerLayoutMenuClass;	// The menu that shows controller layout
var config autoload String		ConnectingMenuClass;	// Menu that appears when you are connecting
var config autoload String		DisconnectMenuClass;	// Menu that appears when you are disconnected
var config 			String		DisconnectMenuArgs;		// args to disconnectmenuclass, needed for various xlive menus
var config autoload String		LoadingClass;			// Loading screen that appears
var config autoload String		PauseMenuClass;			// In-game pause screen
var config autoload String		LoadingMenuClass;		// Loading screen
var config autoload String		LoadingInfoMenuClass;	// Loading screen with configurable text/pic
var config autoload String		TeamMenuClass;			// Choose Team screen
var config autoload String		ProfileMenuClass;		// Choose Profile screen
var config autoload String		MultiplayerPauseMenuClass;	// In-game multiplayer pause screen

var config int			FPSAlarm;				// Min FPS before triggering alarm.
var config bool			bMemoryAlarm;			// Automatically display mem stats on overflow.
var config bool			bAutoSaveStats;			// Automatically save time/mem stat reports.
var bool				bCheatProtection;
var bool				FramePresentPending;	// cg: split screen mod  

var const  float		TimeUntilExit;			// Time (in seconds) that must pass with no input activity before the app exits.
var const  bool			ExitDuringMovie;		// Whether or not to exit during movie playback.
var		   float		ElapsedExitTime;		// Time (in seconds) that has elapsed with no input activity (potentially incremented during level load).

var config float		TimeUntilAutoPause;		// Time (in seconds) that must pass with no input activity before the app automatically pauses the game in single player.
var		   float		ElapsedAutoPauseTime;	// Time (in seconds) that has elapsed with no input activity (potentially incremented during level load).

var		   float		ElapsedInactiveTime;	// Time (in seconds) that has elapsed with no input activity.

var const  float		InitialAttractTime;		// Time (in seconds) that the first attract mode playback will start after.
var	config float		TimeUntilAttractMode;	// Time (in seconds) that attract mode will be activated if no activity takes place.
var		   float		ElapsedInactiveAttractTime;	// Time (in seconds) that has elapsed with no input activity (not incremented during movies, level loading).
var config String		AttractModeMovie;		// Movie to play in attract mode.
var config bool			bAttractDuringLevel;	// Whether or not to enter attract mode in-game (during a level, including at a pause screen).
var config bool			bAttractExitLevel;		// Whether or not to exit the level when entering attract mode.
var config bool			bLoopAttractModeMovie;	// Whether or not to loop the attract mode movie.
var config String		AttractModeMenuClass;	// If set, attract mode will only be entered if we're showing a menu of this class (with the above options taken into account as well).

var const bool			bMovieInterruptable;	// Whether or not the currently playing movie is interruptable (via keypress)
var const bool			bMoviePausedLevelMusic; // Whether or not the currently playing movie paused the level music
var const bool			bMoviePausedLevelSound; // Whether or not the currently playing movie paused the level sound
var const bool			bMovieDisassociatePads;	// Whether or not to disassociate controllers from their viewports when the movie is interrupted

var localized string	ProfilePrefix;

var localized string	DeathMatchStr;
var localized string	TeamDeathMatchStr;
var localized string	CaptureTheFlagStr;
var localized string	AssaultStr;

struct LevelLoadingInfoEntry
{
	var() String MapName;
	var() String LoadingMenuClass;
	var() String LoadingPic;
	var() String LoadingTitle;
	var() String LoadingText;
	var() bool   LoadingShowHints;
};

var() Array<LevelLoadingInfoEntry> LevelLoadingInfo;

struct LoadingHint
{
	var() String Title;
	var() String Text;
	var() Int	 PicIndex;
};

var() Array<LoadingHint> LoadingHints;

var() localized String	LocalizedKeyNames[255];
var() localized String	LocalizedXboxButtonNames[16];

const					NUM_PROGRESS_LEVELS = 56;
var				String	ProgressLevels[NUM_PROGRESS_LEVELS];

var				String	LevelMissionStartLevels[NUM_PROGRESS_LEVELS];

var() localized String	DiscReadError;



var int NumFriendRequests;
var int NumGameInvites;
// for drawing the live icons where appropriate:
var int LastNumFriendRequests;
var int LastNumGameInvites;
var float FriendRequestTimeout;
var float GameInviteTimeout;
var float NextMatchmakingQueryTime;


simulated native event String GetLocalizedKeyName(Interactions.EInputKey k);

simulated event InitLevelLoadingInfo()
{
	local string key;
	local string value;
	local bool bGotValue;
	local int i;
	local string file;
	
	file = "LevelLoadingInfo";
	
	i = 0;
	
	do
	{
		key = "MapName[" $ i $ "]";
		value = Localize( "LevelLoadingInfo", key, File, True );
		bGotValue = (value != "");
		if ( !bGotValue )
			continue;
		
		LevelLoadingInfo.Length = i + 1;
		
		LevelLoadingInfo[i].MapName = value;

		key = "LoadingMenuClass[" $ i $ "]";
		value = Localize( "LevelLoadingInfo", key, File, True );
		LevelLoadingInfo[i].LoadingMenuClass = value;
		
		key = "LoadingPic[" $ i $ "]";
		value = Localize( "LevelLoadingInfo", key, File, True );
		LevelLoadingInfo[i].LoadingPic = value;
	
		key = "LoadingTitle[" $ i $ "]";
		value = Localize( "LevelLoadingInfo", key, File, True );
		LevelLoadingInfo[i].LoadingTitle = value;
	
		key = "LoadingText[" $ i $ "]";
		value = Localize( "LevelLoadingInfo", key, File, True );
		LevelLoadingInfo[i].LoadingText = value;
		
		key = "LoadingShowHints[" $ i $ "]";
		value = Localize( "LevelLoadingInfo", key, File, True );
		if ( int(value) != 0 )
			LevelLoadingInfo[i].LoadingShowHints = True;
		else
			LevelLoadingInfo[i].LoadingShowHints = False;
	
		++i;
		
	} until( !bGotValue );
	
	UnloadInts( File );
}

simulated event PotentialSubtitledSoundPlayed(string SoundName, float Duration, Actor A, int Priority)
{
	local PlayerController PC;
	
	ForEach A.DynamicActors(class'PlayerController', PC)
	{
		if ( Viewport(PC.Player) != None )
		{
			PC.PotentialSubtitledSoundPlayed(SoundName, Duration, Priority);
			break;
		}
	}
}

simulated event int GetLevelProgressIdx( String Level )
{
	local int i;

	// Strip any extension off the level name
	if ( Caps(Right(Level, 4)) == ".CTM" )
		Level = Left(Level, Len(Level) - 4);
		
	for ( i = 0; i < NUM_PROGRESS_LEVELS; ++i )
	{
		if ( Caps(Level) == Caps(ProgressLevels[i]) )
		{
			return i;
		}
	}
	
	return -1;
}

simulated event bool HasReachedLevel( String Level, int CurrentProgress )
{
	local int i;

	// Strip any extension off the level name
	if ( Caps(Right(Level, 4)) == ".CTM" )
		Level = Left(Level, Len(Level) - 4);
	
	for ( i = 0; i < NUM_PROGRESS_LEVELS; ++i )
	{
		if ( Caps(Level) == Caps(ProgressLevels[i]) )
		{
			if ( i <= CurrentProgress )
			{
				// We've made it this far
				return True;			
			}
			else
			{
				// Haven't gotten this far yet
				return False;
			}
		}
	}
	
	return False;
}

simulated event string GetNextLevel( int afterThisIdx )
{
	if (afterThisIdx < NUM_PROGRESS_LEVELS - 1)
	{
		return ProgressLevels[afterThisIdx + 1];
	}
	return "";
}


simulated event string GetLevelMissionStart( string Level )
{
	local int i;

	// Strip any extension off the level name
	if ( Caps(Right(Level, 4)) == ".CTM" )
		Level = Left(Level, Len(Level) - 4);
	
	for ( i = 0; i < NUM_PROGRESS_LEVELS; ++i )
	{
		if ( Caps(Level) == Caps(ProgressLevels[i]) )
		{
			return LevelMissionStartLevels[i];
		}
	}
	
	return Level;
}


defaultproperties
{
     ServerActors(0)="IpDrv.MasterServerUplink"
     ServerPackages(0)="Core"
     ServerPackages(1)="Engine"
     ServerPackages(2)="CTGame"
     ServerPackages(3)="MPGame"
     ServerPackages(4)="CTInventory"
     ServerPackages(5)="CTCharacters"
     ServerPackages(6)="Properties"
     ServerPackages(7)="VoicePacks"
     bUseXInterface=True
     InitialMenuClass="XInterfaceCTMenus.CTStartPCMenu"
     PauseMenuClass="XInterfaceCTMenus.CTPausePCMenu"
     LoadingMenuClass="XInterfaceCTMenus.CTLoading"
     LoadingInfoMenuClass="XInterfaceCTMenus.CTLoadingInfo"
     TeamMenuClass="XInterfaceCommon.MenuSelectTeam"
     MultiplayerPauseMenuClass="XInterfaceCTMenus.CTMultiplayerPausePCMenu"
     AttractModeMovie="e3_2004_trailer_av1400.xmv"
     ProfilePrefix="Profile"
     DeathMatchStr="DEATHMATCH"
     TeamDeathMatchStr="TEAM DEATHMATCH"
     CaptureTheFlagStr="CAPTURE THE FLAG"
     AssaultStr="ASSAULT"
     LocalizedKeyNames(1)="L MOUSE"
     LocalizedKeyNames(2)="R MOUSE"
     LocalizedKeyNames(3)="CANCEL"
     LocalizedKeyNames(4)="M MOUSE"
     LocalizedKeyNames(5)="UNKNOWN05"
     LocalizedKeyNames(6)="UNKNOWN06"
     LocalizedKeyNames(7)="UNKNOWN07"
     LocalizedKeyNames(8)="BACKSP"
     LocalizedKeyNames(9)="TAB"
     LocalizedKeyNames(10)="UNKNOWN0A"
     LocalizedKeyNames(11)="UNKNOWN0B"
     LocalizedKeyNames(12)="UNKNOWN0C"
     LocalizedKeyNames(13)="ENTER"
     LocalizedKeyNames(14)="UNKNOWN0E"
     LocalizedKeyNames(15)="UNKNOWN0F"
     LocalizedKeyNames(16)="SHIFT"
     LocalizedKeyNames(17)="CTRL"
     LocalizedKeyNames(18)="ALT"
     LocalizedKeyNames(19)="PAUSE"
     LocalizedKeyNames(20)="CAPSLOCK"
     LocalizedKeyNames(21)="UNKNOWN15"
     LocalizedKeyNames(22)="UNKNOWN16"
     LocalizedKeyNames(23)="UNKNOWN17"
     LocalizedKeyNames(24)="UNKNOWN18"
     LocalizedKeyNames(25)="UNKNOWN19"
     LocalizedKeyNames(26)="UNKNOWN1A"
     LocalizedKeyNames(27)="ESCAPE"
     LocalizedKeyNames(28)="UNKNOWN1C"
     LocalizedKeyNames(29)="UNKNOWN1D"
     LocalizedKeyNames(30)="UNKNOWN1E"
     LocalizedKeyNames(31)="UNKNOWN1F"
     LocalizedKeyNames(32)="SPACE"
     LocalizedKeyNames(33)="PAGE UP"
     LocalizedKeyNames(34)="PAGE DWN"
     LocalizedKeyNames(35)="END"
     LocalizedKeyNames(36)="HOME"
     LocalizedKeyNames(37)="LEFT"
     LocalizedKeyNames(38)="UP"
     LocalizedKeyNames(39)="RIGHT"
     LocalizedKeyNames(40)="DOWN"
     LocalizedKeyNames(41)="SELECT"
     LocalizedKeyNames(42)="PRINT"
     LocalizedKeyNames(43)="EXECUTE"
     LocalizedKeyNames(44)="PRNTSCRN"
     LocalizedKeyNames(45)="INSERT"
     LocalizedKeyNames(46)="DELETE"
     LocalizedKeyNames(47)="HELP"
     LocalizedKeyNames(48)="0"
     LocalizedKeyNames(49)="1"
     LocalizedKeyNames(50)="2"
     LocalizedKeyNames(51)="3"
     LocalizedKeyNames(52)="4"
     LocalizedKeyNames(53)="5"
     LocalizedKeyNames(54)="6"
     LocalizedKeyNames(55)="7"
     LocalizedKeyNames(56)="8"
     LocalizedKeyNames(57)="9"
     LocalizedKeyNames(58)="UNKNOWN3A"
     LocalizedKeyNames(59)="UNKNOWN3B"
     LocalizedKeyNames(60)="UNKNOWN3C"
     LocalizedKeyNames(61)="UNKNOWN3D"
     LocalizedKeyNames(62)="UNKNOWN3E"
     LocalizedKeyNames(63)="UNKNOWN3F"
     LocalizedKeyNames(64)="UNKNOWN40"
     LocalizedKeyNames(65)="A"
     LocalizedKeyNames(66)="B"
     LocalizedKeyNames(67)="C"
     LocalizedKeyNames(68)="D"
     LocalizedKeyNames(69)="E"
     LocalizedKeyNames(70)="F"
     LocalizedKeyNames(71)="G"
     LocalizedKeyNames(72)="H"
     LocalizedKeyNames(73)="I"
     LocalizedKeyNames(74)="J"
     LocalizedKeyNames(75)="K"
     LocalizedKeyNames(76)="L"
     LocalizedKeyNames(77)="M"
     LocalizedKeyNames(78)="N"
     LocalizedKeyNames(79)="O"
     LocalizedKeyNames(80)="P"
     LocalizedKeyNames(81)="Q"
     LocalizedKeyNames(82)="R"
     LocalizedKeyNames(83)="S"
     LocalizedKeyNames(84)="T"
     LocalizedKeyNames(85)="U"
     LocalizedKeyNames(86)="V"
     LocalizedKeyNames(87)="W"
     LocalizedKeyNames(88)="X"
     LocalizedKeyNames(89)="Y"
     LocalizedKeyNames(90)="Z"
     LocalizedKeyNames(91)="UNKNOWN5B"
     LocalizedKeyNames(92)="UNKNOWN5C"
     LocalizedKeyNames(93)="UNKNOWN5D"
     LocalizedKeyNames(94)="UNKNOWN5E"
     LocalizedKeyNames(95)="UNKNOWN5F"
     LocalizedKeyNames(96)="PAD 0"
     LocalizedKeyNames(97)="PAD 1"
     LocalizedKeyNames(98)="PAD 2"
     LocalizedKeyNames(99)="PAD 3"
     LocalizedKeyNames(100)="PAD 4"
     LocalizedKeyNames(101)="PAD 5"
     LocalizedKeyNames(102)="PAD 6"
     LocalizedKeyNames(103)="PAD 7"
     LocalizedKeyNames(104)="PAD 8"
     LocalizedKeyNames(105)="PAD 9"
     LocalizedKeyNames(106)="PAD *"
     LocalizedKeyNames(107)="PAD +"
     LocalizedKeyNames(108)="SEPARATOR"
     LocalizedKeyNames(109)="PAD -"
     LocalizedKeyNames(110)="PAD ."
     LocalizedKeyNames(111)="PAD /"
     LocalizedKeyNames(112)="F1"
     LocalizedKeyNames(113)="F2"
     LocalizedKeyNames(114)="F3"
     LocalizedKeyNames(115)="F4"
     LocalizedKeyNames(116)="F5"
     LocalizedKeyNames(117)="F6"
     LocalizedKeyNames(118)="F7"
     LocalizedKeyNames(119)="F8"
     LocalizedKeyNames(120)="F9"
     LocalizedKeyNames(121)="F10"
     LocalizedKeyNames(122)="F11"
     LocalizedKeyNames(123)="F12"
     LocalizedKeyNames(124)="F13"
     LocalizedKeyNames(125)="F14"
     LocalizedKeyNames(126)="F15"
     LocalizedKeyNames(127)="F16"
     LocalizedKeyNames(128)="F17"
     LocalizedKeyNames(129)="F18"
     LocalizedKeyNames(130)="F19"
     LocalizedKeyNames(131)="F20"
     LocalizedKeyNames(132)="F21"
     LocalizedKeyNames(133)="F22"
     LocalizedKeyNames(134)="F23"
     LocalizedKeyNames(135)="F24"
     LocalizedKeyNames(136)="UNKNOWN88"
     LocalizedKeyNames(137)="UNKNOWN89"
     LocalizedKeyNames(138)="UNKNOWN8A"
     LocalizedKeyNames(139)="UNKNOWN8B"
     LocalizedKeyNames(140)="UNKNOWN8C"
     LocalizedKeyNames(141)="UNKNOWN8D"
     LocalizedKeyNames(142)="UNKNOWN8E"
     LocalizedKeyNames(143)="UNKNOWN8F"
     LocalizedKeyNames(144)="NUMLOCK"
     LocalizedKeyNames(145)="SCRLLOCK"
     LocalizedKeyNames(146)="UNKNOWN92"
     LocalizedKeyNames(147)="UNKNOWN93"
     LocalizedKeyNames(148)="UNKNOWN94"
     LocalizedKeyNames(149)="UNKNOWN95"
     LocalizedKeyNames(150)="UNKNOWN96"
     LocalizedKeyNames(151)="UNKNOWN97"
     LocalizedKeyNames(152)="UNKNOWN98"
     LocalizedKeyNames(153)="UNKNOWN99"
     LocalizedKeyNames(154)="UNKNOWN9A"
     LocalizedKeyNames(155)="UNKNOWN9B"
     LocalizedKeyNames(156)="UNKNOWN9C"
     LocalizedKeyNames(157)="UNKNOWN9D"
     LocalizedKeyNames(158)="UNKNOWN9E"
     LocalizedKeyNames(159)="UNKNOWN9F"
     LocalizedKeyNames(160)="L SHIFT"
     LocalizedKeyNames(161)="R SHIFT"
     LocalizedKeyNames(162)="L CNTRL"
     LocalizedKeyNames(163)="R CNTRL"
     LocalizedKeyNames(164)="UNKNOWNA4"
     LocalizedKeyNames(165)="UNKNOWNA5"
     LocalizedKeyNames(166)="UNKNOWNA6"
     LocalizedKeyNames(167)="UNKNOWNA7"
     LocalizedKeyNames(168)="UNKNOWNA8"
     LocalizedKeyNames(169)="UNKNOWNA9"
     LocalizedKeyNames(170)="UNKNOWNAA"
     LocalizedKeyNames(171)="UNKNOWNAB"
     LocalizedKeyNames(172)="UNKNOWNAC"
     LocalizedKeyNames(173)="UNKNOWNAD"
     LocalizedKeyNames(174)="UNKNOWNAE"
     LocalizedKeyNames(175)="UNKNOWNAF"
     LocalizedKeyNames(176)="UNKNOWNB0"
     LocalizedKeyNames(177)="UNKNOWNB1"
     LocalizedKeyNames(178)="UNKNOWNB2"
     LocalizedKeyNames(179)="UNKNOWNB3"
     LocalizedKeyNames(180)="UNKNOWNB4"
     LocalizedKeyNames(181)="UNKNOWNB5"
     LocalizedKeyNames(182)="UNKNOWNB6"
     LocalizedKeyNames(183)="UNKNOWNB7"
     LocalizedKeyNames(184)="UNKNOWNB8"
     LocalizedKeyNames(185)="UNKNOWNB9"
     LocalizedKeyNames(186)=";"
     LocalizedKeyNames(187)="="
     LocalizedKeyNames(188)=","
     LocalizedKeyNames(189)="-"
     LocalizedKeyNames(190)="."
     LocalizedKeyNames(191)="/"
     LocalizedKeyNames(192)="~"
     LocalizedKeyNames(193)="UNKNOWNC1"
     LocalizedKeyNames(194)="UNKNOWNC2"
     LocalizedKeyNames(195)="UNKNOWNC3"
     LocalizedKeyNames(196)="UNKNOWNC4"
     LocalizedKeyNames(197)="UNKNOWNC5"
     LocalizedKeyNames(198)="UNKNOWNC6"
     LocalizedKeyNames(199)="UNKNOWNC7"
     LocalizedKeyNames(200)="JOY1"
     LocalizedKeyNames(201)="JOY2"
     LocalizedKeyNames(202)="JOY3"
     LocalizedKeyNames(203)="JOY4"
     LocalizedKeyNames(204)="JOY5"
     LocalizedKeyNames(205)="JOY6"
     LocalizedKeyNames(206)="JOY7"
     LocalizedKeyNames(207)="JOY8"
     LocalizedKeyNames(208)="JOY9"
     LocalizedKeyNames(209)="JOY10"
     LocalizedKeyNames(210)="JOY11"
     LocalizedKeyNames(211)="JOY12"
     LocalizedKeyNames(212)="JOY13"
     LocalizedKeyNames(213)="JOY14"
     LocalizedKeyNames(214)="JOY15"
     LocalizedKeyNames(215)="JOY16"
     LocalizedKeyNames(216)="UNKNOWND8"
     LocalizedKeyNames(217)="UNKNOWND9"
     LocalizedKeyNames(218)="UNKNOWNDA"
     LocalizedKeyNames(219)="["
     LocalizedKeyNames(221)="]"
     LocalizedKeyNames(222)="'"
     LocalizedKeyNames(223)="UNKNOWNDF"
     LocalizedKeyNames(224)="JOYX"
     LocalizedKeyNames(225)="JOYY"
     LocalizedKeyNames(226)="JOYZ"
     LocalizedKeyNames(227)="JOYR"
     LocalizedKeyNames(228)="MOUSEX"
     LocalizedKeyNames(229)="MOUSEY"
     LocalizedKeyNames(230)="MOUSEZ"
     LocalizedKeyNames(231)="MOUSEW"
     LocalizedKeyNames(232)="JOYU"
     LocalizedKeyNames(233)="JOYV"
     LocalizedKeyNames(234)="UNKNOWNEA"
     LocalizedKeyNames(235)="UNKNOWNEB"
     LocalizedKeyNames(236)="M WHEEL+"
     LocalizedKeyNames(237)="M WHEEL-"
     LocalizedKeyNames(238)="UNKNOWN10E"
     LocalizedKeyNames(239)="UNKNOWN10F"
     LocalizedKeyNames(240)="PAD UP"
     LocalizedKeyNames(241)="PAD DOWN"
     LocalizedKeyNames(242)="PAD LEFT"
     LocalizedKeyNames(243)="PAD RIGHT"
     LocalizedKeyNames(244)="UNKNOWNF4"
     LocalizedKeyNames(245)="UNKNOWNF5"
     LocalizedKeyNames(246)="ATTN"
     LocalizedKeyNames(247)="CRSEL"
     LocalizedKeyNames(248)="EXSEL"
     LocalizedKeyNames(249)="EREOF"
     LocalizedKeyNames(250)="PLAY"
     LocalizedKeyNames(251)="ZOOM"
     LocalizedKeyNames(252)="NONAME"
     LocalizedKeyNames(253)="PA1"
     LocalizedKeyNames(254)="OEMCLEAR"
     LocalizedXboxButtonNames(0)="A"
     LocalizedXboxButtonNames(1)="B"
     LocalizedXboxButtonNames(2)="X"
     LocalizedXboxButtonNames(3)="Y"
     LocalizedXboxButtonNames(4)="BLACK"
     LocalizedXboxButtonNames(5)="WHITE"
     LocalizedXboxButtonNames(6)="LEFT TRIGGER"
     LocalizedXboxButtonNames(7)="RIGHT TRIGGER"
     LocalizedXboxButtonNames(8)="D-PAD UP"
     LocalizedXboxButtonNames(9)="D-PAD DOWN"
     LocalizedXboxButtonNames(10)="D-PAD LEFT"
     LocalizedXboxButtonNames(11)="D-PAD RIGHT"
     LocalizedXboxButtonNames(12)="START"
     LocalizedXboxButtonNames(13)="BACK"
     LocalizedXboxButtonNames(14)="LEFT THUMBSTICK"
     LocalizedXboxButtonNames(15)="RIGHT THUMBSTICK"
     ProgressLevels(0)="PRO"
     ProgressLevels(1)="GEO_01Briefing"
     ProgressLevels(2)="GEO_01A"
     ProgressLevels(3)="GEO_01B"
     ProgressLevels(4)="GEO_01C"
     ProgressLevels(5)="GEO_03A"
     ProgressLevels(6)="GEO_03C"
     ProgressLevels(7)="GEO_03D"
     ProgressLevels(8)="GEO_04A"
     ProgressLevels(9)="GEO_04B"
     ProgressLevels(10)="GEO_04C"
     ProgressLevels(11)="GEO_04D"
     ProgressLevels(12)="GEO_05A"
     ProgressLevels(13)="GEO_05B"
     ProgressLevels(14)="GEO_05C"
     ProgressLevels(15)="RAS_01Briefing"
     ProgressLevels(16)="RAS_01A"
     ProgressLevels(17)="RAS_01B"
     ProgressLevels(18)="RAS_01C"
     ProgressLevels(19)="RAS_02A"
     ProgressLevels(20)="RAS_02B"
     ProgressLevels(21)="RAS_02C"
     ProgressLevels(22)="RAS_02D"
     ProgressLevels(23)="RAS_02E"
     ProgressLevels(24)="RAS_03A"
     ProgressLevels(25)="RAS_03B"
     ProgressLevels(26)="RAS_03C"
     ProgressLevels(27)="RAS_04A"
     ProgressLevels(28)="RAS_04B"
     ProgressLevels(29)="RAS_04C"
     ProgressLevels(30)="RAS_04D"
     ProgressLevels(31)="YYY_01Briefing"
     ProgressLevels(32)="YYY_01B"
     ProgressLevels(33)="YYY_01C"
     ProgressLevels(34)="YYY_01D"
     ProgressLevels(35)="YYY_01E"
     ProgressLevels(36)="YYY_35A"
     ProgressLevels(37)="YYY_35B"
     ProgressLevels(38)="YYY_35C"
     ProgressLevels(39)="YYY_04A"
     ProgressLevels(40)="YYY_04B"
     ProgressLevels(41)="YYY_04C"
     ProgressLevels(42)="YYY_04E"
     ProgressLevels(43)="YYY_04F"
     ProgressLevels(44)="YYY_05A"
     ProgressLevels(45)="YYY_05B"
     ProgressLevels(46)="YYY_05C"
     ProgressLevels(47)="YYY_05D"
     ProgressLevels(48)="YYY_05E"
     ProgressLevels(49)="YYY_05F"
     ProgressLevels(50)="YYY_06A"
     ProgressLevels(51)="YYY_06B"
     ProgressLevels(52)="YYY_06C"
     ProgressLevels(53)="YYY07Briefing"
     ProgressLevels(54)="YYY_06D"
     ProgressLevels(55)="EPILOGUE"
     LevelMissionStartLevels(0)="PRO"
     LevelMissionStartLevels(1)="GEO_01Briefing"
     LevelMissionStartLevels(2)="GEO_01A"
     LevelMissionStartLevels(3)="GEO_01A"
     LevelMissionStartLevels(4)="GEO_01A"
     LevelMissionStartLevels(5)="GEO_03A"
     LevelMissionStartLevels(6)="GEO_03A"
     LevelMissionStartLevels(7)="GEO_03D"
     LevelMissionStartLevels(8)="GEO_04A"
     LevelMissionStartLevels(9)="GEO_04A"
     LevelMissionStartLevels(10)="GEO_04A"
     LevelMissionStartLevels(11)="GEO_04A"
     LevelMissionStartLevels(12)="GEO_05A"
     LevelMissionStartLevels(13)="GEO_05A"
     LevelMissionStartLevels(14)="GEO_05A"
     LevelMissionStartLevels(15)="RAS_01Briefing"
     LevelMissionStartLevels(16)="RAS_01A"
     LevelMissionStartLevels(17)="RAS_01A"
     LevelMissionStartLevels(18)="RAS_01A"
     LevelMissionStartLevels(19)="RAS_02A"
     LevelMissionStartLevels(20)="RAS_02A"
     LevelMissionStartLevels(21)="RAS_02A"
     LevelMissionStartLevels(22)="RAS_02A"
     LevelMissionStartLevels(23)="RAS_02A"
     LevelMissionStartLevels(24)="RAS_03A"
     LevelMissionStartLevels(25)="RAS_03A"
     LevelMissionStartLevels(26)="RAS_03A"
     LevelMissionStartLevels(27)="RAS_04A"
     LevelMissionStartLevels(28)="RAS_04A"
     LevelMissionStartLevels(29)="RAS_04A"
     LevelMissionStartLevels(30)="RAS_04A"
     LevelMissionStartLevels(31)="YYY_01Briefing"
     LevelMissionStartLevels(32)="YYY_01B"
     LevelMissionStartLevels(33)="YYY_01B"
     LevelMissionStartLevels(34)="YYY_01B"
     LevelMissionStartLevels(35)="YYY_01B"
     LevelMissionStartLevels(36)="YYY_35A"
     LevelMissionStartLevels(37)="YYY_35A"
     LevelMissionStartLevels(38)="YYY_35A"
     LevelMissionStartLevels(39)="YYY_04A"
     LevelMissionStartLevels(40)="YYY_04A"
     LevelMissionStartLevels(41)="YYY_04A"
     LevelMissionStartLevels(42)="YYY_04A"
     LevelMissionStartLevels(43)="YYY_04A"
     LevelMissionStartLevels(44)="YYY_05A"
     LevelMissionStartLevels(45)="YYY_05A"
     LevelMissionStartLevels(46)="YYY_05A"
     LevelMissionStartLevels(47)="YYY_05A"
     LevelMissionStartLevels(48)="YYY_05A"
     LevelMissionStartLevels(49)="YYY_05A"
     LevelMissionStartLevels(50)="YYY_06A"
     LevelMissionStartLevels(51)="YYY_06A"
     LevelMissionStartLevels(52)="YYY_06A"
     LevelMissionStartLevels(53)="YYY07Briefing"
     LevelMissionStartLevels(54)="YYY_06D"
     LevelMissionStartLevels(55)="EPILOGUE"
     DiscReadError="THERE'S A PROBLEM WITH THE DISC YOU'RE USING. IT MAY BE DIRTY OR DAMAGED."
     AudioDevice=None
     Console=None
     DefaultPlayerMenu=None
     NetworkDevice=None
}

