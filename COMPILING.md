# Table of contents

* [Building International Doom on Windows](#building-international-doom-on-windows)
  * [Step 1: Setting up build environment](#step-1-setting-up-build-environment)
    * [1.a: MSYS2 environment](#1a-msys2-environment)
      * [1.a.a: MSYS2 GCC environment](#1aa-msys2-gcc-environment)
      * [1.a.b: MSYS2 Clang environment](#1ab-msys2-clang-environment)
  * [Step 2: Compiling International Doom](#step-2-compiling-international-doom)
  * [Step 3: Installing International Doom](#step-3-installing-international-doom)
  * [Step 4: Packaging International Doom](#step-4-packaging-international-doom)
* [Building International Doom on Linux](#building-international-doom-on-linux)
  * [Step 1: Setting up build environment](#step-1-setting-up-build-environment-1)
  * [Step 2: Compiling International Doom](#step-2-compiling-international-doom-1)
  * [Step 3: Installing International Doom](#step-3-installing-international-doom-1)
* [Building a DOS version of International Doom](#building-a-dos-version-of-international-doom)
  * [Step 1: Setting up build environment](#step-1-setting-up-build-environment-2)
  * [Step 2: Compiling project](#step-2-compiling-project)

# Building International Doom on Windows

## Step 1: Setting up build environment

Download source code archive of the latest release version and unpack it.
Or clone repository from Git.
The primary way of building International Doom on Windows is using an MSYS environment.

### 1.a: MSYS2 environment

Download and install [MSYS2](https://www.msys2.org/).
For configuring and building International Doom,
you **must** use the appropriate MSYS2 terminal (**mingw32.exe**, **mingw64.exe**, **clang32.exe**, **clang64.exe**)
so that the $PATH environment variable points to the proper toolchain.

### **1.a.a: MSYS2 GCC environment**

For the **mingw-w64-i686** toolchain (**32**-bit), use the **MSYS MinGW 32-bit** start menu/screen shortcut
and for the **mingw-w64-x86_64** toolchain (**64**-bit), use the **MSYS MinGW 64-bit** start menu/screen shortcut.

Open the terminal and install dependencies using the following command:
```shell
pacman -S mingw-w64-{i686,x86_64}-{pkgconf,gcc,ninja,cmake,SDL2{,_net,_mixer},libsamplerate,libpng} \
       git
```

If you want to use `MinGW Makefiles` or `MSYS Makefiles` cmake generators, you need
to install `make` by the following command:
```shell
pacman -S mingw-w64-{i686,x86_64}-make
```

If you want to run CTest tests, you must also install `gdb` by the following command:
```shell
pacman -S mingw-w64-{i686,x86_64}-gdb
```

### **1.a.b: MSYS2 Clang environment**

For the **mingw-w64-clang-i686** toolchain (**32**-bit), use the **MSYS MinGW Clang 32-bit** start menu/screen shortcut
and for the **mingw-w64-clang-x86_64** toolchain (**64**-bit), use the **MSYS MinGW Clang 64-bit** start menu/screen shortcut.

Open the terminal and install dependencies using the following command:
```shell
pacman -S mingw-w64-clang-{i686,x86_64}-{pkgconf,clang,ninja,cmake,SDL2{,_net,_mixer},libsamplerate,libpng} \
       git
```

If you want to use `MinGW Makefiles` or `MSYS Makefiles` cmake generators you need
to install `make` by the following command:
```shell
pacman -S mingw-w64-clang-{i686,x86_64}-make
```

If you want to run CTest tests, you must also install `gdb` by the following command:
```shell
pacman -S mingw-w64-clang-{i686,x86_64}-gdb
```

## Step 2: Compiling International Doom

There are several supported cmake generators: `Ninja`, `Ninja Multi-Config`, `MinGW Makefiles`, `MSYS Makefiles`.  
**Note** that `* Makefiles` generators fail to compile 32-bit builds after GCC update to version 12 for some reason.

To configure the project, use the following command:
```shell
cmake -G "Ninja" -D CMAKE_BUILD_TYPE="Release" -S . -B build
```
Available build types are `Release`, `Debug`, `RelWithDebInfo`, `MinSizeRel`.

You can enable and disable the compilation of modules by changing cmake options:
`COMPILE_DOOM`, `COMPILE_HERETIC`, `COMPILE_HEXEN`, `COMPILE_STRIFE`, `COMPILE_SETUP`
to `ON` or `OFF`.

To build the project, use the following command:
```shell
cmake --build build
```

After successful compilation, the resulting binaries can be found in the `build\src\` folder.

## Step 3: Installing International Doom

To install International Doom to `<install directory>` use the following command:
```shell
cmake --install build --prefix <install directory>
```
If you want to install only one game, you can use the following command:
```shell
cmake --install build --prefix <install directory> --component <game>
```
Available components are `doom`, `heretic`, `hexen`.

## Step 4: Packaging International Doom

To package International Doom, run the following commands:
```shell
cd build
cpack -G ZIP
```
The resulting packages of the previously compiled components and their checksums can be found in the `build` directory.

The only supported generators are [Archive Generators](https://cmake.org/cmake/help/latest/cpack_gen/archive.html).  
If you want to overwrite the package version, set `BUILD_VERSION_OVERWRITE` cmake variable at configuring time,
instead of using `-R` Cpack parameter.

It is **recommended** to configure, build and package only one component at a time selecting it with the following cmake options
by setting required one to `ON` and the rest to `OFF`:
`COMPILE_DOOM`, `COMPILE_HERETIC`, `COMPILE_HEXEN`, `COMPILE_STRIFE`.

# Building International Doom on Linux

Compiling on Linux is rather simple. 

## Step 1: Setting up build environment

First, make sure you have all the necessary dependencies for compilation.
On Ubuntu Linux, they can be installed by the following command:
```shell
sudo apt install gcc ninja-build python3 cmake \
libsdl2-dev libsdl2-mixer-dev libsdl2-net-dev libpng-dev libsamplerate-dev
```
On Fedora Linux, they can be installed by the following command:
```shell
sudo dnf install gcc ninja-build python3 cmake \
SDL2-devel SDL2_mixer-devel SDL2_net-devel libpng-devel libsamplerate-devel
```
On Manjaro Linux, they can be installed by the following command:
```shell
sudo pacman -S gcc ninja-build python3 cmake \
sdl2 sdl2_mixer sdl2_net libpng libsamplerate
```

The `gcc` compiler can de replaced with `clang`.
If you want to use `Unix Makefiles` cmake generator, you need to install `make`.
The `python3` is not mandatory, but required for generation of man pages and bash completion.
If you want to run CTest tests, you must also install `gdb`.

## Step 2: Compiling International Doom

Download source code archive of the latest release version and unpack it.
Or clone repository from Git.

There are several supported cmake generators: `Ninja`, `Ninja Multi-Config`, `Unix Makefiles`.

To configure the project, use the following command:
```shell
cmake -G "Ninja" -D CMAKE_BUILD_TYPE="Release" -S . -B build
```
Available build types are `Release`, `Debug`, `RelWithDebInfo`, `MinSizeRel`.

You can enable and disable the compilation of modules by changing cmake options:
`COMPILE_DOOM`, `COMPILE_HERETIC`, `COMPILE_HEXEN`, `COMPILE_STRIFE`, `COMPILE_SETUP`
to `ON` or `OFF`.

If you want to use a portable version, like on Windows, set the `BUILD_PORTABLE` cmake option to `ON`.
Portable version searches and stores internal resources,
config files, and savegames in the installation directory among executables.

To build the project, use the following command:
```shell
cmake --build build
```
After successful compilation, the resulting binaries can be found in the `build/src/` folder.

## Step 3: Installing International Doom

To install International Doom, use the following command:
```shell
sudo cmake --install build
```
If you want to install only one game, you can use the following command:
```shell
sudo cmake --install build --component <game>
```
Available components are `doom`, `heretic`, `hexen`. 

For the portable version, you probably want to use `--prefix <install directory>` parameter to set install directory.
Note that config files and savegames will be stored in that directory.

# Building a DOS version of International Doom

## Step 1: Setting up build environment

The primary IDE for building International Doom is Open Watcom C,
which may be downloaded on its [official site](http://www.openwatcom.org/download.php).

## Step 2: Compiling project

After installation of Open Watcom C, open its IDE and then choose menu "File" > "Open Project..."  
Choose `src_dos/rusdoom.wpj` and press 'Make target' button.

At the end of the compilation process, you will find compiled binary `rusdoom.exe` in the source code directory.

Please note: to run the compiled game executable, you will need to copy
`rusdoom.exe`, `rusdoom.wad` and `dos4gw.exe` files to your Doom directory, as well as official IWAD files.
