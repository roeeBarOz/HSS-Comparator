#ifndef NTL_VEC_MAT_LIB_H
#define NTL_VEC_MAT_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

    void free_vec_mat_string(char* str);
    ZZ centered_mod(const ZZ& x, const ZZ& modulus);

    // --- Vector Operations (vec_ZZ) ---
    char* vec_zz_add(const char* a_str, const char* b_str);
    char* vec_zz_sub(const char* a_str, const char* b_str);
    char* vec_zz_mul_scalar(const char* vec_str, const char* scalar_str);
    char* vec_zz_inner_product(const char* a_str, const char* b_str);
    char* vec_zz_p_random(long length);
    char* vec_zz_get(const char* vec_str, long index);
    char* vec_zz_gaussian(long length, long k);
    char* vec_zz_prepend_one(const char* vec_str);
    char* vec_zz_create_e(const char* val_str, long length, long k);
    char* vec_zz_random_binary(long length);
    vec_ZZ centered_mod(const vec_ZZ& vec, const ZZ& modulus);
    char* vec_add_scalar(const char* vec_str, const char* scalar_str);
    
    // --- Matrix Operations (mat_ZZ) ---
    char* mat_zz_add(const char* A_str, const char* B_str);
    char* mat_zz_sub(const char* A_str, const char* B_str);
    char* mat_zz_mul(const char* A_str, const char* B_str);          // Matrix * Matrix
    char* mat_zz_mul_vec(const char* A_str, const char* v_str);      // Matrix * Vector
    char* mat_zz_mul_scalar(const char* A_str, const char* x_str);   // Matrix * Scalar
    char* mat_zz_transpose(const char* A_str);
    char* mat_zz_inv(const char* A_str);                             // Invert matrix
    char* mat_zz_determinant(const char* A_str);
    char* mat_zz_p_random(long rows, long cols);
    char* mat_zz_get_row(const char* matrix_str, long row_idx);
    char* mat_zz_negate(const char* matrix_str);
    char* mat_zz_concat_col_first(const char* col_vec_str, const char* matrix_str);
    mat_ZZ centered_mod(const mat_ZZ& mat, const ZZ& modulus);
    char* mat_add_scalar(const char* A_str, const char* scalar_str);

    // --- HSS-specific Operations ---
    char* DDEC(const char* s, const char* C, const char* p, const char* q);
    ZZ Round(const ZZ& x_q, const ZZ& p, const ZZ& q);
    vec_ZZ Round(const vec_ZZ& x_q, const ZZ& p, const ZZ& q);
    char* OKDM(const char* x, const char* C, const char* p, const char* q);

#ifdef __cplusplus
}
#endif

#endif
