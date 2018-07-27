class MenuTemplateTitledBXA extends MenuTemplateTitledBA
    native;

var() MenuSprite        XButtonIcon;
var() MenuText          XLabel;
var() MenuButtonText	XButton;

simulated function OnXButton();

simulated function bool HandleInputGamePad( String ButtonName )
{
    if( ButtonName ~= "X" )
    {
        OnXButton();
        return( true );
    }
    
	return( Super.HandleInputGamePad( ButtonName ) );
}

simulated function HideXButton(int hide)
{
	if ( !IsOnConsole() )
	{
		XButton.bHidden = hide;	
		return;
	}
		
	XButtonIcon.bHidden = hide;
    XLabel.bHidden = hide;
}

simulated function HandleStripInfoOpacity(float Opacity)
{
	XButtonIcon.DrawColor.A = 255 * Opacity;
    XLabel.DrawColor.A = 255 * Opacity;
    Super.HandleStripInfoOpacity(Opacity);
}


defaultproperties
{
     XButtonIcon=(DrawPivot=DP_MiddleRight,PosX=0.37,PosY=0.896,Platform=MWP_Console,Style="XboxButtonX")
     XLabel=(Text=": OPTIONS",DrawPivot=DP_MiddleLeft,PosX=0.36,PosY=0.896,Platform=MWP_Console,Style="LabelText")
     XButton=(Blurred=(Text="OPTIONS",PosX=0.5,PosY=0.896),BackgroundBlurred=(PosX=0.5,PosY=0.896,ScaleX=0.19,ScaleY=0.04333),OnSelect="OnXButton",Pass=2,Platform=MWP_PC,Style="ButtonTextStyle1")
}

