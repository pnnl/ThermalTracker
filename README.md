# ThermalTracker

**Welcome to ThermalTracker!**

ThermalTracker tracks birds and bats in thermal video.  It will process video in any standard format that was recorded with a thermal video camera<sup>1</sup>.  The output is images of the flight tracks of birds and bats in PNG format, and a comma-separated value (CSV) file listing the details of the extracted flight tracks.  For algorithm details see [Matzner, Cullinan and Duberstein 2015](http://www.sciencedirect.com/science/article/pii/S1574954115001478).

<sup>1</sup>The video must be grayscale (not false color) with the hottest temperature as white.

**Build Instructions**

The source code is organized into a library with a command line interface and a graphical user interface (GUI).  The GUI is based on Qt and is licensed under a GPL-style license because that's what Qt requires.  The library and command line interface is licensed under a BSD-style license that's more flexible.  

Requirements:
* CMake 3.0 or greater
* OpenCV 2.4
* Boost 
* Qt5

On MacOS and Linux:
1.  Create a build directory and cd into it.
2.  `cmake -DQT_VERSION_DIR=<path to Qt5 install dir>  <path to ThermalTracker src directory>`
3.  `make`

On Windows:
1.  Install [MSYS](http://downloads.sourceforge.net/mingw/MSYS-1.0.11.exe)
2.  Install [cmake](https://cmake.org/download/)
3.  Install Qt.  Be sure to enable the MinGW install option.
4.  Install [openCV](https://github.com/Itseez/opencv/archive/2.4.12.3.zip)
5.  Install [boost](https://sourceforge.net/projects/boost/) using these commands at the Windows Command Prompt:
  * `bootstrap mingw`
  * `b2 toolset=gcc --build-type=complete stage`
6.  Build ThermalTracker in MSYS:
  * Create a build directory and cd into it.
  * `cmake -G"MSYS Makefiles" -DCMAKE_PREFIX_PATH=<full path to Qt root>  \`
            `-DQT_VERSION_DIR=<full path to Qt root> -DBOOST_ROOT=<full path to boost lib dir> \`
            `-DOpenCV_DIR=<full path to compiled opencv root> ..`
  * `make`

**Getting Started**

After following the build instructions above, the executable files for the command line interface will be in the tracker subdirectory of the build directory and the GUI executable will be in the gui subdirectory.  

For examples of using the command line interface, see the shell scripts in the scripts directory.  Run the command line executables with `--help` to display a help message.  

For the GUI, start the application and select Help from the menu.  

Enjoy!


