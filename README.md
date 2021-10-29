<p align="center" style="text-align:center"><img width="125" height="122" src="https://raw.githubusercontent.com/taskinoz/Enhanced-Menu-Mod/master/assets/icon.png" alt="Titanfall 2 - Enhanced Menu Mod" /></p>

![VPK Build Status](https://github.com/taskinoz/Enhanced-Menu-Mod/actions/workflows/buildvpks.yml/badge.svg)


# Enhanced Menu Mod (Minimal Build)
Adds more options to the settings menu, such as extra keybinds, hud enhancements and cleaner interface
All custom settings added by this mod are added into a separate 'Speedrunning' Menu, accessible either through the ingame pause menu or the controls menu

# Features
* Removed **Spotlight** and **Whats New** from the main menu
* Keybinds
  * Save/Load 3 Quicksaves
  * Individual Level & Full game resets
  * No Cutscene Saves
    * Load save 1-9 with 3 alternate saves
  * No Cutscene Helmet Saves
    * Load save 1 and 2
  * Demos
    * Record Demo incrementally
    * Record Demo
    * Stop recording Demo
    * Pause Demo
    * Resume Demo
    * Toggle Play/Pause Demo
* Speedrunning options
  * Enable/Disable Bloom
  * HUD Options
    * Bloom - `mat_disable_bloom`
    * Show FPS - `cl_showfps`
    * Show FPS Big - `showfps_enabled`
    * Show Player Position - `cl_showpos`
    * Speedometer (KPH & MPH)
  * Demos
    * Enable Demos
    * Save Demos
    * Interpolate Playback
    * Demo record rate Single Player
    * Demo record rate Multiplayer
  * Reset Helmets
* Replaced Chinese subtitles with English
* Replaced Chinese UI text with English
* Removed crashes from Dev launch arguments `-dev +developer 1`

## Installation

1) Download the pre-compiled version from the [releases](https://github.com/taskinoz/Enhanced-Menu-Mod/releases) page or [ModDB(OLD)](https://www.moddb.com/mods/enhanced-menu)

2) Backup your `englishclient_frontend.bsp.pak000_dir.vpk` and copy the 2 `.vpk` files from the downloaded zip to your `Titanfall2/vpk` folder

**The Icepick installation method has been deprecated in favour of a vpk mod**

## Screenshots

![Main Menu with custom menu video](https://raw.githubusercontent.com/taskinoz/Enhanced-Menu-Mod/master/assets/menu-main.jpg)

Main Menu with the original Titanfall 2 menu video

![Main Menu with no showcase content](https://raw.githubusercontent.com/taskinoz/Enhanced-Menu-Mod/master/assets/menu-main1.jpg)

Main Menu with the showcase removed to declutter the menu

![Cheats setting](https://raw.githubusercontent.com/taskinoz/Enhanced-Menu-Mod/master/assets/menu-cheats.jpg)

New Cheat options that can be enabled as well as the extras menu

![New custom keybinds](https://raw.githubusercontent.com/taskinoz/Enhanced-Menu-Mod/master/assets/menu-keys.jpg)

Added keybinds for speedrunners and extra commands for fun

![Advanced Look Options with values](https://raw.githubusercontent.com/taskinoz/Enhanced-Menu-Mod/master/assets/menu-advanced-look.jpg)

Shows the values for each setting on the Advanced Look Options

![New HUD options](https://raw.githubusercontent.com/taskinoz/Enhanced-Menu-Mod/master/assets/menu-hud.jpg)

Added extra HUD display options for showing FPS, Position, Velocity, and Server Tick rate

![Extras Menu](https://raw.githubusercontent.com/taskinoz/Enhanced-Menu-Mod/master/assets/menu-extras.jpg)

Added a menu for enabling and changing demo settings for recording single and multiplayer demos as well as turning of wallrunning and enabling titan jumping.
