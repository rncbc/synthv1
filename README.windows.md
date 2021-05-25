# Install SynthV1 on Windows

By default, SynthV1 is for Linux only. But now LV2 is also cross-platform, and some DAWs like REAPER start supporting it. So it is with SynthV1.

[TOC]

## Prerequisites

- Msys2 environment
- CMake GUI

Assumes that you've installed Msys2 into `C:\msys64`.

## Run MinGW64 shell

Launch Msys2 MinGW64 shell. Go to `C:\msys64\`, then run `mingw64.exe`.

All the commands below will be run here.

**NOTE:** Do not switch to Msys shell, as tools in it are POSIX-compatible. All binaries in this guide built within Msys shell won't run on native Win32 environment!

## Install dependencies

Install required packages via Pacman:

```bash
# Install build dependencies
pacman -Sy mingw-w64-x86_64-gcc make mingw-w64-x86_64-cmake git mingw-w64-x86_64-python mingw-w64-x86_64-waf

# Install Qt 6
pacman -S mingw-w64-x86_64-qt6-base
```

> **Note:** You can use Qt5 instead. Simply run:
>
> ```bash
> pacman -S mingw-w64-x86_64-qt5
> ```
> 
> But notice that **Qt5 support will be deprecated! Use Qt6 instead.**

## Install LV2 SDK

Msys2 doesn't provide LV2 SDK. You have to build yourself.

I have made a `PKGBUILD` to generate a LV2 install package for Msys2.

1. Fetch my hand-made `PKGBUILD` into an empty directory.

```bash
git clone https://github.com/anclark/msys2-lv2-pkgbuild ~/msys2-lv2-pkgbuild/
```

2. Build package.

```bash
cd ~/msys2-lv2-pkgbuild/
makepkg -f
```

When done, you'll get a package file: ```mingw-w64-x86_64-lv2-1.18.2-4-x86_64.pkg.tar.zst```.

3. Install package.

```bash
pacman -U mingw-w64-x86_64-lv2-1.18.2-4-x86_64.pkg.tar.zst
```

4. Copy LV2 bundles. Simply copy the whole `C:\msys64\opt\LV2` into `C:\Program Files\Common Files\`. 

    DAWs will search `C:\Program Files\Common Files\LV2\` for LV2 plugins.

## Install static Qt6

**It's highly recommended that you build a statically-linked SynthV1.** Since you won't need to ship Qt library files with your distribution, it would be much more stable and portable. 

Now let's build a static version of Qt6 for Msys2. By default Qt6 is also unavailable, you need to build it from scratch as well.

1. Fetch my hand-made `PKGBUILD` into an empty directory.

```bash
git clone https://github.com/AnClark/msys2-qt6base-static ~/msys2-qt6base-static
```

2. Build package.

```bash
cd ~/msys2-qt6base-static/
makepkg -f
```

When done, you'll get a package file: ```mingw-w64-x86_64-qtbase6-static-6.1.0-x86_64.pkg.tar.zst```.

3. Install package.

```bash
pacman -U mingw-w64-x86_64-qtbase6-static-6.1.0-x86_64.pkg.tar.zst
```

Finally, Qt6 base static library will be installed into `/opt/qtbase6-static`.

## Build SynthV1 - Statically linking (Recommended)

When Qt6 static library is OK, just build it!

Assume that you've already unpacked source code into `/c/Sources/synthv1` (aka. `C:\Sources\synthv1` on native Win32).

1. Keep in Msys2 MinGW64 shell. 

2. Configure Qt6 static environment. Including this script will configure some necessary environment variables to let CMake choose static version of Qt6 library.

```bash
source /opt/qtbase6-static/6.1.0/bin/qtbase6-static-env.sh
```

3. CMake configure:

```bash
cd /c/Sources/synthv1
cmake -S . -B build/ -G "MSYS Makefiles"
```

4. After configuration, start building:

```bash
cd build/
make
```

You will get `libsynthv1_lv2.dll` in `src/` subdirectory.

## Build SynthV1 - Link against shared Qt library

You can also use your shared Qt library provided by Msys2. It would be easier to build or debug, but **you may meet bugs on some DAWs like REAPER** (for example, when you add a SynthV1 instance to REAPER's FX chain, then remove and re-add it, your REAPER will crash).

Assume that you've already unpacked source code into `C:\Sources\synthv1`.

1. Launch CMake GUI, then specify work paths:

- Source (where is the source code): `C:/Sources/synthv1`
- Build (where to build the binaries): `C:/Sources/synthv1/build`

2. Click "Configure", then choose **"MSYS Makefiles"** as generator, and select "Use default native compilers". Click "Finish".

3. You may encount an error soon, because Qt 6 is not enabled by default. 

    Simply check `CONFIG_QT6` in the parameter list shown in CMake GUI, then click "Configure" again.

4. When CMake informs "`Configuring done`", click "Generate". It will generate an `Makefile` in `C:/Sources/synthv1/build`.

5. Go back to Msys2 MinGW64 shell, then run:

```bash
cd C:/Sources/synthv1/build
make
```

You will get `libsynthv1_lv2.dll` in `src/` subdirectory.

## Install SynthV1

Open Msys2 MinGW64 shell with Administration permission (right click `mingw64.exe`, then choose "Run as administrator"), then run:

```bash
cd C:/Sources/synthv1/build
make install
```

SynthV1 will be automatically installed into LV2 path (Default: `C:\Program Files\Common Files\LV2\`).

If you have trouble running shell with elevated permission, you can try:

```bash
cd C:/Sources/synthv1/build
mkdir _install_files
make DESTDIR=_install_files/ install
```

then copy all folders in `_install_files/` directly into your system drive (e.g. `C:\`).

Finally, open your DAW (e.g. REAPER), scan for new plugins. You will find SynthV1 in "LV2i" category.

## Known issues

- Just as I've mentioned above, avoid using shared Qt library. Use static build instead.
