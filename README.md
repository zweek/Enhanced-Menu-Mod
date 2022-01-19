<p align="left"><img src="https://raw.githubusercontent.com/zweek/TF2SR-Menu-Mod/main/assets/logo.png" alt="Titanfall 2 - Enhanced Menu Mod" /></p>

[![current release](https://img.shields.io/github/v/release/zweek/TF2SR-Menu-Mod?color=34ffcd&style=flat-square)](https://github.com/zweek/TF2SR-Menu-Mod/releases)
[![EMM](https://img.shields.io/badge/taskinoz-Enhanced--Menu--Mod-00a080?style=flat-square&logo=github)](https://github.com/taskinoz/Enhanced-Menu-Mod)
[![FzzyMod](https://img.shields.io/badge/Fzzy2j-FzzyMod-00a080?style=flat-square&logo=github)](https://github.com/Fzzy2j/FzzyMod)

This is a forked version of taskinoz's Enhanced Menu Mod geared specifically toward speedrunners and TASers. I cut a few features that were not relevant for runners, and added a bunch more specifically for speedrunning to make runner's lives a bit easier.

[contribute? :\]](https://github.com/zweek/TF2SR-Menu-Mod/blob/main/contribute.md)

# Features
**Settings**
* Bloom - `mat_disable_bloom`
* Speedometer (KPH & MPH)
  * Enable/Disable Fadeout
  * Enable/Disable Vertical Axis
* Show FPS - `cl_showfps`
* Show FPS (large display) - `showfps_enabled`
* Show Player Position - `cl_showpos`
* Cheats toggle - `sv_cheats`
* Toggle to enable/disable multiplayer
* Button to Reset Helmets
* Button to unlock all levels
* Dedicated TAS mode
* Demos

**Bindings**
* Binds for Individual Level & Full Game Resets
* Save/Load 3 Quicksaves
* NCS Saves 1-9
* NCS Helmet Saves 2 & 5
* Demo Binds
* `host_timescale` `5`, `0.2` & `0.05`

**Other features**
* Remove clutter from the main menu
* Info Hud that shows `sv_cheats` and other ConVars or `TAS`
* Separate releases for ENG and CHI text for the chinese version of the game
* Removed crashes from Dev launch arguments `-dev +developer 1`

# Installation

1) Download the latest release from the releases page
2) (Optional, but recommended) Backup your `englishclient_frontend.bsp.pak000_dir.vpk` in `Titanfall2/vpk`
3) Drag all contents of the downloaded .zip into your `Titanfall2` folder

## Uninstalling

1) Delete `midimap.dll` in the main `Titanfall2` folder
2) Delete `client_frontend.bsp.pak000_228.vpk` in `Titanfall2/vpk`
3) Replace `englishclient_frontend.bsp.pak000_dir.vpk` in `Titanfall2/vpk` with the backup file you've (hopefully) made previously
4) If not, you'll have to verify the game files through Origin/Steam or reinstall the game

## Screenshots

**Main Menu**

![image](https://raw.githubusercontent.com/zweek/TF2SR-Menu-Mod/main/assets/screenshot_mainmenu.png)

**Settings Menu**

![image](https://raw.githubusercontent.com/zweek/TF2SR-Menu-Mod/main/assets/screenshot_settings.png)
