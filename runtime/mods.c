// Mods in loose files: RomFS replacements and HD texture packs (mods.h). The folders are read once, at the first use.
#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "mods.h"
#include "paths.h"
#include "rt.h"

enum { MODS_MAX = 64 };
static char *g_mods[MODS_MAX];  // (their folders, alphabetical)
static int g_nmods;
typedef struct { uint64_t hash; int order; char *path; } TexRepl;  // (order: the later mod's first)
static TexRepl *g_tex;
static int g_ntex;
static pthread_once_t g_once = PTHREAD_ONCE_INIT;

static int str_cmp(const void *a, const void *b) { return strcmp(*(char *const *)a, *(char *const *)b); }
static int tex_cmp(const void *a, const void *b) {
    const TexRepl *x = a, *y = b;
    return x->hash != y->hash ? (x->hash < y->hash ? -1 : 1) : x->order - y->order;
}

static void scan_textures(const char *mod) {
    char dir[1200];
    snprintf(dir, sizeof dir, "%s/textures", mod);
    DIR *d = opendir(dir);
    if (!d) return;
    int cap = g_ntex;
    for (struct dirent *e; (e = readdir(d));) {
        char *end;
        uint64_t h = strtoull(e->d_name, &end, 16);
        if (end != e->d_name + 16 || strcmp(end, ".png")) continue;
        if (g_ntex == cap) { cap = cap ? cap * 2 : 256; g_tex = realloc(g_tex, sizeof *g_tex * cap); }
        size_t n = strlen(dir) + strlen(e->d_name) + 2;
        g_tex[g_ntex].hash = h;
        g_tex[g_ntex].order = g_ntex;
        g_tex[g_ntex].path = malloc(n);
        snprintf(g_tex[g_ntex].path, n, "%s/%s", dir, e->d_name);
        g_ntex++;
    }
    closedir(d);
}

static void mods_init(void) {
    char root[1100];
    snprintf(root, sizeof root, "%s/mods", rt_data_root());
    DIR *d = opendir(root);
    if (!d) return;
    for (struct dirent *e; (e = readdir(d)) && g_nmods < MODS_MAX;) {
        if (e->d_name[0] == '.' || e->d_name[0] == '_') continue;
        char p[1200];
        struct stat sb;
        snprintf(p, sizeof p, "%s/%s", root, e->d_name);
        if (stat(p, &sb) || !S_ISDIR(sb.st_mode)) continue;
        g_mods[g_nmods++] = strdup(p);
    }
    closedir(d);
    qsort(g_mods, g_nmods, sizeof *g_mods, str_cmp);
    for (int i = g_nmods - 1; i >= 0; i--) scan_textures(g_mods[i]);  // (later mods first: lower order)
    qsort(g_tex, g_ntex, sizeof *g_tex, tex_cmp);
    for (int i = 0; i < g_nmods; i++) LOG("[mods] %s\n", g_mods[i]);
    if (g_nmods) LOG("[mods] %d mods, %d replacement textures\n", g_nmods, g_ntex);
}

bool mods_romfs_override(const char *path, char *out, size_t cap) {
    pthread_once(&g_once, mods_init);
    for (int i = g_nmods - 1; i >= 0; i--) {
        snprintf(out, cap, "%s/romfs/%s", g_mods[i], path);
        struct stat sb;
        if (stat(out, &sb) == 0 && S_ISREG(sb.st_mode)) return true;
    }
    return false;
}

const char *mods_texture(uint64_t hash) {
    pthread_once(&g_once, mods_init);
    if (!g_ntex) return NULL;
    int lo = 0, hi = g_ntex;  // (the first of equal hashes: the latest mod's)
    while (lo < hi) {
        int m = (lo + hi) / 2;
        if (g_tex[m].hash < hash) lo = m + 1;
        else hi = m;
    }
    return lo < g_ntex && g_tex[lo].hash == hash ? g_tex[lo].path : NULL;
}

int mods_texture_count(void) {
    pthread_once(&g_once, mods_init);
    return g_ntex;
}

const char *mods_texture_dump_dir(void) {
    static char dir[1100];
    static int on = -1;
    if (on < 0) {
        const char *e = getenv("LARECOMP_TEX_DUMP");
        on = e && atoi(e);
        if (on) {
            snprintf(dir, sizeof dir, "%s/texdump", rt_data_root());
            mkdir(dir, 0755);
            LOG("[mods] dumping textures to %s\n", dir);
        }
    }
    return on ? dir : NULL;
}
