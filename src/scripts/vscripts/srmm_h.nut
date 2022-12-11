global function SRMM_getSetting
global function SRMM_setSetting
global function SRMM_toggleSetting

// aliases for srmm settings
// enableSpeedometer = 0, speedometerIncludeZ = 1, ...
global enum SRMM_settings {
    enableSpeedometer,
    speedometerIncludeZ,
    speedometerFadeout,
    practiceMode,
    CKfix,
}

global string srmmVersion = "SRMM v2.4"

bool function SRMM_getSetting(int i) {
	if ((GetConVarInt("voice_forcemicrecord") & (1 << i)) > 0) {
		return true
	}
	return false
}

void function SRMM_setSetting(int i, int value) {
	int settings = GetConVarInt("voice_forcemicrecord")
	if (value == 1) {
		// set bit at position i to 1
		SetConVarInt("voice_forcemicrecord", settings | (1 << i))
	} else if (value == 0) {
		// set bit at position i to 0
		SetConVarInt("voice_forcemicrecord", settings & ~(1 << i))
	} else return
}

void function SRMM_toggleSetting(int i) {
	int settings = GetConVarInt("voice_forcemicrecord")
	SetConVarInt("voice_forcemicrecord", settings ^ (1 << i))
}