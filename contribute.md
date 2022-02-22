<p align="left"><img src="https://raw.githubusercontent.com/zweek/TF2SR-Menu-Mod/main/assets/logo.png" alt="Titanfall 2 - Enhanced Menu Mod" /></p>

# Contributing

## Things you'll need

* [RSPNVPK](https://github.com/taskinoz/RSPNVPK) (to build the `.vpk` files)
* [Visual Studio](https://visualstudio.microsoft.com/downloads/) (to build the `.dll` file)
* [7-zip](https://www.7-zip.org/) (Optional, to make release builds through the batch files)

## Setting up the VPK build

Once you have RSPNVPK installed somewhere, I'd recommend adding that folder to your PATH so that the command line can access the `RSPNVPK.exe` from anywhere and you can simply run the `.bat` file to build the vpk's

Create a folder called `dir` in the main source code directory right next to `src` and `dllmod`. This is where you want to put the `_dir.vpk` file that you'll be repacking with the files located within `src`. In this case that file is `englishclient_frontend.bsp.pak000_dir.vpk`. If you have a vanilla install of Titanfall 2, you can just grab that file from your `Titanfall2/vpk` folder.

When running the `.bat` file, it will create a new `export/vpk` folder structure. There it will drop the newly built .vpk files

You can also run the batch file with extra arguments like `test` and `release` and it'll do all of the grunt work for you when you want to put the files in your game or create a zipped folder containing all the necessary files for a release.

If you want to use these arguments, make sure that when using `test`, the line copying the vpk folder into the game files should point to your actual game directory. You may need to edit that line for it to point to the right place. 
When using `release`, you'll also want to make sure you have 7-zip downloaded and added to your PATH

## Setting up the DLL build

Ok so this part is a bit scuffed because you need to set a bunch of obscure settings in Visual Studio, and you can thank Fzzy for that :] (here is [Fzzy's repo with his list of settings you need to set](https://github.com/Fzzy2j/FzzyMod), but I'll try by best to list them here aswell lol)

When opening the .sln file inside `dllmod`, you'll want to Right click -> Properties on the **Project** from within the Solution explorer.

Here, you want to set these settings:
* Advanced -> Character Set = Use Multi-Byte Character Set
* C/C++ -> Precompiled Headers -> Precompiled Header = Not Using Precompiled Headers
* Linker -> Input > Module Definition File = midimap.def

Once that's set, close the Properties window and Right click on the Project in the Solution explorer again, then go to Build Dependencies -> Build Customizations and check `masm`

To build the .dll, you should be able to hit `Ctrl + B`, which should then also drop the built .dll file in the main `export` folder. And because it's Visual Studio it'll probably also create a bunch of garbage files you don't want in there.

(in case the version matters, which I hope it doesn't, I'm using Visual Studio 2019)

## Actually contributing to the repo

just make a pull request :] or submit an issue :]