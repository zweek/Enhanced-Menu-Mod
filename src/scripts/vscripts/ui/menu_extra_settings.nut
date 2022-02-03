global function InitExtrasMenu
global function getSRMMsetting
global function setSRMMsetting
global function toggleSRMMsetting

struct
{
	var menu
	table<var,string> buttonDescriptions
	var classicMusicSwitch
} file

void function InitExtrasMenu()
{
	var menu = GetMenu( "ExtrasMenu" )
	file.menu = menu

	AddMenuEventHandler( menu, eUIEvent.MENU_OPEN, OnOpenExtrasMenu )
	AddMenuEventHandler( menu, eUIEvent.MENU_CLOSE, OnCloseExtrasMenu )

	var button
	
	SetupButton( Hud_GetChild( menu, "SwitchBloomEnable" ), "Bloom", "Toggles the bloom to reduce brightness and glare" )
	
	button = Hud_GetChild( menu, "BtnSpeedometerEnable" )
	SetupButton( button, "Enable/Disable Speedometer", "Enables a speedometer in single player.\nRequires a reload for changes to take effect" )
	AddButtonEventHandler( button, UIE_CLICK, SpeedometerEnableDialog )
	
	SetupButton( Hud_GetChild( menu, "SwitchSpeedometerMode" ), "Speedometer Mode", "Sets which unit the speedometer measures\nRequires a reload for changes to take effect" )
	
	button = Hud_GetChild( menu, "BtnSpeedometerIncludeZ" )
	SetupButton( button, "Include Z axis", "Include vertical axis in the speedometer display" )
	AddButtonEventHandler( button, UIE_CLICK, SpeedometerIncludeZDialog )
	
	button = Hud_GetChild( menu, "BtnSpeedometerEnableFadeout" )
	SetupButton( button, "Enable/Disable Fadeout", "Fade out the speedometer display when moving slowly" )
	AddButtonEventHandler( button, UIE_CLICK, SpeedometerFadeoutDialog )
	
	SetupButton( Hud_GetChild( menu, "SwitchShowFps" ), "Show FPS", "Shows an overlay with FPS and server tickrate\n\nTop-right: Displays the FPS and server tickrate in the Top-right hand side of the screen\n\nTop-left: Displays the FPS and server tickrate in the Top-left hand side of the screen\n\nServer: Displays only the server tickrate\n\nMinimal: Displays a smaller FPS and tickrate display on the top left hand side of the screen" )
	SetupButton( Hud_GetChild( menu, "SwitchShowFpsBig" ), "Show Large FPS", "FPS: Shows a large overlay with FPS and server tickrate\n\nFPS/Graph: Shows a large FPS overlay and performance graph" )
	SetupButton( Hud_GetChild( menu, "SwitchShowPos" ), "Show Positional Information", "Player Position: Shows position, angle and velocity from the player model\n\nCamera Position: Shows position, angle and velocity from the player camera" )

	SetupButton( Hud_GetChild( menu, "SwitchEnableDemos" ), "Enable Demos", "Enable recording demos (must be set true before loading a map)." )
	SetupButton( Hud_GetChild( menu, "SwitchDemosWrite" ), "Save Demos", "Demos write to a local file when recording a demo." )
	SetupButton( Hud_GetChild( menu, "SwitchDemosInterpolate" ), "Interpolate Playback", "Do view interpolation during dem playback." )
	SetupButton( Hud_GetChild( menu, "SwitchDemosUpdateRateSp" ), "Demo record rate Single Player", "Change the tick recording rate in Single Player." )
	SetupButton( Hud_GetChild( menu, "SwitchDemosUpdateRateMp" ), "Demo record rate Multiplayer", "Change the tick recording rate in Multiplayer." )
	SetupButton( Hud_GetChild( menu, "SwitchDemosAutorecord" ), "Auto Record", "Automatically record multiplayer matches as demos." )

	SetupButton( Hud_GetChild( menu, "SwitchEnableCheats" ), "Enable Cheats", "NOT LEADERBOARD LEGAL!\n\nSets the sv_cheats console variable.\nEnables use of host_timescale console command" )
	SetupButton( Hud_GetChild( menu, "SwitchEnableMP" ), "Enable Multiplayer", "Enables or disables the multiplayer buttons in the main menu" )
	
	button = Hud_GetChild( menu, "BtnMouseKeyboardBindings" )
	SetupButton( button, "Key Bindings", "Key bindings for speedrun related actions" )
	AddButtonEventHandler( button, UIE_CLICK, AdvanceMenuEventHandler( GetMenu( "MouseKeyboardBindingsMenu" ) ) )

	button = Hud_GetChild( menu, "BtnResetHelmets" )
	SetupButton( button, "Reset Helmets", "Reset every helmet collectible to be uncollected" )
	AddButtonEventHandler( button, UIE_CLICK, ResetHelmetsDialog )

	button = Hud_GetChild( menu, "BtnUnlockLevels" )
	SetupButton( button, "Unlock all Levels", "Unlocks all levels to be selectable from the menu" )
	AddButtonEventHandler( button, UIE_CLICK, UnlockLevelsDialog )

	button = Hud_GetChild( menu, "BtnCKassist" )
	SetupButton( button, "Crouch Kick Assist", "Adds an 8 ms Buffer to your jump and crouch inputs.\nPressing both Jump and Crouch up to 8 ms apart from each other will register both inputs at the same time\nThe combined input will be registered at the time of your second input" )
	AddButtonEventHandler( button, UIE_CLICK, CKassistDialog )

	button = Hud_GetChild( menu, "BtnTASMode" )
	SetupButton( button, "TAS Mode", "NOT LEADERBOARD LEGAL!\n\nChanges your game settings to be TAS compatible\n- Disables load audio fade\n- Changes your binds to be TAS compatible (check the Key Bindings Menu to see what changed)" )
	AddButtonEventHandler( button, UIE_CLICK, TASModeDialog )

	AddEventHandlerToButtonClass( menu, "RuiFooterButtonClass", UIE_GET_FOCUS, FooterButton_Focused )

	AddMenuFooterOption( menu, BUTTON_A, "#A_BUTTON_SELECT" )
	AddMenuFooterOption( menu, BUTTON_B, "#B_BUTTON_BACK", "#BACK" )
}

