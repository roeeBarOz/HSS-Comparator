#ifndef NTL_VEC_MAT_LIB_H
#define NTL_VEC_MAT_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

    void free_vec_mat_string(char* str);

    // --- Vector Operations (vec_ZZ_p) ---
    char* vec_zz_p_add(const char* a_str, const char* b_str);
    char* vec_zz_p_sub(const char* a_str, const char* b_str);
    char* vec_zz_p_mul_scalar(const char* vec_str, const char* scalar_str);
    char* vec_zz_p_inner_product(const char* a_str, const char* b_str);
    char* vec_zz_p_random(long length);
    char* vec_zz_p_get(const char* vec_str, long index);
    char* vec_zz_p_gaussian(long length, long k);
    char* vec_zz_p_prepend_one(const char* vec_str);
    char* vec_zz_p_create_e1(const char* val_str, long length);
    char* vec_zz_p_random_binary(long length);
    
    // --- Matrix Operations (mat_ZZ_p) ---
    char* mat_zz_p_add(const char* A_str, const char* B_str);
    char* mat_zz_p_sub(const char* A_str, const char* B_str);
    char* mat_zz_p_mul(const char* A_str, const char* B_str);          // Matrix * Matrix
    char* mat_zz_p_mul_vec(const char* A_str, const char* v_str);      // Matrix * Vector
    char* mat_zz_p_mul_scalar(const char* A_str, const char* x_str);   // Matrix * Scalar
    char* mat_zz_p_transpose(const char* A_str);
    char* mat_zz_p_inv(const char* A_str);                             // Invert matrix
    char* mat_zz_p_determinant(const char* A_str);
    char* mat_zz_p_random(long rows, long cols);
    char* mat_zz_p_get_row(const char* matrix_str, long row_idx);
    char* mat_zz_p_negate(const char* matrix_str);
    char* mat_zz_p_concat_col_first(const char* col_vec_str, const char* matrix_str);

#ifdef __cplusplus
}
#endif

#endif
