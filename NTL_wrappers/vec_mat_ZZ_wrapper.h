#ifndef NTL_VEC_MAT_LIB_H
#define NTL_VEC_MAT_LIB_H
#include <NTL/ZZ.h>
#include <NTL/vec_ZZ.h>
#include <NTL/mat_ZZ.h>

#ifdef __cplusplus
extern "C" {
#endif

    void free_vec_mat_string(unsigned char* str);
    NTL::ZZ centered_mod_ZZ(const NTL::ZZ& x, const NTL::ZZ& modulus);

    // --- Vector Operations (vec_ZZ) ---
    unsigned char* vec_zz_add(const unsigned char* a_buf, const unsigned char* b_buf, long length);
    unsigned char* vec_zz_sub(const unsigned char* a_buf, const unsigned char* b_buf, long length);
    unsigned char* vec_zz_mul_scalar(const unsigned char* vec_buf, long length, const char* scalar_str);
    unsigned char* vec_zz_inner_product(const unsigned char* a_buf, const unsigned char* b_buf, long length);
    unsigned char* vec_zz_p_random(long length);
    unsigned char* vec_zz_get(const unsigned char* vec_buf, long length, long index);
    unsigned char* vec_zz_gaussian(long length, long k);
    unsigned char* vec_zz_prepend_one(const unsigned char* vec_buf, long length);
    unsigned char* vec_zz_create_e(const char* val_str, long length, long k);
    unsigned char* vec_zz_random_binary(long length);
    NTL::vec_ZZ centered_mod_ZZ_vec(const NTL::vec_ZZ& vec, const NTL::ZZ& modulus);
    unsigned char* vec_add_scalar(const unsigned char* vec_buf, long length, const char* scalar_str);
    
    // --- Matrix Operations (mat_ZZ) ---
    unsigned char* mat_zz_add(const unsigned char* A_buf, const unsigned char* B_buf, long size);
    unsigned char* mat_zz_sub(const unsigned char* A_buf, const unsigned char* B_buf, long size);
    unsigned char* mat_zz_mul(const unsigned char* A_buf, const unsigned char* B_buf, long size);
    unsigned char* mat_zz_mul_vec(const unsigned char* A_buf, const unsigned char* v_buf, long size);
    unsigned char* mat_zz_mul_scalar(const unsigned char* A_buf, long size, const char* x_str);
    unsigned char* mat_zz_transpose(const unsigned char* A_buf, long size);
    unsigned char* mat_zz_inv(const unsigned char* A_buf, long size);
    unsigned char* mat_zz_p_random(long size);
    unsigned char* mat_zz_get_row(const unsigned char* matrix_buf, long size, long row_idx);
    unsigned char* mat_zz_negate(const unsigned char* matrix_buf, long size);
    unsigned char* mat_zz_concat_col_first(const unsigned char* col_vec_buf, long col_len, const unsigned char* matrix_buf, long size);
    unsigned char* mat_add_scalar(const unsigned char* A_buf, long size, const char* scalar_str);
    NTL::mat_ZZ centered_mod(const NTL::mat_ZZ& mat, const NTL::ZZ& modulus);

    // --- HSS-specific Operations ---
    unsigned char* DDEC(const unsigned char* s_buf, long s_len, const unsigned char* C_buf, long size, const char* p_str, const char* q_str);
    NTL::ZZ Round_ZZ(const NTL::ZZ& x_q, const NTL::ZZ& p, const NTL::ZZ& q);
    NTL::vec_ZZ Round_ZZ_vec(const NTL::vec_ZZ& x_q, const NTL::ZZ& p, const NTL::ZZ& q);
    unsigned char* OKDM(const char* x_str, const unsigned char* c_buf, long c_len, const char* p_str, const char* q_str);
    unsigned char* add_vec_then_center(const unsigned char* vec_buf1, long length, const unsigned char* vec_buf2, const char* modulus_str);

    // --- Conversion utilities ---
    void export_zz_vector_to_bytes(const NTL::vec_ZZ& vec, unsigned char* buffer);
    void export_zz_p_vector_to_bytes(const NTL::vec_ZZ_p& vec, unsigned char* buffer);
    void export_zz_matrix_to_bytes(const NTL::mat_ZZ& matrix, unsigned char* buffer);
    void export_zz_p_matrix_to_bytes(const NTL::mat_ZZ_p& matrix, unsigned char* buffer);
    void export_bytes_to_zz_vector(const unsigned char* buffer, long length, NTL::vec_ZZ& vec);
    void export_bytes_to_zz_p_vector(const unsigned char* buffer, long length, NTL::vec_ZZ_p& vec);
    void export_bytes_to_zz_matrix(const unsigned char* buffer, long size, NTL::mat_ZZ& matrix);
    void export_bytes_to_zz_p_matrix(const unsigned char* buffer, long size, NTL::mat_ZZ_p& matrix);
    int find_zz_length(NTL::ZZ* num);
    int get_modulus_byte_length();
    NTL::ZZ* get_modulus();

#ifdef __cplusplus
}
#endif

#endif
