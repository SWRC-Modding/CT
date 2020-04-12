//=============================================================================
// HUD: Superclass of the heads-up display.
//=============================================================================
class HUD extends Actor
	native
	config(user)
	transient;

//=============================================================================
// Variables.

#exec Texture Import File=Textures\Border.pcx

//#exec new TrueTypeFontFactory PACKAGE="Engine" Name=MediumFont FontName="Arial Bold" Height=16 AntiAlias=1 CharactersPerPage=128
//#exec new TrueTypeFontFactory PACKAGE="Engine" Name=SmallFont FontName="Verdana" Height=12 AntiAlias=1 CharactersPerPage=256

// Stock fonts.
var font SmallFont;          // Small system font.
var font MedFont;            // Medium system font.
var font BigFont;            // Big system font.
var font LargeFont;            // Largest system font.

var string HUDConfigWindowType;
var HUD nextHUD;	// list of huds which render to the canvas
var PlayerController PlayerOwner; // always the actual owner

var localized string ProgressFontName;
var Font ProgressFontFont;
var float ProgressFadeTime;
var Color MOTDColor, DefaultTextColor;

var ScoreBoard ScoreBoard;
var bool bHideHUD;
var bool	bShowScores;
var bool	bShowDebugInfo;				// if true, show properties of current ViewTarget
var bool	bHideCenterMessages;		// don't draw centered messages (screen center being used)
var bool    bBadConnectionAlert;	// display warning about bad connection
var config bool bMessageBeep;
var Material GameTypeMaterial;

var Material XNotificationFriendIcon;  // icon for xbox live notifications
var Material XNotificationInviteIcon;  // icon for xbox live notifications
var float	 XNotificationTimer; // when times out, takes off the notification icon.
var int LastNumInvites;  // saved last # invites
var int LastNumFriendReqs; // saved last # friend req


var globalconfig float HudCanvasScale;    // Specifies amount of screen-space to use (for TV's).

var localized string LoadingMessage;
var localized string SavingMessage;
var localized string ConnectingMessage;
var localized string PausedMessage;
var localized string PrecachingMessage;

// Added by Demiurge Studios (Movie)
var const transient Movie Movie;
var int MoviePosX;
var int MoviePosY;
var float TexMovieTop;
var float TexMovieLeft;
var float TexMovieBottom;
var float TexMovieRight;
var bool TexMovieTranslucent;
var MovieTexture TextureMovie;
// End Demiurge Studios (Movie)

var globalconfig bool bShowSubtitles;
var globalconfig bool bShowPromptText;
var bool bShowCurrentObjective;

var Color ConsoleColor;
var globalconfig int ConsoleMessageCount;
var globalconfig int CriticalMessageCount;
var globalconfig int CriticalMessageDisplayCount;
var globalconfig int ConsoleFontSize;
var globalconfig int MessageFontOffset;


struct native ConsoleMessage
{
	var string Text;
	var color TextColor;
	var float MessageLife;
	var PlayerReplicationInfo PRI;
};

var ConsoleMessage TextMessages[8];
var ConsoleMessage CriticalMessages[8];

var() float ConsoleMessagePosX, ConsoleMessagePosY; // DP_LowerLeft

var localized string FontArrayNames[9];
var Font FontArrayFonts[9];
var int FontScreenWidthMedium[9];
var int FontScreenWidthSmall[9];


// HUD Vars
var int					StaticAlpha;			// Static interfering with HUD
var bool				bScavangerHead;			// Scavanger is attached to helmet
var DynamicProjector	FlashlightProjector;	// The projector for flashlights
var Material			FlashlightTexture;		// The texture for the projector
var Actor				Markers[4];				// Record the visible markers
var Pickup				LastPickup;				// Last item picked up
var float				LastPickupTime;			// Last time item picked up

// HUD Sound Vars
var(Sound) Sound		TargetNormal;
var(Sound) Sound        TargetFriend;
var(Sound) Sound        TargetFriendInjured;
var(Sound) Sound        TargetFriendToHeal;
var(Sound) Sound        TargetMarker;
var(Sound) Sound        TargetMarkerCancel;
var(Sound) Sound        TargetPickup;
var(Sound) Sound        TargetEnemy;
var(Sound) Sound        TargetPanel;
var(Sound) Sound        HealingTarget;
var(Sound) Sound        TextPrint;
var(Sound) Sound		HealthRecharging;
var(Sound) Sound		HealthCharged;
var(Sound) Sound        ShieldRecharging;
var(Sound) Sound		ShieldCharged;
var(Sound) Sound        ActivateStanceDefense;
var(Sound) Sound        ActivateStanceHold;
var(Sound) Sound        ActivateStanceOffense;
var(Sound) Sound		StaticScavenger;
var(Sound) Sound		StaticIncapacitated;
var(Sound) Sound		FlashlightOn;
var(Sound) Sound		FlashlightOff;
var(Sound) Sound		ZoomIn;
var(Sound) Sound		ZoomOut;

