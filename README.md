# HxC File Selector for FlashFloppy

A modified version of the HxC project's file selector/manager for
enhanced compatibility with FlashFloppy on Amiga and Atari ST. The
FlashFloppy project should work with unmodified original HxC
selectors (AUTOBOOT.HFE) on other platforms (notably Amstrad CPC).

This software is licensed under GPLv3. See
[COPYING_FULL](/COPYING_FULL) for full copyright and license
information.

This project is cross-compiled on an x86 Ubuntu Linux system. However
other similar Linux-base systems (or a Linux virtual environment on
another OS) can likely be made to work quite easily.

## Building for Amiga

You must build and install bebbo's GCC v6 port for Amiga. This can be
done to a private path in your home directory, for example:
```
 # mkdir bebbo
 # cd bebbo
 # mkdir install
 # git clone https://github.com/bebbo/amigaos-cross-toolchain.git repo
 # cd repo
 # ./toolchain-m68k --prefix=$HOME/install build
```

Building the ADF image from the selector executable requires
bonefish's exe2adf on your path. Download it
[here](http://www.exe2adf.com).

To produce the final HFE images from ADF requires disk-analyse. Download
and build it:
```
 # git clone https://github.com/keirf/Disk-Utilities.git
 # cd Disk-Utilities
 # PREFIX=$HOME/install make && make install
```

The compiler and disk-analyse must be on your PATH when you build
the selector software:
```
 # export PATH=$HOME/install/bin:$PATH
 # export LD_LIBRARY_PATH=$HOME/install/lib:$LD_LIBRARY_PATH
```

To build the HFE image:
```
 # git clone https://github.com/keirf/HxC_FF_File_Selector.git
 # cd HxC_FF_File_Selector/amiga
 # make
```

## Building for Atari ST

You must build and install the m68k-atari-mint cross compiler, maintained
by Vincent Riviere. The simplest method is to use Miro Kropacek's automated
build scripts:
```
 # mkdir atari-mint
 # cd atari-mint
 # mkdir install
 # git clone https://github.com/mikrosk/m68k-atari-mint-build
 # cd m68k-atari-mint-build
 # git checkout 1fd98b25ceb
 # INSTALL_DIR="$HOME/install" make m68000-skip-native
```

The compiler and disk-analyse (see Amiga build instructions above) must be
on your PATH when you build the selector software:
```
 # export PATH=$HOME/install/bin:$PATH
 # export LD_LIBRARY_PATH=$HOME/install/lib:$LD_LIBRARY_PATH
```

To build the HFE image:
```
 # git clone https://github.com/keirf/HxC_FF_File_Selector.git
 # cd HxC_FF_File_Selector/atari_st
 # make
```

## Building All Targets

Install prerequisites as above. Then:
```
 # git clone https://github.com/keirf/HxC_FF_File_Selector.git
 # cd HxC_FF_File_Selector
 # make
```
