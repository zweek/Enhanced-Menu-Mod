global function SRMM_InitBNRWarpsMenu

array<string> startPointNames = ["Start", "Tone Pickup", "Post Control Room", "Post Sniper Canal", "Post Grunt Hallways", "BT Reunion", "Sludge Fight", "Kane"]

void function SRMM_InitBNRWarpsMenu() {
	SRMM_InitWarpsMenu("SRMM_BNRWarpsMenu", startPointNames, LoadBNRWarp)
}