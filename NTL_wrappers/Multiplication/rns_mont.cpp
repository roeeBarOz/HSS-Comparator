#include "rns_mont.h"
#include <iostream>
#include <cstdint>
#include <stdint.h>
#include <cstdio>
#include <string>
#include <algorithm>
#include <vector>

/**
 * @brief Computes n^{-1} mod 2^32 using Newton's method.
 * 'n' must be odd.
 */
uint32_t modInverse32(uint32_t n) {
    uint32_t x = n; // x = n^{-1} mod 2^2
    x = x * (2 - n * x); // x = n^{-1} mod 2^4
    x = x * (2 - n * x); // x = n^{-1} mod 2^8
    x = x * (2 - n * x); // x = n^{-1} mod 2^16
    x = x * (2 - n * x); // x = n^{-1} mod 2^32
    return x;
}

// Compare a > b for uint64_t values using AVX2.
// Returns a mask of all-ones (true) or all-zeros (false) per lane.
static inline __m256i unsigned_gt(__m256i a, __m256i b) {
    const __m256i signbit = _mm256_set1_epi64x(0x8000000000000000ULL);

    // Flip the sign bit on both operands.
    // This maps unsigned order -> signed order.
    __m256i ax = _mm256_xor_si256(a, signbit);
    __m256i bx = _mm256_xor_si256(b, signbit);

    // Now signed comparison gives the correct unsigned result
    return _mm256_cmpgt_epi64(ax, bx);
}

uint32_t inverse(uint32_t R, uint32_t m) {
    mpz_t mpzR, mpzm, mpzR_inv;
    mpz_init_set_ui(mpzR, R);
    mpz_init_set_ui(mpzm, m);
    mpz_init(mpzR_inv);

    mpz_invert(mpzR_inv, mpzR, mpzm);

    uint32_t result = mpz_get_ui(mpzR_inv);

    mpz_clear(mpzR);
    mpz_clear(mpzm);
    mpz_clear(mpzR_inv);

    return result;
}
// --- Constructor / Destructor for RnsContext ---

