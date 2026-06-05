# Build RealRTCW natively on macOS (Apple Silicon)

A native arm64 build of RealRTCW for modern macOS. Tested on macOS 26.5
(Tahoe) on M1 / M2 / M3; should also work on later Apple Silicon and on
recent Intel Macs (the same Homebrew toolchain handles both arches — only
the `ARCH=` flag changes).

There is no Apple-Silicon build of RealRTCW on ModDB or the upstream
repository: the bundled `make-macosx-ub.sh` targets gcc-4.0 and the
10.5/10.6 SDKs and has not been updated for Apple Silicon. This file
documents the minimal set of changes needed to get a clean native build
on a current macOS install.

## 1. Prerequisites

Xcode is not required. The build only needs the Command Line Tools and a
handful of Homebrew libraries.

```bash
xcode-select --install   # if you don't already have CLT installed

brew install pkgconf sdl3 ffmpeg freetype jpeg opus opusfile libogg libvorbis
```

(`pkgconf` provides the `pkg-config` binary the Makefile calls. `sdl3` is
required because RealRTCW upstream migrated from SDL2 to SDL3 — the
bundled `code/libs/macosx/libSDL2-2.0.0.dylib` predates that migration
and no longer satisfies symbols like `SDL_PutAudioStreamData` or
`SDL_UpdateGamepads`.)

## 2. Build

From the repo root:

```bash
make ARCH=arm64 USE_INTERNAL_LIBS=0
```

`USE_INTERNAL_LIBS=0` tells the Makefile to link against Homebrew's
zlib / freetype / jpeg / opus / ogg / vorbis instead of the in-tree
vendored copies. The vendored `zlib-1.2.11` and `freetype-2.9` were
written before C23 and no longer compile cleanly under clang 21 (the
stock CLT compiler in late-2025 macOS releases). This is also what
MacSourcePorts use for their iortcw build.

For an Intel Mac, replace `ARCH=arm64` with `ARCH=x86_64`.

The build deposits these artefacts under
`build/release-darwin-<arch>-nosteam/`:

* `RealRTCW.arm64` — main executable
* `renderer_sp_opengl1_arm64.dylib` — OpenGL 1 renderer (Apple's GL on
  Apple Silicon is GL 2.1 backed by Metal, which is plenty for this
  renderer)
* `main/qagame.sp.arm64.dylib`
* `main/cgame.sp.arm64.dylib`
* `main/ui.sp.arm64.dylib`

You will see a few link-time warnings of the form:

```
ld: warning: building for macOS-11.0, but linking with dylib
    '/opt/homebrew/opt/ffmpeg/lib/libavcodec.62.dylib'
    which was built for newer version 26.0
```

These are cosmetic — Homebrew ffmpeg is compiled against a newer minimum
SDK than RealRTCW's `-mmacosx-version-min=11.0`. Linker still resolves
all symbols.

## 3. Game data

RealRTCW needs both the original RTCW pak files **and** the RealRTCW mod
data (pak files, `realrtcwdefault.cfg`, cinematic videos). Both are
distributed via Steam:

| Steam AppID | Title | Required for |
|-------------|-------|--------------|
| 9010 | Return to Castle Wolfenstein | `pak0.pk3`, `sp_pak1.pk3`, etc. |
| 1379630 | RealRTCW | mod paks, `realrtcwdefault.cfg`, `video/` |

Both depots ship only Windows binaries on Steam; we force the Windows
platform on steamcmd. The .dll / .exe files inside those depots aren't
needed — we only consume their data files.

You can use `scripts/mac/install-rtcw-steamcmd.sh` to fetch RTCW and
symlink its paks into the homepath the native build looks at. Then run
the same `steamcmd` for AppID 1379630 to fetch RealRTCW's mod data and
symlink the rest:

```bash
brew install --cask steamcmd

scripts/mac/install-rtcw-steamcmd.sh <your_steam_login>
# steamcmd will prompt for password + Steam Guard the first time.

# Now grab the RealRTCW mod data (free Steam game; requires you to own RTCW).
mkdir -p ~/Games/RealRTCW-data
steamcmd +@sSteamCmdForcePlatformType windows \
         +force_install_dir ~/Games/RealRTCW-data \
         +login <your_steam_login> \
         +app_update 1379630 validate \
         +quit

# Symlink RealRTCW mod paks, configs, and cinematic videos:
SRC=~/Games/RealRTCW-data/Main
DST="$HOME/Library/Application Support/RealRTCW/main"
mkdir -p "$DST"
for f in "$SRC"/*.pk3 "$SRC"/*.cfg; do ln -sfh "$f" "$DST/$(basename "$f")"; done
ln -sfh "$SRC/video" "$DST/video"
```

After this, `~/Library/Application Support/RealRTCW/main/` should
contain `pak0.pk3` + `sp_pak1..4.pk3` (vanilla RTCW), all
`z_realrtcw_*.pk3` (mod content), `realrtcwdefault.cfg`, `autoexec.cfg`,
and `video/` (cinematics).

## 4. Run

```bash
./build/release-darwin-arm64-nosteam/RealRTCW.arm64
```

The engine opens a fullscreen SDL3/Cocoa window at native desktop
resolution, initialises Apple's GL-on-Metal driver, and drops you into
the RealRTCW main menu.

## What the macOS patches actually do

Three minimal changes against upstream:

1. **`Makefile`** — wire `ffmpeg` into the `darwin` block via
   `pkg-config`. Upstream only adds `libavcodec/libavformat/libavutil/
   libswscale/libswresample` flags in the Linux block, so on macOS
   `cl_cin.c` failed with `'libavcodec/avcodec.h' file not found` even
   with ffmpeg installed.

2. **`Makefile`** — replace the darwin SDL block. The stale `-framework
   SDL2` (and bundled `libSDL2-2.0.0.dylib`) cannot satisfy upstream's
   new SDL3 audio/gamepad API calls. Now links Homebrew SDL3 via
   `pkg-config sdl3`.

3. **`code/game/g_main.c`** — fix a stray
   `#include "../../steam/steam.h"` (every other file in `code/game/`
   correctly uses `"../steam/steam.h"`). This is an upstream typo
   that happens to compile under the maintainer's Windows toolchain.

That is the complete patch set. No engine code, no platform layer, no
renderer code was changed.
