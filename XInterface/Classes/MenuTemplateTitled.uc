class MenuTemplateTitled extends MenuTemplate
    native;

var() MenuSprite    Background;
var() MenuSprite    TitleStrip;
var() MenuText      MenuTitle;
//var() MenuSprite	Ulogo;
var() MenuSprite	ControllerIcon;
var() MenuText      HostText;
var() MenuText      ControllerNumText;

var() bool bShowGamertag;
var() bool bShowController;

var() bool bRequiresXOnline;

var() localized string LiveLabel;
var() localized string LiveSigningIn;
var() localized string LiveSignedIn;
var() localized string LiveFailed;
var() localized string LiveNotInPasscode;
var() localized string LiveNotIn;


//////////////////////////////////////////////////////////////////////
// U Stamp Logo thingy.. just used for some animation in the menus
//////////////////////////////////////////////////////////////////////

var() class<Actor> UstaticMeshActor;
//var() MenuActor		UStamp;
//var() Vector        UStampPosition;
//var() StaticMesh    UStampMesh;
//var() Rotator		UStampRotation;
//var() Rotator		CurUStampRotation;
var() color         AmbientLight;

var() Material ActorMaterials[2]; // To avoid cutdown!


// Menu Title Animation
var() bool	bAnimateStamp;

simulated function Init( String Args )
{
	// U Stamp Spinner
	//UStampSpin();
	Super.Init(Args);
    UpdateGamePadIndex();
}

simulated function Destroyed()
{
	/*
    if( UStamp.Actor != None )
    {
        UStamp.Actor.Destroy();
        UStamp.Actor = None;
    }
    */
    Super.Destroyed();
}

simulated function Tick(float deltaTime)
{
//    local Color ModColor;
    
	Super.Tick(deltaTime);

    if( CrossFadeDir != TD_None )
	    MenuTitle.ScaleX = default.LabelText.ScaleX * CrossFadeLevel;
	else if( ModulateDir != TD_None )
	    MenuTitle.ScaleX = default.LabelText.ScaleX * ModulateLevel;
    else
        MenuTitle.ScaleX = default.LabelText.ScaleX;

	/*
	if( UStamp.Actor != None && (bHasFocus || (CrossFadeDir == TD_In) || (ModulateDir == TD_In)) )
	{
		class'MenuTemplateTitled'.default.CurUStampRotation = class'MenuTemplateTitled'.default.CurUStampRotation + ( UStampRotation * deltaTime);
		UStamp.Actor.SetRotation( class'MenuTemplateTitled'.default.CurUStampRotation );
		ModColor = AmbientLight;
		ModColor.A = byte(ModulateLevel * float(ModColor.A));
        ConstantColor(Combiner(FinalBlend(UStamp.Actor.Skins[0]).Material).Material2).Color = ModColor;
        ConstantColor(Combiner(FinalBlend(UStamp.Actor.Skins[1]).Material).Material2).Color = ModColor;
	}
*/
    UpdateGamePadIndex();

    HandleStripInfoOpacity(1.0-HelpTextOpacity);
}

