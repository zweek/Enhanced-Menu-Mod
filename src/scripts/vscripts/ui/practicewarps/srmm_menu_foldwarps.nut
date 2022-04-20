global function SRMM_InitFoldWarpsMenu

array<string> startPointNames = ["Start", "Intro Cutscene", "Skinny Door", "Curved Hallway", "New BT", "Embark", "Big Door", "Post Archer Hallway", "Pre Slone 1", "Pre Slone 2", "Post Slone", "Ark Injector Cutscene", "Escape"]

void function SRMM_InitFoldWarpsMenu() {
	SRMM_InitWarpsMenu("SRMM_FoldWarpsMenu", startPointNames, LoadFoldWarp)
}