var(Sound) Sound		IncapMenuChange;
var(Sound) Sound		IncapMenuSelect;

// Visor Effect Sounds
var(Sound) Sound		BloodSplatter;
var(Sound) Sound		Wiper;

// Weapon switching HUD sounds
var(Sound) Sound		SwitchToThermalGrenade;
var(Sound) Sound		SwitchToSonicGrenade;
var(Sound) Sound		SwitchToEMPGrenade;
var(Sound) Sound		SwitchToFlashBang;

// Sniper charging and charged sounds (only zoomed)
var(Sound) Sound		SniperZoomCharging;
var(Sound) Sound		SniperZoomCharged;

/* Draw3DLine()
draw line in world space. Should be used when engine calls RenderWorldOverlays() event.
*/
native final function Draw3DLine(vector Start, vector End, color LineColor);
static native event font GetConsoleFont(Canvas C);

simulated event PostLoadBeginPlay()
{
	Super.PostLoadBeginPlay();
	PlayerOwner = PlayerController(Owner);

	// Setup the flashlight
	// Note that the projectors starts at off, hence MaxTraceDistance = 0

	// NathanM: We're not using it, so why spawn it?
	//FlashlightProjector = spawn(class'DynamicProjector',self);
	if( FlashlightProjector != None )
	{
		FlashlightProjector.SetBase( self );
		FlashlightProjector.SetRelativeLocation(vect(25,0,0));
		FlashlightProjector.ProjTexture = None;
		FlashlightProjector.FOV = 7;
		FlashlightProjector.MaxTraceDistance = 10000;
		FlashlightProjector.FramebufferBlendingOp = PB_Add;
		FlashlightProjector.bProjectBSP = true;
		FlashlightProjector.bProjectStaticMesh = false;
		FlashlightProjector.bProjectTerrain = false;
		FlashlightProjector.bProjectActor = false;
		FlashlightProjector.bProjectOnBackfaces = false;
		FlashlightProjector.SetCollision(false,false,false);
	}
}

function SpawnScoreBoard(class<Scoreboard> ScoringType)
{
	if ( ScoringType != None )
		Scoreboard = Spawn(ScoringType, PlayerOwner);
}

simulated event Destroyed()
{
    if( ScoreBoard != None )
    {
        ScoreBoard.Destroy();
        ScoreBoard = None;
    }
	Super.Destroyed();
}


//=============================================================================
// Execs

/* toggles displaying scoreboard
*/
exec function ShowScores()
{
	bShowScores = !bShowScores;
}

/* toggles displaying properties of player's current viewtarget
*/
exec function ShowDebug()
{
	bShowDebugInfo = !bShowDebugInfo;
}

simulated event WorldSpaceOverlays()
{
}

event CheckCountdown(GameReplicationInfo GRI);

event ConnectFailure(string FailCode, string URL)
{
	PlayerOwner.ReceiveLocalizedMessage(class'FailedConnect', class'FailedConnect'.Static.GetFailSwitch(FailCode));
}
/* ShowUpgradeMenu()
Event called when the engine version is less than the MinNetVer of the server you are trying
to connect with.
*/
event ShowUpgradeMenu();

function PlayStartupMessage(byte Stage);