simulated function UpdateGamePadIndex()
{
	local PlayerController PC;
	local string s;
	local string err;

	if( !bShowController )
	{
		ControllerNumText.bHidden = 1;
		ControllerIcon.bHidden = 1;
	}	
	else
	{
		ControllerNumText.Text = string(PC.Player.GamePadIndex+1);
		ControllerNumText.bHidden = 0;
		ControllerIcon.bHidden = 0;
	}			


	PC = PlayerController(Owner);
	if( PC == None )
        return;

	if( PC.Player == None )
        return;


	if( !bShowGamertag || !IsOnConsole() )
	{
		HostText.bHidden = 1;
	}
   else        
	{
		HostText.bHidden = 0;

		if( GetPlayerOwner().Player.Console.CurMenu != self )
			return;

		s = ConsoleCommand("XLIVE GETAUTHSTATE");

        if( s == "SIGNING_ON" || s == "CHANGING_LOGON" )
			HostText.Text = LiveSigningIn;
        else if ( s == "ONLINE" )
			HostText.Text = LiveSignedIn@PC.GamerTag;
		else
		{
			if ( bRequiresXOnline )
			{
				// can't BE in this menu without a xlive connect
				if (!self.IsA('MenuLivePasscode'))
				{
					CloseMenu();
					GotoMenuClass ("XInterfaceLive.MenuLiveErrorMessage");
				}
			}
			else if ( s == "OFFLINE" )
			{
				if(ConsoleCommand("XLIVE REQUIRE_PASSCODE_SILENT") == "FALSE" )
					HostText.Text = LiveNotIn;
				else
					HostText.Text = LiveNotInPasscode;
			}
			else
			{
			    s = ConsoleCommand("XLIVE PEEKERROR");
				err = ParseToken(s);

				if(err == "XONLINE_E_LOGON_KICKED_BY_DUPLICATE_LOGON")
				{
					if ( !self.IsA('MenuLiveErrorMessage' ) && !self.IsA('MenuLivePasscode'))
					{
						ConsoleCommand("XLIVE LOGOFF -1");
					}
					HostText.Text = LiveNotIn;
				}
				else if	(ConsoleCommand("XLIVE REQUIRE_PASSCODE_SILENT") == "TRUE" )
				{
					HostText.Text = LiveNotInPasscode;
				}
				else
				{
					HostText.Text = LiveFailed;
				}
			}
		}
	}
}

simulated function HandleStripInfoOpacity(float Opacity)
{
}

/*simulated function UStampSpin()
{
    local int i;
    
	if( bAnimateStamp )
	{
		UstaticMeshActor = class<Actor>(DynamicLoadObject("XInterface.MenuActorStaticMesh",class'Class'));
		UStamp.Actor = Spawn( UstaticMeshActor, self,, UStampPosition, class'MenuTemplateTitled'.default.CurUStampRotation );
	
	    for( i = 0; i < ArrayCount(ActorMaterials); i++ )
	        UStamp.Actor.Skins[i] = ActorMaterials[i];

		// TODO: reenable this if we ever have one
//		UStamp.Actor.SetStaticMesh( UStampMesh );
		UStamp.Actor.SetDrawScale(0.25);
	}
}
*/
///////////////////////////////////////////////////////////////////////



defaultproperties
{
     TitleStrip=(DrawPivot=DP_UpperMiddle,PosX=0.5,PosY=0.0753138,ScaleX=1.5,ScaleY=0.0627615,bHidden=1,Style="Border")
     MenuTitle=(DrawPivot=DP_MiddleMiddle,PosX=0.5,PosY=0.108787,ScaleX=1.75,ScaleY=1.75,bHidden=1,Style="LabelText")
     ControllerIcon=(WidgetTexture=Texture'Engine.TerrainBad',DrawPivot=DP_MiddleMiddle,PosX=0.92,PosY=0.108787,ScaleX=1,ScaleY=1,Pass=1)
     HostText=(MenuFont=Font'OrbitFonts.OrbitBold15',DrawPivot=DP_MiddleMiddle,PosX=0.5,PosY=0.158787,MaxSizeX=0.75,Pass=4,bHidden=1,Style="LabelText")
     ControllerNumText=(Text="1",DrawPivot=DP_MiddleMiddle,PosX=0.921,PosY=0.108787,Style="LabelText")
     LiveLabel="Xbox Live:"
     LiveSigningIn="Signing In..."
     LiveSignedIn="Signed In:"
     LiveFailed="Sign In Failed."
     LiveNotInPasscode="Not Signed In: Passcode Needed"
     LiveNotIn="Not Signed In."
     AmbientLight=(B=255,G=255,R=255,A=32)
     ActorMaterials(0)=Texture'Engine.TerrainBad'
     ActorMaterials(1)=Texture'Engine.TerrainBad'
     bFullscreenOnly=True
}

