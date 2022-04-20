global function SRMM_InitWarpsMenu

global function LoadGauntletWarp
global function LoadBTWarp
global function LoadBNRWarp
global function LoadITAWarp
global function LoadENCWarp
global function LoadBWarp
global function LoadTBFWarp
global function LoadArkWarp
global function LoadFoldWarp

void function SRMM_InitWarpsMenu(string menuName, array<string> startPointNames, void functionref(var) loadFunc)
{
	var menu = GetMenu(menuName)
	var button

	for (int i=0; i<startPointNames.len(); i++)
	{
		button = Hud_GetChild( menu, "BtnWarp" + i.tostring() )
		SetButtonRuiText( button, startPointNames[i] )
		AddButtonEventHandler( button, UIE_CLICK, loadFunc )
	}
}

void function GetStartpoint(var button)
{
	SetConVarInt("sp_difficulty", 0)
	SetConVarInt("sp_startpoint", Hud_GetScriptID(button).tointeger() - (Hud_GetScriptID(button).tointeger()/100) * 100)
}

void function LoadGauntletWarp(var button)
{
	GetStartpoint(button)
	ClientCommand("map sp_training")
}

void function LoadBTWarp(var button)
{
	GetStartpoint(button)
	ClientCommand("map sp_crashsite")
}

void function LoadBNRWarp(var button)
{
	GetStartpoint(button)
	ClientCommand("map sp_sewers1")
}

void function LoadITAWarp(var button)
{
	array<string> itaBsps = ["sp_boomtown_start", "sp_boomtown", "sp_boomtown_end"]
	GetStartpoint(button)
	string levelBsp = itaBsps[Hud_GetScriptID(button).tointeger()/100 - 1]
	ClientCommand("map " + levelBsp)
}

void function LoadENCWarp(var button)
{
	array<string> encBsps = ["sp_hub_timeshift", "sp_timeshift_spoke02"]
	GetStartpoint(button)
	string levelBsp = encBsps[Hud_GetScriptID(button).tointeger()/100 - 1]
	ClientCommand("map " + levelBsp)
}

void function LoadBWarp(var button)
{
	array<string> bBsps = ["sp_beacon", "sp_beacon_spoke0"]
	GetStartpoint(button)
	string levelBsp = bBsps[Hud_GetScriptID(button).tointeger()/100 - 1]
	ClientCommand("map " + levelBsp)
}

void function LoadTBFWarp(var button)
{
	GetStartpoint(button)
	ClientCommand("map sp_tday")
}

void function LoadArkWarp(var button)
{
	GetStartpoint(button)
	ClientCommand("map sp_s2s")
}

void function LoadFoldWarp(var button)
{
	GetStartpoint(button)
	ClientCommand("map sp_skyway_v1")
}