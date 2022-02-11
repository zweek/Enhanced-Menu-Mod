enum SRMM_settings {
    enableSpeedometer,
    speedometerIncludeZ,
    speedometerFadeout,
    TASmode,
    CKassist
}

bool function getSRMMsetting(int i) {
	if ((GetConVarInt("voice_forcemicrecord") & (1 << i)) > 0) {
		return true
	}
	return false
}