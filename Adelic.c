#include <stdio.h>
#include <stdint.h>
#include <emscripten.h>
#include <stdlib.h> // For rand()



// Universal Adelic Constants
#define N_MAX 25 
uint64_t manifold[N_MAX * N_MAX];
uint64_t full_bits;
int N, bR, bC;

// Forward Declarations
uint64_t get_sieve(int r, int c);
static inline int get_first_bit(uint64_t mask);

void get_factors(int n, int *r, int *c) {
    int root = 1;
    for (int i = 1; i * i <= n; i++) {
        if (n % i == 0) root = i;
    }
    *r = root;
    *c = n / root;
}

static inline int get_first_bit(uint64_t mask) {
    return __builtin_ctzll(mask) + 1;
}

uint64_t get_sieve(int r, int c) {
    uint64_t mask = 0;
    for (int i = 0; i < N; i++) {
        if (manifold[r * N + i]) mask |= (1ULL << (manifold[r * N + i] - 1));
        if (manifold[i * N + c]) mask |= (1ULL << (manifold[i * N + c] - 1));
        int box_r = (r / bR) * bR + (i / bC);
        int box_c = (c / bC) * bC + (i % bC);
        if (manifold[box_r * N + box_c]) mask |= (1ULL << (manifold[box_r * N + box_c] - 1));
    }
    return (~mask) & full_bits;
}

// Tier 2.5: Deep Annihilation (Hidden Singles)
// This forces residues into their only possible homes in Rows, Columns, and Sectors.
int annihilate_noise() {
    int grounded = 0;
    for (int v = 1; v <= N; v++) {
        uint64_t v_bit = (1ULL << (v - 1));
        
        // Row/Column Pass
        for (int i = 0; i < N; i++) {
            int r_count = 0, last_c = -1;
            int c_count = 0, last_r = -1;
            for (int j = 0; j < N; j++) {
                // Check Row i
                if (manifold[i * N + j] == 0) {
                    if (get_sieve(i, j) & v_bit) { r_count++; last_c = j; }
                } else if (manifold[i * N + j] == (uint64_t)v) { r_count = -1; }
                
                // Check Column i
                if (manifold[j * N + i] == 0) {
                    if (get_sieve(j, i) & v_bit) { c_count++; last_r = j; }
                } else if (manifold[j * N + i] == (uint64_t)v) { c_count = -1; }
            }
            if (r_count == 1) { manifold[i * N + last_c] = (uint64_t)v; grounded = 1; }
            if (c_count == 1) { manifold[last_r * N + i] = (uint64_t)v; grounded = 1; }
        }
    }
    return grounded;
}

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

int solve_internal() {
    // 1. Destructive Interference: Loop Annihilation until stable
    while (annihilate_noise());

    // 2. Sentinel Check: Identify the most-constrained cell
    int idx = find_sentinel();
    if (idx == -1) return 1; 
    if (idx == -2) return 0; 

    // 3. Superposition Branching
    uint64_t sieve = get_sieve(idx / N, idx % N);
    while (sieve) {
        int val = get_first_bit(sieve);
        manifold[idx] = (uint64_t)val;
        if (solve_internal()) return 1;
        manifold[idx] = 0;
        sieve &= ~(1ULL << (val - 1));
    }
    return 0;
}

EMSCRIPTEN_KEEPALIVE
int solve_manifold(int n_val, uint64_t* external_grid) {
    N = n_val;
    get_factors(N, &bR, &bC);
    full_bits = (1ULL << N) - 1;
    
    int is_blank = 1;
    for(int i = 0; i < N * N; i++) {
        manifold[i] = external_grid[i];
        if (manifold[i] != 0) is_blank = 0;
    }

    // STOCHASTIC INJECTION: Seed the blank manifold to create a signal
    if (is_blank) {
        srand(time(NULL)); 
        for (int i = 0; i < N; i++) { // Inject N random cornerstone residues
            int r = rand() % N;
            int c = rand() % N;
            uint64_t sieve = get_sieve(r, c);
            if (sieve) {
                // Pick a random bit from the available residues
                int count = __builtin_popcountll(sieve);
                int pick = rand() % count;
                for (int v = 1; v <= N; v++) {
                    if (sieve & (1ULL << (v - 1))) {
                        if (pick-- == 0) {
                            manifold[r * N + c] = (uint64_t)v;
                            break;
                        }
                    }
                }
            }
        }
    }

    int result = solve_internal();

    if (result == 1) {
        for(int i = 0; i < N * N; i++) external_grid[i] = manifold[i];
    }
    return result;
}
