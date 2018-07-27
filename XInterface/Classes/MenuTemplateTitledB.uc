class MenuTemplateTitledB extends MenuTemplateTitled
    native;

var() MenuSprite        BottomStrip;

var() MenuSprite        BButtonIcon;
var() MenuText          BLabel;
var() MenuButtonText	BButton;

simulated function OnBButton()
{
    HandleInputBack();
}

simulated function HideBButton(int hide)
{
	if ( !IsOnConsole() )
	{
	    BButton.bHidden = hide;
		return;
	}
	
	BButtonIcon.bHidden = hide;
    BLabel.bHidden = hide;
}

simulated function bool HandleInputGamePad( String ButtonName )
{
    if( ButtonName ~= "B" )
    {
        OnBButton();
        return( true );
    }
    
	return( Super.HandleInputGamePad( ButtonName ) );
}


simulated function HandleStripInfoOpacity(float Opacity)
{
    BButtonIcon.DrawColor.A = 255 * Opacity;
    BLabel.DrawColor.A = 255 * Opacity;
    Super.HandleStripInfoOpacity(Opacity);
}


defaultproperties
{
     BottomStrip=(WidgetTexture=Texture'GUIContent.Menu.CT_ButtonFocus',DrawColor=(A=128),DrawPivot=DP_MiddleMiddle,PosX=0.5,PosY=0.893305,ScaleX=0.95,ScaleY=0.055,ScaleMode=MSCM_FitStretch,Platform=MWP_Console)
     BButtonIcon=(DrawPivot=DP_MiddleRight,PosX=0.105,PosY=0.896,Platform=MWP_Console,Style="XboxButtonB")
     BLabel=(Text=": BACK",DrawPivot=DP_MiddleLeft,PosX=0.095,PosY=0.896,Platform=MWP_Console,Style="LabelText")
     BButton=(Blurred=(Text="BACK",PosX=0.15,PosY=0.896),BackgroundBlurred=(PosX=0.15,PosY=0.896,ScaleX=0.19,ScaleY=0.04333),OnSelect="OnBButton",Pass=2,Platform=MWP_PC,Style="ButtonTextStyle1")
}

