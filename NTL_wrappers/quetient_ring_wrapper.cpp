#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pE.h>
#include <NTL/vector.h>
#include <cstdlib>
#include <random>
#include <algorithm>
#include <sstream>
#include <chrono>
#include "quetient_ring_wrapper.h"
#include "../PRF/aesni_ctr.c"

using namespace NTL;

vec_ZZ_pX* precomp = nullptr;
Context default_ctx = {ZZ(0), ZZ(0)};

extern "C" {

    ZZ_pX* from_string_poly(const char* str) {
        ZZ_pX* result = new ZZ_pX;
        std::istringstream iss(str);
        iss >> *result;
        return result;
    }

    void free_ZZ_pX(ZZ_pX* poly) {
        delete poly;
    }

    char* to_string_poly(const ZZ_pX* poly) {
        std::ostringstream oss;
        oss << *poly;
        std::string str = oss.str();
        return strdup(str.c_str());
    }

    void init_modulus(const ZZ* r, ZZ_pX* modulus) {
        ZZ_p::init(*r);
        ZZ_pE::init(*modulus);
    }

    void generate_distributions(ZZ_pX& raw_s_hat, ZZ_pX& raw_e, long n, long h_sk, double sigma) {
        // Set up standard C++ random number generation
        std::random_device rd;
        std::mt19937 gen(rd());

        // ---------------------------------------------------------
        // 1. Generate Secret Key (raw_s_hat): Sparse Ternary
        // ---------------------------------------------------------
        raw_s_hat = 0; // Clear polynomial
        
        // Create an array of indices from 0 to n-1, and shuffle them
        std::vector<long> indices(n);
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), gen);
        
        std::uniform_int_distribution<> coin(0, 1);
        
        // Pick the first h_sk indices to be non-zero
        for (long i = 0; i < h_sk; ++i) {
            long val = coin(gen) ? 1 : -1;
            SetCoeff(raw_s_hat, indices[i], to_ZZ_p(val)); 
        }

        // ---------------------------------------------------------
        // 2. Generate Error (raw_e): Discrete Gaussian
        // ---------------------------------------------------------
        raw_e = 0; // Clear polynomial
        std::normal_distribution<double> gaussian(0.0, sigma);
        
        for (long i = 0; i < n; ++i) {
            // Sample from the normal distribution and round to the nearest integer
            long val = std::round(gaussian(gen));
            
            // Only set the coefficient if it is non-zero to save memory/time
            if (val != 0) {
                SetCoeff(raw_e, i, to_ZZ_p(val));
            }
        }
    }

    void apply_prf(const uint8_t key[16], uint8_t id, ZZ_pE& output) {

        // Use a fixed nonce for simplicity; in practice, this should be unique per invocation
        uint8_t nonce[8] = {0}; // 64-bit nonce initialized to zero

        // Prepare the output buffer for the keystream
        uint8_t keystream[8192] = {id}; // AES block size is 8192 bytes

        // Generate the keystream using AES-CTR mode
        aesni_ctr_encrypt(key, nonce, keystream, keystream, 8192); // 8192 bytes for the polynomial coefficients

        // Convert the generated keystream into a polynomial in ZZ_pE
        ZZ_pX poly;
        for (int i = 0; i < 8192; ++i) {
            SetCoeff(poly, i, to_ZZ_p(keystream[i]));
        }
        
        output = conv<ZZ_pE>(poly);
    }

    void PRF(int b, const uint8_t key[16], const uint8_t id, Memory_Value& mem) {
        ZZ_pE p0, p1;
        apply_prf(key, id, p0);
        apply_prf(key, id, p1);

        if (b == 0) {
            mem.mem_0 += p0;
            mem.mem_1 += p1;
        } else {
            mem.mem_0 -= p0;
            mem.mem_1 -= p1;
        }
    }

    // LPR.Gen function, detailed at page 17
    PKE_Gen_keys PKE_Gen() {

        ZZ_pE a, b, s_hat;

        // 1. Generate 'a' uniformly at random from the ring
        random(a);

        // 2. Generate the raw polynomials for s_hat and e.
        ZZ_pX raw_s_hat; 
        ZZ_pX raw_e;
        
        generate_distributions(raw_s_hat, raw_e, 8192, 64, 8);

        // 3. Convert the raw polynomials into the cryptographic ring context.
        // Because raw_s_hat and raw_e have degrees strictly less than A(x),
        // this 'conv' is essentially a free memory cast.
        conv(s_hat, raw_s_hat);
        
        ZZ_pE e;
        conv(e, raw_e);

        // 4. The Cryptographic Computation
        // NTL natively computes the polynomial multiplication, adds the error, 
        // and continuously reduces everything modulo q and modulo A(x).
        b = a * s_hat + e; 

        public_key pk;
        pk.a = a;
        pk.b = b;

        PKE_Gen_keys keys;
        keys.pk = pk;
        keys.sk = s_hat; // sk should be (1, s_hat), but we handle it in HSS_Gen.

        return keys;
    }

    Context generate_context(int n, int log_p, int log_q) {
        Context ctx;
        // Should compute the size of q, p and the polynomial based on the security parameter
        // and the function computed, but for now, we will use fixed sizes for demonstration.
        ZZ p = RandomPrime_ZZ(log_p);
        ZZ q_div_by_p = RandomPrime_ZZ(log_q - log_p);
        ZZ q = p * q_div_by_p;
        ZZ_p::init(q);

        ZZ_pX modulus_poly;
        SetCoeff(modulus_poly, n, 1); // x^n
        SetCoeff(modulus_poly, 0, 1); // x^n + 1
        ZZ_pE::init(modulus_poly);

        ctx.p = p;
        ctx.q = q;

        return ctx;
    }

    HSS_Gen_keys HSS_Gen() {
        // Implementation for HSS key generation

        // setting seed for NTL random number generator
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<unsigned long> dis;
        SetSeed(conv<ZZ>(dis(gen)));

        PKE_Gen_keys pke_gen_output = PKE_Gen();
        ZZ_pE s_0_0, s_0_1, s_1_0, s_1_1;
        random(s_0_0);
        random(s_0_1);
        s_1_0 = s_0_0 - 1;
        s_1_1 = s_0_1 - pke_gen_output.sk;

        // Generate a random 128-bit key for AES
        uint8_t k[16];
        std::random_device rd_key;
        std::mt19937 gen_key(rd_key());
        std::uniform_int_distribution<> dis_key(0, 255);
        for (int i = 0; i < 16; ++i) {
            k[i] = dis_key(gen_key); // Random byte for AES key
        }

        eval_key ek_0, ek_1;
        memcpy(ek_0.prf_key, k, 16);
        memcpy(ek_1.prf_key, k, 16);
        ek_0.share_of_1 = s_0_0;
        ek_1.share_of_1 = s_1_0;
        ek_0.share_of_sk = s_0_1;
        ek_1.share_of_sk = s_1_1;

        HSS_Gen_keys keys;
        keys.eval_key0 = ek_0;
        keys.eval_key1 = ek_1;
        keys.pke_keys = pke_gen_output;
        return keys;
    }

    Input_Value HSS_Enc(const public_key& pk, const ZZ& x, const Context& ctx) {
        return OKDM(pk, x, ctx.p, ctx.q);
    }

    Memory_Value load(int b, int id, const Input_Value& input, const eval_key& ek, const Context& ctx) {
        Memory_Value result;
        result = DDEC(input, {ek.share_of_1, ek.share_of_sk}, ctx);
        PRF(b, ek.prf_key, id, result);
        return result;
    }

    Memory_Value add_memory_values(int b, int id, const Memory_Value& mem0, const Memory_Value& mem1, const eval_key& ek) {
        Memory_Value result;
        result.mem_0 = mem0.mem_0 + mem1.mem_0;
        result.mem_1 = mem0.mem_1 + mem1.mem_1;
        PRF(b, ek.prf_key, id, result);
        return result;
    }

    Input_Value add_input_values(const Input_Value& input0, const Input_Value& input1) {
        Input_Value result;
        result.c_00 = input0.c_00 + input1.c_00;
        result.c_01 = input0.c_01 + input1.c_01;
        result.c_10 = input0.c_10 + input1.c_10;
        result.c_11 = input0.c_11 + input1.c_11;
        return result;
    }

    Memory_Value multiply(int b, int id, const Input_Value& input, const Memory_Value& memory, const eval_key& ek, const Context& ctx) {
        Memory_Value result;
        result = DDEC(input, memory, ctx);
        PRF(b, ek.prf_key, id, result);
        return result;
    }

    // LPR.OKDM function, detailed at page 17
    Input_Value OKDM(const public_key& pk, const ZZ& x, const ZZ& p, const ZZ& q) {
        Input_Value result;
        ZZ_pX raw_v, raw_e0, raw_e1;
        ZZ_pE v, e0, e1, poly_x;

        ZZ_p x_p = conv<ZZ_p>(x * q / p); // Scale x to the modulus q

        encryption enc_0 = Enc(pk, conv<ZZ_p>(0));
        encryption enc_x = Enc(pk, x_p);

        result.c_00 = enc_x.c_0;
        result.c_10 = enc_x.c_1;
        result.c_01 = enc_0.c_0;
        result.c_11 = enc_0.c_1 + x_p;

        return result;
    }

    //LPR.Enc function, detailed at page 17
    encryption Enc(const public_key& pk, const ZZ_p& x) {
        encryption result;
        ZZ_pE r, e0, e1;
        ZZ_pX r_raw, e0_raw, e1_raw;
        generate_distributions(r_raw, e0_raw, 8192, 64, 8);
        generate_distributions(r_raw, e1_raw, 8192, 64, 8);

        conv(r, r_raw);
        conv(e0, e0_raw);
        conv(e1, e1_raw);

        result.c_0 = pk.a * r + e0;
        result.c_1 = pk.b * r + e1 + x;

        return result;
    }

    ZZ_pE round_poly(const ZZ_pE& value, const ZZ& p, const ZZ& q) {
        ZZ_pX raw_value = conv<ZZ_pX>(value);
        ZZ_pX rounded_value;
        ZZ q_div_2 = q / 2;
        long degree = deg(raw_value);
        for (long i = 0; i < degree; i++) {
            ZZ coeff = rep(raw_value.rep[i]);
            coeff = (coeff * p + q_div_2) / q; // Scale down and round
            SetCoeff(rounded_value, i, conv<ZZ_p>(coeff));
        }
        return conv<ZZ_pE>(rounded_value);
    }

    Memory_Value DDEC(const Input_Value& input, const Memory_Value& memory, const Context& ctx) {
        Memory_Value result;

        ZZ_pE term0, term1;
        term0 = round_poly(input.c_00 * memory.mem_0 + input.c_01 * memory.mem_1, ctx.p, ctx.q);
        term1 = round_poly(input.c_10 * memory.mem_0 + input.c_11 * memory.mem_1, ctx.p, ctx.q);
        result.mem_0 = term0;
        result.mem_1 = term1;

        return result;
    }

    void benchmark_HSS_Gen(int iterations) {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            HSS_Gen();
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for HSS_Gen over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void benchmark_HSS_Enc(int iterations, PKE_Gen_keys pke_gen_keys, Context ctx = default_ctx) {
        if (ctx.p == 0 && ctx.q == 0) {
            ctx = generate_context(8192, 72, 146);
            pke_gen_keys = PKE_Gen();
        }
        ZZ x = RandomBits_ZZ(1); // Random input value

        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            HSS_Enc(pke_gen_keys.pk, x, ctx);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for HSS_Enc over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void benchmark_load(int iterations, HSS_Gen_keys hss_gen_keys, Context ctx = default_ctx) {
        int b = 0; // Example bit
        if (ctx.p == 0 && ctx.q == 0) {
            ctx = generate_context(8192, 72, 146);
            hss_gen_keys = HSS_Gen();
        }
        ZZ x = RandomBits_ZZ(1); // Random input value
        Input_Value input = HSS_Enc(hss_gen_keys.pke_keys.pk, x, ctx);
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            load(b, i, input, hss_gen_keys.eval_key0, ctx);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for load over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void benchmark_add_memory_values(int iterations, HSS_Gen_keys hss_gen_keys, Context ctx = default_ctx) {
        int b = 0; // Example bit
        if (ctx.p == 0 && ctx.q == 0) {
            ctx = generate_context(8192, 72, 146);
            hss_gen_keys = HSS_Gen();
        }
        ZZ x1 = RandomBits_ZZ(1); // Random input value 1
        ZZ x2 = RandomBits_ZZ(1);
        Input_Value input1 = HSS_Enc(hss_gen_keys.pke_keys.pk, x1, ctx);
        Input_Value input2 = HSS_Enc(hss_gen_keys.pke_keys.pk, x2, ctx);
        Memory_Value mem1 = load(b, 0, input1, hss_gen_keys.eval_key0, ctx);
        Memory_Value mem2 = load(b, 0, input2, hss_gen_keys.eval_key0, ctx);
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            add_memory_values(b, i, mem1, mem2, hss_gen_keys.eval_key0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for add_memory_values over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void benchmark_add_input_values(int iterations, HSS_Gen_keys hss_gen_keys, Context ctx = default_ctx) {
        if (ctx.p == 0 && ctx.q == 0) {
            ctx = generate_context(8192, 72, 146);
            hss_gen_keys = HSS_Gen();
        }
        ZZ x1 = RandomBits_ZZ(1); // Random input value 1
        ZZ x2 = RandomBits_ZZ(1); // Random input value 2
        Input_Value input1 = HSS_Enc(hss_gen_keys.pke_keys.pk, x1, ctx);
        Input_Value input2 = HSS_Enc(hss_gen_keys.pke_keys.pk, x2, ctx);
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            add_input_values(input1, input2);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for add_input_values over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void benchmark_multiply(int iterations, HSS_Gen_keys hss_gen_keys, Context ctx = default_ctx) {
        int b = 0;
        if (ctx.p == 0 && ctx.q == 0) {
            ctx = generate_context(8192, 72, 146);
            hss_gen_keys = HSS_Gen();
        }
        ZZ x1 = RandomBits_ZZ(1);
        ZZ x2 = RandomBits_ZZ(1);
        Input_Value input1 = HSS_Enc(hss_gen_keys.pke_keys.pk, x1, ctx);
        Input_Value input2 = HSS_Enc(hss_gen_keys.pke_keys.pk, x2, ctx);
        Memory_Value mem1 = load(b, 0, input1, hss_gen_keys.eval_key0, ctx);
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            multiply(b, i, input2, mem1, hss_gen_keys.eval_key0, ctx);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for multiply over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void benchmark_all(int iterations) {
        Context ctx = generate_context(8192, 72, 146);
        HSS_Gen_keys hss_gen_keys = HSS_Gen();
        PKE_Gen_keys pke_gen_keys = hss_gen_keys.pke_keys;
        benchmark_HSS_Gen(iterations);
        benchmark_HSS_Enc(iterations, pke_gen_keys, ctx);
        benchmark_load(iterations, hss_gen_keys, ctx);
        benchmark_add_memory_values(iterations, hss_gen_keys, ctx);
        benchmark_add_input_values(iterations, hss_gen_keys, ctx);
        benchmark_multiply(iterations, hss_gen_keys, ctx);
    }

}
