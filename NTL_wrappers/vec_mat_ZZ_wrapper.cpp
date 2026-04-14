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

    void Setup(const char* lambda, long n, long m, long q_length, long p_length) {
        ZZ* p = new ZZ(INIT_SIZE, p_length);
        RandomPrime(*p, p_length, 100);
        ZZ* q_divided_by_p = new ZZ(INIT_SIZE, q_length - p_length);
        RandomPrime(*q_divided_by_p, q_length - p_length, 100);
        ZZ* q = new ZZ();
        *q = (*p) * (*q_divided_by_p);
        ZZ_p::init(*q);
        LWE_Keypair key = Gen(lambda, n, m);
        vec_ZZ_p s_0 = random_vec_ZZ_p(n);
        vec_ZZ_p s_1 = key.s - s_0;
        // TODO: need to send s_0, s_1, seed, b, prf_key to python
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

    void benchmark_ntl_gen_okdm_chunk(const uint8_t* seed_A, const char* b, const char* message, long start_row, long num_rows, long m, long iterations) {
        vec_ZZ_p b_vec;
        from_cstring(b_vec, b);
        ZZ_p message_zzp;
        from_cstring(message_zzp, message);
        auto start = std::chrono::high_resolution_clock::now();
        for (long i = 0; i < iterations; i++) {
            Gen_OKDM_Chunk(seed_A, b_vec, message_zzp, start_row, num_rows, m);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        double avg = (diff.count() / iterations) * 1000.0;
        std::cout << "Average time per Gen_OKDM_Chunk: " << avg << " ms" << std::endl;
    }

    std::vector<vec_ZZ_p> Gen_OKDM_Chunk(const uint8_t* seed_A, const vec_ZZ_p& b, const ZZ_p& message, long start_row, long num_rows, long m) {
        std::vector<vec_ZZ_p> chunk(num_rows);
        int n = b.length() - 1; // since b is (n+1) long after prepending the message
        int d = b.length(); // dimension of the output vector (which is n+1)

        for (long i = 0; i < num_rows; i++) {
            chunk[i].SetLength(d);
            clear(chunk[i]);
        }

        std::vector<std::vector<Request>> needed_indices(m);

        int hw_r = 2048; // the hamming weight of r, might need to be modified.
        for (long i = 0; i < num_rows; i++) {
            long non_zeros = 0;
            while (non_zeros < hw_r) 
            {
                long idx = rand() % m;
                needed_indices[idx].push_back({(int)i, (rand() % 2 == 0) ? 1 : -1});
                non_zeros++; 
            }
        }

        for (int i = 0; i < m; i++) {
            if (needed_indices[i].empty()) continue;

            vec_ZZ_p A_row = generate_A_row(seed_A, i, n);

            for (const auto& req : needed_indices[i]) {
                for (long j = 0; j < n; j++) {
                    chunk[req.chunk_row][j] += req.sign * A_row[j];
                }
                chunk[req.chunk_row][n] += req.sign * b[i];
            }
        }

        for (long i = 0; i < num_rows; i++) {
            long global_row_idx = start_row + i;
            if (global_row_idx <= n) {
                chunk[i][global_row_idx] += message; // Add the message to the diagonal element
            }
        }

        return chunk;
    }

    void save_chunk_to_file(const std::vector<vec_ZZ_p>& chunk, std::ofstream& out_file) {
        long num_rows = chunk.size();
        long row_length = chunk[0].length();
        int item_size_in_bytes = get_modulus_byte_length();
        long chunk_size = num_rows * row_length * item_size_in_bytes;
        std::vector<uint8_t> chunk_buffer(chunk_size, 0);

        for (long i = 0; i < num_rows; i++) {
            for (long j = 0; j < row_length; j++) {
                unsigned char* current_pos = chunk_buffer.data() + ((i * row_length + j) * item_size_in_bytes);
                NTL::BytesFromZZ(current_pos, rep(chunk[i][j]), item_size_in_bytes);
            }
        }

        out_file.write(reinterpret_cast<const char*>(chunk_buffer.data()), chunk_size);
    }

    void OKDM(const uint8_t* seed, const vec_ZZ_p& b, const ZZ_p& message, std::string filename, long m) {
        std::ofstream out_file(filename, std::ios::binary);
        if (!out_file.is_open()) {
            throw std::runtime_error("Failed to open output file");
        }

        for (long start_row = 0; start_row < b.length(); start_row += CHUNK_SIZE) {
            long current_chunk_size = std::min(CHUNK_SIZE, b.length() - start_row);
            std::vector<vec_ZZ_p> chunk = Gen_OKDM_Chunk(seed, b, message, start_row, current_chunk_size, m);
            save_chunk_to_file(chunk, out_file);
        }
        out_file.close();
    }

    std::vector<vec_ZZ_p> Load_OKDM_Chunk(std::ifstream& in_file, long num_rows, long row_length) {
        int item_size_in_bytes = get_modulus_byte_length();
        long chunk_size = num_rows * row_length * item_size_in_bytes;
        std::vector<uint8_t> chunk_buffer(chunk_size, 0);
        in_file.read(reinterpret_cast<char*>(chunk_buffer.data()), chunk_size);

        std::vector<vec_ZZ_p> chunk(num_rows);
        for (long i = 0; i < num_rows; i++) {
            chunk[i].SetLength(row_length);
            for (long j = 0; j < row_length; j++) {
                unsigned char* current_pos = chunk_buffer.data() + ((i * row_length + j) * item_size_in_bytes);
                ZZ val = ZZFromBytes(current_pos, item_size_in_bytes);
                chunk[i][j] = conv<ZZ_p>(val);
            }
        }
        return chunk;
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

    vec_ZZ DDEC(int b, const vec_ZZ_p& memory_value, const std::string& matrix_filename, 
                const uint8_t prf_key, long step_index, const ZZ& p, const ZZ& q) {
        vec_ZZ_p output;
        long row_length = memory_value.length();
        output.SetLength(row_length);

        std::ifstream in_file(matrix_filename, std::ios::binary);
        if (!in_file.is_open()) {
            throw std::runtime_error("Failed to open matrix file");
        }

        for (long start_row = 0; start_row < row_length; start_row += CHUNK_SIZE) {
            long current_chunk_size = std::min(CHUNK_SIZE, row_length - start_row);
            std::vector<vec_ZZ_p> chunk = Load_OKDM_Chunk(in_file, current_chunk_size, row_length);

            for (long i = 0; i < current_chunk_size; i++) {
                for (long j = 0; j < row_length; j++) {
                    output[j] += chunk[i][j] * memory_value[start_row + i];
                }
            }
        }
        in_file.close();
        vec_ZZ mask = generate_PRF_mask(&prf_key, step_index, p, row_length);
        vec_ZZ result;
        result.SetLength(row_length);
        for (long i = 0; i < row_length; i++) { 
            if (b == 0) result[i] = Round_ZZ(rep(output[i]), p, q) + mask[i] % q;
            else result[i] = Round_ZZ(rep(output[i]), p, q) - mask[i] % q;
        }
        return result;
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
