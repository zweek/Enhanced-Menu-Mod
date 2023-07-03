// untyped
global function SRMM_ResetsInit

void function SRMM_ResetsInit ( entity player )
{
    AddClientCommandCallback( "sr_reset_il", Pressed_ResetIL )
    AddClientCommandCallback( "sr_reset_anypercent", Pressed_ResetAnyPercent )
    AddClientCommandCallback( "sr_reset_allhelmets", Pressed_ResetAllHelmets )
    AddClientCommandCallback( "resethelmets", Pressed_ResetCollectibles )
    AddClientCommandCallback( "sr_loadprompt", Pressed_Loadprompt )

    // full game reset
    ClientCommand( player, "alias fgr \"sp_startpoint 0;map sp_training;sp_difficulty 0;sv_cheats 0;\"" )

    // ClientCommand( player, "alias lp \"" + SRMM_MakeUnixTimestampedDetentCommand() + "\"" )
    // BAD. this gets the timestamp of when this command is sent upon initialization
}

bool function Pressed_ResetIL ( entity player, array<string> args )
{
    // TODO: if somehow possible, disable input debounce, then reenable while keeping practice mode enabled
    ClientCommand( player, "restart; sr_loadprompt" )
    return true
}

bool function Pressed_ResetAnyPercent ( entity player, array<string> args )
{
    ClientCommand( player, "fgr; sr_loadprompt" )
    SRMM_setSetting( SRMM_settings.practiceMode, false )
	SetConVarFloat("player_respawnInputDebounceDuration", 0.5)
    return true
}

bool function Pressed_ResetAllHelmets ( entity player, array<string> args )
{
    ResetCollectiblesProgress_All()
    ClientCommand( player, "fgr; sr_loadprompt" )
    SRMM_setSetting( SRMM_settings.practiceMode, false )
	SetConVarFloat("player_respawnInputDebounceDuration", 0.5)
    return true
}

bool function Pressed_ResetCollectibles ( entity player, array<string> args )
{
    ResetCollectiblesProgress_All()
    return true
}

bool function Pressed_Loadprompt ( entity player, array<string> args )
{
    
    return true
}