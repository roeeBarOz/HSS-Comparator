#include <NTL/vec_ZZ_p.h>
#include <NTL/mat_ZZ_p.h>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include "vec_mat_ZZ_p_wrapper.h"

using namespace NTL;
using namespace std;

// Helper: Convert NTL object to C-string
template <typename T>
char* to_cstring(const T& obj) {
    ostringstream oss;
    oss << obj;
    string str = oss.str();
    return strdup(str.c_str());
}

// Helper: Parse C-string to NTL object
template <typename T>
void from_cstring(T& obj, const char* str) {
    stringstream ss(str);
    ss >> obj;
}

extern "C" {

    void free_vec_mat_string(char* str) {
        free(str);
    }

    // ================== Vector Operations ==================

    char* vec_zz_p_add(const char* a_str, const char* b_str) {
        vec_ZZ_p a, b;
        from_cstring(a, a_str);
        from_cstring(b, b_str);
        return to_cstring(a + b);
    }

    char* vec_zz_p_sub(const char* a_str, const char* b_str) {
        vec_ZZ_p a, b;
        from_cstring(a, a_str);
        from_cstring(b, b_str);
        return to_cstring(a - b);
    }

    char* vec_zz_p_mul_scalar(const char* vec_str, const char* scalar_str) {
        vec_ZZ_p v;
        ZZ_p s;
        from_cstring(v, vec_str);
        from_cstring(s, scalar_str);
        return to_cstring(v * s);
    }

    char* vec_zz_p_inner_product(const char* a_str, const char* b_str) {
        vec_ZZ_p a, b;
        ZZ_p res;
        from_cstring(a, a_str);
        from_cstring(b, b_str);
        InnerProduct(res, a, b);
        return to_cstring(res);
    }

    char* vec_zz_p_random(long length) {
        vec_ZZ_p v;
        v.SetLength(length);
        for(long i = 0; i < length; i++) random(v[i]);
        return to_cstring(v);
    }

    char* vec_zz_p_get(const char* vec_str, long index) {
        vec_ZZ_p v;
        from_cstring(v, vec_str);
        // Check bounds ideally, but NTL throws if out of bounds
        // Note: NTL vectors are 0-indexed
        return to_cstring(v[index]);
    }

    // ================== Matrix Operations ==================

    char* mat_zz_p_add(const char* A_str, const char* B_str) {
        mat_ZZ_p A, B;
        from_cstring(A, A_str);
        from_cstring(B, B_str);
        return to_cstring(A + B);
    }

    char* mat_zz_p_sub(const char* A_str, const char* B_str) {
        mat_ZZ_p A, B;
        from_cstring(A, A_str);
        from_cstring(B, B_str);
        return to_cstring(A - B);
    }

    char* mat_zz_p_mul(const char* A_str, const char* B_str) {
        mat_ZZ_p A, B;
        from_cstring(A, A_str);
        from_cstring(B, B_str);
        return to_cstring(A * B);
    }

    char* mat_zz_p_mul_vec(const char* A_str, const char* v_str) {
        mat_ZZ_p A;
        vec_ZZ_p v;
        from_cstring(A, A_str);
        from_cstring(v, v_str);
        return to_cstring(A * v);
    }

    char* mat_zz_p_mul_scalar(const char* A_str, const char* x_str) {
        mat_ZZ_p A;
        ZZ_p x;
        from_cstring(A, A_str);
        from_cstring(x, x_str);
        return to_cstring(A * x);
    }

    char* mat_zz_p_transpose(const char* A_str) {
        mat_ZZ_p A;
        from_cstring(A, A_str);
        return to_cstring(transpose(A));
    }

    char* mat_zz_p_inv(const char* A_str) {
        mat_ZZ_p A;
        from_cstring(A, A_str);
        return to_cstring(inv(A));
    }

    char* mat_zz_p_determinant(const char* A_str) {
        mat_ZZ_p A;
        from_cstring(A, A_str);
        return to_cstring(determinant(A));
    }

    char* mat_zz_p_random(long rows, long cols) {
        mat_ZZ_p A;
        A.SetDims(rows, cols);
        for(long i = 0; i < rows; i++)
            for(long j = 0; j < cols; j++)
                random(A[i][j]);
        return to_cstring(A);
    }

    char* mat_zz_p_get_row(const char* matrix_str, long row_idx) {
        mat_ZZ_p A;
        from_cstring(A, matrix_str);
        // Return the row as a vector string
        return to_cstring(A[row_idx]);
    }
}