event AddCriticalMessage( string Message, float Duration, color TextColor )
{
	local int i, LastUsedSlot, NextSlot, MaxLen, SearchPos;
	local float FadeTime;
	local string WrappedText;

	// Find the first open slot
	NextSlot = 0;
	LastUsedSlot = -1;
    for( i = 0; i < CriticalMessageCount; i++ )
    {
        if ( CriticalMessages[i].Text == "" )
		{
			NextSlot = i;
            break;
		}
		LastUsedSlot = i;
    }

	if ( (LastUsedSlot >= 0) &&
		 (CriticalMessages[LastUsedSlot].Text == Message) )
	{
		// Same message as the last one...discard it.
		return;
	}

	// If the message is too big then split it into multiple lines
	MaxLen = 80;
	if( Len(Message) > MaxLen )
	{
		// find first space
		SearchPos = MaxLen;
		while( SearchPos > 0 )
		{
			if( Mid( Message, SearchPos, 1) == " " )
			{
				SearchPos = SearchPos + 1;
				break;
			}

			SearchPos = SearchPos - 1;
		}

		// If no space found then just wrap in middle of word
		if( SearchPos <= 0 )
			SearchPos = MaxLen;

		WrappedText = Right( Message, Len(Message) - SearchPos );
		Message = Left( Message, SearchPos );
	}

	// Add the message
	FadeTime=1;
    CriticalMessages[NextSlot].Text = Message;
    CriticalMessages[NextSlot].TextColor = TextColor;
    CriticalMessages[NextSlot].PRI = None;
    // Is this line displayed?
	if( NextSlot < CriticalMessageDisplayCount )
		CriticalMessages[NextSlot].MessageLife = Level.TimeSeconds + Duration + FadeTime;
	else
		// Just store the duration for later
		CriticalMessages[NextSlot].MessageLife = Duration;

	// Do wrapped lines
	if( WrappedText != "" )
		AddCriticalMessage( WrappedText, Duration, TextColor );
}

simulated function Tick( float DeltaTime )
{
	if (XNotificationTimer > 0)
	{
		XNotificationTimer = max(0,XNotificationTimer - DeltaTime);
		if (XNotificationTimer <= 0)
		{
			LastNumInvites = PlayerOwner.GetNumGameInvites();
			LastNumFriendReqs = PlayerOwner.GetNumFriendRequests();
		}
	}
}


//=============================================================================
// Status drawing.

simulated event PreRender( canvas Canvas )
{
}

simulated event DrawXLiveNotification( canvas C )
{
	local float X, Y;
	local int size;

	X = (0.9 * HudCanvasScale * C.SizeX);
	Y = (0.2 * HudCanvasScale * C.SizeY);

	size = 64; //0.05 * HudCanvasScale * C.SizeX;

	C.Style = ERenderStyle.STY_Alpha;
	C.SetPos(X, Y);

	if( PlayerOwner.GetNumGameInvites() > LastNumInvites)
		C.DrawTileScaled( XNotificationInviteIcon, 1.0, 1.0);
	else if( PlayerOwner.GetNumFriendRequests() > LastNumFriendReqs )
		C.DrawTileScaled( XNotificationFriendIcon, 1.0, 1.0);
	else
		XNotificationTimer = 0;	// uh? nothing to draw? reset timer then
}

simulated event PostRender( canvas Canvas )
{
	local float XPos,YPos;
	local Pawn P;

	CheckCountDown(PlayerOwner.GameReplicationInfo);

	P = Pawn(PlayerOwner.ViewTarget);
	if ( !PlayerOwner.bBehindView )
	{
		if ( (P != None) && (P.Weapon != None) )
		{
			P.Weapon.RenderOverlays(Canvas);
		}
	}

	// xbox live notifications, if any
	if( PlayerOwner.GetNumGameInvites() > 0 || PlayerOwner.GetNumFriendRequests() > 0 )
	{
		if ( (XNotificationTimer <= 0) &&
			 ((PlayerOwner.GetNumGameInvites() > LastNumInvites) ||
			  (PlayerOwner.GetNumFriendRequests() > LastNumFriendReqs)) )
		{
			XNotificationTimer = 300;  // 3 seconds for notification display, as per current xlive TCR
		}
	}

	if (XNotificationTimer > 0)
		DrawXLiveNotification( Canvas );

	bHideCenterMessages = DrawLevelAction(Canvas);

	if ( !bHideCenterMessages && (PlayerOwner.ProgressTimeOut > Level.TimeSeconds) )
		DisplayProgressMessage(Canvas);

	if ( bBadConnectionAlert )
		DisplayBadConnectionAlert();

	if ( bShowDebugInfo )
    {
        Canvas.Font = GetConsoleFont(Canvas);
        Canvas.Style = ERenderStyle.STY_Alpha;
        Canvas.DrawColor = ConsoleColor;

        PlayerOwner.ViewTarget.DisplayDebug (Canvas, XPos, YPos);
    }
	else if( !bHideHud )
    {

		if( bShowScores )
		{
			if(ScoreBoard != None)
			{
				ScoreBoard.DrawScoreboard(Canvas);
				if ( Scoreboard.bDisplayMessages )
					DisplayMessages(Canvas);
			}
		}
		else
		{
			if ( (PlayerOwner == None) || (P == None) || (P.PlayerReplicationInfo == None) || PlayerOwner.IsSpectating() )
				DrawSpectatingHud(Canvas);
			else if( !P.bHideRegularHUD )
				DrawHud(Canvas);

			if ( !DrawLevelAction(Canvas) )
			{
				if ((PlayerOwner != None) && (PlayerOwner.ProgressTimeOut > Level.TimeSeconds))
					DisplayProgressMessage(Canvas);
			}
			DisplayMessages(Canvas);
		}
    }

    PlayerOwner.RenderOverlays (Canvas);

	// USED TO BE POST RENDER MENUS HERE

	if(TextureMovie != NONE && TextureMovie.Movie != NONE && TextureMovie.Movie.IsPlaying())
	{
		if(TexMovieTranslucent)
			canvas.Style = ERenderStyle.STY_Translucent;
		else
			canvas.Style = ERenderStyle.STY_Normal;
		canvas.SetDrawColor(255,255,255);
		canvas.SetPos( TexMovieLeft*canvas.SizeX, TexMovieTop*canvas.SizeY );
		canvas.DrawTile(TextureMovie, (TexMovieRight-TexMovieLeft)*canvas.SizeX, (TexMovieBottom-TexMovieTop)*canvas.SizeY, 0, 0, TextureMovie.Movie.GetWidth(), TextureMovie.Movie.GetHeight());
	}
}

