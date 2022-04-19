global function SRMM_InitPracticeWarpsMenu

struct SPLevelStartStruct
{
	int levelNum
	string levelBsp
	string startPoint
	string levelId
	string levelName
	string levelDesc
	asset levelImage = $""
	bool showLions
}

struct
{
	var menu
	GridMenuData gridData
	bool isGridInitialized = false
	array<SPLevelStartStruct> mainLevels
	table< int, array<SPLevelStartStruct> > allLevels
	int lastLevelSelected = 0
	int lastLevelUnlocked = 0
	int difficulty = DIFFICULTY_NORMAL
	int selectedLevelNum = -1
	string selectedLevel = ""
	string selectedStartPoint = ""
	bool playIntro = false
	int currentBackground = 0
	bool addObjectiveReminderOnSaveLoad
	array<void functionref()> levelPartSelectFunc
	int focusedElemNum = 0
} file

void function SRMM_InitPracticeWarpsMenu()
{
	var menu = GetMenu( "SRMM_PracticeWarpsMenu" )
	file.menu = menu

	var dataTable = GetDataTable( $"datatable/sp_levels.rpak" )
	int numRows = GetDatatableRowCount( dataTable )

	for ( int i=0; i<numRows; i++ )
	{
		bool isMain = GetDataTableBool( dataTable, i, GetDataTableColumnByName( dataTable, "mainEntry" ) )
		SPLevelStartStruct data
		data.levelBsp = GetDataTableString( dataTable, i, GetDataTableColumnByName( dataTable, "level" ) )
		data.startPoint = GetDataTableString( dataTable, i, GetDataTableColumnByName( dataTable, "startPoint" ) )
		data.levelId = GetDataTableString( dataTable, i, GetDataTableColumnByName( dataTable, "levelId" ) )
		data.levelNum = GetDataTableInt( dataTable, i, GetDataTableColumnByName( dataTable, "levelNum" ) )
		data.levelName = GetDataTableString( dataTable, i, GetDataTableColumnByName( dataTable, "title" ) )
		data.levelDesc = GetDataTableString( dataTable, i, GetDataTableColumnByName( dataTable, "desc" ) )
		data.levelImage = GetDataTableAsset( dataTable, i, GetDataTableColumnByName( dataTable, "missionSelectImage" ) )
		data.showLions = GetDataTableBool( dataTable, i, GetDataTableColumnByName( dataTable, "showLions" ) )

		if ( isMain )
		{
			file.mainLevels.append( data )
		}

		if (!( data.levelNum in file.allLevels ))
		{
			file.allLevels[ data.levelNum ] <- []
		}

		file.allLevels[ data.levelNum ].append( data )
	}

	file.gridData.rows = 2
	file.gridData.columns = 5
	file.gridData.paddingVert = 5
	file.gridData.paddingHorz = 5
	file.gridData.numElements = file.mainLevels.len()
	file.gridData.pageType = eGridPageType.HORIZONTAL
	file.gridData.tileWidth = Grid_GetMaxWidthForSettings( menu, file.gridData )

	float tileHeight = ( file.gridData.tileWidth * 9.0 ) / 21.0

	file.gridData.tileHeight = minint( Grid_GetMaxHeightForSettings( menu, file.gridData ), int( tileHeight ) + 80 )
	file.gridData.initCallback = SPButtonInit
	file.gridData.buttonFadeCallback = SP_FadeDefaultElementChildren
	file.gridData.getFocusCallback = SPButton_GetFocus
	file.gridData.clickCallback = SPButton_Click

	AddMenuEventHandler( menu, eUIEvent.MENU_OPEN, OnOpenPracticeWarpsMenu )

	AddMenuFooterOption( menu, BUTTON_A, "#A_BUTTON_SELECT", "", null, IsUnlockedChapterFocused )
	AddMenuFooterOption( menu, BUTTON_B, "#B_BUTTON_BACK", "#BACK" )
}


bool function SPButtonInit( var button, int elemNum )
{
	var rui = Hud_GetRui( button )

	SPLevelStartStruct data = file.mainLevels[ elemNum ]

	asset levelImage = data.levelImage

	RuiSetImage( rui, "itemImage", levelImage )

	UpdateButtonData( button, elemNum )

	return true
}

void function UpdateButtonData( var button, int elemNum )
{
	SPLevelStartStruct data = file.mainLevels[ elemNum ]
	var rui = Hud_GetRui( button )
	string bspName = data.levelBsp
	string levelName = data.levelName

	RuiSetString( rui, "title", levelName )
	Hud_SetLocked( button, false )

	int foundLions = GetCombinedCollectiblesFoundForLevel( bspName )
	int maxLions = GetCombinedLionsInLevel( bspName )

	bool completedMaster = GetCompletedMasterForLevel( elemNum )
	RuiSetInt( rui, "blueLionCount", foundLions )
	RuiSetInt( rui, "blueLionTotal", maxLions )
	RuiSetBool( rui, "finishedMaster", completedMaster )

	if ( elemNum == 0 )
		RuiSetBool( rui, "showMaster", false )
	else
		RuiSetBool( rui, "showMaster", true )
}

