global function SRMM_InfoHUD_Init

int displayLines = 3 // maximum number of lines that can be displayed

global struct InfoDisplay
{
    var infoTitle
}

struct SRMM_ConVarInfo
{
    string infoName
    float defautValue
    float value
    bool isModded
}

struct SRMM_SettingInfo
{
    string infoName
    int settingInt
    bool isModded
    int displayPriority
}

enum SRMM_settingDisplayPriority
{
    // speedmod,
    // TASmode,
}

array<SRMM_ConVarInfo> SRMM_ConVarInfos = []
array<SRMM_SettingInfo> SRMM_SettingInfos = []
array<InfoDisplay> InfoDisplays = []
array<InfoDisplay> CKF_infoDisplay = []

void function SRMM_InfoHUD_Init()
{
    RegisterConVar("sv_cheats", 0)
    RegisterConVar("host_timescale", 1)
    RegisterConVar("player_respawnInputDebounceDuration", 0.5)
    // RegisterSetting("TAS", SRMM_settings.TASmode, SRMM_settingDisplayPriority.TASmode)
    // RegisterSetting("speedmod", SRMM_settings.enableSpeedmod, SRMM_settingDisplayPriority.speedmod)

    for (int i = 0; i < displayLines; i++)
    {
        InfoDisplays.append(CreateInfoDisplay())
    }
    CKF_infoDisplay.append(CreateCKFInfoDisplay())
    thread SRMM_InfoHUD_Thread()
}

void function SRMM_InfoHUD_Thread()
{
    while (true)
    {
        WaitFrame()

        // setting display
        bool isSettingModded = false
        int highestPriority = 0
        int highestPriorityPos = 0

        UpdateModdedSettings()
        for (int i = 0; i < SRMM_SettingInfos.len(); i++)
        {
            if (SRMM_SettingInfos[i].isModded)
            {
                highestPriority = SRMM_SettingInfos[i].displayPriority
                highestPriorityPos = i
                isSettingModded = true
                break;
            }
        }

        for (int i = highestPriorityPos + 1; i < SRMM_SettingInfos.len(); i++)
        {
            if (!SRMM_SettingInfos[i].isModded) continue;
            if (highestPriority < SRMM_SettingInfos[i].displayPriority) 
            {
                highestPriority = SRMM_SettingInfos[i].displayPriority
                highestPriorityPos = i
            } 
        }
        
        if (isSettingModded)
        {
            SetInfoName(InfoDisplays[0], SRMM_SettingInfos[highestPriorityPos].infoName)
            SetHudPos(InfoDisplays[0], 0)
            for (int i = 1; i < displayLines; i++)
            {
                SetInfoName(InfoDisplays[i], "")
            }
        }
        else
        {

            // ConVar display
            UpdateModdedConVars()
            int slot = 0
            for (int i = 0; i < displayLines; i++)
            {
                if (SRMM_ConVarInfos[i].isModded) {
                    SetInfoName(InfoDisplays[slot], SRMM_ConVarInfos[i].infoName + " " + SRMM_ConVarInfos[i].value.tostring())
                    SetHudPos(InfoDisplays[i], i)
                    slot++
                }
            }
            for (int i = displayLines - 1; i >= slot; i--)
            {
                SetInfoName(InfoDisplays[i], "")
            }

            // Crouch kick fix display
            if (SRMM_getSetting(SRMM_settings.CKfix)) {
                SetInfoName(CKF_infoDisplay[0], "CKF")
            } else {
                SetInfoName(CKF_infoDisplay[0], "")
            }

        }
        
    }
}

void function RegisterConVar(string infoName, float defautValue)
{
    SRMM_ConVarInfo info

    info.infoName = infoName
    info.value = GetConVarFloat(infoName)
    info.defautValue = defautValue
    info.isModded = info.value != info.defautValue

    SRMM_ConVarInfos.append(info)
}

void function RegisterSetting(string infoName, int SRMM_setting, int displayPriority)
{
    SRMM_SettingInfo info

    info.infoName = infoName
    info.settingInt = SRMM_setting
    info.isModded = SRMM_getSetting(SRMM_setting)
    info.displayPriority = displayPriority

    SRMM_SettingInfos.append(info)
}

void function UpdateModdedConVars()
{
    for(int i = 0; i < SRMM_ConVarInfos.len(); i++)
    {
        SRMM_ConVarInfo ConVar = SRMM_ConVarInfos[i]
        ConVar.value = GetConVarFloat(ConVar.infoName)
        ConVar.isModded = ConVar.value != ConVar.defautValue
    }
}

void function UpdateModdedSettings()
{
    for (int i = 0; i < SRMM_SettingInfos.len(); i++)
    {
        SRMM_SettingInfo setting = SRMM_SettingInfos[i]
        setting.isModded = SRMM_getSetting(setting.settingInt)
    }
}

void function SetInfoName(InfoDisplay display, string name)
{
    RuiSetString( display.infoTitle, "msgText", name )
}

void function SetHudPos(InfoDisplay display, int line) {
    float showposOffset = 0.0
    bool isShowposOffsetActive = false
    float aspectRatio = GetScreenSize()[0] / GetScreenSize()[1]
    
    if (GetConVarInt("cl_showpos") > 0) {
        isShowposOffsetActive = true
        showposOffset += 0.05
    } 
    if (GetConVarInt("cl_showfps") > 1) {
        isShowposOffsetActive = true
        showposOffset += 0.01
    } 
    if (isShowposOffsetActive) showposOffset += 0.03

    // scale vertical position with screen size since cl_showpos scales badly
    showposOffset *= 3 - GetScreenSize()[1] / 540
    RuiSetFloat2( display.infoTitle, "msgPos", <0.01, 0.04*line - 0.04 + aspectRatio/90 + showposOffset, 0.0> )
    // actual scaling of the position seems to be <1.0, 0.9>
}

InfoDisplay function CreateInfoDisplay()
{
    InfoDisplay display

    var rui
    rui = CreateFullscreenRui( $"ui/cockpit_console_text_top_left.rpak" )
    RuiSetString( rui, "msgText", "" )
    RuiSetInt( rui, "maxLines", 1 )
    RuiSetInt( rui, "lineNum", 1 )
    RuiSetFloat( rui, "msgFontSize", 40.0 )
    RuiSetFloat( rui, "msgAlpha", 0.7 )
    RuiSetFloat3( rui, "msgColor", <1.0, 1.0, 1.0> )
    display.infoTitle = rui

    return display
}

InfoDisplay function CreateCKFInfoDisplay()
{
    InfoDisplay display
    var rui
    rui = CreateCockpitRui( $"ui/cockpit_console_text_top_left.rpak" )
    RuiSetFloat2( rui, "msgPos", <0.15, 0.86, 0.0> )
    RuiSetString( rui, "msgText", "CKF" )
    RuiSetFloat( rui, "msgFontSize", 35.0 )
    RuiSetFloat( rui, "msgAlpha", 0.7 )
    RuiSetFloat3( rui, "msgColor", <1.0, 1.0, 1.0> )
    display.infoTitle = rui

    return display
}