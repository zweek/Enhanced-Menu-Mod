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

global string srmmVersion = "SRMM v2.4.1"

bool function SRMM_getSetting( int i ) {
	if ( (GetConVarInt( "voice_forcemicrecord" ) & (1 << i)) > 0 ) {
		return true
	}
	return false
}

void function SRMM_setSetting( int i, bool enableSetting ) {
	int settings = GetConVarInt( "voice_forcemicrecord" )
	if ( enableSetting ) {
		// set bit at position i to 1
		SetConVarInt( "voice_forcemicrecord", settings | (1 << i) )
	} else {
		// set bit at position i to 0
		SetConVarInt( "voice_forcemicrecord", settings & ~(1 << i) )
	}
}

void function SRMM_toggleSetting( int i ) {
	int settings = GetConVarInt( "voice_forcemicrecord" )
	SetConVarInt( "voice_forcemicrecord", settings ^ (1 << i) )
}