void function SRMMenableSpeedo() {setSRMMsetting(0, 1)}
void function SRMMdisableSpeedo() {setSRMMsetting(0, 0)}
void function SpeedometerEnableDialog(var button)
{
	DialogData dialogData
	dialogData.header = "Speedometer"
	dialogData.message = "Do you want to enable or disable the speedometer?"

	AddDialogButton( dialogData, "Enable", SRMMenableSpeedo )
	AddDialogButton( dialogData, "Disable", SRMMdisableSpeedo )

	OpenDialog( dialogData )
}

void function SRMMspeedoIncludeZ() {setSRMMsetting(1, 0)}
void function SRMMspeedoExcludeZ() {setSRMMsetting(1, 1)}
void function SpeedometerIncludeZDialog(var button)
{
	DialogData dialogData
	dialogData.header = "Include Z Axis"
	dialogData.message = "Do you want the speedometer to include the vertical axis?"

	AddDialogButton( dialogData, "Include", SRMMspeedoIncludeZ )
	AddDialogButton( dialogData, "Don't Include", SRMMspeedoExcludeZ )

	OpenDialog( dialogData )
}

void function SRMMspeedoEnableFadeout() {setSRMMsetting(2, 0)}
void function SRMMspeedoDisableFadeout() {setSRMMsetting(2, 1)}
void function SpeedometerFadeoutDialog(var button)
{
	DialogData dialogData
	dialogData.header = "Fadeout"
	dialogData.message = "Do you want to enable or disable the speedometer fadeout?"

	AddDialogButton( dialogData, "Enable", SRMMspeedoEnableFadeout )
	AddDialogButton( dialogData, "Disable", SRMMspeedoDisableFadeout )

	OpenDialog( dialogData )
}

