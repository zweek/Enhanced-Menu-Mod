global function UITimer_Init
global function SRMM_SetIsPaused

// WHY IS THIS NOT IN CLIENT?

// doing this in UI instead of CLIENT is done because
// CLIENT simulates IN-GAME time, not real-time.
// IN-GAME time, when at a lag spike, is capped at 0.1s. UI doesn't do that.
// When paused, in-game time stops too. Not UI.
// 

// we use 2 integers instead of a simple float because we want to avoid float precision fucking up our times
int seconds = 0
int microSeconds = 0

bool isPaused = false

void function UITimer_Init()
{
    //print("\n\n\n\n\n\n\nPAIN")
    thread Timer()
}

void function Timer()
{
    while (true)
    {
        float startTime = Time()
        WaitFrame()
        float delta = Time() - startTime
        if (uiGlobal.activeMenu == null && !isPaused)
            delta *= GetConVarFloat( "host_timescale" )
        int microSecondDelta = int(delta * 1000000)

        microSeconds += microSecondDelta

        seconds += microSeconds / 1000000

        microSeconds = microSeconds % 1000000

        // not when in loading screen
        if (uiGlobal.loadingLevel != "")
            continue
        
        // not when in menus
        if (uiGlobal.loadedLevel == "")
            continue

        //print(format("%i.%06i", seconds, microSeconds))
        // in a try/catch because I'm not bothering to check if Client VM is active lel
        try
        {
            RunClientScript("SRMM_SetTime", seconds, microSeconds)
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