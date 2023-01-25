untyped
global function SRMM_SaveStateInit

struct {
    bool savestateExists = false
    vector position
    vector angles
    vector velocity
} file

void function SRMM_SaveStateInit( entity player )
{
    AddClientCommandCallback( "createsavestate", Pressed_CreateSaveState )
    AddClientCommandCallback( "loadsavestate", Pressed_LoadSaveState )
}

bool function Pressed_CreateSaveState( entity player, array<string> args )
{
    file.position = player.GetOrigin()
    file.angles = player.EyeAngles()
    file.velocity = player.GetVelocity()
    file.savestateExists = true
    return true
}

bool function Pressed_LoadSaveState( entity player, array<string> args )
{
    if (!file.savestateExists || !SRMM_getSetting(SRMM_settings.practiceMode)) return false
    player.SetOrigin(file.position)
    player.SetAngles(file.angles)
    player.SetVelocity(file.velocity)
    return true
}