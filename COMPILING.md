### Building International Doom on Windows

The primary way of building International Doom on Windows is using an MSYS environment.

##### Step 1: Setting up the environment

Install [MSYS2](https://www.msys2.org/). Open the MSYS2 terminal and install dependencies using the following command:
```
pacman -S base-devel msys2-devel \
       mingw-w64-{i686,x86_64}-{toolchain,cmake,SDL2{,_net,_mixer},libsamplerate,libpng} \
       git
```

#### Step 2: Compiling International Doom

Download source code archive of latest release version or from Git repository and unpack it.

At this stage, you **must** use the appropriate MSYS2 terminal (**mingw32.exe** or **mingw64.exe**) 
so that the $PATH environment variable points to the proper toolchain. 
For the **mingw-w64-i686** toolchain (**32**-bit), use the **MSYS MinGW 32-bit** start menu/screen shortcut 
and for the **mingw-w64-x86_64** toolchain (**64**-bit), use the **MSYS MinGW 64-bit** start menu/screen shortcut.

To configure the project use the following command:
```
cmake -G "MinGW Makefiles" -D CMAKE_BUILD_TYPE="Release" -S . -B build
```
Available build types are `Release`, `Debug`, `RelWithDebInfo`, `MinSizeRel`.

You can enable and disable the compilation of modules by changing cmake options:
`COMPILE_DOOM`, `COMPILE_HERETIC`, `COMPILE_HEXEN`, `COMPILE_STRIFE`, `COMPILE_SETUP`
to `ON` or `OFF`.

To build the project use the following command:
```
cmake --build build
```

After successful compilation, the resulting binaries can be found in the `build\src\` folder.

##### Step 3: Installing International Doom

To install International Doom to `<install directory>` use the following command:
```
cmake --install build --prefix <install directory>
```
If you want to install only one game, you can use the following command:
```
cmake --install build --prefix <install directory> --component <game>
```
Available components are `doom`, `heretic`, `hexen`.

##### Step 4: Packaging International Doom

To package International Doom run following commands:
```
cd build
cpack -G ZIP
```
The resulting packages of the previously compiled components can be found in the `build` directory.

The only supported generators are [Archive Generators](https://cmake.org/cmake/help/latest/cpack_gen/archive.html).  
If you want to overwrite the package version, set `BUILD_VERSION_OVERWRITE` cmake variable at configuring time
instead of using `-R` Cpack parameter.

It is **recommended** to configure, build and package only one component at a time selecting it with the following cmake options
by setting required one to `ON` and the rest to `OFF`:
`COMPILE_DOOM`, `COMPILE_HERETIC`, `COMPILE_HEXEN`, `COMPILE_STRIFE`.

### Building International Doom on Linux

Compiling on Linux is rather simple. 

##### Step 1: Checking compilation dependencies

First, make sure you have all the necessary dependencies for compilation.
On Ubuntu Linux they can be installed by the following command:
```
sudo apt-get install gcc cmake make libsdl2-dev libsdl2-net-dev \
libsdl2-mixer-dev libpng-dev libsamplerate-dev
```
On Fedora Linux they can be installed by the following command:
```
sudo dnf install gcc cmake make SDL2-devel SDL2_mixer-devel \
SDL2_net-devel libpng-devel libsamplerate-devel
```
On Manjaro Linux they can be installed by the following command:
```
sudo pacman -S base-devel cmake sdl2 sdl2_net sdl2_mixer \
libpng libsamplerate
```

##### Step 2: Compiling International Doom

Download source code archive of latest release version or from Git repository and unpack it.

To configure the project use the following command:
```
cmake -D CMAKE_BUILD_TYPE="Release" -S . -B build
```
Available build types are `Release`, `Debug`, `RelWithDebInfo`, `MinSizeRel`.

You can enable and disable the compilation of modules by changing cmake options:
`COMPILE_DOOM`, `COMPILE_HERETIC`, `COMPILE_HEXEN`, `COMPILE_STRIFE`, `COMPILE_SETUP`
to `ON` or `OFF`.

If you want to use a portable version like on Windows, set the `BUILD_PORTABLE` cmake option to `ON`.
Portable version searches and stores internal resources, config files, and savegames in install directory among executables.

To build the project use the following command:
```
cmake --build build
```
After successful compilation, the resulting binaries can be found in the `build/src/` folder.

##### Step 3: Installing International Doom

To install International Doom use the following command:
```
cmake --install build
```
If you want to install only one game, you can use the following command:
```
cmake --install build --component <game>
```
Available components are `doom`, `heretic`, `hexen`. 

For the portable version, you probably want to use `--prefix <install directory>` key to set install directory.
Note that config files and savegames will be stored in that directory.

### Building a DOS version of International Doom

##### Step 1: Getting Open Watcom C

The primary IDE for building International Doom is Open Watcom C, which may be downloaded on it's [official site](http://www.openwatcom.org/download.php).

##### Step 2: Compiling project

After installation of Open Watcom C, open it's IDE and then choose menu File > Open Project...  
Choose `src_dos/rusdoom.wpj` and press 'Make target' button.

At the end of the compilation process, you will find compiled binary `rusdoom.exe` in the source code directory.

Please note: to run the compiled game executable you will need to copy
`rusdoom.exe`, `rusdoom.wad` and `dos4gw.exe` files to your Doom directory, as well as official IWAD files.
