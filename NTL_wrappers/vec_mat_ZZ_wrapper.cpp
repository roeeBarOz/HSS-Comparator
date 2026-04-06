#include <NTL/vec_ZZ.h>
#include <NTL/mat_ZZ.h>
#include <NTL/vec_ZZ_p.h>
#include <NTL/mat_ZZ_p.h>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include "vec_mat_ZZ_wrapper.h"
#include <chrono>
#include <vector>

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

// Helper: Get modulus byte length
int get_modulus_byte_length() {
    ZZ mod = ZZ_p::modulus();
    return NumBytes(mod);
}

extern "C" {

    void free_vec_mat_string(unsigned char* str) {
        delete[] str;
    }

    // Universal Centered Modulo: Forces 'x' into the centered ring of 'modulus'
    ZZ centered_mod_ZZ(const ZZ& x, const ZZ& modulus) {
        ZZ res = x % modulus; 
        if (res > modulus / 2) {
            res -= modulus;
        }
        return res;
    }

    // ================== Vector Operations ==================

    unsigned char* vec_zz_add(const unsigned char* a_buf, const unsigned char* b_buf, long length) {
        vec_ZZ a, b;
        export_bytes_to_zz_vector(a_buf, length, a);
        export_bytes_to_zz_vector(b_buf, length, b);
        vec_ZZ result = a + b;
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[length * item_size];
        export_zz_vector_to_bytes(result, buffer);
        return buffer;
    }

    unsigned char* vec_zz_sub(const unsigned char* a_buf, const unsigned char* b_buf, long length) {
        vec_ZZ a, b;
        export_bytes_to_zz_vector(a_buf, length, a);
        export_bytes_to_zz_vector(b_buf, length, b);
        vec_ZZ result = a - b;
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[length * item_size];
        export_zz_vector_to_bytes(result, buffer);
        return buffer;
    }

    unsigned char* vec_zz_mul_scalar(const unsigned char* vec_buf, long length, const char* scalar_str) {
        vec_ZZ v;
        ZZ s;
        export_bytes_to_zz_vector(vec_buf, length, v);
        from_cstring(s, scalar_str);
        vec_ZZ result = v * s;
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[length * item_size];
        export_zz_vector_to_bytes(result, buffer);
        return buffer;
    }

    unsigned char* vec_zz_inner_product(const unsigned char* a_buf, const unsigned char* b_buf, long length) {
        vec_ZZ a, b;
        ZZ res;
        export_bytes_to_zz_vector(a_buf, length, a);
        export_bytes_to_zz_vector(b_buf, length, b);
        InnerProduct(res, a, b);
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[item_size];
        NTL::BytesFromZZ(buffer, res, item_size);
        return buffer;
    }

    unsigned char* vec_zz_p_random(long length) {
        vec_ZZ_p v = random_vec_ZZ_p(length);
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[length * item_size];
        export_zz_p_vector_to_bytes(v, buffer);
        return buffer;
    }

    unsigned char* vec_zz_get(const unsigned char* vec_buf, long length, long index) {
        vec_ZZ v;
        export_bytes_to_zz_vector(vec_buf, length, v);
        ZZ val = v[index];
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[item_size];
        NTL::BytesFromZZ(buffer, val, item_size);
        return buffer;
    }

    unsigned char* vec_zz_gaussian(long length, long k) {
        vec_ZZ v;
        v.SetLength(length);

        for (long i = 0; i < length; i++) {
            long a = 0;
            long b = 0;

            for (long j = 0; j < k; j++) {
                a += RandomBnd(2);
                b += RandomBnd(2);
            }

            long val = a - b; 
            v[i] = conv<ZZ>(val);
        }

        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[length * item_size];
        export_zz_vector_to_bytes(v, buffer);
        return buffer;
    }

    unsigned char* vec_zz_prepend_one(const unsigned char* vec_buf, long length) {
        vec_ZZ v;
        export_bytes_to_zz_vector(vec_buf, length, v);
        
        vec_ZZ res;
        res.SetLength(length + 1);
        
        res[0] = 1;
        for(long i = 0; i < length; i++) {
            res[i+1] = v[i];
        }
        
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[(length + 1) * item_size];
        export_zz_vector_to_bytes(res, buffer);
        return buffer;
    }

    unsigned char* vec_zz_create_e(const char* val_str, long length, long k) {
        vec_ZZ v;
        v.SetLength(length);
        
        ZZ val;
        from_cstring(val, val_str);
        
        if (length > k && k >= 0) {
            v[k] = val;
        }
        
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[length * item_size];
        export_zz_vector_to_bytes(v, buffer);
        return buffer;
    }

    unsigned char* vec_zz_random_binary(long length) {
        vec_ZZ v;
        v.SetLength(length);
        
        for(long i = 0; i < length; i++) {
            v[i] = conv<ZZ>(RandomBnd(2));
        }
        
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[length * item_size];
        export_zz_vector_to_bytes(v, buffer);
        return buffer;
    }

    vec_ZZ centered_mod_ZZ_vec(const vec_ZZ& vec, const ZZ& modulus) {
        long n = vec.length();
        vec_ZZ result;
        result.SetLength(n);
        for (long i = 0; i < n; ++i) {
            result[i] = centered_mod_ZZ(vec[i], modulus);
        }
        return result;
    }

    unsigned char* vec_add_scalar(const unsigned char* vec_buf, long length, const char* scalar_str) {
        vec_ZZ v;
        ZZ s;
        export_bytes_to_zz_vector(vec_buf, length, v);
        from_cstring(s, scalar_str);

        vec_ZZ res = v;
        for (long i = 0; i < length; ++i) {
            res[i] += s;
        }

        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[length * item_size];
        export_zz_vector_to_bytes(res, buffer);
        return buffer;
    }

    // ================== Matrix Operations ==================

    unsigned char* mat_zz_add(const unsigned char* A_buf, const unsigned char* B_buf, long size) {
        mat_ZZ A, B;
        export_bytes_to_zz_matrix(A_buf, size, A);
        export_bytes_to_zz_matrix(B_buf, size, B);
        mat_ZZ result = A + B;
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[size * size * item_size];
        export_zz_matrix_to_bytes(result, buffer);
        return buffer;
    }

    unsigned char* mat_zz_sub(const unsigned char* A_buf, const unsigned char* B_buf, long size) {
        mat_ZZ A, B;
        export_bytes_to_zz_matrix(A_buf, size, A);
        export_bytes_to_zz_matrix(B_buf, size, B);
        mat_ZZ result = A - B;
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[size * size * item_size];
        export_zz_matrix_to_bytes(result, buffer);
        return buffer;
    }

    unsigned char* mat_zz_mul(const unsigned char* A_buf, const unsigned char* B_buf, long size) {
        mat_ZZ A, B;
        export_bytes_to_zz_matrix(A_buf, size, A);
        export_bytes_to_zz_matrix(B_buf, size, B);
        mat_ZZ result = A * B;
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[size * size * item_size];
        export_zz_matrix_to_bytes(result, buffer);
        return buffer;
    }

    unsigned char* mat_zz_mul_vec(const unsigned char* A_buf, const unsigned char* v_buf, long size) {
        mat_ZZ_p A;
        vec_ZZ_p v;
        export_bytes_to_zz_p_matrix(A_buf, size, A);
        export_bytes_to_zz_p_vector(v_buf, size, v);
        vec_ZZ_p result = A * v;
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[size * item_size];
        export_zz_p_vector_to_bytes(result, buffer);
        return buffer;
    }

    unsigned char* mat_zz_mul_scalar(const unsigned char* A_buf, long size, const char* x_str) {
        mat_ZZ_p A;
        ZZ_p x;
        export_bytes_to_zz_p_matrix(A_buf, size, A);
        ZZ x_zz;
        from_cstring(x_zz, x_str);
        x = conv<ZZ_p>(x_zz);
        mat_ZZ_p result = A * x;
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[size * size * item_size];
        export_zz_p_matrix_to_bytes(result, buffer);
        return buffer;
    }

    unsigned char* mat_zz_transpose(const unsigned char* A_buf, long size) {
        mat_ZZ A;
        export_bytes_to_zz_matrix(A_buf, size, A);
        mat_ZZ result = transpose(A);
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[size * size * item_size];
        export_zz_matrix_to_bytes(result, buffer);
        return buffer;
    }

    unsigned char* mat_zz_inv(const unsigned char* A_buf, long size) {
        mat_ZZ A;
        export_bytes_to_zz_matrix(A_buf, size, A);
        mat_ZZ inv_A = inv(A);
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[size * size * item_size];
        export_zz_matrix_to_bytes(inv_A, buffer);
        return buffer;
    }

    unsigned char* mat_zz_p_random(long size) {
        mat_ZZ_p A = random_mat_ZZ_p(size, size);
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[size * size * item_size];
        export_zz_p_matrix_to_bytes(A, buffer);
        return buffer;
    }

    unsigned char* mat_zz_get_row(const unsigned char* matrix_buf, long size, long row_idx) {
        mat_ZZ A;
        export_bytes_to_zz_matrix(matrix_buf, size, A);
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[size * item_size];
        export_zz_vector_to_bytes(A[row_idx], buffer);
        return buffer;
    }

    unsigned char* mat_zz_negate(const unsigned char* matrix_buf, long size) {
        mat_ZZ A;
        export_bytes_to_zz_matrix(matrix_buf, size, A);
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[size * size * item_size];
        export_zz_matrix_to_bytes(-A, buffer);
        return buffer;
    }

    unsigned char* mat_zz_concat_col_first(const unsigned char* col_vec_buf, long col_len, const unsigned char* matrix_buf, long size) {
        vec_ZZ b;
        mat_ZZ A;
        export_bytes_to_zz_vector(col_vec_buf, col_len, b);
        export_bytes_to_zz_matrix(matrix_buf, size, A);

        if (b.length() != A.NumRows()) return nullptr;

        mat_ZZ res;
        res.SetDims(size, size + 1);

        for(long i = 0; i < size; i++) {
            res[i][0] = b[i];
            for(long j = 0; j < size; j++) {
                res[i][j+1] = A[i][j];
            }
        }
        
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[size * (size + 1) * item_size];
        export_zz_matrix_to_bytes(res, buffer);
        return buffer;
    }

    mat_ZZ centered_mod(const mat_ZZ& mat, const ZZ& modulus) {
        long rows = mat.NumRows();
        long cols = mat.NumCols();
        mat_ZZ result;
        result.SetDims(rows, cols);
        for (long i = 0; i < rows; ++i) {
            for (long j = 0; j < cols; ++j) {
                result[i][j] = centered_mod_ZZ(mat[i][j], modulus);
            }
        }
        return result;
    }

    unsigned char* mat_add_scalar(const unsigned char* A_buf, long size, const char* scalar_str) {
        mat_ZZ A;
        ZZ s;
        export_bytes_to_zz_matrix(A_buf, size, A);
        from_cstring(s, scalar_str);

        mat_ZZ res = A;
        for (long i = 0; i < size; ++i) {
            for (long j = 0; j < size; ++j) {
                res[i][j] += s;
            }
        }

        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[size * size * item_size];
        export_zz_matrix_to_bytes(res, buffer);
        return buffer;
    }

    // --- HSS operations ---

    ZZ Round_ZZ(const ZZ& x_q, const ZZ& p, const ZZ& q) {
        ZZ numerator = x_q * p;
        ZZ rounded_val;
        
        if (numerator >= 0) {
            rounded_val = (numerator + (q / 2)) / q;
        } else {
            rounded_val = (numerator - (q / 2)) / q;
        }

        return centered_mod_ZZ(rounded_val, p);
    }

    vec_ZZ Round_ZZ_vec(const vec_ZZ& x_q, const ZZ& p, const ZZ& q) {
        long n = x_q.length();
        vec_ZZ result;
        result.SetLength(n);

        ZZ q_half = q / 2; 

        for (long i = 0; i < n; ++i) {
            ZZ numerator = x_q[i] * p;
            ZZ rounded_val;
            
            if (numerator >= 0) {
                rounded_val = (numerator + q_half) / q;
            } else {
                rounded_val = (numerator - q_half) / q;
            }

            result[i] = centered_mod_ZZ(rounded_val, p);
        }

        return result;
    }

    unsigned char* DDEC(const unsigned char* s_buf, long s_len, const unsigned char* C_buf, long size, const char* p_str, const char* q_str) {
        ZZ p_zz, q_zz;
        from_cstring(p_zz, p_str);
        from_cstring(q_zz, q_str);
        vec_ZZ s_vec;
        mat_ZZ C_mat;
        
        export_bytes_to_zz_vector(s_buf, s_len, s_vec);
        export_bytes_to_zz_matrix(C_buf, size, C_mat);
        
        vec_ZZ raw_dots = s_vec * C_mat; 
        vec_ZZ dots_mod_q = centered_mod_ZZ_vec(raw_dots, q_zz);
        vec_ZZ rounded_vec = Round_ZZ_vec(dots_mod_q, p_zz, q_zz);

        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[s_len * item_size];
        export_zz_vector_to_bytes(centered_mod_ZZ_vec(rounded_vec, q_zz), buffer);
        return buffer;
    }

    unsigned char* OKDM(const char* x_str, const unsigned char* c_buf, long c_len, const char* p_str, const char* q_str) {
        ZZ x_zz, p_zz, q_zz;
        vec_ZZ c_vec;
        
        from_cstring(x_zz, x_str);
        from_cstring(p_zz, p_str);
        from_cstring(q_zz, q_str);
        export_bytes_to_zz_vector(c_buf, c_len, c_vec);
        
        mat_ZZ C;
        long d = c_vec.length();
        C.SetDims(d, d);
        
        ZZ Delta = q_zz / p_zz;
        ZZ scaled_x = Delta * x_zz;
        
        for (long i = 0; i < d; ++i) {
            for (long j = 0; j < d; ++j) {
                C[i][j] = c_vec[i];
            }
        }
        
        for (long j = 0; j < d; ++j) {
            C[j][j] += scaled_x;
            C[j][j] = centered_mod_ZZ(C[j][j], q_zz);
        }
        
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[d * d * item_size];
        export_zz_matrix_to_bytes(C, buffer);
        return buffer;
    }

    int find_zz_length(ZZ* num) {
        return NumBytes(*num);
    }

    unsigned char* add_vec_then_center(const unsigned char* vec_buf1, long length, const unsigned char* vec_buf2, const char* modulus_str) {
        vec_ZZ v1, v2;
        ZZ modulus;
        export_bytes_to_zz_vector(vec_buf1, length, v1);
        export_bytes_to_zz_vector(vec_buf2, length, v2);
        from_cstring(modulus, modulus_str);

        vec_ZZ sum = v1 + v2;

        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[length * item_size];
        export_zz_vector_to_bytes(centered_mod_ZZ_vec(sum, modulus), buffer);
        return buffer;
    }

    void export_zz_vector_to_bytes(const vec_ZZ& vec, unsigned char* buffer) {
        long length = vec.length();
        int item_size = get_modulus_byte_length();

        for (long i = 0; i < length; i++) {
            unsigned char* current_pos = buffer + (i * item_size);
            const ZZ& val = vec[i];
            NTL::BytesFromZZ(current_pos, val, item_size);
        }
    }

    void export_zz_p_vector_to_bytes(const NTL::vec_ZZ_p& vec, unsigned char* buffer) {
        long length = vec.length();
        int item_size = get_modulus_byte_length();

        for (long i = 0; i < length; i++) {
            unsigned char* current_pos = buffer + (i * item_size);
            const NTL::ZZ& val = NTL::rep(vec[i]);
            NTL::BytesFromZZ(current_pos, val, item_size);
        }
    }

    void benchmark_ntl_mul(long size, long iterations, const char* p_str) {
        NTL::ZZ p = NTL::to_ZZ(p_str);
        NTL::ZZ_p::init(p);

        std::vector<NTL::mat_ZZ_p> matrices(iterations);
        std::vector<NTL::vec_ZZ_p> vectors(iterations);
        std::vector<NTL::vec_ZZ_p> results(iterations);

        for (long i = 0; i < iterations; i++) {
            NTL::random(matrices[i], size, size);
            NTL::random(vectors[i], size);
        }

        std::cout << "Starting benchmark for " << iterations << " iterations..." << std::endl;
        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[size * item_size];

        auto start = std::chrono::high_resolution_clock::now();

        for (long i = 0; i < iterations; i++) {
            NTL::mul(results[i], matrices[i], vectors[i]);
            export_zz_p_vector_to_bytes(results[i], buffer);
        }

        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double> diff = end - start;
        double avg = (diff.count() / iterations) * 1000.0;

        std::cout << "Total time: " << diff.count() << " seconds" << std::endl;
        std::cout << "Average time per multiplication: " << avg << " ms" << std::endl;
        delete[] buffer;
    }

    void export_zz_matrix_to_bytes(const mat_ZZ& matrix, unsigned char* buffer) {
        long rows = matrix.NumRows();
        long cols = matrix.NumCols();
        int item_size = get_modulus_byte_length();

        for (long i = 0; i < rows; i++) {
            for (long j = 0; j < cols; j++) {
                unsigned char* current_pos = buffer + ((i * cols + j) * item_size);
                const ZZ& val = matrix[i][j];
                NTL::BytesFromZZ(current_pos, val, item_size);
            }
        }
    }

    void export_zz_p_matrix_to_bytes(const NTL::mat_ZZ_p& matrix, unsigned char* buffer) {
        long rows = matrix.NumRows();
        long cols = matrix.NumCols();
        int item_size = get_modulus_byte_length();

        for (long i = 0; i < rows; i++) {
            for (long j = 0; j < cols; j++) {
                unsigned char* current_pos = buffer + ((i * cols + j) * item_size);
                const NTL::ZZ& val = NTL::rep(matrix[i][j]);
                NTL::BytesFromZZ(current_pos, val, item_size);
            }
        }
    }

    void export_bytes_to_zz_vector(const unsigned char* buffer, long length, vec_ZZ& vec) {
        int item_size = get_modulus_byte_length();
        vec.SetLength(length);
        for (long i = 0; i < length; i++) {
            const unsigned char* current_pos = buffer + (i * item_size);
            ZZ val;
            NTL::ZZFromBytes(val, current_pos, item_size);
            vec[i] = val;
        }
    }

    void export_bytes_to_zz_p_vector(const unsigned char* buffer, long length, vec_ZZ_p& vec) {
        int item_size = get_modulus_byte_length();
        vec.SetLength(length);
        for (long i = 0; i < length; i++) {
            const unsigned char* current_pos = buffer + (i * item_size);
            ZZ val;
            NTL::ZZFromBytes(val, current_pos, item_size);
            vec[i] = conv<ZZ_p>(val);
        }
    }

    void export_bytes_to_zz_matrix(const unsigned char* buffer, long size, mat_ZZ& matrix) {
        int item_size = get_modulus_byte_length();
        matrix.SetDims(size, size);
        for (long i = 0; i < size; i++) {
            for (long j = 0; j < size; j++) {
                const unsigned char* current_pos = buffer + ((i * size + j) * item_size);
                ZZ val;
                NTL::ZZFromBytes(val, current_pos, item_size);
                matrix[i][j] = val;
            }
        }
    }

    void export_bytes_to_zz_p_matrix(const unsigned char* buffer, long size, mat_ZZ_p& matrix) {
        int item_size = get_modulus_byte_length();
        matrix.SetDims(size, size);
        for (long i = 0; i < size; i++) {
            for (long j = 0; j < size; j++) {
                const unsigned char* current_pos = buffer + ((i * size + j) * item_size);
                ZZ val;
                NTL::ZZFromBytes(val, current_pos, item_size);
                matrix[i][j] = conv<ZZ_p>(val);
            }
        }
    }

    void benchmark_ntl_add_mat(long size, long iterations, const char* p_str) {
        NTL::ZZ p = NTL::to_ZZ(p_str);
        NTL::ZZ_p::init(p);

        std::vector<NTL::mat_ZZ_p> matricesA(iterations);
        std::vector<NTL::mat_ZZ_p> matricesB(iterations);
        std::vector<NTL::mat_ZZ_p> results(iterations);

        for (long i = 0; i < iterations; i++) {
            NTL::random(matricesA[i], size, size);
            NTL::random(matricesB[i], size, size);
        }

        std::cout << "Starting benchmark for " << iterations << " iterations..." << std::endl;

        int item_size = get_modulus_byte_length();
        unsigned char* buffer = new unsigned char[size * size * item_size];

        auto start = std::chrono::high_resolution_clock::now();

        for (long i = 0; i < iterations; i++) {
            NTL::add(results[i], matricesA[i], matricesB[i]);
            export_zz_p_matrix_to_bytes(results[i], buffer);
        }

        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double> diff = end - start;
        double avg = (diff.count() / iterations) * 1000.0;

        std::cout << "Total time: " << diff.count() << " seconds" << std::endl;
        std::cout << "Average time per addition: " << avg << " ms" << std::endl;
        delete[] buffer;
    }

    ZZ* get_modulus() {
        return new ZZ(ZZ_p::modulus());
    }
}
