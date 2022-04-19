global function SRMM_InitGauntletWarpsMenu
global function SRMM_InitBTWarpsMenu
global function SRMM_InitBNRWarpsMenu

struct
{
	var menu
	array<string> gauntletStartPointNames = ["Start", "Simulation Spawn", "Hallway", "Circle Room Hallway", "Gauntlet Intro", "Gauntlet Outro", "Titan Intro", "Outro Cutscene (1)", "Outro Cutscene (2)", "Gauntlet Practice Only"]
	array<string> btStartPointNames = ["Start", "Post Crashlanding", "18hr Cutscene 1", "18hr Cutscene 2", "18hr Cutscene 3", "Lastimosa dies", "Lastimosa burial", "Post Lastimosa Burial", "Circle Room", "Prowler Pad", "Bat1 Insert", "Encampment", "Post Encampment", "Bat2 Enemy Clear"]
	array<string> bnrStartPointNames = ["Start", "Tone Pickup", "Post Control Room", "Post Sniper Canal", "Post Grunt Hallways", "BT Reunion", "Sludge Fight", "Kane"]
	string levelBsp
} file

void function SRMM_InitWarpsMenu(string menu, string levelBsp, array<string> startPointNames)
{
	file.menu = GetMenu(menu)
	file.levelBsp = levelBsp
	var button

	for (int i=0; i<startPointNames.len(); i++)
	{
		button = Hud_GetChild( file.menu, "BtnWarp" + i.tostring() )
		SetButtonRuiText( button, startPointNames[i] )
		AddButtonEventHandler( button, UIE_CLICK, LoadStartpoint )
	}
}

void function LoadStartpoint(var button)
{
	SetConVarInt("sp_startpoint", Hud_GetScriptID(button).tointeger())
	SetConVarInt("sp_difficulty", 0)
	ClientCommand("map " + file.levelBsp)
}

void function SRMM_InitGauntletWarpsMenu() {
	SRMM_InitWarpsMenu("SRMM_GauntletWarpsMenu", "sp_training", file.gauntletStartPointNames)
}

void function SRMM_InitBTWarpsMenu() {
	SRMM_InitWarpsMenu("SRMM_BTWarpsMenu", "sp_crashsite", file.btStartPointNames)
}

void function SRMM_InitBNRWarpsMenu() {
	SRMM_InitWarpsMenu("SRMM_BNRWarpsMenu", "sp_sewers1", file.bnrStartPointNames)
}