simulated event PostRenderMenus ( canvas Canvas )
{
	local Console PlayerConsole;

	 // O_o  [uncommented] because it handles the fading in/out of menus, and without it, the xbox live menus fail to draw cuz the "previous" menu never fades out.
	// O_o  brought in from UC, for XInterface menu
	 if (class'GameEngine'.default.bUseXInterface)
	 {
		 if (PlayerOwner.Player != None)
		    PlayerConsole = PlayerOwner.Player.Console;
       else
		    PlayerConsole = None;

		 if (PlayerConsole != None)
		 {
			 PlayerConsole.MenuRender (Canvas);
		 }
	 }
}


function DrawSpectatingHud (Canvas C);


/* DrawHUD() Draw HUD elements on canvas.
*/
function DrawHUD(canvas Canvas);

/* Display Progress Messages
display progress messages in center of screen
*/
simulated function DisplayProgressMessage(Canvas C)
{
    local int i, LineCount;
    local GameReplicationInfo GRI;
    local float FontDX, FontDY;
    local float X, Y;
    local int Alpha;
    local float TimeLeft;

	// ****  SBD - We don't want these messages on the X-Box
	if ( IsOnConsole() )
		return;
	// ****

	//*** SBD - In fact, let's not display these messages at all
	return;
	//***

    TimeLeft = PlayerOwner.ProgressTimeOut - Level.TimeSeconds;

    if( TimeLeft >= ProgressFadeTime )
        Alpha = 255;
    else
        Alpha = (255 * TimeLeft) / ProgressFadeTime;

    GRI = PlayerOwner.GameReplicationInfo;

    LineCount = 0;

    for (i = 0; i < ArrayCount (PlayerOwner.ProgressMessage); i++)
    {
		//** SBD - Only show the first entry
		if ( i > 0 )
			continue;
		//**

        if (PlayerOwner.ProgressMessage[i] == "")
            continue;

        LineCount++;
    }

    if (GRI != None)
    {
        if( GRI.MOTDLine1 != "" ) LineCount++;
        if( GRI.MOTDLine2 != "" ) LineCount++;
        if( GRI.MOTDLine3 != "" ) LineCount++;
        if( GRI.MOTDLine4 != "" ) LineCount++;
    }

    C.Font = LoadProgressFont();

    C.Style = ERenderStyle.STY_Alpha;

    C.TextSize ("A", FontDX, FontDY);

    X = (0.5 * HudCanvasScale * C.SizeX) + (((1.0 - HudCanvasScale) / 2.0) * C.SizeX);
    Y = (0.5 * HudCanvasScale * C.SizeY) + (((1.0 - HudCanvasScale) / 2.0) * C.SizeY);

    Y -= FontDY * (float (LineCount) / 2.0);

    for (i = 0; i < ArrayCount (PlayerOwner.ProgressMessage); i++)
    {
		//** SBD - Only show the first entry
		if ( i > 0 )
			continue;
		//**

        if (PlayerOwner.ProgressMessage[i] == "")
            continue;

        C.DrawColor = PlayerOwner.ProgressColor[i];
        C.DrawColor.A = Alpha;

        C.TextSize (PlayerOwner.ProgressMessage[i], FontDX, FontDY);
        C.SetPos (X - (FontDX / 2.0), Y);
        C.DrawText (PlayerOwner.ProgressMessage[i]);

        Y += FontDY;
    }

    if( (GRI != None) && (Level.NetMode != NM_StandAlone) )
    {
        C.DrawColor = MOTDColor;
        C.DrawColor.A = Alpha;

        if( GRI.MOTDLine1 != "" )
        {
            C.TextSize (GRI.MOTDLine1, FontDX, FontDY);
            C.SetPos (X - (FontDX / 2.0), Y);
            C.DrawText (GRI.MOTDLine1);
            Y += FontDY;
        }

        if( GRI.MOTDLine2 != "" )
        {
            C.TextSize (GRI.MOTDLine2, FontDX, FontDY);
            C.SetPos (X - (FontDX / 2.0), Y);
            C.DrawText (GRI.MOTDLine2);
            Y += FontDY;
        }

        if( GRI.MOTDLine3 != "" )
        {
            C.TextSize (GRI.MOTDLine3, FontDX, FontDY);
            C.SetPos (X - (FontDX / 2.0), Y);
            C.DrawText (GRI.MOTDLine3);
            Y += FontDY;
        }

        if( GRI.MOTDLine4 != "" )
        {
            C.TextSize (GRI.MOTDLine4, FontDX, FontDY);
            C.SetPos (X - (FontDX / 2.0), Y);
            C.DrawText (GRI.MOTDLine4);
            Y += FontDY;
        }
    }
}


