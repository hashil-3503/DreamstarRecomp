# Mods

Mods are loose files in the `mods` folder of the data folder (next to `larecomp.ini`), one folder per mod:

```
mods/
  my_mod/
    romfs/...            replaces the game file with the same path
    textures/<hash>.png  replaces one texture layer (HD texture pack)
```

- Mods load in alphabetical order. A later mod wins over an earlier one.
- A folder whose name starts with `_` or `.` is turned off.
- Changes are read at start-up.

## Texture packs

`<hash>` is 16 hex digits naming one layer of one texture. To find them, start the game with
`LARECOMP_TEX_DUMP=1`: each texture layer the game loads is written to `texdump/<hash>.png` in the data folder.
Copy the ones you want into `mods/<your mod>/textures/`, edit them, and save them at any size (2x or 4x for an HD pack).
Keep the alpha channel: some textures use it. Normal maps and other data textures dump as their raw channels.

All layers of one texture are scaled to the largest replacement's size. Vulkan only.
