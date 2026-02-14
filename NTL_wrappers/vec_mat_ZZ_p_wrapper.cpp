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

    /**
     * Samples a vector from a Centered Binomial Distribution (CBD).
     * This is a fast, constant-time approximation of a Discrete Gaussian.
     * * @param length: Length of the vector (e.g., n or M)
     * @param k: The "width" parameter. 
     * k=2 (range -2..2) or k=3 (range -3..3) are standard for LWE.
     */
    char* vec_zz_p_gaussian(long length, long k) {
        vec_ZZ_p v;
        v.SetLength(length);

        for (long i = 0; i < length; i++) {
            long a = 0;
            long b = 0;

            // Simulate 2k coin flips
            for (long j = 0; j < k; j++) {
                // RandomBnd(2) returns 0 or 1 with 50% probability
                a += RandomBnd(2);
                b += RandomBnd(2);
            }

            // Result is in range [-k, k] and centered at 0
            long val = a - b; 

            // Convert to ZZ_p (handles negative wrapping automatically)
            v[i] = conv<ZZ_p>(val);
        }

        return to_cstring(v);
    }

    // --- Vector: Prepend 1 ---
    // Used to create s' = (1, s)
    char* vec_zz_p_prepend_one(const char* vec_str) {
        vec_ZZ_p v;
        from_cstring(v, vec_str);
        
        vec_ZZ_p res;
        res.SetLength(v.length() + 1);
        
        res[0] = 1; // Set first element to 1
        for(long i = 0; i < v.length(); i++) {
            res[i+1] = v[i];
        }
        
        return to_cstring(res);
    }

    // --- Vector: Create Scaled Basis Vector (val, 0, ..., 0) ---
    // Useful for constructing the message vector m = (m, 0, ..., 0)
    char* vec_zz_p_create_e1(const char* val_str, long length) {
        vec_ZZ_p v;
        v.SetLength(length); // NTL automatically initializes to zero
        
        ZZ_p val;
        from_cstring(val, val_str);
        
        if (length > 0) {
            v[0] = val;
        }
        
        return to_cstring(v);
    }

    // --- Vector: Sample Binary Vector ---
    // Samples a vector with elements uniformly from {0, 1}
    // Used for the randomness 'r' in LWE encryption
    char* vec_zz_p_random_binary(long length) {
        vec_ZZ_p v;
        v.SetLength(length);
        
        for(long i = 0; i < length; i++) {
            // RandomBnd(2) returns 0 or 1
            v[i] = conv<ZZ_p>(RandomBnd(2));
        }
        
        return to_cstring(v);
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

    // --- Matrix: Negate ---
    // Used if you need -A
    char* mat_zz_p_negate(const char* matrix_str) {
        mat_ZZ_p A;
        from_cstring(A, matrix_str);
        return to_cstring(-A);
    }

    // --- Matrix: Concatenate Column (Add b to start) ---
    // Used to create P = [b | A]
    char* mat_zz_p_concat_col_first(const char* col_vec_str, const char* matrix_str) {
        vec_ZZ_p b;
        mat_ZZ_p A;
        from_cstring(b, col_vec_str);
        from_cstring(A, matrix_str);

        // Validation: Rows must match
        if (b.length() != A.NumRows()) return nullptr;

        mat_ZZ_p res;
        long rows = A.NumRows();
        long cols = A.NumCols();
        
        // New dimensions: Same rows, Cols + 1
        res.SetDims(rows, cols + 1);

        for(long i = 0; i < rows; i++) {
            // Set first column to b[i]
            res[i][0] = b[i];
            
            // Copy rest of A
            for(long j = 0; j < cols; j++) {
                res[i][j+1] = A[i][j];
            }
        }
        
        return to_cstring(res);
    }
}
