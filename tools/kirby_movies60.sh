#!/bin/bash
# Kirby: 60 fps copies of the game's 30 fps movies, motion-interpolated (FFmpeg minterpolate), into
# <data root>/cache/movie60/ - runtime/hle/movie.c plays them instead of the originals. Made from the player's own
# extracted game files, on their machine, once. minterpolate runs on one core: each movie is cut into JOBS parts
# (default: half the cores) made at the same time, then joined, with the original's audio.
#   tools/kirby_movies60.sh <romfs>/mov/Product <data root>
set -e
src=${1:?romfs movie folder}
out=${2:?data root}/cache/movie60
jobs=${JOBS:-$(( $(nproc) / 2 ))}
[ "$jobs" -lt 1 ] && jobs=1
mkdir -p "$out"
for f in "$src"/*.mp4; do
    n=$(basename "$f" .mp4)
    [ -f "$out/$n.mp4" ] && { echo "$n: done already"; continue; }
    dur=$(ffprobe -v error -show_entries format=duration -of default=nw=1:nk=1 "$f")
    part=$(awk -v d="$dur" -v j="$jobs" 'BEGIN { printf "%.3f", d / j }')
    tmp="$out/$n.tmp"
    rm -rf "$tmp"; mkdir -p "$tmp"
    echo "$n: ${dur}s in $jobs parts, interpolating to 60 fps..."
    for ((i = 0; i < jobs; i++)); do
        ss=$(awk -v p="$part" -v i="$i" 'BEGIN { printf "%.3f", p * i }')
        # (scd=fdiff: a cut is not morphed; aobmc + bidirectional search: the least warping of minterpolate's modes)
        # (each part reads half a second more - its last in-betweens need the next frame - and is cut to its exact
        # length: parts a few frames short each put the picture ahead of the sound, 0.3 s by the end)
        ffmpeg -v error -y -ss "$ss" -t "$(awk -v p="$part" 'BEGIN { print p + 0.5 }')" -i "$f" -an \
            -vf "minterpolate=fps=60:mi_mode=mci:mc_mode=aobmc:me_mode=bidir:vsbmc=1:scd=fdiff" \
            -t "$part" -c:v libx264 -crf 16 -preset medium -pix_fmt yuv420p "$tmp/p$i.mp4" &
    done
    wait
    for ((i = 0; i < jobs; i++)); do echo "file 'p$i.mp4'"; done > "$tmp/list.txt"
    ffmpeg -v error -y -f concat -safe 0 -i "$tmp/list.txt" -i "$f" -map 0:v -map 1:a? -c copy -movflags +faststart \
        "$tmp/joined.mp4"
    mv "$tmp/joined.mp4" "$out/$n.mp4"
    rm -rf "$tmp"
done