RnsContext::RnsContext(const std::string& mod_str) {
    mpz_init_set_str(p, mod_str.c_str(), 10);
    mpz_init(M);
    mpz_init(N);
    
    // 1. DETERMINE RNS PARAMETERS
    size_t p_bits = mpz_sizeinbase(p, 2);
    this->t = ((p_bits + WORD_SIZE - 1) / WORD_SIZE) + 1; // e.g., 4096/32 + 1 = 129
    this->num_vecs = (this->t + LANE_COUNT - 1) / LANE_COUNT; // e.g., (129 + 3) / 4 = 33
    // Per paper: u >= w + log2(t) + 1
    mpz_t mpzT;
    mpz_init_set_ui(mpzT, t);
    this->u = WORD_SIZE + mpz_sizeinbase(mpzT, 2) + 1; // Fixed-point precision for CRNS
    mpz_clear(mpzT);
    
    // 2. GENERATE RNS BASES (m_moduli, n_moduli)
    mpz_t k; // Our prime candidate
    mpz_init(k);
    
    // Start k at 2^32 - 1
    mpz_set_ui(k, 1);
    mpz_mul_2exp(k, k, 32); // k = 2^32
    mpz_sub_ui(k, k, 1);    // k = 2^32 - 1

    mpz_set_ui(M, 1);
    mpz_set_ui(N, 1);

    // --- Find 't' primes for m_moduli ---
    m_moduli.reserve(t);
    std::vector<uint32_t> R_inverses = std::vector<uint32_t>(t);
    R_inverses.reserve(t);
    for (int i = 0; i < t; ) {
        if (mpz_probab_prime_p(k, 30) > 0) {
            uint32_t m_i = mpz_get_ui(k);
            m_moduli.push_back(m_i);
            mpz_mul_ui(M, M, m_i);
            R_inverses[i] = inverse((1ULL << 32) - m_i, m_i); // R^{-1} mod m_i
            i++;
        }
        mpz_sub_ui(k, k, 2);
    }

    // --- Find 't' primes for n_moduli ---
    // We continue from where k left off.
    n_moduli.reserve(t);
    for (int i = 0; i < t; ) {
        if (mpz_probab_prime_p(k, 30) > 0) {
            uint32_t n_i = mpz_get_ui(k);
            n_moduli.push_back(n_i);
            mpz_mul_ui(N, N, n_i);
            i++;
        }
        mpz_sub_ui(k, k, 2);
    }

    mpz_clear(k);
    
    icrt_m_constants.reserve(t);
    icrt_n_constants.reserve(t);
    for (int i = 0; i < t; ++i) {
        mpz_ptr const_i = new __mpz_struct;
        mpz_init(const_i); // Initialize our new object

        mpz_t m_i_mpz, M_i, M_i_inv;
        mpz_init(m_i_mpz);
        mpz_init(M_i);
        mpz_init(M_i_inv);

        mpz_set_ui(m_i_mpz, m_moduli[i]);
        mpz_div_ui(M_i, M, m_moduli[i]);
        mpz_invert(M_i_inv, M_i, m_i_mpz);

        mpz_mul(const_i, M_i, M_i_inv); // Calculate value
        icrt_m_constants.push_back(const_i); // Add the pointer to the vector

        mpz_ptr const_i_n = new __mpz_struct;
        mpz_init(const_i_n); // Initialize our new object

        mpz_t n_i_mpz, N_i, N_i_inv;
        mpz_init(n_i_mpz);
        mpz_init(N_i);
        mpz_init(N_i_inv);

        mpz_set_ui(n_i_mpz, n_moduli[i]);
        mpz_div_ui(N_i, N, n_moduli[i]);
        mpz_invert(N_i_inv, N_i, n_i_mpz);
        mpz_mul(const_i_n, N_i, N_i_inv); // Calculate value
        icrt_n_constants.push_back(const_i_n); // Add the pointer to the vector

        mpz_clear(m_i_mpz);
        mpz_clear(M_i);
        mpz_clear(M_i_inv);

        mpz_clear(n_i_mpz);
        mpz_clear(N_i);
        mpz_clear(N_i_inv);
    }
    // 3. LOAD CONSTANTS INTO VECTORS
    //    We now "vectorize" our scalar RNS bases and their
    //    inverses for fast lookup during the main loop.
    m_moduli_vec.resize(num_vecs);
    n_moduli_vec.resize(num_vecs);
    m_inv_32_vec.resize(num_vecs);
    n_inv_32_vec.resize(num_vecs);
    m_z_vecs.resize(num_vecs);
    n_z_vecs.resize(num_vecs);
    m_z2_vecs.resize(num_vecs);
    n_z2_vecs.resize(num_vecs);
    R_inverse_mod_m_vec.resize(num_vecs);

    for (int i = 0; i < num_vecs; ++i) {
        // We are at the i-th vector, which corresponds to
        // residues t*i to t*i + 3
        uint64_t m_vals[4] = {0}, n_vals[4] = {0}, m_inv_vals[4] = {0}, n_inv_vals[4] = {0},
                m_z_vals[4] = {0}, n_z_vals[4] = {0}, m_z2_vals[4] = {0}, n_z2_vals[4] = {0}, R_inverse_mod_m_vals[4] = {0};

        for (int j = 0; j < LANE_COUNT; ++j) {
            int idx = i * LANE_COUNT + j;
            if (idx < t) {
                // Get the 32-bit scalar prime
                uint32_t m = m_moduli[idx];
                uint32_t n = n_moduli[idx];

                // Store in 64-bit lanes (as 32-bit values)
                m_vals[j] = m;
                n_vals[j] = n;

                m_z_vals[j] = (1ULL << 32) - m; // z_m = 2^32 - m_i
                n_z_vals[j] = (1ULL << 32) - n; // z_n = 2^32 - n_i
                m_z2_vals[j] = (uint64_t)(((unsigned __int128) m_z_vals[j] * m_z_vals[j]) % m); // z_m^2 mod m_i
                n_z2_vals[j] = (uint64_t)(((unsigned __int128) n_z_vals[j] * n_z_vals[j]) % n); // z_n^2 mod n_i
                
                // Compute and store 32-bit inverse
                m_inv_vals[j] = modInverse32(m);
                n_inv_vals[j] = modInverse32(n);
                R_inverse_mod_m_vals[j] = R_inverses[idx];
            }
            // else: leave as 0 (padding)
        }

        // AVX _set_ functions are "backwards"
        // _mm256_set_epi64x(lane 3, lane 2, lane 1, lane 0)
        m_moduli_vec[i] = _mm256_set_epi64x(m_vals[3], m_vals[2], m_vals[1], m_vals[0]);
        n_moduli_vec[i] = _mm256_set_epi64x(n_vals[3], n_vals[2], n_vals[1], n_vals[0]);
        m_inv_32_vec[i] = _mm256_set_epi64x(m_inv_vals[3], m_inv_vals[2], m_inv_vals[1], m_inv_vals[0]);
        n_inv_32_vec[i] = _mm256_set_epi64x(n_inv_vals[3], n_inv_vals[2], n_inv_vals[1], n_inv_vals[0]);
        m_z_vecs[i] = _mm256_set_epi64x(m_z_vals[3], m_z_vals[2], m_z_vals[1], m_z_vals[0]);
        n_z_vecs[i] = _mm256_set_epi64x(n_z_vals[3], n_z_vals[2], n_z_vals[1], n_z_vals[0]);
        m_z2_vecs[i] = _mm256_set_epi64x(m_z2_vals[3], m_z2_vals[2], m_z2_vals[1], m_z2_vals[0]);
        n_z2_vecs[i] = _mm256_set_epi64x(n_z2_vals[3], n_z2_vals[2], n_z2_vals[1], n_z2_vals[0]);
        R_inverse_mod_m_vec[i] = _mm256_set_epi64x(R_inverse_mod_m_vals[3], R_inverse_mod_m_vals[2], R_inverse_mod_m_vals[1], R_inverse_mod_m_vals[0]);
    }
    
    // 4. RUN CRNS PRECOMPUTATION (Algorithm 2)
    //    (STUBBED: You must calculate E, f, and g constants)
    precompute_crns_constants(true);  // M -> N
    precompute_crns_constants(false); // N -> M
}

