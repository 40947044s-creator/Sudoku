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

int solve() {
    int idx = find_sentinel();
    if (idx == -1) return 1; // H=0 achieved
    if (idx == -2) return 0; // Friction detected

    uint64_t sieve = get_sieve(idx / N, idx % N);
    while (sieve) {
        int val = get_first_bit(sieve);
        manifold[idx] = val;
        if (solve()) return 1;
        manifold[idx] = 0;
        sieve &= ~(1ULL << (val - 1)); // Bit-peeling
    }
    return 0;
}
