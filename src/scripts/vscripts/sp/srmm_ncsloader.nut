global function SRMM_NCSLoader_Init

void function SRMM_NCSLoader_Init ( entity player )
{
	AddClientCommandCallback( "loadncs1", Pressed_LoadNCS1 )
	AddClientCommandCallback( "loadncs2", Pressed_LoadNCS2 )
	AddClientCommandCallback( "loadncs3", Pressed_LoadNCS3 )
	AddClientCommandCallback( "loadncs4", Pressed_LoadNCS4 )
	AddClientCommandCallback( "loadncs5", Pressed_LoadNCS5 )
	AddClientCommandCallback( "loadncs6", Pressed_LoadNCS6 )
	AddClientCommandCallback( "loadncs7", Pressed_LoadNCS7 )
	AddClientCommandCallback( "loadncs8", Pressed_LoadNCS8 )
	AddClientCommandCallback( "loadncs9", Pressed_LoadNCS9 )
}

bool function Pressed_LoadNCS1 ( entity player, array<string> args )
{
	// check if loadprompt setting is enabled for this level
	// check which equipment option setting is enabled
	// concatenate load command with loadprompt command
	// run it
}