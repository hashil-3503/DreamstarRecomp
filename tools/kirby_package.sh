#!/bin/bash
# Kirby: a ready-to-play folder for this machine (personal use, not a release: MSYS2's FFmpeg is a GPL build).
# Usage (MSYS2 UCRT64, from recomp/): tools/kirby_package.sh <out dir> <game dir> <data dir>
#   <out dir>: the program (a release exe - no console window, log in <data dir>/lasrecomp.log - plus the game code
#   lasgame.dll and the DLLs they load); <game dir>: the exefs/romfs folder; <data dir>: saves, settings and caches
#   (data_root.txt points there). Also writes "Kirby Star Allies.lnk" in <out dir>, starting the game folder.
# Never puts game files in <out dir>. Build the game code first (work/build.sh: build/lasgame.dll).
set -e
out=$1 game=$2 data=$3
[ -n "$data" ] || { echo "usage: $0 <out dir> <game dir> <data dir>"; exit 1; }
[ -f build/lasgame.dll ] || { echo "build/lasgame.dll missing: build the game code first"; exit 1; }
B=build/kirby-release
make -j"$(nproc)" B=$B RELEASE=1 $B/lasrecomp.exe
mkdir -p "$out"
cp $B/lasrecomp.exe "$out/"  # (name kept: lasgame.dll imports from lasrecomp.exe)
cp build/lasgame.dll build/lasgame.dll.id "$out/"
cp third_party/fonts/Baloo2/Baloo2-SemiBold.ttf third_party/fonts/DejaVu/DejaVuSans.ttf \
   third_party/fonts/DejaVu/DejaVuSans-Bold.ttf docs/icon/icon_1024.png "$out/"
ldd $B/lasrecomp.exe build/lasgame.dll | awk '/ucrt64/ && !/vulkan-1/ {print $3}' | sort -u | xargs -I{} cp -u {} "$out/"
cygpath -w "$data" > "$out/data_root.txt"
w=$(cygpath -w "$out") g=$(cygpath -w "$game")
powershell.exe -NoProfile -Command "\$s = (New-Object -ComObject WScript.Shell).CreateShortcut('$w\\Kirby Star Allies.lnk'); \
  \$s.TargetPath = '$w\\lasrecomp.exe'; \$s.Arguments = '\"$g\"'; \$s.WorkingDirectory = '$w'; \$s.Save()"
echo "ready: $out"