RnsContext::~RnsContext() {
    mpz_clear(p);
    mpz_clear(M);
    mpz_clear(N);

    for (auto& c : icrt_m_constants) {
        mpz_clear(c);
    }
}

/**
 * @brief Implements Algorithm 2 (CRNSPRECOMPUTATION)
 * This is the most complex part of the setup.
 */
void RnsContext::precompute_crns_constants(bool m_to_n) {
    // 1. Identify source and target bases
    const std::vector<uint32_t>& source_moduli = m_to_n ? m_moduli : n_moduli;
    const std::vector<uint32_t>& target_moduli = m_to_n ? n_moduli : m_moduli;
    const mpz_t& Source_M = m_to_n ? M : N;
    const mpz_t& Target_M = m_to_n ? N : M;

    // 2. Get scalar pre/post-multipliers (y, z) from Algorithm 3
    mpz_t y, z, neg_zM, r;
    mpz_init(y);
    mpz_init(z);
    mpz_init(neg_zM);
    mpz_init(r);
    // set r = 2**32
    mpz_set_ui(r, 1);
    mpz_mul_2exp(r, r, 32);

    if (m_to_n) {
        // M -> N: y = -p^{-1} mod M, z = p mod N
        mpz_t p_inv_M, r_inv_M, M_inv_N;
        mpz_init(p_inv_M);
        mpz_init(r_inv_M);
        mpz_init(M_inv_N);

        // y = -p^{-1} * R^{-1} mod M
        mpz_set_ui(y, 1);
        mpz_sub(p_inv_M, M, p);
        mpz_invert(p_inv_M, p_inv_M, M); // p_inv_M = -p^{-1} mod M
        mpz_mul(y, y, p_inv_M);         // y = -p^{-1} mod M
        mpz_invert(r_inv_M, r, M);         // r_inv_M = R^{-1} mod M
        mpz_mul(y, y, r_inv_M);         // y = -p^{-1} * R^{-1} mod M
        mpz_mod(y, y, M);                    // y = -p^{-1} * R^{-1} mod M

        // z = p * R * R mod N
        mpz_mod(z, p, N);                    // z = p mod N
        mpz_mul(z, z, r);             // z = p * R mod N
        mpz_mul(z, z, r);             // z = p * R * R mod N
        mpz_mod(z, z, N);                    // z = p * R * R mod N
    } else {
        mpz_t r_inv_N, M_inv_N;
        mpz_init(r_inv_N);
        mpz_init(M_inv_N);

        // y = M^{-1} * R^{-1} mod N
        mpz_set_ui(y, 1);
        mpz_invert(r_inv_N, r, N);         // r_inv_N = R^{-1} mod N
        mpz_invert(M_inv_N, M, N);
        mpz_mul(y, M_inv_N, r_inv_N);     // y = M^{-1} * R^{-1} mod N
        mpz_mod(y, y, N);                    // y = M^{-1} * R^{-1} mod N

        // z = R^2 mod M
        mpz_set_ui(z, 1);
        mpz_mul(z, r, r);             // z = R * R
        mpz_mod(z, z, M);                    // z = R^2 mod M
        
        mpz_clear(r_inv_N);
        mpz_clear(M_inv_N);
    }

    // 3. Allocate memory for constants
    std::vector<uint64_t>& f_vec = m_to_n ? f_m_to_n : f_n_to_m;
    std::vector<__m256i>& g_vec = m_to_n ? g_m_to_n : g_n_to_m;
    std::vector<std::vector<__m256i>>& E_vec = m_to_n ? E_m_to_n : E_n_to_m;

    f_vec.resize(t, 0);
    g_vec.resize(num_vecs, _mm256_setzero_si256());
    
    E_vec.resize(t);
    for (auto& col : E_vec) {
        col.resize(num_vecs, _mm256_setzero_si256());
    }

    // // 4. Calculate neg_zM = (-z * Source_M) mod Target_M
    mpz_sub(neg_zM, Target_M, z);
    mpz_mul(neg_zM, neg_zM, Source_M);
    mpz_mod(neg_zM, neg_zM, Target_M);

    // 5. Calculate g_j (Alg 2, Line 5)
    for(int i = 0; i < num_vecs; ++i) {
        uint64_t g_vals[4] = {0};
        for(int j = 0; j < LANE_COUNT; ++j) {
            int idx = i * LANE_COUNT + j;
            if(idx < t) {
                mpz_t g_j;
                mpz_init(g_j);
                mpz_mod_ui(g_j, neg_zM, target_moduli[idx]);
                g_vals[j] = mpz_get_ui(g_j);
                mpz_clear(g_j);
            } else {
                g_vals[j] = 0;
            }
        }
        g_vec[i] = _mm256_set_epi64x(g_vals[3], g_vals[2], g_vals[1], g_vals[0]);
    }

    // 6. Calculate I_i, f_i, and E_ij (The t-loop)
    mpz_t M_i, M_i_inv, I_i, f_i_num, f_i_ceil, E_ij, m_i_mpz;
    mpz_init(M_i); mpz_init(M_i_inv); mpz_init(I_i); mpz_init(m_i_mpz);
    mpz_init(f_i_num); mpz_init(f_i_ceil); mpz_init(E_ij);

    for (int i = 0; i < t; ++i) {
        uint64_t m_i = source_moduli[i];
        mpz_set_ui(m_i_mpz, m_i);
        // I_i = (((M/m_i)^{-1} % m_i) * (y * M/m_i))
        mpz_div_ui(M_i, Source_M, m_i);      // M_i = M / m_i
        mpz_invert(M_i_inv, M_i, m_i_mpz);       // M_i_inv = (*M_i)^{-1} % m_i
        mpz_mul(I_i, M_i_inv, M_i);          // I_i = M_i_inv * M_i
        mpz_mul(I_i, I_i, y);              // I_i = y * M_i_inv * M_i
        mpz_mod(I_i, I_i, Source_M);        // I_i = I_i % M
        // f_i = (I_i / M) * 2^u. I_i/M is ~1. So f_i is ~2^u.
        mpz_mul_2exp(f_i_num, I_i, u);
        // to get the ceiling of the division:
        // we do (I_i * 2^u + M - 1) / M
        mpz_add(f_i_ceil, f_i_num, Source_M);
        mpz_sub_ui(f_i_ceil, f_i_ceil, 1);
        mpz_div(f_i_ceil, f_i_ceil, Source_M);
        f_vec[i] = mpz_get_ui(f_i_ceil); // This is correct, f_i is small.
        // E_ij = (z * I_i) % n_j (Alg 2, Line 3)
        mpz_mul(E_ij, z, I_i); // E_ij = z * I_i (a huge number)
        // Now we store this column (E_i) in our E_vec
        std::vector<__m256i>& E_column = E_vec[i]; // alias
        for (int j_vec = 0; j_vec < num_vecs; ++j_vec) {
            uint64_t E_vals[4] = {0};
            for (int j_lane = 0; j_lane < LANE_COUNT; ++j_lane) {
                int j = j_vec * LANE_COUNT + j_lane;
                if (j < t) {
                    mpz_t E_mod_nj;
                    mpz_init(E_mod_nj);
                    mpz_mod_ui(E_mod_nj, E_ij, target_moduli[j]);
                    E_vals[j_lane] = mpz_get_ui(E_mod_nj);
                    mpz_clear(E_mod_nj);
                }
            }
            E_column[j_vec] = _mm256_set_epi64x(E_vals[3], E_vals[2], E_vals[1], E_vals[0]);
        }
    }

    mpz_clear(M_i); mpz_clear(M_i_inv); mpz_clear(I_i);
    mpz_clear(f_i_num); mpz_clear(f_i_ceil); mpz_clear(E_ij);
    mpz_clear(y); mpz_clear(z); mpz_clear(neg_zM), mpz_clear(r);
}

