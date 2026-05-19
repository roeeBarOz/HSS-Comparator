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
        NTL::ZZ q; // modulus
        NTL::ZZ p; // plaintext modulus
        NTL::ZZ q_half; // q/2 for centered reduction
        NTL::ZZ p_half; // p/2 for centered reduction
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
    NTL::ZZ centered_mod_ZZ(const NTL::ZZ& x, general_data* data);

    // --- HSS-specific Operations ---
    NTL::vec_ZZ generate_gaussian_vec(long length, long k);
    NTL::ZZ Round_ZZ(const NTL::ZZ& x_q, general_data* data);
    void benchmark_ntl_mul(long size, long iterations, const char* p_str);    
    void benchmark_ntl_add_mat(long size, long iterations, const char* p_str);
    void benchmark_ntl_setup(long n, long m, long q_length, long p_length, long times);
    Public_Key generate_public_key(const LWE_Keypair& key, const NTL::vec_ZZ_p& s_share);     
    void Setup(const char* lambda, long n, long m, long q_length, long p_length);
    NTL::ZZ_p generate_A_ij(const uint8_t* seed, long i, long j);
    void generate_A_row(const uint8_t* seed, long row_i, long num_cols, NTL::vec_ZZ& output_row, std::vector<uint8_t>& plaintext, std::vector<uint8_t>& ciphertext, NTL::ZZ& temp, const NTL::ZZ& q);
    NTL::vec_ZZ_p generate_sparse_ternary_vec(long length, long hw);
    LWE_Keypair Gen(const char* lambda, long n, long m);
    NTL::vec_ZZ generate_PRF_mask(const uint8_t* prf_key, long step_index, const NTL::ZZ& p, long row_length);
    NTL::mat_ZZ* OKDM(general_data* data, Public_Key* pk, const NTL::ZZ_p& message);
    void free_OKDM_matrix(NTL::mat_ZZ* matrix);
    NTL::vec_ZZ DDEC(const NTL::mat_ZZ* input_value, const NTL::vec_ZZ& memory_value, const uint8_t* prf_key, long step_index, general_data* data);
    void run_benchmark_OKDM(long n, long m, long q_len, long p_len, int iterations);
    void run_benchmark_DDEC(long n, long m, long q_len, long p_len, int iterations);
    void benchmark_OKDM(int iterations, general_data* data, Public_Key* pk, const NTL::ZZ_p& message);
    void benchmark_DDEC(int iterations, const NTL::mat_ZZ* input_value, const NTL::vec_ZZ& memory_value, const uint8_t* prf_key, long step_index, general_data* data);
    void benchmark_ntl_add_memory_values(int b, const char* val1, const char* val2, const char* q,
                         const uint8_t prf_key, long step_index, long row_length, long iterations);
    NTL::vec_ZZ add_memory_values(int b, const NTL::vec_ZZ& val1, const NTL::vec_ZZ& val2,
                         const NTL::ZZ& q, const uint8_t prf_key, long step_index, long row_length);


    NTL::ZZ last_mul(const NTL::mat_ZZ* input_value, const NTL::vec_ZZ& memory_value, const uint8_t* prf_key, long step_index, general_data* data);
    void benchmark_last_mul(const NTL::mat_ZZ* input_value, const NTL::vec_ZZ& memory_value, const uint8_t* prf_key, long step_index, general_data* data, int iterations);
    NTL::ZZ_p last_mem_add(int b, const NTL::vec_ZZ& val1, const NTL::vec_ZZ& val2, general_data* data, const uint8_t* prf_key, long step_index);
    void benchmark_last_mem_add(int b, const NTL::vec_ZZ& val1, const NTL::vec_ZZ& val2, general_data* data, const uint8_t* prf_key, long step_index, int iterations);
    void run_benchmark_last_mul(long n, long m, long q_len, long p_len, int iterations);
    void run_benchmark_last_mem_add(int b, long n, long m, long q_len, long p_len, int iterations);
    void run_benchmark_add_memory_values(long n, long m, long q_len, long p_len, int iterations);

#ifdef __cplusplus
}

extern "C" {
    void aesni_ctr_encrypt(const uint8_t *key, const uint8_t *nonce,
                           const uint8_t *plaintext, uint8_t *ciphertext,
                           uint64_t length);
}
#endif

#endif
