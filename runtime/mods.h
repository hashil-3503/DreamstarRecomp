// Mods in loose files (runtime/mods.c): <data root>/mods/<mod>/romfs/... replaces the game's own RomFS files of the
// same path, <data root>/mods/<mod>/textures/<hash>.png replaces a texture (an HD texture pack). Mods load in
// alphabetical order; a later one wins over an earlier one. A folder whose name starts with "_" or "." is off.
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// The host file replacing RomFS file `path` (relative, "a/b.dat"), or false.
bool mods_romfs_override(const char *path, char *out, size_t cap);
// The PNG replacing the texture whose data hashes to `hash` (tex_hash_full of its whole data), or NULL.
const char *mods_texture(uint64_t hash);
int mods_texture_count(void);  // (replacement textures in all mods)
// LARECOMP_TEX_DUMP=1 (pack makers): where each texture's first level is written as <hash>.png (<data>/texdump), or NULL
const char *mods_texture_dump_dir(void);
