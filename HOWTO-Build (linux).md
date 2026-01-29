# How to Build on Linux

Requires FFmpeg 8 and at the time of writing only Arch Linux has it.


## Prerequisites

### Arch Linux

Install required packages:
```bash
sudo pacman -S git base-devel ffmpeg
```


## Pull Source

```bash
git clone https://github.com/wolfetplayer/RealRTCW
cd RealRTCW
# Switch to tag 5.3 to match 5.3 files found on moddb
git checkout tags/5.3
```


## Build

Run `make` from RealRTCW directory.

Look within build directory structure for output files(x86_64 architecture as example):
```
main\cgame.sp.x86_64.so
main\qagame.sp.x86_64.so
main\ui.sp.x86_64.so
RealRTCW.x86_64
renderer_sp_opengl1_x86_64.so
```
