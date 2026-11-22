// Entry of the game code library (runtime/gamelib.h), compiled into it with the generated code: the function
// tables, found by name when the app loads the library. Everything else in it stays hidden.
#include "gamelib.h"

extern const uint64_t main_fn_addrs[], sdk_fn_addrs[], main_fn_count, sdk_fn_count;
extern const GuestFn main_fn_ptrs[], sdk_fn_ptrs[];
#ifdef GAMELIB_SUBSDK0  // (a game with a third module, Kirby's subsdk0: the Makefile sets it when it was generated)
extern const uint64_t subsdk0_fn_addrs[], subsdk0_fn_count;
extern const GuestFn subsdk0_fn_ptrs[];
#define SUB subsdk0_fn_addrs, subsdk0_fn_ptrs, &subsdk0_fn_count
#else
static const uint64_t no_fns = 0;
#define SUB NULL, NULL, &no_fns
#endif

GAMELIB_EXPORT const GameLibTables *lasgame_tables(void) {
    static const GameLibTables t = {GAMELIB_ABI,  GAMELIB_ID,   main_fn_addrs,  sdk_fn_addrs,
                                    main_fn_ptrs, sdk_fn_ptrs,  &main_fn_count, &sdk_fn_count,
                                    SUB};
    return &t;
}