/* Draw the Level Action
*/
native event bool DrawLevelAction( canvas C );

/* DisplayBadConnectionAlert()
Warn user that net connection is bad
*/
function DisplayBadConnectionAlert();

// Added by Demiurge Studios (Movie)
function PlayMovieDirect(String MovieFilename, int XPos, int YPos, bool UseSound, bool LoopMovie)
{
	StopMovie();

	MoviePosX = XPos;
	MoviePosY = YPos;
	Movie.Play(MovieFilename, UseSound, LoopMovie);
}


function PlayMovieScaled(MovieTexture InMovie, float Left, float Top, float Right, float Bottom, bool UseSound, bool LoopMovie, optional bool Translucent)
{
	StopMovie();

	if(Top < 0)
		Top = 0;
	if(Top > 1)
		Top = 1;

	if(Left < 0)
		Left = 0;
	if(Left > 1)
		Left = 1;

	if(Bottom < 0)
		Bottom = 0;
	if(Bottom > 1)
		Bottom = 1;

	if(Right < 0)
		Right = 0;
	if(Right > 1)
		Right = 1;

	TexMovieTop = Top;
	TexMovieLeft = Left;
	TexMovieBottom = Bottom;
	TexMovieRight = Right;
	TexMovieTranslucent = Translucent;

	TextureMovie = InMovie;
	TextureMovie.InitializeMovie();
	TextureMovie.Movie.Play(TextureMovie.MovieFilename, UseSound, LoopMovie);
}


function bool IsMoviePlaying()
{
	return Movie.IsPlaying() || TextureMovie.Movie.IsPlaying();
}


function PauseMovie(bool Pause)
{
	Movie.Pause(Pause);
	TextureMovie.Movie.Pause(Pause);
}


function bool IsMoviePaused()
{
	if(Movie.IsPlaying())
		return Movie.IsPaused();

	if(TextureMovie.Movie.IsPlaying())
		return TextureMovie.Movie.IsPaused();

	return false;
}


function StopMovie()
{
	Movie.StopNow();
	TextureMovie.Movie.StopNow();
}
// End Demiurge Studios (Movie)

//=============================================================================
// Messaging.

simulated function Message( PlayerReplicationInfo PRI, coerce string Msg, name MsgType )
{
	if ( bMessageBeep )
		PlayerOwner.PlayBeepSound();
	if ( (MsgType == 'Say') || (MsgType == 'TeamSay') )
		Msg = PRI.GetPlayerName()$": "$Msg;

	// TimR: Try putting this in the new HUD only
	//AddTextMessage(Msg,class'LocalMessage',PRI);
	AddCriticalMessage(Msg, 3, class'Canvas'.Static.MakeColor(200,200,200) );
}

function DisplayPortrait(PlayerReplicationInfo PRI);

