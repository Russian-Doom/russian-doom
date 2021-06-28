### Building Russian Doom on Windows

The primary way of building Russian Doom on Windows is using an MSYS environment.

##### Step 1: Setting up the environment

Install [MSYS2](https://www.msys2.org/). Open the MSYS2 terminal and install dependencies using the following command:
```
pacman -S base-devel msys2-devel \
       mingw-w64-{i686,x86_64}-{toolchain,cmake,SDL2{,_net,_mixer},libsamplerate,libpng} \
       git
```

#### Step 2: Compiling Russian Doom

Download source code archive of latest release version or from Git repository and unpack it.

At this stage, you **must** use the appropriate MSYS2 terminal (**mingw32.exe** or **mingw64.exe**) 
so that the $PATH environment variable points to the proper toolchain. 
For the **mingw-w64-i686** toolchain (**32**-bit), use the **MSYS MinGW 32-bit** start menu/screen shortcut 
and for the **mingw-w64-x86_64** toolchain (**64**-bit), use the **MSYS MinGW 64-bit** start menu/screen shortcut.

There are two options to build with: **Cmake** or **Autotools**.  
Autotools is the primary way but CMake is recommended.

##### Step 2.1: Building with CMake

To configure the project use the following command:
```
cmake -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -S . -B build
```
To build the project use the following command:
```
cmake --build build
```

After successful compilation, the resulting binaries can be found in the `build\src\` folder.

##### Step 2.2: Building with Autotools

To configure the project for 32-bit environment run
```
./autogen.sh --host=i686-w64-mingw64
```
or to configure the project for 64-bit environment run
```
./autogen.sh --host=x86_64-w64-mingw64
```
To build the project use the following command:
```
make
```
After successful compilation, the resulting binaries can be found in the `src\` folder.

##### Step 3: Installing Russian Doom

To install Russian Doom to `<install directory>` use the following command:
```
cmake --install build --prefix <install directory>
```
If you are using Autotools, you have to install Russian Doom manually. Copy built executables and `base` folder from `src\`
directory to `<install directory>` along with DLL libraries from `win_libs\<arch>\`.

### Building Russian Doom on Linux

Compiling on Linux is rather simple. 

##### Step 1: Checking compilation dependencies

First, make sure you have all the necessary dependencies for compilation.
On Ubuntu Linux they can be installed by the following command:
```
sudo apt-get install gcc cmake make automake libsdl2-dev libsdl2-net-dev
libsdl2-mixer-dev libpng-dev libsamplerate-dev
```
On Fedora Linux they can be installed by the following command:
```
sudo dnf install gcc cmake make automake SDL2-devel SDL2_mixer-devel
SDL2_net-devel libpng-devel libsamplerate-devel
```

##### Step 2: Compiling Russian Doom

Download source code archive of latest release version or from Git repository and unpack it.

There are two options to build with: **Cmake** or **Autotools**.  
Autotools is the primary way but CMake is recommended.

##### Step 2.1: Building with CMake

To configure the project use the following command:
```
cmake -DCMAKE_BUILD_TYPE=Release -DDEV_ENV=OFF -S . -B build
```
If you want to use dev builds without installing, set DEV_ENV option to `ON` and
point `<current directory>` to `build/src/`.

To build the project use the following command:
```
cmake --build build
```
After successful compilation, the resulting binaries can be found in the `build/src/` folder.

##### Step 2.2: Building with Autotools

To configure the project use the following command:
```
./autogen.sh
```
To build the project use the following command:
```
make
```
After successful compilation, the resulting binaries can be found in the `src/` directory.

##### Step 3: Installing Russian Doom

To install Russian Doom use the following command:
```
cmake --install build
```

If you are using Autotools, you have to install Russian Doom manually. Copy built executables from `build/src/`
to `/usr/local/bin/` directory. Copy content of `src/base/` folder to `/usr/local/share/russian-doom/` directory.

### Building a DOS version of Russian Doom

##### Step 1: Getting Open Watcom C

The primary IDE for building Russian Doom is Open Watcom C, which may be downloaded on it's [official site](http://www.openwatcom.org/download.php).

##### Step 2: Compiling project

After installation of Open Watcom C, open it's IDE and then choose menu File > Open Project...  
Choose `src_dos/rusdoom.wpj` and press 'Make target' button.

At the end of the compilation process, you will find compiled binary `rusdoom.exe` in the source code directory.

Please note: to run the compiled game executable you will need to copy
`rusdoom.exe`, `rusdoom.wad` and `dos4gw.exe` files to your Doom directory, as well as official IWAD files.