void function SPButton_GetFocus( var button, int elemNum )
{
	file.focusedElemNum = elemNum

	SPLevelStartStruct data = file.mainLevels[ elemNum ]
	string levelName = data.levelName
	string desc = data.levelDesc

	file.lastLevelSelected = elemNum

	HudElem_SetText( GetMenuChild( file.menu, "ContentDescriptionTitle" ), levelName )
	HudElem_SetText( GetMenuChild( file.menu, "ContentDescription" ), desc )
}

void function SPButton_Click( var button, int elemNum )
{
	SPLevelStartStruct data = file.mainLevels[ elemNum ]
	file.selectedLevelNum = elemNum
	file.selectedLevel = data.levelBsp
	file.selectedStartPoint = data.startPoint
	file.playIntro = false

	if(elemNum == 0) AdvanceMenu( GetMenu( "SRMM_GauntletWarpsMenu" ) )
	if(elemNum == 1) AdvanceMenu( GetMenu( "SRMM_BTWarpsMenu" ) )
	if(elemNum == 2) AdvanceMenu( GetMenu( "SRMM_BNRWarpsMenu" ) )
	// if(elemNum == 3) AdvanceMenu( GetMenu( "SRMM_ITAWarpsMenu" ) )
	// if(elemNum == 4) AdvanceMenu( GetMenu( "SRMM_ENCWarpsMenu" ) )
	// if(elemNum == 5) AdvanceMenu( GetMenu( "SRMM_BWarpsMenu" ) )
	// if(elemNum == 6) AdvanceMenu( GetMenu( "SRMM_TBFWarpsMenu" ) )
	// if(elemNum == 7) AdvanceMenu( GetMenu( "SRMM_ArkWarpsMenu" ) )
	// if(elemNum == 8) AdvanceMenu( GetMenu( "SRMM_FoldWarpsMenu" ) )
}

void function OnOpenPracticeWarpsMenu()
{
	if ( !file.isGridInitialized )
	{
		GridMenuInit( file.menu, file.gridData )
		file.isGridInitialized = true
	}

	file.lastLevelUnlocked = GetLastLevelUnlocked()

	Grid_InitPage( file.menu, file.gridData )

	int levelFocus = minint( file.lastLevelUnlocked, file.lastLevelSelected )

	int row = Grid_GetRowFromElementNumber( levelFocus, file.gridData )
	int col = Grid_GetColumnFromElementNumber( levelFocus, file.gridData )
	Hud_SetFocused( Grid_GetButtonAtRowColumn( file.menu, row, col ) )
}

bool function GetCompletedMasterForLevel( int elemNum )
{
	array<SPLevelStartStruct> datas = file.allLevels[ elemNum ]
	foreach( data in datas )
	{
		if ( !GetCompletedMasterForLevelId( data.levelId ) )
		{
			return false
		}
}

	return true
}

bool function GetCompletedMasterForLevelId( string levelId )
{
	return GetCompletedDifficultyForLevelId( levelId, "sp_missionMasterCompletion" )
}

string function GetBestCompletedDifficultyForLevel( int elemNum )
{
	int lowestDifficulty = 999

	array<SPLevelStartStruct> datas = file.allLevels[ elemNum ]
	foreach( data in datas )
	{
		int d = GetBestCompletedDifficultyForLevelId( data.levelId )
		if ( d < lowestDifficulty )
			lowestDifficulty = d
	}

	switch ( lowestDifficulty )
	{
		case DIFFICULTY_MASTER:
			return "#SP_DIFFICULTY_MASTER_TITLE"
		case DIFFICULTY_HARD:
			return "#SP_DIFFICULTY_HARD_TITLE"
		case DIFFICULTY_NORMAL:
			return "#SP_DIFFICULTY_NORMAL_TITLE"
		case DIFFICULTY_EASY:
			return "#SP_DIFFICULTY_EASY_TITLE"
	}

	return ""
}

void function SP_FadeDefaultElementChildren( var elem, int fadeTarget, float fadeTime )
{

}

void function Beacon_FreeTrialOverMessage()
{
	DialogData dialogData
	dialogData.header = "#SP_TRIAL_OVER_TITLE"
	dialogData.message = "#SP_TRIAL_OVER_MSG"
	dialogData.forceChoice = true

	AddDialogButton( dialogData, "#MENU_GET_THE_FULL_GAME", SP_Trial_LaunchGamePurchase )
	AddDialogButton( dialogData, "#CANCEL_AND_QUIT_TO_MAIN_MENU", Disconnect )

	AddDialogFooter( dialogData, "#A_BUTTON_SELECT" )

	OpenDialog( dialogData )
}

bool function IsUnlockedChapterFocused()
{
	if ( file.focusedElemNum <= file.lastLevelUnlocked )
		return true

	return false
}