simulated function LocalizedMessage( class<LocalMessage> Message, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject, optional string CriticalString );

simulated function PlayReceivedMessage( string S, string PName, ZoneInfo PZone )
{
	PlayerOwner.ClientMessage(S);
	if ( bMessageBeep )
		PlayerOwner.PlayBeepSound();
}

function bool ProcessKeyEvent( int Key, int Action, FLOAT Delta )
{
	if ( NextHud != None )
		return NextHud.ProcessKeyEvent(Key,Action,Delta);
	return false;
}

/* DisplayMessages() - display current messages
*/
function DisplayMessages(Canvas C)
{
    local int i, j, XPos, YPos,MessageCount;
    local float XL, YL;

	C.Flush();
	C.Reset();

    for( i = 0; i < ConsoleMessageCount; i++ )
    {
        if ( TextMessages[i].Text == "" )
            break;
        else if( TextMessages[i].MessageLife < Level.TimeSeconds )
        {
            TextMessages[i].Text = "";

            if( i < ConsoleMessageCount - 1 )
            {
                for( j=i; j<ConsoleMessageCount-1; j++ )
                    TextMessages[j] = TextMessages[j+1];
            }
            TextMessages[j].Text = "";
            break;
        }
        else
			MessageCount++;
    }

    XPos = (ConsoleMessagePosX * HudCanvasScale * C.SizeX) + (((1.0 - HudCanvasScale) / 2.0) * C.SizeX);
    YPos = (ConsoleMessagePosY * HudCanvasScale * C.SizeY) + (((1.0 - HudCanvasScale) / 2.0) * C.SizeY);
	if(IsOnConsole())
		XPos += 20;

	// Set font sytle
	C.Style = ERenderStyle.STY_Alpha;
    C.Font = GetConsoleFont(C);
    C.DrawColor = ConsoleColor;

    C.TextSize ("A", XL, YL);

    YPos -= YL * MessageCount+1; // DP_LowerLeft
    YPos -= YL; // Room for typing prompt

    for( i=0; i<MessageCount; i++ )
    {
        if ( TextMessages[i].Text == "" )
            break;

        C.StrLen( TextMessages[i].Text, XL, YL );
        C.SetPos( XPos, YPos );
        C.DrawColor = TextMessages[i].TextColor;
        C.DrawText( TextMessages[i].Text, false );
        YPos += YL;
    }
	C.Flush();
	C.Reset();
}

function AddTextMessage(string M, class<LocalMessage> MessageClass, PlayerReplicationInfo PRI)
{
	local int i;

	if( bMessageBeep && MessageClass.Default.bBeep )
		PlayerOwner.PlayBeepSound();

    for( i=0; i<ConsoleMessageCount; i++ )
    {
        if ( TextMessages[i].Text == "" )
            break;
    }

    if( i == ConsoleMessageCount )
    {
        for( i=0; i<ConsoleMessageCount-1; i++ )
            TextMessages[i] = TextMessages[i+1];
    }

    TextMessages[i].Text = M;
    TextMessages[i].MessageLife = Level.TimeSeconds + MessageClass.Default.LifeTime;
    TextMessages[i].TextColor = MessageClass.static.GetConsoleColor(PRI);
    TextMessages[i].PRI = PRI;
}

//=============================================================================
// Font Selection.

function UseSmallFont(Canvas Canvas)
{
	if ( Canvas.ClipX <= 640 )
		Canvas.Font = SmallFont;
	else
		Canvas.Font = MedFont;
}

function UseMediumFont(Canvas Canvas)
{
	if ( Canvas.ClipX <= 640 )
		Canvas.Font = MedFont;
	else
		Canvas.Font = BigFont;
}

function UseLargeFont(Canvas Canvas)
{
	if ( Canvas.ClipX <= 640 )
		Canvas.Font = BigFont;
	else
		Canvas.Font = LargeFont;
}

function UseHugeFont(Canvas Canvas)
{
	Canvas.Font = LargeFont;
}

static event Font LoadFontStatic(int i)
{
	if( default.FontArrayFonts[i] == None )
	{
		default.FontArrayFonts[i] = Font(DynamicLoadObject(default.FontArrayNames[i], class'Font'));
		if( default.FontArrayFonts[i] == None )
			Log("Warning: "$default.Class$" Couldn't dynamically load font "$default.FontArrayNames[i]);
	}

	return default.FontArrayFonts[i];
}

