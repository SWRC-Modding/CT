class ModCTLoading extends CTLoading;

simulated function Init(String Args){
	LoadingBar.Low = 0.0;
	LoadingBar.High = 1.0;
	LoadingBar.Value = 0.0;

	SetupNewHint();

	LastHintTime = GetAppTime();
}

function SetInfoOptions(String Pic, String Title, String Text, String NewLevel, bool ShowHints){
	local int i;

	InfoBlurb.Text = Title;

	bNoHints = !ShowHints;


	if (( Title == "" ) || bNoHints){
		bNoHints = True;
		HintBorder.bHidden = 1;
		HintTitle.bHidden = 1;
		HintText.bHidden = 1;
		HintSeparator.bHidden = 1;
		
		for (i = 0; i < NUM_HINT_PICS; ++i)
			HintPics[i].bHidden = 1;
	}
}