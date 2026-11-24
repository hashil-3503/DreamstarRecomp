// Runs lifted single-instruction tests; prints final state per case.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#include "cpu.h"

extern GuestFn tests[];
extern int ntests;

#define BUF 0x7200000000ull
#define BUF_SIZE 0x40000

void rt_dispatch(Cpu *c) { (void)c; }
void rt_svc(Cpu *c, uint32_t imm, uint64_t pc) { (void)c; (void)imm; (void)pc; }
void rt_brk(Cpu *c, uint32_t imm, uint64_t pc) { (void)c; (void)imm; (void)pc; abort(); }
void rt_udf(Cpu *c, uint64_t pc) { (void)c; (void)pc; abort(); }
uint64_t rt_ticks(void) { return 0; }

static void *map_file(const char *path, uint64_t addr, size_t *size) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); exit(1); }
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);
    size_t len = (*size + 0x3fff) & ~0x3fffull;
#ifdef _WIN32
    void *p = VirtualAlloc((void *)addr, len, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
    void *p = mmap((void *)addr, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_FIXED, -1, 0);
#endif
    if (p != (void *)addr) { perror("mmap"); exit(1); }
    fread(p, 1, *size, f);
    fclose(f);
    return p;
}

int main(int argc, char **argv) {
    uint64_t base = strtoull(argv[4], 0, 16);
    size_t isz, bsz;
    map_file(argv[1], base, &isz);
    map_file(argv[2], BUF, &bsz);
    uint8_t *pristine = malloc(BUF_SIZE);
    memcpy(pristine, (void *)BUF, BUF_SIZE);
    FILE *f = fopen(argv[3], "rb");
    struct { uint64_t k, pc, x[32], sp, nzcv; __uint128_t v[32]; } in;
    static Cpu c;
    while (fread(&in, sizeof in, 1, f) == 1) {
        memcpy((void *)BUF, pristine, BUF_SIZE);
        memset(&c, 0, sizeof c);
        memcpy(c.x, in.x, sizeof c.x);
        c.sp = in.sp;
        c.nzcv = in.nzcv;
        memcpy(c.v, in.v, sizeof c.v);
        c.excl_addr = ~0ull;
        tests[in.k](&c);
        uint32_t crc = 0xffffffffu;  // (zlib.crc32 on the Python side)
        for (const uint8_t *b = (const uint8_t *)BUF, *e = b + BUF_SIZE; b < e; b++) {
            crc ^= *b;
            for (int k = 0; k < 8; k++) crc = crc >> 1 ^ (0xedb88320u & -(crc & 1));
        }
        for (int i = 0; i < 31; i++) printf("%llx ", c.x[i]);
        printf("%llx %llx %llx ", c.sp, c.nzcv & 0xf0000000ull, c.pc);
        printf("%032x", ~crc);
        for (int i = 0; i < 32; i++) printf(" %016llx%016llx", (uint64_t)(c.v[i] >> 64), (uint64_t)c.v[i]);
        printf("\n");
    }
    return 0;
}