simulated function Font LoadFont(int i)
{
	if( FontArrayFonts[i] == None )
	{
		FontArrayFonts[i] = Font(DynamicLoadObject(FontArrayNames[i], class'Font'));
		if( FontArrayFonts[i] == None )
			Log("Warning: "$Self$" Couldn't dynamically load font "$FontArrayNames[i]);
	}
	return FontArrayFonts[i];
}

function Font GetFontSizeIndex(Canvas C, int FontSize)
{
//gdr Select one size smaller than requested in split screen,
//and don't bump the size at 640 unless clipy is 480.
    if ( C.ClipX < 640 || C.ClipY < 480)
		FontSize--;

	if ( C.ClipX >= 640 && C.ClipY >= 480)
		FontSize++;
	if ( C.ClipX >= 800 )
		FontSize++;
	if ( C.ClipX >= 1024 )
		FontSize++;
	if ( C.ClipX >= 1280 )
		FontSize++;
	if ( C.ClipX >= 1600 )
		FontSize++;

	return LoadFont(Clamp( 8-FontSize, 0, 8));
}

static function Font GetSmallFontFor(Canvas Canvas)
{
	local int i;

	for ( i=0; i<8; i++ )
	{
		if ( Default.FontScreenWidthSmall[i] <= Canvas.ClipX )
			return LoadFontStatic(i);
	}
	return LoadFontStatic(8);
}

static function Font GetMediumFontFor(Canvas Canvas)
{
	local int i;

	for ( i=0; i<8; i++ )
	{
		if ( Default.FontScreenWidthMedium[i] <= Canvas.ClipX )
			return LoadFontStatic(i);
	}
	return LoadFontStatic(8);
}

static function Font SmallerFontThan(Font aFont)
{
	local int i;

	for ( i=0; i<7; i++ )
		if ( LoadFontStatic(i) == aFont )
			return LoadFontStatic(Min(8,i+2));	//One size smaller
	return LoadFontStatic(8);	//Orbit 8
}

static function Font LargerFontThan(Font aFont)
{
	local int i;

	for ( i=0; i<7; i++ )
		if ( LoadFontStatic(i) == aFont )
			return LoadFontStatic(Max(0,i-2));	//One size bigger
	return LoadFontStatic(4);	//Orbit 15
}

simulated function font LoadProgressFont()
{
	if( ProgressFontFont == None )
	{
		ProgressFontFont = Font(DynamicLoadObject(ProgressFontName, class'Font'));
		if( ProgressFontFont == None )
		{
			Log("Warning: "$Self$" Couldn't dynamically load font "$ProgressFontName);
			ProgressFontFont = SmallFont;
		}
	}
	return ProgressFontFont;
}

simulated function DrawTargeting( Canvas C );


