#pragma once
#include <vector>
#include <string>
#include <immintrin.h> // For AVX/AVX2
#include <gmp.h>       // For string/big-int conversion

/*
 * ===================================================================
 * 32-bit RNS-AVX2 Port
 * ===================================================================
 * This is the "Option 2" implementation, ported to AVX2.
 * w = 32 (32-bit residues)
 * We use __m256i (256-bit register)
 * We treat it as 4 lanes of 64-bit integers (epi64).
 * Each 64-bit lane holds one 32-bit residue in its low 32 bits.
 */

// A 256-bit vector register holds 4 64-bit lanes.
const int LANE_COUNT = 4;
// Our RNS word size is 32 bits.
const int WORD_SIZE = 32;
// A 64-bit mask to grab the low 32 bits (2^32 - 1)
const uint64_t MASK_32 = 0xFFFFFFFF;

// Holds all precomputed constants for a given modulus
class RnsContext {
public:
    mpz_t p;
    mpz_t M;
    mpz_t N;
    
    // RNS parameters
    int t; // Number of residues (e.g., 129 for 4096-bit)
    int num_vecs; // Number of __m256i vectors needed (e.g., 33)
    uint64_t u; // Fixed-point precision for CRNS (e.g., 40)

    // RNS moduli (32-bit values)
    std::vector<uint32_t> m_moduli;
    std::vector<uint32_t> n_moduli;

    // Vector RNS bases
    std::vector<__m256i> m_moduli_vec;
    std::vector<__m256i> n_moduli_vec;
    std::vector<__m256i> m_inv_32_vec; // -m_i^{-1} mod 2^32
    std::vector<__m256i> n_inv_32_vec; // -n_i^{-1} mod 2^32

    std::vector<__m256i> R_inverse_mod_m_vec; // R^{-1} mod m_i

    // --- Precomputed Constants for Subroutines ---

    // For WideMontReduce
    std::vector<__m256i> m_z_vecs; // 2^32 - m_i
    std::vector<__m256i> n_z_vecs; // 2^32 - n_i
    std::vector<__m256i> m_z2_vecs; // (z_m*z_m) % m_i
    std::vector<__m256i> n_z2_vecs; // (z_n*z_n) % n_i

    // For CRNS (Alg 2 & 6)
    std::vector<std::vector<__m256i>> E_m_to_n;  // The 'E' matrix, flattened
    std::vector<uint64_t> f_m_to_n; // The 'f_dagger' vector
    std::vector<__m256i> g_m_to_n;  // The 'g' vector

    std::vector<std::vector<__m256i>> E_n_to_m;
    std::vector<uint64_t> f_n_to_m;
    std::vector<__m256i> g_n_to_m;

    std::vector<mpz_ptr> icrt_m_constants; // For final ICRT conversion
    std::vector<mpz_ptr> icrt_n_constants;

    RnsContext(const std::string& mod_str);
    ~RnsContext();

private:
    /**
     * @brief Implements Algorithm 2 (CRNSPRECOMPUTATION).
     * @param m_to_n true if M->N, false if N->M
     */
    void precompute_crns_constants(bool m_to_n);
};

// Represents a large number in our 32-bit AVX2 RNS system
struct RnsNumber {
    std::vector<__m256i> vec_m; 
    std::vector<__m256i> vec_n; 
    
    RnsNumber(int num_vecs);
};


// --- Core Functions (to be implemented in rns_mont.cpp) ---

RnsNumber* stringToRns(const std::string& num_str, RnsContext* ctx);
void destroyRnsNumber(RnsNumber* num);
std::string rnsToString(const RnsNumber* num, RnsContext* ctx);

void rnsPowerMod(RnsNumber* r, const RnsNumber* a, const mpz_t e, RnsContext* ctx);
void rnsOptMontMul(RnsNumber* r, const RnsNumber* a, const RnsNumber* b, RnsContext* ctx);

// --- Subroutines (The "Difficult Part") ---

std::vector<__m256i> multiply_rns_vectors(
    const std::vector<__m256i>& a_vec,
    const std::vector<__m256i>& b_vec
);

__m256i multiply_single_rns_vector(
    __m256i a_vec,
    __m256i b_vec
);

std::vector<__m256i> add_s_and_qp(
    const std::vector<__m256i>& s_vec,
    const std::vector<__m256i>& q_hi_vec,
    const std::vector<__m256i>& q_lo_vec,
    const std::vector<__m256i>& mod_vec
);

__m256i add_single_s_and_qp(
    __m256i s_vec,
    __m256i q_hi_vec,
    __m256i q_lo_vec,
    __m256i mod_vec
);

std::vector<__m256i> reduce(
    const std::vector<__m256i>& product,
    const std::vector<__m256i>& mod_vec,
    const std::vector<__m256i>& mod_inv_vec
);

__m256i single_reduce(
    __m256i product,
    __m256i mod_vec,
    __m256i mod_inv_vec
);

/**
 * @brief Runs the full CRNS conversion (Algorithm 6).
 * @param out_vec       Vector to write the 32-bit results to.
 * @param in_vec        The 32-bit input vector (t residues).
 * @param ctx           The RNS context.
 * @param m_to_n        true for M->N, false for N->M
 */
void run_crns(
    std::vector<__m256i>& out_vec_hi,
    std::vector<__m256i>& out_vec_lo,
    const std::vector<__m256i>& in_vec,
    RnsContext* ctx,
    bool m_to_n
);

void crns_accumulate(
    __m256i& acc_hi, __m256i& acc_lo,
    uint64_t x_m_scalar,
    __m256i E_col_vec
);

void crns_accumulate_v(
    uint64_t x_m_scalar,
    uint64_t f_scalar,
    __uint128_t& v_scalar
);

void crns_correct(
    __m256i& acc_hi, __m256i& acc_lo,
    uint64_t k_scalar,
    __m256i g_vec
);
