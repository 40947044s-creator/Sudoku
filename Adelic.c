#include <emscripten.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

// Universal Adelic Constants
#define N_MAX 25 
uint64_t manifold[N_MAX * N_MAX];
uint64_t full_bits;
int N, bR, bC;

// Use hardware to find the first valid residue in O(1)
static inline int get_first_bit(uint64_t mask) {
    return __builtin_ctzll(mask) + 1;
}

// The Adelic Sieve: The core of the Complexity Collapse
uint64_t get_sieve(int r, int c) {
    uint64_t mask = 0;
    for (int i = 0; i < N; i++) {
        // Row/Col interference
        if (manifold[r * N + i]) mask |= (1ULL << (manifold[r * N + i] - 1));
        if (manifold[i * N + c]) mask |= (1ULL << (manifold[i * N + c] - 1));
        
        // Sector interference (The Marilyn Fold)
        int br = (r / bR) * bR + (i / bC);
        int bc = (c / bC) * bC + (i % bC);
        if (manifold[br * N + bc]) mask |= (1ULL << (manifold[br * N + bc] - 1));
    }
    return (~mask) & full_bits;
}

// The Grandparent Sentinel: Finding the Most Fixed Grid
int find_sentinel() {
    int best_cell = -1;
    int min_entropy = N + 1;
    for (int i = 0; i < N * N; i++) {
        if (manifold[i] == 0) {
            uint64_t sieve = get_sieve(i / N, i % N);
            int entropy = __builtin_popcountll(sieve);
            if (entropy == 0) return -2; // RAF Overflow
            if (entropy < min_entropy) {
                min_entropy = entropy;
                best_cell = i;
            }
        }
    }
    return best_cell;
}
// This is the function the browser calls
EMSCRIPTEN_KEEPALIVE
int solve_manifold(int n_val, uint64_t* external_grid) {
    N = n_val;
    get_factors(N, &bR, &bC);
    full_bits = (1ULL << N) - 1;

    // Transfer external data into internal manifold
    for(int i = 0; i < N * N; i++) {
        manifold[i] = external_grid[i];
    }

    // Start the recursion
    int result = solve_internal();

    // Transfer solved data back to external memory for the UI to read
    if (result == 1) {
        for(int i = 0; i < N * N; i++) {
            external_grid[i] = manifold[i];
        }
    }
    return result;
}
// Rename your existing solve() to solve_internal()
int solve_internal() {
    int idx = find_sentinel();
    if (idx == -1) return 1; 
    if (idx == -2) return 0; 

    uint64_t sieve = get_sieve(idx / N, idx % N);
    while (sieve) {
        int val = get_first_bit(sieve);
        manifold[idx] = val;
        if (solve_internal()) return 1;
        manifold[idx] = 0;
        sieve &= ~(1ULL << (val - 1));
    }
    return 0;
}
