global function SRMM_InitSettingsMenu

struct
{
	var menu
	table<var,string> buttonDescriptions
	var itemDescriptionBox
} file

void function SRMM_InitSettingsMenu()
{
	var menu = GetMenu( "SRMM_SettingsMenu" )
	file.menu = menu

	AddMenuEventHandler( menu, eUIEvent.MENU_OPEN, SRMM_OnOpenSettingsMenu )
	AddMenuEventHandler( menu, eUIEvent.MENU_CLOSE, SRMM_OnCloseSettingsMenu )

	file.itemDescriptionBox = Hud_GetChild( menu, "LblMenuItemDescription" )

	var button
	
	// Video
	SetupButton(
		Hud_GetChild( menu, "SwitchBloomEnable" ),
		"Bloom",
		"Toggles the bloom to reduce brightness and glare"
	)
	
	// HUD
	button = Hud_GetChild( menu, "BtnSpeedometerEnable" )
	SRMM_SetupButton(
		button,
		"Speedometer",
		"Enables a speedometer in single player.\n\n`2Requires a reload for changes to take effect",
		SRMM_getSetting(SRMM_settings.enableSpeedometer)
	)
	AddButtonEventHandler( button, UIE_CLICK, SpeedometerEnableToggle )
	
	SetupButton(
		Hud_GetChild( menu, "SwitchSpeedometerMode" ),
		"Speedometer Mode",
		"Sets which unit the speedometer measures\n\n`2Requires a reload for changes to take effect"
	)
	
	button = Hud_GetChild( menu, "BtnSpeedometerIncludeZ" )
	SRMM_SetupButton(
		button,
		"Include Z Axis",
		"Include vertical axis in the speedometer display",
		SRMM_getSetting(SRMM_settings.speedometerIncludeZ)
	)
	AddButtonEventHandler( button, UIE_CLICK, SpeedometerIncludeZToggle )
	
	button = Hud_GetChild( menu, "BtnSpeedometerEnableFadeout" )
	SRMM_SetupButton(
		button,
		"Fadeout",
		"Fade out the speedometer display when moving slowly",
		SRMM_getSetting(SRMM_settings.speedometerFadeout)
	)
	AddButtonEventHandler( button, UIE_CLICK, SpeedometerFadeoutToggle )
	
	SetupButton(
		Hud_GetChild( menu, "SwitchShowFps" ),
		"Show FPS",
		"Shows an overlay with FPS and server tickrate\n\n`1Top-right: `0Displays the FPS and server tickrate in the Top-right hand side of the screen\n\n`1Top-left: `0Displays the FPS and server tickrate in the Top-left hand side of the screen\n\n`1Server: `0Displays only the server tickrate\n\n`1Minimal: `0Displays a smaller FPS and tickrate display on the top left hand side of the screen"
	)
	SetupButton(
		Hud_GetChild( menu, "SwitchShowFpsBig" ),
		"Show Large FPS",
		"`1FPS: `0Shows a large overlay with FPS and server tickrate\n\n`1FPS/Graph: `0Shows a large FPS overlay and performance graph"
	)
	SetupButton(
		Hud_GetChild( menu, "SwitchShowPos" ),
		"Show Positional Information",
		"`1Player Position: `0Shows position, angle and velocity from the player model\n\n`1Camera Position: `0Shows position, angle and velocity from the player camera"
	)

	// Misc
	SetupButton(
		Hud_GetChild( menu, "SwitchEnableCheats" ),
		"Enable Cheats",
		"`2NOT LEADERBOARD LEGAL!\n\n`0Sets the sv_cheats console variable.\nEnables use of host_timescale console command"
	)
	SetupButton(
		Hud_GetChild( menu, "SwitchEnableMP" ),
		"Enable Multiplayer",
		"Enables or disables the multiplayer buttons in the main menu"
	)

	button = Hud_GetChild( menu, "BtnCKfix" )
	SRMM_SetupButton(button,
		"Crouch Kick Fix",
		"Adds an 8 ms Buffer to your jump and crouch inputs.\n\nPressing both Jump and Crouch up to 8 ms apart from each other will register both inputs at the same time\nThe combined input will be registered at the time of your second input",
		SRMM_getSetting(SRMM_settings.CKfix)
	)
	AddButtonEventHandler( button, UIE_CLICK, CKfixToggle )
    
	button = Hud_GetChild( menu, "BtnPracticeMode" )
	SRMM_SetupButton(
		button,
		"Practice Mode",
		"`2NOT LEADERBOARD LEGAL!\n\n`0Changes some settings to make practice a bit easier\n\n- Disables input prevention on saveload\n- Enables use of host_timescale",
		SRMM_getSetting(SRMM_settings.practiceMode)
	)
	AddButtonEventHandler( button, UIE_CLICK, PracticeModeToggle )

	// Actions
	button = Hud_GetChild( menu, "BtnResetHelmets" )
	SetupButton(
		button,
		"Reset Helmets",
		"Reset every helmet collectible to be uncollected"
	)
	AddButtonEventHandler( button, UIE_CLICK, ResetHelmetsDialog )

	button = Hud_GetChild( menu, "BtnUnlockLevels" )
	SetupButton(
		button,
		"Unlock all Levels",
		"Unlocks all levels to be selectable from the menu"
	)
	AddButtonEventHandler( button, UIE_CLICK, UnlockLevelsDialog )

	button = Hud_GetChild( menu, "BtnMouseKeyboardBindings" )
	SetupButton(
		button,
		"Key Bindings",
		"Modify mouse / keyboard bindings."
	)
	AddButtonEventHandler( button, UIE_CLICK, AdvanceMenuEventHandler( GetMenu( "MouseKeyboardBindingsMenu" ) ) )

	button = Hud_GetChild( menu, "BtnPracticeWarps" )
	SetupButton(
		button,
		"Practice Warps",
		"Warp to dev start points throughout the game to practice segments"
	)
	AddButtonEventHandler( button, UIE_CLICK, AdvanceMenuEventHandler( GetMenu( "SRMM_PracticeWarpsMenu" ) ) )

	AddEventHandlerToButtonClass( menu, "RuiFooterButtonClass", UIE_GET_FOCUS, FooterButton_Focused )
	
	AddMenuFooterOption( menu, BUTTON_A, "#A_BUTTON_SELECT" )
	AddMenuFooterOption( menu, BUTTON_B, "#B_BUTTON_BACK", "#BACK" )
}



