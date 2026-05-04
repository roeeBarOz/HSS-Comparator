#include <NTL/vec_ZZ.h>
#include <NTL/mat_ZZ.h>
#include <NTL/vec_ZZ_p.h>
#include <NTL/mat_ZZ_p.h>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include "vec_mat_ZZ_wrapper.h"
#include <chrono>
#include <vector>
#include "ZZ_wrapper.h"

using namespace NTL;
using namespace std;

long CHUNK_SIZE = 1024; // Number of rows per chunk for OKDM, can be tuned based on memory constraints

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

    vec_ZZ generate_gaussian_vec(long length, long k) {
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

        return v;
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
        }

        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double> diff = end - start;
        double avg = (diff.count() / iterations) * 1000.0;

        std::cout << "Total time: " << diff.count() << " seconds" << std::endl;
        std::cout << "Average time per multiplication: " << avg << " ms" << std::endl;
        delete[] buffer;
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
        }

        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double> diff = end - start;
        double avg = (diff.count() / iterations) * 1000.0;

        std::cout << "Total time: " << diff.count() << " seconds" << std::endl;
        std::cout << "Average time per addition: " << avg << " ms" << std::endl;
        delete[] buffer;
    }

    void benchmark_ntl_setup(long n, long m, long q_length, long p_length, long times) {
        char* lambda;
        string lambda_str = "128"; // Example security parameter, can be modified as needed
        lambda = strdup(lambda_str.c_str());
        auto start = std::chrono::high_resolution_clock::now();
        for (long i = 0; i < times; i++) {
            Setup(lambda, n, m, q_length, p_length);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        double avg = (diff.count() / times) * 1000.0;
        std::cout << "Average time per Setup: " << avg << " ms" << std::endl;
    }

    Public_Key generate_public_key(const LWE_Keypair& key, const vec_ZZ_p& s_share) {
        Public_Key pk;
        std::memcpy(pk.seed, key.seed, 16);
        std::memcpy(pk.prf_key, key.prf_key, 16);
        pk.b = key.b;
        pk.s = s_share;
        return pk;
    }

    void Setup(const char* lambda, long n, long m, long q_length, long p_length) {
        ZZ* p = new ZZ(INIT_SIZE, p_length);
        RandomPrime(*p, p_length, 100);
        ZZ* q_divided_by_p = new ZZ(INIT_SIZE, q_length - p_length);
        RandomPrime(*q_divided_by_p, q_length - p_length, 100);
        ZZ* q = new ZZ();
        *q = (*p) * (*q_divided_by_p);
        ZZ_p::init(*q);
        general_data* data = new general_data{n, m};
        LWE_Keypair key = Gen(lambda, n, m);
        vec_ZZ_p s_0 = random_vec_ZZ_p(n);
        vec_ZZ_p s_1 = key.s - s_0;
        Public_Key pk0, pk1;
        pk0 = generate_public_key(key, s_0);
        pk1 = generate_public_key(key, s_1);
    }

    // HELPER: Generates exactly one element of Matrix A at coordinate (row_i, col_j)
    ZZ_p generate_A_ij(const uint8_t* seed, long row_i, long col_j) {
        uint8_t plaintext[64] = {0};  // Dummy plaintext
        uint8_t ciphertext[64] = {0}; // Holds the AES keystream
        uint8_t nonce[16] = {0};

        // We make the generation uniquely deterministic by embedding the exact 
        // row and column indices directly into the 16-byte AES nonce.
        std::memcpy(nonce, &row_i, sizeof(long));
        std::memcpy(nonce + 8, &col_j, sizeof(long));

        // Blast 64 bytes of AES-CTR
        aesni_ctr_encrypt(seed, nonce, plaintext, ciphertext, 64);

        // Convert the raw AES bytes to NTL's BigInt, then to ZZ_p.
        // ZZ_p automatically applies modulo q based on your global ZZ_p::init() setup.
        ZZ val = ZZFromBytes(ciphertext, 64);
        return conv<ZZ_p>(val);
    }

    vec_ZZ_p generate_A_row(const uint8_t* seed, long row_i, long num_cols) {
        vec_ZZ_p row;
        row.SetLength(num_cols);
        uint64_t stream_length = num_cols * 64; // Each A_ij requires 64 bytes of AES keystream
        std::vector<uint8_t> plaintext(stream_length, 0);
        std::vector<uint8_t> ciphertext(stream_length, 0);
        uint8_t nonce[16] = {0};

        std::memcpy(nonce, &row_i, sizeof(long));
        aesni_ctr_encrypt(seed, nonce, plaintext.data(), ciphertext.data(), stream_length);

        for (long col_j = 0; col_j < num_cols; col_j++) {
            row[col_j] = conv<ZZ_p>(ZZFromBytes(ciphertext.data() + (col_j * 64), 64));
        }
        return row;
    }

    vec_ZZ_p generate_sparse_ternary_vec(long n, long hw) {
        vec_ZZ_p s;
        s.SetLength(n);

        // 1. Initialize the entire vector to strictly zero
        for (long i = 0; i < n; ++i) {
            s[i] = conv<ZZ_p>(0);
        }

        long non_zeros_added = 0;

        // 2. Loop until we have successfully placed exactly 'hw' elements
        while (non_zeros_added < hw) {
            // Pick a random index inside the vector bounds
            long idx = rand() % n; 

            // 3. Only assign a value if this index is currently empty (zero)
            if (IsZero(s[idx])) {
                
                // Randomly flip a coin to assign either 1 or -1
                if (rand() % 2 == 0) {
                    s[idx] = conv<ZZ_p>(1);
                } else {
                    // NTL safely wraps -1 to (q - 1) under the hood
                    s[idx] = conv<ZZ_p>(-1); 
                }
                
                non_zeros_added++;
            }
        }

        return s;
    }

    // MAIN GEN FUNCTION
    LWE_Keypair Gen(const char* lambda, long n, long m) {
        LWE_Keypair keys;

        // 1. Generate the secret s and error e
        keys.s = generate_sparse_ternary_vec(n, 100);
        vec_ZZ e = generate_gaussian_vec(m, 3); 
        
        // 2. Generate a random 16-byte seed for the public matrix A
        for (int i = 0; i < 16; i++) {
            keys.seed[i] = rand() % 256; 
            keys.prf_key[i] = rand() % 256; // Also generate a random PRF key
        }

        keys.b.SetLength(m);
        std::vector<long> nz_indices; // non-zero indices of s for efficient generation of b = A*s + e
        for (long j = 0; j < n; ++j) {
            if (!IsZero(keys.s[j])) {
                nz_indices.push_back(j);
            }
        }

        // 4. Compute b = A*s + e (Row by Row)
        for (long i = 0; i < m; ++i) {
            ZZ_p dot_product = conv<ZZ_p>(0);

            // ONLY generate the elements of A that correspond to a non-zero secret!
            for (long col_j : nz_indices) {
                ZZ_p A_ij = generate_A_ij(keys.seed, i, col_j);
                dot_product += A_ij * keys.s[col_j];
            }

            // Add the gaussian error for this specific row
            // b_i = (A_i * s) + e_i
            keys.b[i] = dot_product + conv<ZZ_p>(e[i]);
        }

        return keys;
    }

    vec_ZZ generate_PRF_mask(const uint8_t* prf_key, long step_index, const ZZ& p, long row_length) {
        vec_ZZ mask;
        mask.SetLength(row_length);
        uint64_t stream_length = row_length * 64;

        std::vector<uint8_t> plaintext(stream_length, 0);
        std::vector<uint8_t> ciphertext(stream_length, 0);
        uint8_t nonce[16] = {0};
        std::memcpy(nonce, &step_index, sizeof(long));
        aesni_ctr_encrypt(prf_key, nonce, plaintext.data(), ciphertext.data(), stream_length);

        for (long j = 0; j < row_length; j++) {
            mask[j] = ZZFromBytes(ciphertext.data() + (j * 64), 64);
        }

        return mask;
    }

    vec_ZZ_p generate_OKDM_row(general_data* data, long row_index, const uint8_t* pk_seed, const vec_ZZ_p& pk_b, const ZZ_p& message) {
        vec_ZZ_p row;
        row.SetLength(data->n + 1); // +1 for the OKDM ciphertext component
        for (long i = 0; i <= data->n; i++) row[i] = conv<ZZ_p>(0);

        for (long j = 0; j < data->m; j++) {
            int sign = (rand() % 2 == 0) ? 1 : -1;
            vec_ZZ_p A_row = generate_A_row(pk_seed, j, data->n);
            for (long k = 0; k < data->n; k++) {
                row[k] += sign * A_row[k];
            }
            row[data->n] = sign * pk_b[j];
        }

        row[row_index] += message; // Embed the message into the designated row
        return row;
    }

    mat_ZZ_p* OKDM(general_data* data, Public_Key* pk, const ZZ_p& message) {
        mat_ZZ_p* okdm_matrix = new mat_ZZ_p();
        okdm_matrix->SetDims(data->n + 1, data->n + 1);

        for (long i = 0; i <= data->n; i++) {
            vec_ZZ_p row = generate_OKDM_row(data, i, pk->seed, pk->b, message);
            (*okdm_matrix)[i] = row;
        }
        return okdm_matrix;
    }

    void free_OKDM_matrix(mat_ZZ_p* matrix) {
        delete matrix;
    }

    vec_ZZ_p DDEC(const mat_ZZ_p* input_value, const vec_ZZ_p& memory_value, const uint8_t* prf_key, long step_index, const ZZ& p, const ZZ& q) {
        vec_ZZ_p new_memory = memory_value * (*input_value);
        vec_ZZ mask = generate_PRF_mask(prf_key, step_index, p, new_memory.length());
        for (long i = 0; i < new_memory.length(); i++) {
            new_memory[i] = conv<ZZ_p>(Round_ZZ(conv<ZZ>(new_memory[i]) + mask[i], p, q));
        }
        return new_memory;
    }

    // Wrapper to set up the data and run the OKDM benchmark
    void run_benchmark_OKDM(long n, long m, const char* q_str, int iterations) {
        // 1. Initialize the global modulus
        ZZ q = to_ZZ(q_str);
        ZZ_p::init(q);

        // 2. Set up the data structures
        general_data* data = new general_data{n, m};
        LWE_Keypair key = Gen("128", n, m);
        
        // Generate a random share for the secret
        vec_ZZ_p s_0;
        s_0.SetLength(n);
        for (long i = 0; i < n; i++) random(s_0[i]);

        Public_Key pk = generate_public_key(key, s_0);
        ZZ_p message = conv<ZZ_p>(1); // Dummy message for benchmarking

        // 3. Run the core benchmark
        std::cout << "Starting OKDM Benchmark for " << iterations << " iterations..." << std::endl;
        benchmark_OKDM(iterations, data, &pk, message);

        // 4. Cleanup heap memory
        delete data;
    }

    // Wrapper to set up the data, generate a test matrix, and run the DDEC benchmark
    void run_benchmark_DDEC(long n, long m, const char* q_str, const char* p_str, int iterations) {
        // 1. Initialize the global modulus
        ZZ q = to_ZZ(q_str);
        ZZ p = to_ZZ(p_str);
        ZZ_p::init(q);

        // 2. Set up the base keys
        general_data data{n, m};
        LWE_Keypair key = Gen("128", n, m);
        
        vec_ZZ_p s_0;
        s_0.SetLength(n);
        for (long i = 0; i < n; i++) random(s_0[i]);

        Public_Key pk = generate_public_key(key, s_0);
        ZZ_p message = conv<ZZ_p>(1);

        // 3. Generate a single OKDM matrix to serve as the benchmark input
        std::cout << "Preparing 1 OKDM Matrix for DDEC Benchmark evaluation..." << std::endl;
        mat_ZZ_p* input_matrix = OKDM(&data, &pk, message);

        // 4. Generate a dummy memory state vector for the Server
        vec_ZZ_p memory_value;
        memory_value.SetLength(n + 1);
        for (long i = 0; i <= n; i++) memory_value[i] = conv<ZZ_p>(1);

        long step_index = 1;

        // 5. Run the core benchmark
        std::cout << "Starting DDEC Benchmark for " << iterations << " iterations..." << std::endl;
        benchmark_DDEC(iterations, input_matrix, memory_value, pk.prf_key, step_index, p, q);

        // 6. Cleanup heap memory
        free_OKDM_matrix(input_matrix);
    }

    void benchmark_OKDM(int iterations, general_data* data, Public_Key* pk, const ZZ_p& message) {
        auto start = std::chrono::high_resolution_clock::now();
        for (long i = 0; i < iterations; i++) {
            mat_ZZ_p* okdm_matrix = OKDM(data, pk, message);
            free_OKDM_matrix(okdm_matrix);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        double avg = (diff.count() / iterations) * 1000.0;
        std::cout << "Average time per OKDM: " << avg << " ms" << std::endl;
    }

    void benchmark_DDEC(int iterations, const mat_ZZ_p* input_value, const vec_ZZ_p& memory_value, const uint8_t* prf_key, long step_index, const ZZ& p, const ZZ& q) {
        auto start = std::chrono::high_resolution_clock::now();
        for (long i = 0; i < iterations; i++) {
            DDEC(input_value, memory_value, prf_key, step_index, p, q);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        double avg = (diff.count() / iterations) * 1000.0;
        std::cout << "Average time per DDEC: " << avg << " ms" << std::endl;
    }

    void benchmark_ntl_add_memory_values(int b, const char* val1, const char* val2, const char* q,
                            const uint8_t prf_key, long step_index, long row_length, long iterations) {
        auto start = std::chrono::high_resolution_clock::now();
        vec_ZZ vec_val1, vec_val2;
        from_cstring(vec_val1, val1);
        from_cstring(vec_val2, val2);
        ZZ q_zz;        
        from_cstring(q_zz, q);
        for (long i = 0; i < iterations; i++) {
            add_memory_values(b, vec_val1, vec_val2, q_zz, prf_key, step_index, row_length);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        double avg = (diff.count() / iterations) * 1000.0;
        std::cout << "Average time per add_memory_values: " << avg << " ms" << std::endl;
    }

    vec_ZZ add_memory_values(int b, const vec_ZZ& val1, const vec_ZZ& val2, const ZZ& q,
                            const uint8_t prf_key, long step_index, long row_length) {
        vec_ZZ result;
        long length = val1.length();
        result.SetLength(length);
        vec_ZZ mask = generate_PRF_mask(&prf_key, step_index, q, row_length);
        for (long i = 0; i < length; i++) {
            if (b == 0) result[i] = (val1[i] + val2[i] + mask[i]) % q;
            else result[i] = (val1[i] + val2[i] - mask[i] + q) % q; 
        }
        return result;
    }
}