defaultproperties
{
     SmallFont=Font'OrbitFonts.OrbitBold8'
     MedFont=Font'OrbitFonts.OrbitBold15'
     BigFont=Font'OrbitFonts.OrbitBold15'
     LargeFont=Font'OrbitFonts.OrbitBold15'
     ProgressFontName="OrbitFonts.OrbitBold8"
     ProgressFadeTime=1
     MOTDColor=(B=255,G=255,R=255,A=255)
     DefaultTextColor=(B=255,G=232,R=198,A=255)
     bMessageBeep=True
     XNotificationFriendIcon=Texture'GUIContent.Menu.XBLplayer_add'
     XNotificationInviteIcon=Texture'GUIContent.Menu.XBLinvite_receive'
     XNotificationTimer=-1
     HudCanvasScale=0.95
     LoadingMessage="LOADING"
     SavingMessage="SAVING"
     ConnectingMessage="CONNECTING"
     PausedMessage="PAUSED"
     MoviePosX=100
     MoviePosY=100
     TexMovieBottom=1
     TexMovieRight=1
     bShowPromptText=True
     bShowCurrentObjective=True
     ConsoleColor=(B=253,G=216,R=153,A=255)
     ConsoleMessageCount=6
     CriticalMessageCount=8
     CriticalMessageDisplayCount=5
     ConsoleFontSize=5
     ConsoleMessagePosY=0.8
     FontArrayNames(0)="OrbitFonts.OrbitBold24"
     FontArrayNames(1)="OrbitFonts.OrbitBold24"
     FontArrayNames(2)="OrbitFonts.OrbitBold15"
     FontArrayNames(3)="OrbitFonts.OrbitBold15"
     FontArrayNames(4)="OrbitFonts.OrbitBold15"
     FontArrayNames(5)="OrbitFonts.OrbitBold12"
     FontArrayNames(6)="OrbitFonts.OrbitBold12"
     FontArrayNames(7)="OrbitFonts.OrbitBold8"
     FontArrayNames(8)="OrbitFonts.OrbitBold8"
     FontScreenWidthMedium(0)=3072
     FontScreenWidthMedium(1)=2048
     FontScreenWidthMedium(2)=1600
     FontScreenWidthMedium(3)=1280
     FontScreenWidthMedium(4)=1024
     FontScreenWidthMedium(5)=800
     FontScreenWidthMedium(6)=640
     FontScreenWidthMedium(7)=512
     FontScreenWidthMedium(8)=400
     FontScreenWidthSmall(0)=3072
     FontScreenWidthSmall(1)=2560
     FontScreenWidthSmall(2)=2048
     FontScreenWidthSmall(3)=1900
     FontScreenWidthSmall(4)=1800
     FontScreenWidthSmall(5)=1600
     FontScreenWidthSmall(6)=1280
     FontScreenWidthSmall(7)=1024
     FontScreenWidthSmall(8)=800
     TargetNormal=Sound'GEN_Sound.Interface.int_GEN_textScroll_lp_02'
     TargetFriend=Sound'GEN_Sound.Interface.consoleType_01'
     TargetFriendInjured=Sound'GEN_Sound.Interface.consoleType_01'
     TargetFriendToHeal=Sound'GEN_Sound.Interface.consoleType_01'
     TargetMarker=Sound'GEN_Sound.Interface.consoleType_01'
     TargetMarkerCancel=Sound'GEN_Sound.Interface.consoleType_01'
     TargetPickup=Sound'GEN_Sound.Interface.consoleType_01'
     TargetEnemy=Sound'GEN_Sound.Interface.consoleType_01'
     TargetPanel=Sound'GEN_Sound.Interface.consoleType_01'
     HealingTarget=Sound'GEN_Sound.Interface.consoleTypeConfirm_01'
     TextPrint=Sound'GEN_Sound.Interface.consoleType_01'
     HealthRecharging=Sound'GEN_Sound.Interface.consoleTypeConfirm_01'
     HealthCharged=Sound'GEN_Sound.Interface.consoleType_01'
     ShieldRecharging=Sound'GEN_Sound.Interface.consoleTypeConfirm_01'
     ShieldCharged=Sound'GEN_Sound.Interface.consoleType_01'
     ActivateStanceDefense=Sound'GEN_Sound.Interface.consoleType_01'
     ActivateStanceHold=Sound'GEN_Sound.Interface.consoleType_01'
     ActivateStanceOffense=Sound'GEN_Sound.Interface.consoleType_01'
     StaticScavenger=Sound'GEN_Sound.Interface.int_GEN_squadHeal_lp_01'
     StaticIncapacitated=Sound'GEN_Sound.Interface.int_GEN_squadHeal_lp_01'
     FlashlightOn=Sound'GEN_Sound.Interface.int_GEN_headlightOn_01'
     FlashlightOff=Sound'GEN_Sound.Interface.int_GEN_headlightOff_01'
     ZoomIn=Sound'GEN_Sound.Interface.int_GEN_snipeZoomIn_01'
     ZoomOut=Sound'GEN_Sound.Interface.int_GEN_snipeZoomOut_01'
     IncapMenuChange=Sound'GEN_Sound.Interface.int_GEN_headlightOn_01'
     IncapMenuSelect=Sound'GEN_Sound.Interface.consoleType_01'
     BloodSplatter=SoundMultiple'GEN_Sound.Impacts_Explos.blood_splat'
     Wiper=Sound'GEN_Sound.Interface.int_GEN_squadHeal_lp_01'
     SwitchToThermalGrenade=Sound'GEN_Sound.Interface.int_GEN_headlightOn_01'
     SwitchToSonicGrenade=Sound'GEN_Sound.Interface.int_GEN_headlightOff_01'
     SwitchToEMPGrenade=Sound'GEN_Sound.Interface.int_GEN_snipeZoomIn_01'
     SwitchToFlashBang=Sound'GEN_Sound.Interface.int_GEN_headlightOff_01'
     SniperZoomCharging=Sound'GEN_Sound.Interface.int_GEN_headlightOn_01'
     SniperZoomCharged=Sound'GEN_Sound.Interface.int_GEN_headlightOff_01'
     bHidden=True
     RemoteRole=ROLE_None
}