void function SpeedometerEnableToggle(var button)
{
	SRMM_buttonToggle(button, SRMM_settings.enableSpeedometer, "Speedometer")
	//if (SRMM_getSetting(SRMM_settings.enableSpeedometer)) {
	//	CreatePilotSpeedometer();
	//} else {
	//	DestroyPilotSpeedometer();
	//}
}

void function SpeedometerIncludeZToggle(var button)
{
	SRMM_buttonToggle(button, SRMM_settings.speedometerIncludeZ, "Include Z Axis")
}

void function SpeedometerFadeoutToggle(var button)
{
	SRMM_buttonToggle(button, SRMM_settings.speedometerFadeout, "Fadeout")
}


void function AreYouSureDialog(string header, string message, void functionref() confirmFunc = null)
{
	DialogData dialogData
	dialogData.header = header
	dialogData.message = message

	AddDialogButton(dialogData, "Yes", confirmFunc)
	AddDialogButton(dialogData, "No")

	OpenDialog(dialogData)
}

void function ResetHelmetsDialog(var button)
{
	AreYouSureDialog("Reset Helmets", "Are you sure you want to reset all helmets?", ResetCollectiblesProgress_All)
}

void function UnlockAllLevels() {SetConVarInt("sp_unlockedMission", 9)}
void function UnlockLevelsDialog(var button)
{
	AreYouSureDialog("Unlock Levels", "Are you sure you want to unlock all levels?", UnlockAllLevels)
}


void function CKfixToggle(var button)
{
	SRMM_buttonToggle(button, SRMM_settings.CKfix, "Crouch Kick Fix")
}

void function PracticeModeToggle(var button)
{
	SRMM_toggleSetting(SRMM_settings.practiceMode)
	string settingLabel
	if (SRMM_getSetting(SRMM_settings.practiceMode)) {
		EnablePracticeMode()
		settingLabel = "Enabled"
	} else {
		DisablePracticeMode()
		settingLabel = "Disabled"
	}
	SetButtonRuiText(button, "Practice Mode: " + settingLabel)
}

void function EnablePracticeMode()
{
	// input prevention on load
	SetConVarFloat("player_respawnInputDebounceDuration", 0)

	SetConVarInt("sv_cheats", 1)
}

void function DisablePracticeMode()
{
	// revert to default values
	SetConVarFloat("player_respawnInputDebounceDuration", 0.5)
	SetConVarInt("sv_cheats", 0)
}

void function SRMM_OnOpenSettingsMenu()
{
	UI_SetPresentationType( ePresentationType.NO_MODELS )

}

void function SRMM_OnCloseSettingsMenu()
{
	SavePlayerSettings()
}


void function SetupButton( var button, string buttonText, string description )
{
	SetButtonRuiText( button, buttonText )
	file.buttonDescriptions[ button ] <- description
	AddButtonEventHandler( button, UIE_GET_FOCUS, Button_Focused )
}

void function SRMM_SetupButton(var button, string buttonLabel, string description, bool setting)
{
	string settingLabel
	if (setting) {
		settingLabel = "Enabled"
	} else {
		settingLabel = "Disabled"
	}
	buttonLabel += ": "
	SetButtonRuiText(button, buttonLabel + settingLabel)
	
	file.buttonDescriptions[ button ] <- description
	AddButtonEventHandler( button, UIE_GET_FOCUS, Button_Focused )
}

void function SRMM_buttonToggle(var button, int setting, string buttonLabel)
{
	SRMM_toggleSetting(setting)
	string settingLabel
	if (SRMM_getSetting(setting)) {
		settingLabel = "Enabled"
	} else {
		settingLabel = "Disabled"
	}
	buttonLabel += ": "
	SetButtonRuiText(button, buttonLabel + settingLabel)
}

void function Button_Focused( var button )
{
	string description = file.buttonDescriptions[ button ]
	RuiSetString( Hud_GetRui( file.itemDescriptionBox ), "description", description )
}

void function FooterButton_Focused( var button )
{
	RuiSetString( Hud_GetRui( file.itemDescriptionBox ), "description", "" )
}