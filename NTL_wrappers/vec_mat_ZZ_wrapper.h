#ifndef NTL_VEC_MAT_LIB_H
#define NTL_VEC_MAT_LIB_H
#include <NTL/ZZ.h>
#include <NTL/vec_ZZ.h>
#include <NTL/mat_ZZ.h>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

    struct general_data {
        long n; // dimension of the secret key
        long m; // number of samples
    };

    struct LWE_Keypair {
        NTL::vec_ZZ_p s;       // Secret Key (Sparse)
        uint8_t seed[16]; // Public Key A (Compressed into a 16-byte seed)
        NTL::vec_ZZ_p b;       // Public Key b (b = A*s + e)
        uint8_t prf_key[16]; // PRF key for generating A rows on-the-fly
    };

    struct Public_Key {
        uint8_t seed[16];       // Public Key A (Compressed into a 16-byte seed)
        NTL::vec_ZZ_p b;        // Public Key b (b = A*s + e)
        NTL::vec_ZZ_p s;        // share of the secret key
        uint8_t prf_key[16];    // PRF key for generating A rows on-the-fly
    };

    int get_modulus_byte_length();    
    void free_vec_mat_string(unsigned char* str);
    NTL::ZZ centered_mod_ZZ(const NTL::ZZ& x, const NTL::ZZ& modulus);

    // --- HSS-specific Operations ---
    NTL::vec_ZZ generate_gaussian_vec(long length, long k);
    NTL::ZZ Round_ZZ(const NTL::ZZ& x_q, const NTL::ZZ& p, const NTL::ZZ& q);
    void benchmark_ntl_mul(long size, long iterations, const char* p_str);    
    void benchmark_ntl_add_mat(long size, long iterations, const char* p_str);
    void benchmark_ntl_setup(long n, long m, long q_length, long p_length, long times);
    Public_Key generate_public_key(const LWE_Keypair& key, const NTL::vec_ZZ_p& s_share);     
    void Setup(const char* lambda, long n, long m, long q_length, long p_length);
    NTL::ZZ_p generate_A_ij(const uint8_t* seed, long i, long j);
    NTL::vec_ZZ_p generate_A_row(const uint8_t* seed, long row_i, long length);
    NTL::vec_ZZ_p generate_sparse_ternary_vec(long length, long hw);
    LWE_Keypair Gen(const char* lambda, long n, long m);
    NTL::vec_ZZ generate_PRF_mask(const uint8_t* prf_key, long step_index, const NTL::ZZ& p, long row_length);
    NTL::vec_ZZ_p generate_OKDM_row(general_data* data, long row_index, const uint8_t* pk_seed, const NTL::vec_ZZ_p& pk_b, const NTL::ZZ_p& message);
    NTL::mat_ZZ_p* OKDM(general_data* data, Public_Key* pk, const NTL::ZZ_p& message);
    void free_OKDM_matrix(NTL::mat_ZZ_p* matrix);
    NTL::vec_ZZ_p DDEC(const NTL::mat_ZZ_p* input_value, const NTL::vec_ZZ_p& memory_value, const uint8_t* prf_key, long step_index, const NTL::ZZ& p, const NTL::ZZ& q);
    void benchmark_OKDM(int iterations, general_data* data, Public_Key* pk, const NTL::ZZ_p& message);
    void benchmark_DDEC(int iterations, const NTL::mat_ZZ_p* input_value, const NTL::vec_ZZ_p& memory_value, const uint8_t* prf_key, long step_index, const NTL::ZZ& p, const NTL::ZZ& q);
    void benchmark_ntl_add_memory_values(int b, const char* val1, const char* val2, const char* q,
                         const uint8_t prf_key, long step_index, long row_length, long iterations);
    NTL::vec_ZZ add_memory_values(int b, const NTL::vec_ZZ& val1, const NTL::vec_ZZ& val2,
                         const NTL::ZZ& q, const uint8_t prf_key, long step_index, long row_length);

#ifdef __cplusplus
}

extern "C" {
    void aesni_ctr_encrypt(const uint8_t *key, const uint8_t *nonce,
                           const uint8_t *plaintext, uint8_t *ciphertext,
                           uint64_t length);
}
#endif

#endif
