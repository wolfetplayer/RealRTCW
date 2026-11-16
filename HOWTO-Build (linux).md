# How to Build on Linux

Requires FFmpeg 8 to build and run.

RealRTCW 5.3 is the last version that does not require FFmpeg.


## Prerequisites

### Arch Linux

Install required packages:
```shell
sudo pacman -S git base-devel ffmpeg
```


### Fedora 45

Required package `ffmpeg-devel` is available in RPM Fusion free repository.

Add RPM Fusion free repository:
```shell
sudo dnf install https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm
```

Install required packages:
```shell
sudo dnf install git make gcc gcc-c++ SDL3 SDL3-devel ffmpeg ffmpeg-devel --allowerasing
```

Versions prior to 5.44 need SDL2 `sdl2-compat sdl2-compat-devel`.


### Ubuntu 26.04

Install required packages:
```shell
sudo apt install git build-essential libsdl3-dev ffmpeg libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libswresample-dev
```

Versions prior to 5.44 need SDL2 `libsdl2-dev`.


## Pull Source

```shell
git clone https://github.com/wolfetplayer/RealRTCW
```

Switch to newly created directory:
```shell
cd RealRTCW
```

The current main branch may be ahead of the released mod files. For compatibility, switch to the corresponding release tag (latest at the time of writing: 5.44c). You can return using `git switch -`. Alternatively as rule of thump switch to the last commit prior to mod update on moddb `git switch --detach <commit-hash>`.

Switch to tag 5.44c to match 5.44c mod files:
```shell
git checkout tags/5.44c
```


## Build

Run from RealRTCW directory:
```shell
make
```

Output files are in a subdirectory under build directory.

```shell
# Output files (x86_64 architecture as example):
main/cgame.sp.x86_64.so
main/qagame.sp.x86_64.so
main/ui.sp.x86_64.so
RealRTCW.x86_64
renderer_sp_opengl1_x86_64.so
```
