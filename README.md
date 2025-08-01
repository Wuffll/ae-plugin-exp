# After Effects Plugin

Greetings!

This repository contains files necessary to compile the plugin on Windows. The following chapters provide a detailed overview of the build process, along with potential code enhancements to support potential increased plugin complexity.

## Build steps
This repository is configured with CMake (windows only). Steps for generating project files:
1. Clone the repository
2. Execute the build.bat file
	* This will create a folder called '*build*' which will contain the project files (if you are using Visual Studio then
	       files such as .sln, .vcxproj, etc.)
3.  For Visual Studio users:
	* Open the solution file and select the '*Skeleton*' project as the startup project. Now you can build the project and depending on the selected configuration the output file should be in the '*configuration*' folder (i.e. Debug, Release)

After building the project, the output file should be in the .aex format and can be added to the After Effects Plugins folder.

If you want to clean the project from generated intermediate files, you can execute the **clean.bat** file. This will clean intermediate files and previous builds of the plugin. 
> For more details about building, please inspect the **build.bat** and **clean.bat** files.

## Code enhancements

Since this plugin requirements weren't to complex I didn't want to go too overboard with modularity, but I did want to outline potential improvements that could be implemented if the plugin’s complexity grows.

 1. Instead of only using functions such as *ApplyGainEffectFunction16* and *ApplyGainEffectFunction8*, there could be a interface class that has those functions. Then each gain effect that is implemented in the future would implement that interface and it could be easily interchangeable. So instead of just calling the function, we would store a reference to the interface class which would then point to the gain effect class that we want.

 2. CMake file could me more dynamic. What I mean by that is instead of having to define all the include directories, CMake could detect them by itself and then add them to the target directories. This is mostly because I'm a bit rusty with CMake.

 3. While there may be more efficient ways to implement this plugin, my current approach is based on the Skeleton example, as I’m still familiarizing myself with the Adobe After Effects SDK, hence the project's name. Same goes for the naming conventions, there are really different from my current setup.
