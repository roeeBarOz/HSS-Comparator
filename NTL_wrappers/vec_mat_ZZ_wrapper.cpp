#include <NTL/vec_ZZ.h>
#include <NTL/mat_ZZ.h>
#include <NTL/vec_ZZ_p.h>
#include <NTL/mat_ZZ_p.h>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include "vec_mat_ZZ_wrapper.h"

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

    // Universal Centered Modulo: Forces 'x' into the centered ring of 'modulus'
    ZZ centered_mod(const ZZ& x, const ZZ& modulus) {
        // NTL's % operator strictly returns a positive value in [0, modulus-1]
        ZZ res = x % modulus; 
        
        // Shift to the negative half if necessary
        if (res > modulus / 2) {
            res -= modulus;
        }
        return res;
    }

    // ================== Vector Operations ==================

    char* vec_zz_add(const char* a_str, const char* b_str) {
        vec_ZZ a, b;
        from_cstring(a, a_str);
        from_cstring(b, b_str);
        return to_cstring(a + b);
    }

    char* vec_zz_sub(const char* a_str, const char* b_str) {
        vec_ZZ a, b;
        from_cstring(a, a_str);
        from_cstring(b, b_str);
        return to_cstring(a - b);
    }

    char* vec_zz_mul_scalar(const char* vec_str, const char* scalar_str) {
        vec_ZZ v;
        ZZ s;
        from_cstring(v, vec_str);
        from_cstring(s, scalar_str);
        return to_cstring(v * s);
    }

    char* vec_zz_inner_product(const char* a_str, const char* b_str) {
        vec_ZZ a, b;
        ZZ res;
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

    char* vec_zz_get(const char* vec_str, long index) {
        vec_ZZ v;
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
    char* vec_zz_gaussian(long length, long k) {
        vec_ZZ v;
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

            // Convert to ZZ (handles negative wrapping automatically)
            v[i] = conv<ZZ>(val);
        }

        return to_cstring(v);
    }

    // --- Vector: Prepend 1 ---
    // Used to create s' = (1, s)
    char* vec_zz_prepend_one(const char* vec_str) {
        vec_ZZ v;
        from_cstring(v, vec_str);
        
        vec_ZZ res;
        res.SetLength(v.length() + 1);
        
        res[0] = 1; // Set first element to 1
        for(long i = 0; i < v.length(); i++) {
            res[i+1] = v[i];
        }
        
        return to_cstring(res);
    }

    // --- Vector: Create Scaled Basis Vector (val, 0, ..., 0) ---
    // Useful for constructing the message vector m = (m, 0, ..., 0)
    char* vec_zz_create_e(const char* val_str, long length, long k) {
        vec_ZZ v;
        v.SetLength(length); // NTL automatically initializes to zero
        
        ZZ val;
        from_cstring(val, val_str);
        
        if (length > k && k >= 0) {
            v[k] = val;
        }
        
        return to_cstring(v);
    }

    // --- Vector: Sample Binary Vector ---
    // Samples a vector with elements uniformly from {0, 1}
    // Used for the randomness 'r' in LWE encryption
    char* vec_zz_random_binary(long length) {
        vec_ZZ v;
        v.SetLength(length);
        
        for(long i = 0; i < length; i++) {
            // RandomBnd(2) returns 0 or 1
            v[i] = conv<ZZ>(RandomBnd(2));
        }
        
        return to_cstring(v);
    }

    // Vector Centered Modulo
    vec_ZZ centered_mod(const vec_ZZ& vec, const ZZ& modulus) {
        long n = vec.length();
        vec_ZZ result;
        result.SetLength(n);
        for (long i = 0; i < n; ++i) {
            result[i] = centered_mod(vec[i], modulus);
        }
        return result;
    }

    char* vec_add_scalar(const char* vec_str, const char* scalar_str) {
        vec_ZZ v;
        ZZ s;
        from_cstring(v, vec_str);
        from_cstring(s, scalar_str);

        vec_ZZ res = v;
        long n = v.length();
        for (long i = 0; i < n; ++i) {
            res[i] += s;
        }

        return to_cstring(res);
    }

    // ================== Matrix Operations ==================

    char* mat_zz_add(const char* A_str, const char* B_str) {
        mat_ZZ A, B;
        from_cstring(A, A_str);
        from_cstring(B, B_str);
        return to_cstring(A + B);
    }

    char* mat_zz_sub(const char* A_str, const char* B_str) {
        mat_ZZ A, B;
        from_cstring(A, A_str);
        from_cstring(B, B_str);
        return to_cstring(A - B);
    }

    char* mat_zz_mul(const char* A_str, const char* B_str) {
        mat_ZZ A, B;
        from_cstring(A, A_str);
        from_cstring(B, B_str);
        return to_cstring(A * B);
    }

    char* mat_zz_mul_vec(const char* A_str, const char* v_str) {
        mat_ZZ A;
        vec_ZZ v;
        from_cstring(A, A_str);
        from_cstring(v, v_str);
        return to_cstring(A * v);
    }

    char* mat_zz_mul_scalar(const char* A_str, const char* x_str) {
        mat_ZZ A;
        ZZ x;
        from_cstring(A, A_str);
        from_cstring(x, x_str);
        return to_cstring(A * x);
    }

    char* mat_zz_transpose(const char* A_str) {
        mat_ZZ A;
        from_cstring(A, A_str);
        return to_cstring(transpose(A));
    }

    char* mat_zz_inv(const char* A_str) {
        mat_ZZ A;
        from_cstring(A, A_str);
        return to_cstring(inv(A));
    }

    char* mat_zz_determinant(const char* A_str) {
        mat_ZZ A;
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

    char* mat_zz_get_row(const char* matrix_str, long row_idx) {
        mat_ZZ A;
        from_cstring(A, matrix_str);
        // Return the row as a vector string
        return to_cstring(A[row_idx]);
    }

    // --- Matrix: Negate ---
    // Used if you need -A
    char* mat_zz_negate(const char* matrix_str) {
        mat_ZZ A;
        from_cstring(A, matrix_str);
        return to_cstring(-A);
    }

    // --- Matrix: Concatenate Column (Add b to start) ---
    // Used to create P = [b | A]
    char* mat_zz_concat_col_first(const char* col_vec_str, const char* matrix_str) {
        vec_ZZ b;
        mat_ZZ A;
        from_cstring(b, col_vec_str);
        from_cstring(A, matrix_str);

        // Validation: Rows must match
        if (b.length() != A.NumRows()) return nullptr;

        mat_ZZ res;
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

    // Matrix Centered Modulo
    mat_ZZ centered_mod(const mat_ZZ& mat, const ZZ& modulus) {
        long rows = mat.NumRows();
        long cols = mat.NumCols();
        mat_ZZ result;
        result.SetDims(rows, cols);
        for (long i = 0; i < rows; ++i) {
            for (long j = 0; j < cols; ++j) {
                result[i][j] = centered_mod(mat[i][j], modulus);
            }
        }
        return result;
    }

    char* mat_add_scalar(const char* A_str, const char* scalar_str) {
        mat_ZZ A;
        ZZ s;
        from_cstring(A, A_str);
        from_cstring(s, scalar_str);

        mat_ZZ res = A;
        long rows = A.NumRows();
        long cols = A.NumCols();

        for (long i = 0; i < rows; ++i) {
            for (long j = 0; j < cols; ++j) {
                res[i][j] += s;
            }
        }

        return to_cstring(res);
    }

    // --- HSS operations ---

    ZZ Round(const ZZ& x_q, const ZZ& p, const ZZ& q) {
        
        // 1. Raw integer rounding: \lfloor (p/q) * x \rceil
        // Because x_q can actually be negative now, we use a standard rounding trick
        // that safely handles negative numerators in C++ integer division.
        ZZ numerator = x_q * p;
        ZZ rounded_val;
        
        if (numerator >= 0) {
            rounded_val = (numerator + (q / 2)) / q;
        } else {
            rounded_val = (numerator - (q / 2)) / q;
        }

        // 2. Force the result strictly into R_p
        return centered_mod(rounded_val, p);
    }

    vec_ZZ Round(const vec_ZZ& x_q, const ZZ& p, const ZZ& q) {
        long n = x_q.length();
        vec_ZZ result;
        result.SetLength(n);

        // Precompute q/2 once for the whole vector
        ZZ q_half = q / 2; 

        for (long i = 0; i < n; ++i) {
            // Raw integer rounding: \lfloor (p/q) * x \rceil
            ZZ numerator = x_q[i] * p;
            ZZ rounded_val;
            
            // Safely handle C++ integer division for both positive and negative values
            if (numerator >= 0) {
                rounded_val = (numerator + q_half) / q;
            } else {
                rounded_val = (numerator - q_half) / q;
            }

            // Force the result strictly into R_p using your centered_mod function
            result[i] = centered_mod(rounded_val, p);
        }

        return result;
    }

    char* DDEC(const char* s, const char* C, const char* p, const char* q) {
        
        // 1. Matrix-Vector Multiplication 
        // Uses NTL's native unbounded ZZ multiplication (or your custom mat_vec_mul)
        ZZ p_zz, q_zz;
        from_cstring(p_zz, p);
        from_cstring(q_zz, q);
        vec_ZZ s_vec;
        from_cstring(s_vec, s);
        mat_ZZ C_mat;
        from_cstring(C_mat, C);
        vec_ZZ raw_dots = s_vec * C_mat; 

        // 2. Force the raw dot products into the centered R_q ring
        vec_ZZ dots_mod_q = centered_mod(raw_dots, q_zz);

        // 3. Apply the vectorized Algorithm 7 (Rounding) 
        vec_ZZ rounded_vec = Round(dots_mod_q, p_zz, q_zz);

        // 4. Lift back to R_q
        return to_cstring(centered_mod(rounded_vec, q_zz));
    }

    char* OKDM(const char* x, const char* c, const char* p, const char* q) {

        ZZ x_zz, p_zz, q_zz;
        from_cstring(x_zz, x);
        from_cstring(p_zz, p);
        from_cstring(q_zz, q);
        vec_ZZ c_vec;
        from_cstring(c_vec, c);
        mat_ZZ C;
        long d = c_vec.length();
        C.SetDims(d, d);
        
        // 1. Calculate the scaling factor Delta = \lfloor q/p \rfloor
        // In C++, integer division automatically computes the floor.
        ZZ Delta = q_zz / p_zz;
        ZZ scaled_x = Delta * x_zz;
        
        // 2. Fill the matrix so every column is the base ciphertext vector 'c'
        for (long i = 0; i < d; ++i) {
            for (long j = 0; j < d; ++j) {
                C[i][j] = c_vec[i];
            }
        }
        
        // 3. Add the scaled plaintext to the diagonal (where i == j)
        for (long j = 0; j < d; ++j) {
            C[j][j] += scaled_x;
            
            // 4. Force ONLY the modified diagonal element back into the centered R_q ring
            // (The rest of the matrix is already safely in R_q from the base vector 'c')
            C[j][j] = centered_mod(C[j][j], q_zz);
        }
        
        return to_cstring(C);
    }
}