RnsNumber::RnsNumber(int num_vecs) {
    // Initialize vectors with the correct size, filled with zero
    vec_m.resize(0, _mm256_setzero_si256());
    vec_n.resize(0, _mm256_setzero_si256());
}

// --- CONVERSION STUBS ---

RnsNumber* stringToRns(const std::string& num_str, RnsContext* ctx) {
    RnsNumber* rns = new RnsNumber(ctx->num_vecs);

    mpz_t a, X;
    mpz_init_set_str(a, num_str.c_str(), 10);
    mpz_init(X);

    mpz_set(X, a);
    mpz_mul(X, ctx->M, X); // X = a * M
    mpz_mod(X, X, ctx->p); // X = X % p
    mpz_mul_2exp(X, X, WORD_SIZE); // X = X * 2^32

    for (int i = 0; i < ctx->num_vecs; ++i) {
        uint64_t m_vals[4] = {0};
        uint64_t n_vals[4] = {0};
        for (int j = 0; j < LANE_COUNT; ++j) {
            int idx = i * LANE_COUNT + j;
            if (idx < ctx->t) {
                m_vals[j] = mpz_fdiv_ui(X, ctx->m_moduli[idx]);
                n_vals[j] = mpz_fdiv_ui(X, ctx->n_moduli[idx]);
            }
        }
        
        __m256i m_vec = _mm256_set_epi64x(m_vals[3], m_vals[2], m_vals[1], m_vals[0]);        
        __m256i n_vec = _mm256_set_epi64x(n_vals[3], n_vals[2], n_vals[1], n_vals[0]);
        rns->vec_m.push_back(m_vec);
        rns->vec_n.push_back(n_vec);
    }

    mpz_clear(a);
    mpz_clear(X);

    return rns;
}

