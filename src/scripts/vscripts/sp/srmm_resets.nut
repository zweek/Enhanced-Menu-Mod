// untyped
global function SRMM_ResetsInit

void function SRMM_ResetsInit ( entity player )
{
    AddClientCommandCallback( "sr_reset_il", Pressed_ResetIL )
    AddClientCommandCallback( "sr_reset_anypercent", Pressed_ResetAnyPercent )
    AddClientCommandCallback( "sr_reset_allhelmets", Pressed_ResetAllHelmets )
    AddClientCommandCallback( "resethelmets", Pressed_ResetCollectibles )

    // aliases to shorten commands since there's a limit to the string length (hr = helmet reset, fgr = full game reset, lp = load prompt)
    // ClientCommand("alias hr \"sp_unlocks_level_0 0;sp_unlocks_level_1 0;sp_unlocks_level_2 0;sp_unlocks_level_3 0;sp_unlocks_level_4 0;sp_unlocks_level_5 0;sp_unlocks_level_6 0;sp_unlocks_level_7 0;sp_unlocks_level_8 0;sp_unlocks_level_9 0;sp_unlocks_level_10 0;sp_unlocks_level_11 0;sp_unlocks_level_12 0;sp_unlocks_level_13 0;sp_unlocks_level_14 0\"")
    ClientCommand( player, "alias fgr \"sp_startpoint 0;map sp_training;sp_difficulty 0;sv_cheats 0;\"" )
    ClientCommand( player, "alias lp \"set_loading_progress_detente #INTROSCREEN_HINT_PC #INTROSCREEN_HINT_CONSOLE\"" )
}

bool function Pressed_ResetIL ( entity player, array<string> args )
{
    if ( !SRMM_getSetting(SRMM_settings.practiceMode) ) {
        SetConVarInt( "sv_cheats", 0 )
    }
    ClientCommand( player, "restart; lp" )
    return true
}

bool function Pressed_ResetAnyPercent ( entity player, array<string> args )
{
    ClientCommand( player, "fgr; lp" )
    SRMM_setSetting( SRMM_settings.practiceMode, false )
	SetConVarFloat("player_respawnInputDebounceDuration", 0.5)
    return true
}

bool function Pressed_ResetAllHelmets ( entity player, array<string> args )
{
    ResetCollectiblesProgress_All()
    ClientCommand( player, "fgr; lp" )
    SRMM_setSetting( SRMM_settings.practiceMode, false )
	SetConVarFloat("player_respawnInputDebounceDuration", 0.5)
    return true
}

bool function Pressed_ResetCollectibles ( entity player, array<string> args )
{
    ResetCollectiblesProgress_All()
    return true
}