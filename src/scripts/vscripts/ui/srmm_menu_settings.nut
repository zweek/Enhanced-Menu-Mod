global function SRMM_InitSettingsMenu

struct
{
	var menu
	table<var,string> buttonDescriptions
	var itemDescriptionBox
} file


void function SRMM_InitSettingsMenu()
{
	SRMM_setSetting(SRMM_settings.CKfix, true)
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
	
	button = Hud_GetChild( menu, "BtnTASMode" )
	SRMM_SetupButton(
		button,
		"TAS Mode",
		"`2NOT LEADERBOARD LEGAL!\n\n`0Changes your game settings to be TAS compatible\n\n- Disables load audio fade\n- Disables input prevention on saveload\n- Enables use of host_timescale",
		SRMM_getSetting(SRMM_settings.TASmode)
		)
	AddButtonEventHandler( button, UIE_CLICK, TASModeToggle )
	
	// button = Hud_GetChild( menu, "BtnEnableConsole" )
	// SRMM_SetupButton(
	// 	button,
	// 	"Console",
	// 	"`2Only for testing and debug purposes!\n\n`0Enable Standard Console output that shows information about the crouch kick buffer and your inputs",
	// 	SRMM_getSetting(SRMM_settings.enableConsole)
	// )
	// AddButtonEventHandler( button, UIE_CLICK, ConsoleToggle )
    
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

void function TASModeToggle(var button)
{
	SRMM_toggleSetting(SRMM_settings.TASmode)
	string settingLabel
	if (SRMM_getSetting(SRMM_settings.TASmode)) {
		EnableTASMode()
		settingLabel = "Enabled"
	} else {
		DisableTASMode()
		settingLabel = "Disabled"
	}
	SetButtonRuiText(button, "TAS Mode: " + settingLabel)
}

void function EnableTASMode()
{
	// audio fade on load
	SetConVarFloat("miles_map_begin_fade_time", 0)
	SetConVarFloat("miles_map_begin_silence_time", 0)
	// input prevention on load
	SetConVarFloat("player_respawnInputDebounceDuration", 0)
	// command queue to make lower timescales work
	SetConVarInt("sv_usercmd_max_queued", 1000)

	SetConVarInt("sv_cheats", 1)
}

void function DisableTASMode()
{
	// revert to default values
	SetConVarFloat("miles_map_begin_fade_time", 1.5)
	SetConVarFloat("miles_map_begin_silence_time", 0.5)
	SetConVarFloat("player_respawnInputDebounceDuration", 0.5)
	SetConVarInt("sv_usercmd_max_queued", 40)
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