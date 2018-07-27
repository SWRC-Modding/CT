class MenuQuestionYesNo extends MenuTemplateTitledBXA;

var() MenuText Question;
//var() MenuButtonText	YesButton;
//var() MenuButtonText	NoButton;


var() bool bSelectedYes;
var() bool bYesNoCancel;
var() bool bSelectedCancel;

simulated function Init( String Args )
{
    local MenuTemplateTitled SubMenu;
    local String QuestionText, TitleText;

    Super.Init( Args );
    
	if ( Caps(GetPlayerOwner().GetLanguage()) != "INT" )
	{
		AButton.BackgroundFocused.ScaleX = 0.19;
		AButton.BackgroundBlurred.ScaleX = 0.19;		
	}    

    SubMenu = MenuTemplateTitled( PreviousMenu );
    
    if( SubMenu == None )
        SubMenu = MenuTemplateTitled( UnderlayMenu );
    
    if( SubMenu != None )
        Background = SubMenu.Background;

    if( Args != "" )
    {
        QuestionText = ParseToken( Args );
        TitleText = ParseToken( Args );
        SetText( QuestionText, TitleText );
    }

    if (!bYesNoCancel)
        HideXButton(1);

	/*
	 if (IsOnConsole())
	 {
		 YesButton.bHidden = 1;
		 NoButton.bHidden = 1;
	 }
	 else
	 {
		  FocusOnWidget( NoButton );
	 }
	 */
	 
	 if ( !IsOnConsole() )
		FocusOnWidget( BButton );
}

// YesNoCancel -> A=Yes, X=No, B=Cancel
simulated function SetYesNoCancelMode()
{
    bYesNoCancel = true;
    BLabel.Text = ": " $ StringCancel;
}

simulated event SetText( String QuestionText, optional String TitleText )
{
    if( TitleText != "" )
        MenuTitle.Text = Caps(TitleText);
        
    Question.Text = Caps(QuestionText);
}

simulated function String GetText()
{
    return( Question.Text );
}

simulated function OnYes()
{
    bSelectedYes = true;
    CloseMenu();
}

simulated function OnNo()
{
    CloseMenu();
}

simulated function OnCancel()
{
    bSelectedCancel = true;
    CloseMenu();
}

simulated function HandleInputStart()
{
    OnAButton();
}

simulated function OnAButton()
{
    OnYes();
}

simulated function HandleInputBack()
{
    OnBButton();
}

simulated function OnBButton()
{
    if (bYesNoCancel)
        OnCancel();
    else
        OnNo();
}

simulated function OnXButton()
{
    if (bYesNoCancel)
        OnNo();
}

simulated function bool HandleInputGamePad( String ButtonName )
{
    if( ButtonName == "A" )
    {
        OnAButton();
        return( true );
    }

    if( ButtonName == "B" )
    {
        OnBButton();
        return( true );
    }
    
	return( Super.HandleInputGamePad( ButtonName ) );
}


defaultproperties
{
     Question=(DrawPivot=DP_MiddleLeft,PosX=0.2,PosY=0.5,MaxSizeX=0.6,bWordWrap=1,Pass=2,Style="LabelText")
     XLabel=(Text=": NO")
     XButton=(bHidden=1)
     ALabel=(Text="YES :")
     AButton=(Blurred=(Text="YES"),bHidden=0)
     BLabel=(Text=": NO")
     BButton=(Blurred=(Text="NO"))
     Background=(bHidden=1)
     MenuTitle=(Text="PLEASE CONFIRM",bHidden=0)
     CrossFadeRate=3
}

