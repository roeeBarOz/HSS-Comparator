#ifndef NTL_VEC_MAT_LIB_H
#define NTL_VEC_MAT_LIB_H
#include <NTL/ZZ.h>
#include <NTL/vec_ZZ.h>
#include <NTL/mat_ZZ.h>

#ifdef __cplusplus
extern "C" {
#endif

    struct LWE_Keypair {
        NTL::vec_ZZ_p s;       // Secret Key (Sparse)
        uint8_t seed[16]; // Public Key A (Compressed into a 16-byte seed)
        NTL::vec_ZZ_p b;       // Public Key b (b = A*s + e)
        uint8_t prf_key[16]; // PRF key for generating A rows on-the-fly
    };

    int get_modulus_byte_length();    
    void free_vec_mat_string(unsigned char* str);
    NTL::ZZ centered_mod_ZZ(const NTL::ZZ& x, const NTL::ZZ& modulus);

    // --- HSS-specific Operations ---
    NTL::vec_ZZ generate_gaussian_vec(long length, long k);
    NTL::ZZ Round_ZZ(const NTL::ZZ& x_q, const NTL::ZZ& p, const NTL::ZZ& q);
    void benchmark_ntl_mul(long size, long iterations, const char* p_str);    
    void benchmark_ntl_add_mat(long size, long iterations, const char* p_str);
    char* Setup(const char* lambda, long n, long m, long q_length, long p_length);
    NTL::ZZ_p generate_A_ij(const uint8_t* seed, long i, long j);
    NTL::vec_ZZ_p generate_A_row(const uint8_t* seed, long row_i, long length);
    NTL::vec_ZZ_p generate_sparse_ternary_vec(long length, long hw);
    LWE_Keypair Gen(const char* lambda, long n, long m);
    std::vector<NTL::vec_ZZ_p> Gen_OKDM_Chunk(const uint8_t* seed_A, const NTL::vec_ZZ_p& b, const NTL::ZZ_p& message, long start_row, long num_rows, long m);
    void save_chunk_to_file(const std::vector<NTL::vec_ZZ_p>& chunk, std::ofstream& out_file);
    void OKDM(const uint8_t* seed, const NTL::vec_ZZ_p& b, const NTL::ZZ_p& message, std::string filename, long m);
    std::vector<NTL::vec_ZZ_p> Load_OKDM_Chunk(std::ifstream& in_file, long num_rows, long row_length);
    NTL::vec_ZZ generate_PRF_mask(const uint8_t* prf_key, long step_index, const NTL::ZZ& p, long row_length);
    NTL::vec_ZZ DDEC(int b, const NTL::vec_ZZ_p& memory_value, const std::string& matrix_filename, 
                const uint8_t prf_key, long step_index, const NTL::ZZ& p, const NTL::ZZ& q);
    NTL::vec_ZZ add_memory_values(int b, const NTL::vec_ZZ& val1, const NTL::vec_ZZ& val2,
                const NTL::ZZ& q, const uint8_t prf_key, long step_index, long row_length);

#ifdef __cplusplus
}
#endif

#endif
