# How to Build on Linux

Requires FFmpeg 8 to build and run.

RealRTCW 5.3 is the last version that does not require FFmpeg.


## Prerequisites

### Arch Linux

```bash
# Install required packages:
sudo pacman -S git base-devel ffmpeg
```


## Pull Source

```bash
git clone https://github.com/wolfetplayer/RealRTCW
cd RealRTCW
```

The current main branch may be ahead of the released mod files. For compatibility, switch to the corresponding release tag (latest at the time of writing: 5.3). You can return using `git switch -`.

```bash
# Switch to tag 5.3 to match 5.3 mod files
git checkout tags/5.3
```


## Build

```bash
# Run from RealRTCW directory
make
```

Output files are in a subdirectory under build directory.

```bash
# Output files (x86_64 architecture as example):
main/cgame.sp.x86_64.so
main/qagame.sp.x86_64.so
main/ui.sp.x86_64.so
RealRTCW.x86_64
renderer_sp_opengl1_x86_64.so
```
