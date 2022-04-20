global function SRMM_InitENCWarpsMenu

array<string> startPointNames = [
	"Start", "Post Lecture Room", "Pre Helmet Grab", "Give BT Helmet", "Exposition Dump", "Stalker Lobby", "Bridge",
	"Start", "Hallway 1", "Button 1", "Elevator", "Anderson 1", "Anderson 2", "Hellroom Entry", "Hellroom Double Wall", "Shaft Drop", "Anderson 3",
	"Start", "Lobby Exit", "Terminal", "Pre Timestop", "Scan Ark", "Outro Cutscene"
	]
	
void function SRMM_InitENCWarpsMenu() {
	SRMM_InitWarpsMenu("SRMM_ENCWarpsMenu", startPointNames, LoadENCWarp)
}