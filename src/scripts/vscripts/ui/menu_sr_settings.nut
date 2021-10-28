
global function InitSRMenu

struct
{
	var menu
	table<var,string> buttonDescriptions
	var classicMusicSwitch
} file

void function InitSRMenu()
{
	var menu = GetMenu( "SRMenu" )
	file.menu = menu

	AddMenuEventHandler( menu, eUIEvent.MENU_OPEN, OnOpenSRMenu )
	AddMenuEventHandler( menu, eUIEvent.MENU_CLOSE, OnCloseSRMenu )

	var button

	SetupButton( Hud_GetChild( menu, "SwitchEnableDemos" ), "Enable Demos", "Enable recording demos (must be set true before loading a map)." )
	SetupButton( Hud_GetChild( menu, "SwitchDemosWrite" ), "Save Demos", "Demos write to a local file when recording a demo." )
	SetupButton( Hud_GetChild( menu, "SwitchDemosInterpolate" ), "Interpolate Playback", "Do view interpolation during dem playback." )
	SetupButton( Hud_GetChild( menu, "SwitchDemosUpdateRateSp" ), "Demo record rate Single Player", "Change the tick recording rate in Single Player." )
	SetupButton( Hud_GetChild( menu, "SwitchDemosUpdateRateMp" ), "Demo record rate Multiplayer", "Change the tick recording rate in Multiplayer." )
	SetupButton( Hud_GetChild( menu, "SwitchDemosAutorecord" ), "Auto Record", "Automatically record multiplayer matches as demos." )

	SetupButton( Hud_GetChild( menu, "SwitchBloomEnable" ), "Bloom", "Toggles the bloom to reduce brightness and glare" )

	SetupButton( Hud_GetChild( menu, "SwitchEnableSpeedometer" ), "Speedometer", "Enables a speedometer in single player." )
	SetupButton( Hud_GetChild( menu, "SwitchShowFps" ), "Show FPS", "Shows an overlay with FPS and server tickrate\n\n`1Top-right`0: Displays the FPS and server tickrate in the `1Top-right`0 hand side of the screen\n\n`1Top-left`0: Displays the FPS and server tickrate in the `1Top-left`0 hand side of the screen\n\n`1Server`0: Displays only the server tickrate\n\n`1Minimal`0: Displays a smaller FPS and tickrate display on the top left hand side of the screen" )
	SetupButton( Hud_GetChild( menu, "SwitchShowFpsBig" ), "Show Large FPS", "`1FPS`0: Shows a large overlay with FPS and server tickrate\n\n`1FPS/Graph`0: Shows a large FPS overlay and performance graph" )
	SetupButton( Hud_GetChild( menu, "SwitchShowPos" ), "Show Positional Information", "`1Player Position`0: Shows the speed, velocity and position of the player\n\n`1Camera Position`0: Shows the camera angle and player position" )

	button = Hud_GetChild( menu, "BtnMouseKeyboardBindings" )
	SetupButton( button, "Key Bindings", "Key bindings for speedrun related actions" )
	AddButtonEventHandler( button, UIE_CLICK, AdvanceMenuEventHandler( GetMenu( "MouseKeyboardBindingsMenu" ) ) )

	button = Hud_GetChild( menu, "BtnResetHelmets" )
	SetupButton( button, "Reset Helmets", "Reset every helmet collectible to be uncollected" )
	AddButtonEventHandler( button, UIE_CLICK, ResetHelmets )
	
	AddEventHandlerToButtonClass( menu, "RuiFooterButtonClass", UIE_GET_FOCUS, FooterButton_Focused )

	AddMenuFooterOption( menu, BUTTON_A, "#A_BUTTON_SELECT" )
	AddMenuFooterOption( menu, BUTTON_B, "#B_BUTTON_BACK", "#BACK" )
}

void function ResetHelmets( var button )
{
	ResetCollectiblesProgress_All()
}

void function OnOpenSRMenu()
{
	UI_SetPresentationType( ePresentationType.NO_MODELS )

}

void function OnCloseSRMenu()
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
