global function UITimer_Init
global function SRMM_SetIsPaused
global function SRMM_LoadTimeFromSave

// WHY IS THIS NOT IN CLIENT?

// doing this in UI instead of CLIENT is done because
// CLIENT simulates IN-GAME time, not real-time.
// IN-GAME time, when at a lag spike, is capped at 0.1s. UI doesn't do that.
// When paused, in-game time stops too. Not UI.
// 

// we use 2 integers instead of a simple float because we want to avoid float precision fucking up our times
int seconds = 0
int microSeconds = 0
int loadSeconds = 0
int loadMicroSeconds = 0

bool isPaused = false

void function UITimer_Init()
{
    RegisterSignal("LoadedTime")
    ClientCommand("sv_quota_stringcmdspersecond 1000")
    thread Timer()
}

void function Timer()
{
    while (true)
    {

        // not when in loading screen
        while (uiGlobal.loadingLevel != "")
        {
            //print("\n\n\n\nLOADING LEVEL")
            seconds = 0
            microSeconds = 0
            loadSeconds = 0
            loadMicroSeconds = 0
            WaitFrame()
        }
        
        // not when in menus
        //print(uiGlobal.loadedLevel)
        if (uiGlobal.loadedLevel == "")
        {
            seconds = 0
            microSeconds = 0
            loadSeconds = 0
            loadMicroSeconds = 0
            WaitSignal( uiGlobal.signalDummy, "LoadedTime" )
        }

        float startTime = Time()
        WaitFrame()
        float delta = Time() - startTime
        if (uiGlobal.activeMenu == null || !isPaused)
            delta *= GetConVarFloat( "host_timescale" )
        
        int microSecondDelta = int(delta * 1000000)

        microSeconds += microSecondDelta

        seconds += microSeconds / 1000000

        microSeconds = microSeconds % 1000000
        int totalSeconds = seconds + loadSeconds + (microSeconds + loadMicroSeconds) / 1000000
        int totalMicroSeconds = (microSeconds + loadMicroSeconds) % 1000000
        //printt(totalSeconds, totalMicroSeconds)
        //printt("TOTAL TIME:", format("%02i:%02i.%03i", totalSeconds / 60, totalSeconds % 60, totalMicroSeconds / 1000))
        //printt("TIME IN LEVEL:", format("%02i:%02i.%03i", seconds / 60, seconds % 60, microSeconds / 1000))

        //print(format("%i.%06i", seconds, microSeconds))
        // in a try/catch because I'm not bothering to check if Client VM is active lel
        try
        {
            RunClientScript("SRMM_SetTime", totalSeconds, totalMicroSeconds)
            //print("sending cc")
            ClientCommand( "time " + totalSeconds + " " + totalMicroSeconds)
        }
        catch (ex)
        {

        }
    }
}

// called from cl_timer.nut
void function SRMM_SetIsPaused(bool paused)
{
    isPaused = paused
}

void function SRMM_LoadTimeFromSave(int s, int ms)
{
    //print("HOLY FUCKING SHIT")
    loadSeconds = s
    loadMicroSeconds = ms
    Signal( uiGlobal.signalDummy, "LoadedTime" )
}