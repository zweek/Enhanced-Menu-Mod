global function SvTimer_Init
global function LoadTime
global function GetTimeArray
// since the server VM memeory gets stored in the save, these get stored too - we can abuse that to save the times inside the save file, 
// and when it loads, call a remote func to restore them.
int seconds = 0
int microSeconds = 0

void function SvTimer_Init()
{
    AddClientCommandCallback( "time", SetTime )
    if (GetLevelTransitionStruct() != null)
    {
        LevelTransitionStruct trans = expect LevelTransitionStruct( GetLevelTransitionStruct() )

        seconds = trans.ints[0]
        microSeconds = trans.ints[1]
    }
    //AddCallback_OnLoadSaveGame( OnLoadSaveGame )
}

bool function SetTime( entity player, array<string> args )
{
    if (!player.p.clientScriptInitialized)
        return true
    int s = int( args[0] )
    int ms = int( args[1] )
    seconds = s
    microSeconds = ms
    //printt(seconds, microSeconds)
    
    return true
}

int[3] function GetTimeArray()
{
    print("\n\n\nGET TIME ARRAY")
    int[3] arr

    arr[0] = seconds
    arr[1] = microSeconds
    arr[2] = -1
    return arr
}

void function LoadTime( entity player )
{
    printt("\n\n\n\nLOADED TIME", seconds, microSeconds)
    Remote_CallFunction_Replay( player, "ServerCallback_FPS_Test", seconds, microSeconds )
}