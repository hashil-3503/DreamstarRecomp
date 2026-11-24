#!/bin/bash
# The Dreamstar Recomp release zip (MSYS2 UCRT64, from recomp/): `make dist` (release exe on LGPL FFmpeg, zig, the
# game code builder - no game code and no game files), then renamed and given Dreamstar's RecompLauncher profile.
# Needs: make deps-windows, tools/fetch_zig.sh. Output: build/dist/DreamstarRecomp-<version>-windows-x64.zip
set -e
v=${1:?usage: $0 <version>}
make dist
d=build/dist
rm -rf "$d/DreamstarRecomp"
mv "$d/LAS Recomp" "$d/DreamstarRecomp"
cp tools/recomplauncher_dreamstar.json "$d/DreamstarRecomp/recomplauncher.json"
cp README.md "$d/DreamstarRecomp/README.md"
! ls "$d/DreamstarRecomp" | grep -iE '^lasgame|\.nso$|romfs|exefs'  # (never any game code or game file)
rm -f "$d"/LAS\ Recomp-*.zip
cd "$d" && /c/Windows/System32/tar.exe -a -cf "DreamstarRecomp-$v-windows-x64.zip" DreamstarRecomp
echo "$d/DreamstarRecomp-$v-windows-x64.zip"
