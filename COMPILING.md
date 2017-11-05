### Building Russian Doom on Windows

##### Step 1: Getting Code::Blocks and MinGW compiler

The primary IDE for building Russian Doom is Code::Blocks, which may be downloaded on it's [official site](http://www.codeblocks.org/downloads/binaries). It is recommended to download a package that includes the compiler from TDM-GCC pre-installed. Otherwise, in case you want to install compiler manually, you can find the TDM-GCC installer [here](http://tdm-gcc.tdragon.net/).

##### Step 2: Installing the Development Libraries

Next you will need development libraries for [SDL2](https://www.libsdl.org/download-2.0.php), [SDL2 Mixer](https://www.libsdl.org/projects/SDL_mixer/) and [SDL2 Net](https://www.libsdl.org/projects/SDL_net/). Pay attention, you will need MinGW libraries, not Visual C++. After you get the development libraries, extract the files and copy the 'include', 'lib' and 'bin' folders to the top of your TDM-GCC directory (i.e. C:\TDM-GCC). Also make sure to move the files from the 'include\SDL' subfolder to the main 'include' folder. At this point you should be ready to build Russian Doom.

##### Step 3: Compiling Russian Doom

In the source directory of Russian Doom, you will find the Code::Blocks files in the 'codeblocks' folder, the workspace file is called 'russian_doom.workspace'.

After you open the workspace, you can select what type of build you want to create: Debug or Release. In case you are going to debug Russian Doom, you should compile a debug build (which is quite a lot bigger than the release), otherwise, you can just compile a release build. In order to start building, click on Build and then Rebuild workspace.

At the end of the compilation process, you will find your new Russian Doom build in the 'bin' folder at the top of the source directory.

Please note: to run the compiled game executables you will need the folder 'russian' to be placed in the folder with executables, as well as SDL2.dll, SDL2_mixer.dll and SDL2_net.dll files, and official IWAD files.


### Building Russian Doom on Linux

Compiling on Linux is rather simple. 

##### Step 1: Checking compilation dependencies

First of all, make sure you have all necessary dependencies for compilation. On Debian Linux they can be installed by following command, invoked by root:

`apt-get install gcc make libsdl2-dev libsdl2-net-dev libsdl2-mixer-dev python-imaging`

##### Step 2: Compiling Russian Doom

Download source code archive of latest release version or from Git repository, unpack it and use following commands:

`./autogen.sh` <br />
`make`

After successful compilation the resulting binaries can be found in the src/ directory.

Please note: to run the compiled game executables you will need the folder 'russian' to be placed in the folder with executables, as well as official IWAD files.


### Building a DOS version of Russian Doom

##### Step 1: Getting Open Watcom C

The primary IDE for building Russian Doom is Open Watcom C, which may be downloaded on it's [official site](http://www.openwatcom.org/download.php).

##### Step 2: Compiling project

After installation of Open Watcom C, open it's IDE and then choose menu File > Open Project... 
<br />Choose 'src_dos/rusdoom.wpj' and press 'Make target' button.

At the end of the compilation process, you will find compiled binary 'rusdoom.exe' in the source code directory.

Please note: to run the compiled game executable you will need to copy 'rusdoom.exe', 'rusdoom.wad' and 'dos4gw.exe' files to your Doom directory, as well as official IWAD files.