std::string rnsToString(const RnsNumber* num, RnsContext* ctx) {
    mpz_t result;
    mpz_init(result);
    mpz_set_ui(result, 0);

    std::vector<__m256i> rns_vec = num->vec_m; // Use base M for conversion
    std::vector<__m256i> R_inverse_mod_m_vec = ctx->R_inverse_mod_m_vec;
    rns_vec = multiply_rns_vectors(rns_vec, R_inverse_mod_m_vec);

    mpz_t S, term;
    mpz_init_set_ui(S, 0); // S will be our accumulator \sum(r_i * I_i)
    mpz_init(term);
    // Extract all 't' 32-bit scalar residues from the vector
    alignas(32) uint64_t x_scalars[ctx->num_vecs * LANE_COUNT];
    for (int j = 0; j < ctx->num_vecs; ++j) {
        _mm256_storeu_si256((__m256i*)(x_scalars + j * LANE_COUNT), rns_vec[j]);
    }
    // Apply ICRT: S = \sum_{i=0}^{t-1} (r_i * I_i)
    for (int i = 0; i < ctx->t; ++i) {
        uint64_t r_i = x_scalars[i];
        mpz_mul_ui(term, ctx->icrt_m_constants[i], r_i);
        mpz_add(S, S, term);
    }
    mpz_add(S, S, ctx->p);
    mpz_mod(S, S, ctx->M); // S = S % M

    // Reduce modulo M to get the final result
    mpz_t M_inv_p;
    mpz_init(M_inv_p);
    mpz_invert(M_inv_p, ctx->M, ctx->p); // M^{-1} mod p
    mpz_mul(S, S, M_inv_p);
    mpz_mod(S, S, ctx->p);

    // Convert the result to a string
    char* result_str = mpz_get_str(nullptr, 10, S);
    std::string result_cpp_str(result_str);
    free(result_str);

    mpz_clear(S);
    mpz_clear(term);

    return result_cpp_str;
}

void destroyRnsNumber(RnsNumber* num) {
    if (num) {
        delete num;
    }
}

// --- CORE LOGIC STUBS ---

void rnsPowerMod(RnsNumber* r, const RnsNumber* a, const mpz_t e, RnsContext* ctx) {
    // This function computes r = a^e mod p using
    // a left-to-right sliding window exponentiation.
    // 'r' is pre-initialized to 1 by the caller.

    // 1. --- Setup Sliding Window ---
    const int k = 7; // Window size (k=7 is a good balance)
    const int table_size = 1 << (k - 1); // 2^(k-1) = 64

    // 2. --- Precomputation ---
    // We precompute a table of odd powers: a^1, a^3, ..., a^127
    std::vector<RnsNumber*> table(table_size);
    RnsNumber* a_sq = new RnsNumber(ctx->num_vecs);

    // table[0] = a^1
    table[0] = new RnsNumber(ctx->num_vecs);
    table[0]->vec_m = a->vec_m;
    table[0]->vec_n = a->vec_n;

    // a_sq = a^2
    rnsOptMontMul(a_sq, a, a, ctx);
    //print a_sq as string
    std::string a_sq_str = rnsToString(a_sq, ctx);
    std::cout << "a^2: " << a_sq_str << std::endl;

    // table[i] = table[i-1] * a_sq  (e.g., a^3 * a^2 = a^5)
    for (int i = 1; i < table_size; ++i) {
        table[i] = new RnsNumber(ctx->num_vecs);
        rnsOptMontMul(table[i], table[i - 1], a_sq, ctx);
    }
    for (int i = 0; i < table_size; ++i) {
        std::string table_str = rnsToString(table[i], ctx);
        std::cout << "table[" << (2 * i + 1) << "] = " << table_str << std::endl;
    }
    
    // 3. --- Main Exponentiation Loop ---
    size_t n_bits = mpz_sizeinbase(e, 2);
    long i = n_bits - 1; // Start from the most significant bit

    while (i >= 0) {
        // Check the i-th bit
        if (mpz_tstbit(e, i) == 0) {
            // --- Case 0: Bit is 0 ---
            // Just square and move to the next bit
            rnsOptMontMul(r, r, r, ctx);
            i--;
        } else {
            // --- Case 1: Bit is 1 ---
            // This is the start of a window.
            
            // 1. Find the window's end
            long w_end = i - k + 1;
            if (w_end < 0) {
                w_end = 0;
            }
            
            // 2. Find the *odd* part of the window
            // (e.g., if window is 110 (6), we only use 11 (3))
            while (mpz_tstbit(e, w_end) == 0) {
                w_end++;
            }
            
            // 3. Read the odd value 'w_val' from the window
            uint64_t w_val = 0;
            for (long j = i; j >= w_end; j--) {
                w_val = (w_val << 1) | mpz_tstbit(e, j);
            }
            
            // 4. Square 'w_len' times
            long w_len = i - w_end + 1;
            for (int j = 0; j < w_len; j++) {
                rnsOptMontMul(r, r, r, ctx);
            }

            // 5. Multiply by the precomputed odd power
            // (w_val is 1, 3, 5...; table index is 0, 1, 2...)
            RnsNumber* odd_power = table[w_val / 2];
            rnsOptMontMul(r, r, odd_power, ctx);

            // 6. Slide the index past the entire window
            i = w_end - 1;
        }
    }
    
    // 4. --- Clean up ---
    destroyRnsNumber(a_sq);
    for (int i = 0; i < table_size; ++i) {
        destroyRnsNumber(table[i]);
    }
}

