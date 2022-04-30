global function SRMM_InitITAWarpsMenu

array<string> startPointNames = [
	"Start", "Big Drop", "Vibe Check", "Vibe Check 2", "Post Door Fight",
	"Start", "Post Chute Drop", "Platform 1", "Platform 2", "Platform 3", "Platform 4", "Autoscroller", "Dome Assembly", "Dome Fight",
	"Start", "Cylinder", "Caverns", "Post Caverns", "Prowler Slope"
	]
		
void function SRMM_InitITAWarpsMenu() {
	SRMM_InitWarpsMenu("SRMM_ITAWarpsMenu", startPointNames, LoadITAWarp)
}