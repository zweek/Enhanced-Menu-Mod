global function SRMM_InitGauntletWarpsMenu

array<string> startPointNames = ["Start", "Simulation Spawn", "Hallway", "Circle Room Hallway", "Gauntlet Intro", "Gauntlet Outro", "Titan Intro", "Outro Cutscene (1)", "Outro Cutscene (2)", "Gauntlet Practice Only"]

void function SRMM_InitGauntletWarpsMenu() {
	SRMM_InitWarpsMenu("SRMM_GauntletWarpsMenu", startPointNames, LoadGauntletWarp)
}