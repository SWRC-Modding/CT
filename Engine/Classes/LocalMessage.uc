//=============================================================================
// LocalMessage
//
// LocalMessages are abstract classes which contain an array of localized text.  
// The PlayerController function ReceiveLocalizedMessage() is used to send messages 
// to a specific player by specifying the LocalMessage class and index.  This allows 
// the message to be localized on the client side, and saves network bandwidth since 
// the text is not sent.  Actors (such as the GameInfo) use one or more LocalMessage 
// classes to send messages.  The BroadcastHandler function BroadcastLocalizedMessage() 
// is used to broadcast localized messages to all the players.
//
//=============================================================================
class LocalMessage extends Info;

var(Message) bool	bComplexString;									// Indicates a multicolor string message class.
var(Message) bool	bIsSpecial;										// If true, don't add to normal queue.
var(Message) bool	bIsUnique;										// If true and special, only one can be in the HUD queue at a time.
var(Message) bool   bIsPartiallyUnique;                             // If true and special, only one can be in the HUD queue with the same switch value
var(Message) bool	bIsConsoleMessage;								// If true, put a GetString on the console.
var(Message) bool	bFadeMessage;									// If true, use fade out effect on message.
var(Message) bool	bBeep;											// If true, beep!
//var(Message) bool	bOffsetYPos;									// If the YPos indicated isn't where the message appears.
//var(Message) bool	bCenter;										// Whether or not to center the message.
//var(Message) bool	bFromBottom;									// Subtract YPos.
var(Message) int	LifeTime;										// # of seconds to stay in HUD message queue.

var(Message) class<LocalMessage> ChildMessage;                      // In some cases, we need to refer to a child message.

enum EStackMode
{
    SM_None,
    SM_Up,
    SM_Down,
};

var(Message) Color  DrawColor;
var(Message) EDrawPivot DrawPivot;
var(Message) EStackMode StackMode;                                  // Brutal hack!
var(Message) float XPos, YPos;
var(Message) int FontSize;                                          // 0: Huge, 1: Big, 2: Small ...

static function RenderComplexMessage( 
	Canvas Canvas, 
	out float XL,
	out float YL,
	optional String MessageString,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1, 
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	);

static function string GetRelatedString(
    optional int Switch,
    optional PlayerReplicationInfo RelatedPRI_1, 
    optional PlayerReplicationInfo RelatedPRI_2,
    optional Object OptionalObject
    )
{
    return static.GetString(Switch,RelatedPRI_1,RelatedPRI_2,OptionalObject);
}

static function string GetString(
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1, 
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	if ( class<Actor>(OptionalObject) != None )
		return class<Actor>(OptionalObject).static.GetLocalString(Switch, RelatedPRI_1, RelatedPRI_2);
	return "";
}

static function string AssembleString(
	HUD myHUD,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1, 
	optional String MessageString
	)
{
	return "";
}

static function ClientReceive( 
	PlayerController P,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1, 
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	if ( P.myHud != None )
	P.myHUD.LocalizedMessage( Default.Class, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject );
    if ( P.DemoViewer != None )
		P.DemoViewer.myHUD.LocalizedMessage( Default.Class, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject );

//gdr Don't add duplicate console messages
//    if ( IsConsoleMessage(Switch) && (P.Player != None) && (P.Player.Console != None) )
//		P.Player.InteractionMaster.Process_Message( Static.GetString( Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject ), 6.0, P.Player.LocalInteractions);
}

static function color GetConsoleColor( PlayerReplicationInfo RelatedPRI_1 )
{
    return Default.DrawColor;
}

static function color GetColor(
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1, 
	optional PlayerReplicationInfo RelatedPRI_2
	)
{
	return Default.DrawColor;
}

static function GetPos( int Switch, out EDrawPivot OutDrawPivot, out EStackMode OutStackMode, out float OutPosX, out float OutPosY )
{
    OutDrawPivot = default.DrawPivot;
    OutStackMode = default.StackMode;
    OutPosX = default.XPos;
    OutPosY = default.YPos;
}

static function float GetOffset(int Switch, float YL, float ClipY )
{
	return Default.YPos;
}

static function int GetFontSize( int Switch, PlayerReplicationInfo RelatedPRI1, PlayerReplicationInfo RelatedPRI2, PlayerReplicationInfo LocalPlayer )
{
    return default.FontSize;
}

static function float GetLifeTime(int Switch)
{
    return default.LifeTime;
}

static function bool IsConsoleMessage(int Switch)
{
    return default.bIsConsoleMessage;
}

static function string GetPlayerName(PlayerReplicationInfo PRI)
{
	if(PRI == None)
		return "";

	if(PRI.ConsoleCommand("XLIVE GETAUTHSTATE") == "ONLINE" && !PRI.Level.IsSystemLink() && !PRI.Level.IsSplitScreen())
		return PRI.GamerTag;
	else
		return PRI.PlayerName;
}


defaultproperties
{
     bIsSpecial=True
     bIsConsoleMessage=True
     bFadeMessage=True
     LifeTime=3
     DrawColor=(B=255,G=255,R=255,A=255)
     DrawPivot=DP_MiddleMiddle
     XPos=0.5
     YPos=0.8
}