void function ResetHelmetsDialog(var button)
{
	DialogData dialogData
	dialogData.header = "Reset Helmets"
	dialogData.message = "Are you sure you want to reset all helmets?"

	AddDialogButton( dialogData, "#YES", ResetCollectiblesProgress_All )
	AddDialogButton( dialogData, "#NO" )

	OpenDialog( dialogData )
}

void function UnlockAllLevels() {SetConVarInt("sp_unlockedMission", 9)}
void function UnlockLevelsDialog(var button)
{
	DialogData dialogData
	dialogData.header = "Unlock Levels"
	dialogData.message = "Are you sure you want to unlock all levels?"

	AddDialogButton( dialogData, "#YES", UnlockAllLevels )
	AddDialogButton( dialogData, "#NO" )

	OpenDialog( dialogData )
}

void function EnableCKassist() {setSRMMsetting(4, 1)}
void function DisableCKassist() {setSRMMsetting(4, 0)}
void function CKassistDialog(var button)
{
	DialogData dialogData
	dialogData.header = "Crouch Kick Assist"
	dialogData.message = "Do you want to enable or disable Crouch Kick Assist?"

	AddDialogButton( dialogData, "Enable", EnableCKassist )
	AddDialogButton( dialogData, "Disable", DisableCKassist )

	OpenDialog( dialogData )
}

void function TASModeDialog(var button)
{
	DialogData dialogData
	dialogData.header = "TAS Mode"
	dialogData.message = "Do you want to enable or disable TAS Mode?"

	AddDialogButton( dialogData, "Enable", EnableTASMode )
	AddDialogButton( dialogData, "Disable", DisableTASMode )

	OpenDialog( dialogData )
}

void function EnableTASMode()
{
	setSRMMsetting(3, 1)
	// audio fade on load
	SetConVarFloat("miles_map_begin_fade_time", 0)
	SetConVarFloat("miles_map_begin_silence_time", 0)
	// input prevention on load
	SetConVarFloat("player_respawnInputDebounceDuration", 0)

	SetConVarInt("sv_cheats", 1)
}

void function DisableTASMode()
{
	setSRMMsetting(3, 0)
	// revert to default values
	SetConVarFloat("miles_map_begin_fade_time", 1.5)
	SetConVarFloat("miles_map_begin_silence_time", 0.5)
	SetConVarFloat("player_respawnInputDebounceDuration", 0.5)
	SetConVarInt("sv_cheats", 0)
}

void function OnOpenExtrasMenu()
{
	UI_SetPresentationType( ePresentationType.NO_MODELS )

}

void function OnCloseExtrasMenu()
{
	SavePlayerSettings()
}

void function SetupButton( var button, string buttonText, string description )
{
	SetButtonRuiText( button, buttonText )
	file.buttonDescriptions[ button ] <- description
	AddButtonEventHandler( button, UIE_GET_FOCUS, Button_Focused )
}

void function Button_Focused( var button )
{
	string description = file.buttonDescriptions[ button ]
	SetElementsTextByClassname( file.menu, "MenuItemDescriptionClass", description )
}

void function FooterButton_Focused( var button )
{
	SetElementsTextByClassname( file.menu, "MenuItemDescriptionClass", "" )
}

bool function getSRMMsetting(int i) {
	if ((GetConVarInt("voice_forcemicrecord") & (1 << i)) > 0) {
		return true
	}
	return false
}

void function setSRMMsetting(int i, int value) {
	int settings = GetConVarInt("voice_forcemicrecord")
	if (value == 1) {
		// set bit at position i to 1
		SetConVarInt("voice_forcemicrecord", settings | (1 << i))
	} else if (value == 0) {
		// set bit at position i to 0
		SetConVarInt("voice_forcemicrecord", settings & ~(1 << i))
	} else return
}

void function toggleSRMMsetting(int i) {
	int settings = GetConVarInt("voice_forcemicrecord")
	SetConVarInt("voice_forcemicrecord", settings ^ (1 << i))
}