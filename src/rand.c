typedef struct {
    u64 fast_loop;
    u64 mix;
    u64 loop_mix;
} Biski64State;

static u64
biski64_next(Biski64State* state) {
    const u64 output = state->mix + state->loop_mix;
    const u64 old_loop_mix = state->loop_mix;

    state->loop_mix = state->fast_loop ^ state->mix;
    state->mix = rotate_left_u64(state->mix, 16) + rotate_left_u64(old_loop_mix, 40);
    state->fast_loop += 0x9999999999999999ULL; // Additive constant for the Weyl sequence.

    return output;
}

static u64
biski64_splitmix(u64* seed_state_ptr) {
    *seed_state_ptr += 0x9e3779b97f4a7c15ull;
    
    u64 z = *seed_state_ptr;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    return z ^ (z >> 31);
}

static void
biski64_warmup(Biski64State* state) {
    for (uint i = 0; i < 16; i += 1) {
        biski64_next(state);
    }
}

static void
biski64_seed(Biski64State* state, u64 seed) {
    u64 seeder_state = seed;

    state->mix       = biski64_splitmix(&seeder_state);
    state->loop_mix  = biski64_splitmix(&seeder_state);
    state->fast_loop = biski64_splitmix(&seeder_state);

    biski64_warmup(state);
}
