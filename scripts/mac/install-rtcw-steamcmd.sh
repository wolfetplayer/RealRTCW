#!/usr/bin/env bash
# Install Return to Castle Wolfenstein on macOS via steamcmd, then link its
# pak files into RealRTCW's homepath so the native arm64 build can find them.
#
# RTCW on Steam ships only a Windows depot; we force the Windows platform on
# steamcmd to download it. We only need the pak0.pk3 / sp_pak*.pk3 data files,
# which are content-only and platform-agnostic.
set -euo pipefail

STEAM_LOGIN="${STEAM_LOGIN:-${1:-}}"
INSTALL_DIR="${INSTALL_DIR:-${2:-$HOME/Games/RTCW}}"
APPID="${APPID:-9010}"  # 9010 = Return to Castle Wolfenstein

# Where the native macOS RealRTCW binary looks for `main/*.pk3`. See FS_Startup
# output: ~/Library/Application Support/RealRTCW/main is checked first.
HOMEPATH_MAIN="${HOMEPATH_MAIN:-$HOME/Library/Application Support/RealRTCW/main}"

usage() {
  cat <<USAGE >&2
Usage: STEAM_LOGIN=<login> [INSTALL_DIR=<path>] $0
   or: $0 <login> [<install_dir>]

Environment variables:
  STEAM_LOGIN    Your Steam account name (you must own RTCW, appid 9010).
  INSTALL_DIR    Where steamcmd installs RTCW. Default: \$HOME/Games/RTCW.
  HOMEPATH_MAIN  Where to symlink the .pk3 files for RealRTCW to find them.
                 Default: \$HOME/Library/Application Support/RealRTCW/main.

Notes:
  * Steam Guard: steamcmd will prompt for the code on first login;
    subsequent runs reuse the saved credentials.
  * +@sSteamCmdForcePlatformType windows is mandatory and must come
    before +login - RTCW has no native macOS depot on Steam.
USAGE
}

if [[ -z "$STEAM_LOGIN" ]]; then
  usage
  exit 1
fi

if ! command -v steamcmd >/dev/null 2>&1; then
  echo "ERROR: steamcmd not found in PATH." >&2
  echo "       Install with: brew install --cask steamcmd" >&2
  exit 1
fi

mkdir -p "$INSTALL_DIR"

echo "==> Installing appid $APPID into $INSTALL_DIR as Steam user '$STEAM_LOGIN'"

steamcmd \
  +@sSteamCmdForcePlatformType windows \
  +force_install_dir "$INSTALL_DIR" \
  +login "$STEAM_LOGIN" \
  +app_update "$APPID" validate \
  +quit

# RTCW's Steam depot lays out paks under <install>/Main/ (capital M).
MAIN_SRC="$INSTALL_DIR/Main"
if [[ ! -d "$MAIN_SRC" ]]; then
  echo "==> Install finished but $MAIN_SRC was not found. Layout may have changed." >&2
  exit 1
fi

if [[ ! -f "$MAIN_SRC/pak0.pk3" ]]; then
  echo "==> $MAIN_SRC exists but pak0.pk3 is missing. Likely a partial install." >&2
  exit 1
fi

echo "==> Linking $MAIN_SRC/*.pk3 -> $HOMEPATH_MAIN/"
mkdir -p "$HOMEPATH_MAIN"
for pk3 in "$MAIN_SRC"/*.pk3; do
  ln -sfh "$pk3" "$HOMEPATH_MAIN/$(basename "$pk3")"
done

echo
echo "==> Done. RealRTCW will find these on next launch:"
ls -l "$HOMEPATH_MAIN" | sed 's/^/    /'
echo
echo "==> Run:"
echo "    ./build/release-darwin-arm64-nosteam/RealRTCW.arm64"
