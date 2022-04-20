global function SRMM_InitTBFWarpsMenu

array<string> startPointNames = ["Start", "Drop", "Drop (Walls down)", "Door", "Elevator", "Elevator Top", "Final Stretch", "Outro Cutscene"]

void function SRMM_InitTBFWarpsMenu() {
	SRMM_InitWarpsMenu("SRMM_TBFWarpsMenu", startPointNames, LoadTBFWarp)
}