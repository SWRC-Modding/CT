class MessageTextureClient extends Info
	placeable;

var() ScriptedTexture	MessageTexture;

var() localized string ScrollingMessage;
var localized string HisMessage, HerMessage;
var() Font Font;
var() color FontColor;
var() bool bCaps;

var string OldText;

/* parameters for ScrollingMessage:

   %p - local player name
   %h - his/her for local player
   %lp - leading player's name
   %lf - leading player's frags
*/

simulated function PostNetBeginPlay()
{
	if(MessageTexture != None)
		MessageTexture.Client = Self;

	SetTimer(1,true);
}

simulated function Timer()
{
	local string Text;
	local PlayerReplicationInfo Leading, PRI;

	Text = ScrollingMessage;

	if(InStr(Text, "%lf") != -1 || InStr(Text, "%lp") != -1)
	{
		// find the leading player
		Leading = None;

		ForEach AllActors(class'PlayerReplicationInfo',PRI)
			if ( !PRI.bIsSpectator && (Leading==None || PRI.Score>Leading.Score) )
				Leading = PRI;

		if(Leading != None)
		{
			Text = Replace(Text, "%lp", Leading.GetPlayerName());
			Text = Replace(Text, "%lf", string(int(Leading.Score)));
		}
		else
			Text = "";
	}

	if(bCaps)
		Text = Caps(Text);

	if(Text != OldText)
	{
		OldText = Text;
		MessageTexture.Revision++;
	}
}

simulated event RenderTexture(ScriptedTexture Tex)
{
	local int	SizeX,
				SizeY;

	Tex.TextSize(OldText,Font,SizeX,SizeY);

	Tex.DrawText( (Tex.USize - SizeX) * 0.5, (Tex.VSize - SizeY) * 0.5, OldText, Font, FontColor );
}

simulated function string Replace(string Text, string Match, string Replacement)
{
	local int i;
	
	i = InStr(Text, Match);	

	if(i != -1)
		return Left(Text, i) $ Replacement $ Replace(Mid(Text, i+Len(Match)), Match, Replacement);
	else
		return Text;
}


defaultproperties
{
     HisMessage="his"
     HerMessage="her"
     bNoDelete=True
     bAlwaysRelevant=True
     RemoteRole=ROLE_SimulatedProxy
}