void rnsOptMontMul(RnsNumber* r, const RnsNumber* a, const RnsNumber* b, RnsContext* ctx) {
    int num_vecs = ctx->num_vecs;
    std::vector<__m256i> s_m_before_reduction(num_vecs, _mm256_setzero_si256());
    std::vector<__m256i> s_n_before_reduction(num_vecs, _mm256_setzero_si256());
    std::vector<__m256i> q_hi(num_vecs, _mm256_setzero_si256());
    std::vector<__m256i> q_lo(num_vecs, _mm256_setzero_si256());

    s_m_before_reduction = multiply_rns_vectors(a->vec_m, b->vec_m);
    r->vec_m = reduce(s_m_before_reduction, ctx->m_moduli_vec, ctx->m_inv_32_vec);
    s_n_before_reduction = multiply_rns_vectors(a->vec_n, b->vec_n);

    run_crns(q_hi, q_lo, r->vec_m, ctx, true);

    r->vec_n = add_s_and_qp(s_n_before_reduction, q_hi, q_lo, ctx->n_moduli_vec);
    r->vec_n = reduce(r->vec_n, ctx->n_moduli_vec, ctx->n_inv_32_vec);
    
    std::vector<__m256i> v_hi(num_vecs, _mm256_setzero_si256());
    std::vector<__m256i> v_lo(num_vecs, _mm256_setzero_si256());
    run_crns(v_hi, v_lo, r->vec_n, ctx, false); // false = n_to_m

    std::vector<__m256i> zeros(num_vecs, _mm256_setzero_si256());
    r->vec_m = add_s_and_qp(zeros, v_hi, v_lo, ctx->m_moduli_vec);
    r->vec_m = reduce(r->vec_m, ctx->m_moduli_vec, ctx->m_inv_32_vec);

    s_m_before_reduction.clear();
    s_n_before_reduction.clear();
    q_hi.clear();
    q_lo.clear();
    v_hi.clear();
    v_lo.clear();
}


// ===================================================================
//                        CORE SUBROUTINES
// ===================================================================

std::vector<__m256i> reduce(
    const std::vector<__m256i>& product,
    const std::vector<__m256i>& mod_vec,
    const std::vector<__m256i>& mod_inv_vec
) {
    int num_vecs = product.size();
    std::vector<__m256i> result_vec(num_vecs, _mm256_setzero_si256());
    for (int i = 0; i < num_vecs; ++i) {
        result_vec[i] = single_reduce(
            product[i],
            mod_vec[i],
            mod_inv_vec[i]
        );
    }
    return result_vec;
}

__m256i single_reduce(
    __m256i product,
    __m256i mod_vec,
    __m256i mod_inv_vec
) {
    __m256i MASK_32 = _mm256_set1_epi64x(0xFFFFFFFFu);

    __m256i product_lo = _mm256_and_si256(product, MASK_32);
    __m256i q = multiply_single_rns_vector(product_lo, mod_inv_vec);
    q = _mm256_and_si256(q, MASK_32);

    __m256i qm = multiply_single_rns_vector(q, mod_vec);
    
    __m256i s_hi = _mm256_srli_epi64(product, 32);
    __m256i qm_hi = _mm256_srli_epi64(qm, 32);
    
    __m256i result = _mm256_sub_epi64(s_hi, qm_hi);
    __m256i zero = _mm256_setzero_si256();
    __m256i is_neg = _mm256_cmpgt_epi64(zero, result); // 0 > result
    
    // If negative, add M
    __m256i correction = _mm256_and_si256(is_neg, mod_vec);
    result = _mm256_add_epi64(result, correction);

    return result; // Result is now in [0, M), strictly < 2^32
}

