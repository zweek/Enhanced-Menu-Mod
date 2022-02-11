global function getSRMMsetting
global function setSRMMsetting
global function toggleSRMMsetting

// aliases for srmm settings
// enableSpeedometer = 1, speedometerIncludeZ = 2, ...
global enum SRMM_settings {
    enableSpeedometer,
    speedometerIncludeZ,
    speedometerFadeout,
    TASmode,
    CKfix
}

bool function getSRMMsetting(int i) {
	if ((GetConVarInt("voice_forcemicrecord") & (1 << i)) > 0) {
		return true
	}
	return false
}

void function setSRMMsetting(int i, int value) {
	int settings = GetConVarInt("voice_forcemicrecord")
	if (value == 1) {
		// set bit at position i to 1
		SetConVarInt("voice_forcemicrecord", settings | (1 << i))
	} else if (value == 0) {
		// set bit at position i to 0
		SetConVarInt("voice_forcemicrecord", settings & ~(1 << i))
	} else return
}

void function toggleSRMMsetting(int i) {
	int settings = GetConVarInt("voice_forcemicrecord")
	SetConVarInt("voice_forcemicrecord", settings ^ (1 << i))
}