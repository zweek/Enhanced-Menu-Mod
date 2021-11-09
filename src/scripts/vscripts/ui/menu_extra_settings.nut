global function InitExtrasMenu

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
	
	SetupButton( Hud_GetChild( menu, "SwitchEnableSpeedometer" ), "Speedometer", "Enables a speedometer in single player." )
	SetupButton( Hud_GetChild( menu, "SwitchShowFps" ), "Show FPS", "Shows an overlay with FPS and server tickrate\n\nTop-right: Displays the FPS and server tickrate in the Top-right hand side of the screen\n\nTop-left: Displays the FPS and server tickrate in the Top-left hand side of the screen\n\nServer: Displays only the server tickrate\n\nMinimal: Displays a smaller FPS and tickrate display on the top left hand side of the screen" )
	SetupButton( Hud_GetChild( menu, "SwitchShowFpsBig" ), "Show Large FPS", "FPS: Shows a large overlay with FPS and server tickrate\n\nFPS/Graph: Shows a large FPS overlay and performance graph" )
	SetupButton( Hud_GetChild( menu, "SwitchShowPos" ), "Show Positional Information", "Player Position: Shows position, angle and velocity from the player model\n\nCamera Position: Shows position, angle and velocity from the player camera" )

	SetupButton( Hud_GetChild( menu, "SwitchEnableDemos" ), "Enable Demos", "Enable recording demos (must be set true before loading a map)." )
	SetupButton( Hud_GetChild( menu, "SwitchDemosWrite" ), "Save Demos", "Demos write to a local file when recording a demo." )
	SetupButton( Hud_GetChild( menu, "SwitchDemosInterpolate" ), "Interpolate Playback", "Do view interpolation during dem playback." )
	SetupButton( Hud_GetChild( menu, "SwitchDemosUpdateRateSp" ), "Demo record rate Single Player", "Change the tick recording rate in Single Player." )
	SetupButton( Hud_GetChild( menu, "SwitchDemosUpdateRateMp" ), "Demo record rate Multiplayer", "Change the tick recording rate in Multiplayer." )
	SetupButton( Hud_GetChild( menu, "SwitchDemosAutorecord" ), "Auto Record", "Automatically record multiplayer matches as demos." )

	SetupButton( Hud_GetChild( menu, "SwitchEnableCheats" ), "Enable Cheats", "Sets the sv_cheats console variable.\nEnables use of host_timescale and velocity saves" )
	
	button = Hud_GetChild( menu, "BtnMouseKeyboardBindings" )
	SetupButton( button, "Key Bindings", "Key bindings for speedrun related actions" )
	AddButtonEventHandler( button, UIE_CLICK, AdvanceMenuEventHandler( GetMenu( "MouseKeyboardBindingsMenu" ) ) )

	button = Hud_GetChild( menu, "BtnResetHelmets" )
	SetupButton( button, "Reset Helmets", "Reset every helmet collectible to be uncollected" )
	AddButtonEventHandler( button, UIE_CLICK, ResetHelmetsDialog )

	button = Hud_GetChild( menu, "BtnUnlockLevels" )
	SetupButton( button, "Unlock all Levels", "Unlocks all levels to be selectable from the menu" )
	AddButtonEventHandler( button, UIE_CLICK, UnlockLevelsDialog )
	
	AddEventHandlerToButtonClass( menu, "RuiFooterButtonClass", UIE_GET_FOCUS, FooterButton_Focused )

	AddMenuFooterOption( menu, BUTTON_A, "#A_BUTTON_SELECT" )
	AddMenuFooterOption( menu, BUTTON_B, "#B_BUTTON_BACK", "#BACK" )
}

void function ResetHelmetsDialog( var button )
{
	DialogData dialogData
	dialogData.header = "Reset Helmets"
	dialogData.message = "Are you sure you want to reset all helmets?"

	AddDialogButton( dialogData, "#YES", ResetCollectiblesProgress_All )
	AddDialogButton( dialogData, "#NO" )

	OpenDialog( dialogData )
}

void function UnlockLevelsDialog( var button )
{
	DialogData dialogData
	dialogData.header = "Unlock Levels"
	dialogData.message = "Are you sure you want to unlock all levels?"

	AddDialogButton( dialogData, "#YES", UnlockAllLevels )
	AddDialogButton( dialogData, "#NO" )

	OpenDialog( dialogData )
}

void function UnlockAllLevels() {
	SetConVarInt("sp_unlockedMission", 9)
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