std::vector<__m256i> multiply_rns_vectors(
    const std::vector<__m256i>& a_vec,
    const std::vector<__m256i>& b_vec
) {
    int num_vecs = a_vec.size();
    std::vector<__m256i> result_vec(num_vecs, _mm256_setzero_si256());
    for (int i = 0; i < num_vecs; ++i) {
        result_vec[i] = multiply_single_rns_vector(
            a_vec[i],
            b_vec[i]
        );
    }
    return result_vec;
}

__m256i multiply_single_rns_vector(
    __m256i a_vec,
    __m256i b_vec
) {
    return _mm256_mul_epu32(a_vec, b_vec); // s = ab
}

std::vector<__m256i> add_s_and_qp(
    const std::vector<__m256i>& s_vec,
    const std::vector<__m256i>& q_hi_vec,
    const std::vector<__m256i>& q_lo_vec,
    const std::vector<__m256i>& mod_vec
) {
    int num_vecs = s_vec.size();
    std::vector<__m256i> result_vec(num_vecs, _mm256_setzero_si256());
    for (int i = 0; i < num_vecs; i++) {
        result_vec[i] = add_single_s_and_qp(
            s_vec[i],
            q_hi_vec[i],
            q_lo_vec[i],
            mod_vec[i]
        );
    }
    return result_vec;
}

__m256i add_single_s_and_qp(
    __m256i s_vec,
    __m256i q_hi_vec,
    __m256i q_lo_vec,
    __m256i mod_vec
) {
    // adding qp to s
    __m256i new_q_lo = _mm256_add_epi64(s_vec, q_lo_vec);
    __m256i carry_mask = unsigned_gt(q_lo_vec, new_q_lo);
    __m256i ones = _mm256_set1_epi64x(1);
    __m256i carry = _mm256_and_si256(carry_mask, ones);
    __m256i new_q_hi = _mm256_add_epi64(q_hi_vec, carry);
    // need to reduce the w + log(t) + 1 higher bits to fit into 32 bits
    // the reduction is mod mod_vec
    __m256i lo = _mm256_srli_epi64(new_q_lo, 32);
    __m256i two_to_32 = _mm256_set1_epi64x(1ULL << 32);
    __m256i z = _mm256_sub_epi64(two_to_32, mod_vec); // z = 2^32 - mod_vec
    __m256i uz = _mm256_mul_epu32(new_q_hi, z);
    __m256i sum = _mm256_add_epi64(lo, uz);
    __m256i ge_mask = _mm256_cmpgt_epi64(mod_vec, sum); // sum < mod_vec
    ge_mask = _mm256_andnot_si256(ge_mask, mod_vec);
    sum = _mm256_sub_epi64(sum, ge_mask);
    __m256i q = _mm256_add_epi64(_mm256_and_si256(new_q_lo, _mm256_set1_epi64x(0xFFFFFFFFu)), _mm256_slli_epi64(sum, 32));
    return q;
}

/**
 * @brief Runs the full CRNS conversion (Algorithm 6).
 * Handles domain crossing: Mont -> Std -> Mont
 */
void run_crns(
    std::vector<__m256i>& out_vec_hi,
    std::vector<__m256i>& out_vec_lo,
    const std::vector<__m256i>& in_vec, // in_vec is in MONTGOMERY form
    RnsContext* ctx,
    bool m_to_n
) {
    int t = ctx->t;
    int num_vecs = ctx->num_vecs;

    // 1. Get correct constants (all in STANDARD form)
    const std::vector<std::vector<__m256i>>& E_vec = m_to_n ? ctx->E_m_to_n : ctx->E_n_to_m;
    const std::vector<uint64_t>& f_vec = m_to_n ? ctx->f_m_to_n : ctx->f_n_to_m;
    const std::vector<__m256i>& g_vec = m_to_n ? ctx->g_m_to_n : ctx->g_n_to_m;

    // Get source base constants (for conversion)
    const std::vector<__m256i>& source_mod_vec = m_to_n ? ctx->m_moduli_vec : ctx->n_moduli_vec;
    const std::vector<__m256i>& source_inv_vec = m_to_n ? ctx->m_inv_32_vec : ctx->n_inv_32_vec;

    // 2. Extract 't' 32-bit STANDARD scalars
    alignas(32) uint64_t x_scalars[num_vecs * LANE_COUNT];
    for (int j = 0; j < num_vecs; ++j) {
        _mm256_storeu_si256((__m256i*)(x_scalars + j * LANE_COUNT), in_vec[j]);
    }

    __uint128_t v_scalar = 0;
    // 4. Main Accumulation Loop (All in STANDARD form)
    for (int i = 0; i < t; ++i) {
        uint64_t x_m_scalar = x_scalars[i]; // x_std
        uint64_t f_scalar = f_vec[i];     // f_std
        // Accumulate v_std += x_std * f_std
        crns_accumulate_v(x_m_scalar, f_scalar, v_scalar);
        for (int j = 0; j < num_vecs; ++j) {
            __m256i E_col_vec = E_vec[i][j]; // E_std
            crns_accumulate(out_vec_hi[j], out_vec_lo[j], x_m_scalar, E_col_vec);
        }
    }
    // 5. Final Correction (All in STANDARD form)
    uint64_t k_scalar = (uint64_t)(v_scalar >> ctx->u);
    for (int j = 0; j < num_vecs; ++j) {
        crns_correct(out_vec_hi[j], out_vec_lo[j], k_scalar, g_vec[j]);
    }
}

