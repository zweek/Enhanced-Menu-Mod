global function ClTimer_Init
global function SRMM_SetTime
global function ServerCallback_FPS_Test

const RUI_TEXT_CENTER = $"ui/cockpit_console_text_center.rpak"

int seconds = 0
int microSeconds = 0
var timerRUI = null

void function ClTimer_Init()
{
    timerRUI = RuiCreate( RUI_TEXT_CENTER, clGlobal.topoFullScreen, RUI_DRAW_HUD, RUI_SORT_SCREENFADE + 150 ) // btw, texts like this appear streched when playing in non-16:9 aspect ratios, pls fix
	RuiSetInt( timerRUI, "maxLines", 2 )
    RuiSetInt( timerRUI, "lineNum", 0 )
    RuiSetFloat2( timerRUI, "msgPos", <0.0, 0.45, 0> ) 
    RuiSetFloat3( timerRUI, "msgColor", <0.8, 0.8, 0.8> )
    RuiSetString( timerRUI, "msgText", "Timer real" ) 
    RuiSetFloat( timerRUI, "msgFontSize", 36.0 )
    RuiSetFloat( timerRUI, "msgAlpha", 0.9 )
    RuiSetFloat( timerRUI, "thicken", 0.0 )

    thread Timer()
}

void function Timer()
{
    while (true)
    {
        float startTime = Time()
        wait 0 // waits one real-time frame, not in-game frame, somehow.

        RunUIScript("SRMM_SetIsPaused", Time() - startTime < 0.00001) // if in-game time hasn't changed, but we did wait a frame, game is paused.
    }
}

void function SRMM_SetTime(int s, int ms)
{
    if (timerRUI != null)
    {
        seconds = s
        microSeconds = ms
        if (seconds >= 3600)
            RuiSetString( timerRUI, "msgText", format("%i:%02i:%02i.%03i", seconds / 3600, seconds / 60 % 60, seconds % 60, microSeconds / 1000) ) 
        else RuiSetString( timerRUI, "msgText", format("%02i:%02i.%03i", seconds / 60, seconds % 60, microSeconds / 1000) ) 
    }
}

// pre-existing remote func available in all SP maps
void function ServerCallback_FPS_Test(int s, int ms)
{
    printt("\n\n\n\nLoaded time from server:", s, ms)
    RunUIScript("SRMM_LoadTimeFromSave", s, ms)
}