void crns_accumulate_v(
    uint64_t x_m_scalar,
    uint64_t f_scalar,
    __uint128_t& v_scalar
) {
    v_scalar += (__uint128_t) x_m_scalar * (__uint128_t) f_scalar;
}

/**
 * @brief One step of the O(t^2) CRNS accumulation loop (Alg 6)
 * This shows the "manual carry" logic for our 128-bit accumulator.
 */
void crns_accumulate(
    __m256i& acc_hi, __m256i& acc_lo, // 128-bit accumulator
    uint64_t x_m_scalar,              // One 32-bit residue from x_M
    __m256i E_col_vec                 // One 32-bit col from E
) {
    __m256i x_m_vec = _mm256_set1_epi64x(x_m_scalar);
    __m256i xE = _mm256_mul_epu32(x_m_vec, E_col_vec); // 32b * 32b = 64b product
    __m256i old_lo = acc_lo;
    acc_lo = _mm256_add_epi64(acc_lo, xE); // Add to low part
    __m256i carry_mask = unsigned_gt(old_lo, acc_lo); // Find carries
    const __m256i ones = _mm256_set1_epi64x(1);
    __m256i carry_vec = _mm256_and_si256(carry_mask, ones);
    acc_hi = _mm256_add_epi64(acc_hi, carry_vec); // Add carries
}

/**
 * @brief Final correction step of CRNS (Alg 6)
 */
void crns_correct(
    __m256i& acc_hi, __m256i& acc_lo,
    uint64_t k_scalar,
    __m256i g_vec
) {
    // We'll use these constants for our 64x32 multiplication
    const __m256i ones = _mm256_set1_epi64x(1);

    // 1. Split k_scalar into high and low 32-bit parts
    __m256i k_lo_vec = _mm256_set1_epi64x(k_scalar & MASK_32);
    __m256i k_hi_vec = _mm256_set1_epi64x(k_scalar >> 32);

    // 2. Calculate the two parts of the product
    // prod_lo = (k_lo * g)  (32b * 32b = 64b product)
    __m256i prod_lo = _mm256_mul_epu32(k_lo_vec, g_vec);

    // prod_hi = (k_hi * g)  (32b * 32b = 64b product)
    __m256i prod_hi = _mm256_mul_epu32(k_hi_vec, g_vec);
    
    // 3. Add (k_lo * g) to the 128-bit accumulator
    __m256i old_lo = acc_lo;
    acc_lo = _mm256_add_epi64(acc_lo, prod_lo);
    
    // Find carries (where new_lo < old_lo)
    __m256i carry1_mask = unsigned_gt(old_lo, acc_lo);
    __m256i carry1_vec = _mm256_and_si256(carry1_mask, ones);
    acc_hi = _mm256_add_epi64(acc_hi, carry1_vec);

    // 4. Add (k_hi * g * 2^32) to the 128-bit accumulator
    //    prod_hi is (k_hi * g). We need to shift it left 32 bits.
    //    A 64-bit left shift is split into:
    //    - low part: _mm256_slli_epi64(prod_hi, 32)
    //    - high part: _mm256_srli_epi64(prod_hi, 32)
    
    __m256i prod_hi_shifted_lo = _mm256_slli_epi64(prod_hi, 32);
    __m256i prod_hi_shifted_hi = _mm256_srli_epi64(prod_hi, 32);
    // Add the low part
    old_lo = acc_lo;
    acc_lo = _mm256_add_epi64(acc_lo, prod_hi_shifted_lo);

    // Find carries
    __m256i carry2_mask = unsigned_gt(old_lo, acc_lo);
    __m256i carry2_vec = _mm256_and_si256(carry2_mask, ones);
    
    // Add the high part + carries
    acc_hi = _mm256_add_epi64(acc_hi, prod_hi_shifted_hi);
    acc_hi = _mm256_add_epi64(acc_hi, carry2_vec);
}
