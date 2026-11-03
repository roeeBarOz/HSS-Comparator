#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pE.h>
#include <NTL/vector.h>
#include <NTL/vec_ZZ_pE.h>
#include <cstdlib>
#include <random>
#include <algorithm>
#include <sstream>
#include <chrono>
#include "quetient_ring_wrapper.h"
#include "../PRF/aesni_ctr.c"
#include "schemes/rlwe_hss.hpp"

using namespace NTL;

vec_ZZ_pX* precomp = nullptr;
RLWEContext default_ctx = {ZZ(0), ZZ(0)};

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

    void PRF(int b, const uint8_t key[16], const uint8_t id, ZZ_pE& mem) {
        ZZ_pE p;
        apply_prf(key, id, p);

        if (b == 0) {
            mem += p;
        } else {
            mem -= p;
        }
    }

    // LPR.Gen function, detailed at page 17
    RLWEPKEGenKey PKE_Gen() {

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

        RLWEPublicKey pk;
        pk.a = a;
        pk.b = b;

        RLWEPKEGenKey keys;
        keys.pk = pk;
        keys.sk = s_hat; // sk should be (1, s_hat), but we handle it in HSS_Gen.

        return keys;
    }

    RLWEContext generate_context(int n, int log_p, int log_q) {
        RLWEContext ctx;
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

    HSSGenKeyHandle RLWEHSS::HSS_gen() {
        // Implementation for HSS key generation

        // setting seed for NTL random number generator
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<unsigned long> dis;
        SetSeed(conv<ZZ>(dis(gen)));

        RLWEPKEGenKey pke_gen_output = PKE_Gen();
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

        RLWEEvalKey ek_0, ek_1;
        memcpy(ek_0.prf_key, k, 16);
        memcpy(ek_1.prf_key, k, 16);
        ek_0.share_of_1 = s_0_0;
        ek_1.share_of_1 = s_1_0;
        ek_0.share_of_sk = s_0_1;
        ek_1.share_of_sk = s_1_1;

        RLWEHSSGenKey keys;
        keys.eval_key0 = ek_0;
        keys.eval_key1 = ek_1;
        keys.pke_keys = pke_gen_output;
        return keys;
    }

    InputHandle RLWEHSS::HSS_Enc(const PublicKeyHandle& pk, const ZZ& x, const ContextHandle& ctx, int k) {
        RLWEContext context = *std::static_pointer_cast<RLWEContext>(ctx);
        RLWEPublicKey public_key = *std::static_pointer_cast<RLWEPublicKey>(pk);
        return RLWE_OKDM(public_key, x, context.p, context.q, k);
    }

    MemoryHandle RLWEHSS::load(int party_id, int action_id, const InputHandle& input, const EvalKeyHandle& ek, 
                               const ContextHandle& ctx) {
        MemoryHandle result;

        RLWEInputValue input_value = *std::static_pointer_cast<RLWEInputValue>(input);
        RLWEEvalKey eval_key = *std::static_pointer_cast<RLWEEvalKey>(ek);
        RLWEContext context = *std::static_pointer_cast<RLWEContext>(ctx);
        RLWEMemoryValue value_of_s = {share_of_1, share_of_sk};

        result = RLWE_DDEC(input, value_of_s, ctx);
        PRF(party_id, eval_key.prf_key, action_id, result.v[0]);
        PRF(party_id, eval_key.prf_key, action_id, result.v[1]);
        return result;
    }

    MemoryHandle RLWEHSS::add_memory_values(int party_id, int action_id, const MemoryHandle& mem1, const MemoryHandle& mem2, 
                                            const EvalKeyHandle& ek) {
        RLWEMemoryValue result;

        RLWEMemoryValue mem_1 = *std::static_pointer_cast<RLWEMemoryValue>(mem1);
        RLWEMemoryValue mem_2 = *std::static_pointer_cast<RLWEMemoryValue>(mem2);
        RLWEEvalKey eval_key = *std::static_pointer_cast<RLWEEvalKey>(ek);
    
        result.v[0] = mem_1.v[0] + mem_2.v[0];
        result.v[1] = mem_1.v[1] + mem_2.v[1];
        PRF(party_id, eval_key.prf_key, action_id, result.v[0]);
        PRF(party_id, eval_key.prf_key, action_id, result.v[1]);
        return result;
    }

    InputHandle RLWEHSS::add_input_values(const InputHandle& in1, const InputHandle& in2) {
        RLWEInputValue result;

        RLWEInputValue input0 = *std::static_pointer_cast<RLWEInputValue>(in1);
        RLWEInputValue input1 = *std::static_pointer_cast<RLWEInputValue>(in2);

        result.M[0][0] = input0.M[0][0] + input1.M[0][0];
        result.M[0][1] = input0.M[0][1] + input1.M[0][1];
        result.M[1][0] = input0.M[1][0] + input1.M[1][0];
        result.M[1][1] = input0.M[1][1] + input1.M[1][1];
        return result;
    }

    MemoryHandle RLWEHSS::multiply(int party_id, int action_id, const MemoryHandle& mem, const InputHandle& in, 
                                   const EvalKeyHandle& ek, const ContextHandle& ctx) {
        RLWEMemoryValue result;
        
        auto memory = std::static_pointer_cast<RLWEMemoryValue>(mem);
        auto input = std::static_pointer_cast<RLWEInputValue>(in);
        auto eval_key = std::static_pointer_cast<RLWEEvalKey>(ek);
        auto context = std::static_pointer_cast<RLWEContext>(ctx);

        result = RLWE_DDEC(input, memory, context);
        PRF(party_id, eval_key.prf_key, action_id, result.v[0]);
        PRF(party_id, eval_key.prf_key, action_id, result.v[1]);
        return result;
    }

    OutputValueHandle last_RLWE_multiply(int party_id, int action_id, const InputHandle& input, 
                                         const MemoryHandle& memory, const EvalKeyHandle& ek, const ContextHandle& ctx) {
        RLWEOutputValue result;

        auto input_value = std::static_pointer_cast<RLWEInputValue>(input);
        auto memory_value = std::static_pointer_cast<RLWEMemoryValue>(memory);
        auto eval_key = std::static_pointer_cast<RLWEEvalKey>(ek);
        auto context = std::static_pointer_cast<RLWEContext>(ctx);

        result.out = round_poly(input_value->M[0][0] * memory_value->v[0] + input_value->M[0][1] * memory_value->v[1], context->p, context->q);
        PRF(party_id, eval_key->prf_key, action_id, result.out);
        return result;
    }

    // Multiplication of input value by 2 memory values
    // Done using Strassen algorithm
    // Return value is std::pair, consisting of 2 memory values
    std::pair<MemoryHandle, MemoryHandle> strassen_multiplication(int party_id, int action_id, const InputHandle& input, const MemoryHandle& mem1, const MemoryHandle& mem2, const EvalKeyHandle& ek, const ContextHandle& ctx) {
        Memory_Value result1, result2;

        RLWEInputValue input_value = *std::static_pointer_cast<RLWEInputValue>(input);
        RLWEMemoryValue mem1_value = *std::static_pointer_cast<RLWEMemoryValue>(mem1);
        RLWEMemoryValue mem2_value = *std::static_pointer_cast<RLWEMemoryValue>(mem2);
        RLWEEvalKey eval_key = *std::static_pointer_cast<RLWEEvalKey>(ek);
        RLWEContext context = *std::static_pointer_cast<RLWEContext>(ctx);

        vec_ZZ_pE m;
        m.SetLength(7);

        m[0] = (input_value.M[0][0] + input_value.M[1][1]) * (mem1_value.v[0] + mem2_value.v[1]);
        m[1] = (input_value.M[1][0] + input_value.M[1][1]) * mem1_value.v[0];
        m[2] = input_value.M[0][0] * (mem2_value.v[0] - mem2_value.v[1]);
        m[3] = input_value.M[1][1] * (mem1_value.v[1] - mem1_value.v[0]);
        m[4] = (input_value.M[0][0] + input_value.M[1][1]) * mem2_value.v[1];
        m[5] = (input_value.M[1][0] - input_value.M[0][0]) * (mem1_value.v[0] + mem2_value.v[0]);
        m[6] = (input_value.M[0][1] - input_value.M[1][1]) * (mem1_value.v[1] + mem2_value.v[1]);

        result1.mem_0 = m[0] + m[3] - m[4] + m[6];
        result1.mem_1 = m[1] + m[3];
        result2.mem_0 = m[2] + m[4];
        result2.mem_1 = m[0] - m[1] + m[2] + m[5];

        PRF(party_id, eval_key->prf_key, action_id, result1.mem_0);
        PRF(party_id, eval_key->prf_key, action_id, result1.mem_1);
        PRF(party_id, eval_key->prf_key, action_id, result2.mem_0);
        PRF(party_id, eval_key->prf_key, action_id, result2.mem_1);
        m.kill();

        return std::make_pair(result1, result2);
    }

    // LPR.OKDM function, detailed at page 17
    RLWEInputValue RLWE_OKDM(const RLWEPublicKey& pk, const ZZ& x, const ZZ& p, const ZZ& q, int k) {
        RLWEInputValue result;
        ZZ_pX poly_x;
        ZZ_p x_p = conv<ZZ_p>(x * q / p); // Scale x to the modulus q
        SetCoeff(poly_x, k, x_p); // Set the k-th coefficient to x_p

        RLWEEncryption enc_0 = Enc(pk, conv<ZZ_pX>(0));
        RLWEEncryption enc_x = Enc(pk, poly_x);

        result.c_00 = enc_x.c_0;
        result.c_10 = enc_x.c_1;
        result.c_01 = enc_0.c_0;
        result.c_11 = enc_0.c_1 + x_p;

        return result;
    }

    //LPR.Enc function, detailed at page 17
    RLWEEncryption Enc(const RLWEPublicKey& pk, const ZZ_pX& x) {
        RLWEEncryption result;
        ZZ_pE r, e0, e1, x_raw;
        ZZ_pX r_raw, e0_raw, e1_raw;
        generate_distributions(r_raw, e0_raw, 8192, 64, 8);
        generate_distributions(r_raw, e1_raw, 8192, 64, 8);

        conv(r, r_raw);
        conv(e0, e0_raw);
        conv(e1, e1_raw);
        conv(x_raw, x);

        result.c_0 = pk.a * r + e0;
        result.c_1 = pk.b * r + e1 + x_raw;

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

    RLWEMemoryValue RLWE_DDEC(const RLWEInputValue& input, const RLWEMemoryValue& memory, const RLWEContext& ctx) {
        RLWEMemoryValue result;

        ZZ_pE term0, term1;
        term0 = round_poly(input.M[0][0] * memory.v[0] + input.M[0][1] * memory.v[1], ctx.p, ctx.q);
        term1 = round_poly(input.M[1][0] * memory.v[0] + input.M[1][1] * memory.v[1], ctx.p, ctx.q);
        result.v[0] = term0;
        result.v[1] = term1;

        return result;
    }

    InputHandle generate_UVC_input(const PublicKeyHandle& pk, int k, const ContextHandle& ctx) {
        return HSS_Enc(pk, conv<ZZ>(1), ctx, k);
    }

    ZZ_pX UVC_output(const MemoryHandle& mem, int poly_len) {
        ZZ_pX result, temp;
        temp = conv<ZZ_pX>(mem.mem_0);
        for (int i = 0; i < poly_len; ++i) {
            int coeff = 0;
            for (int j = i; j < deg(temp); j += poly_len) {
                coeff += rep(temp[j]) % 2;
            }
            SetCoeff(result, i, conv<ZZ_p>(coeff));
        }
        return result;
    }

    void benchmark_HSS_Gen(int iterations) {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            HSS_gen();
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for HSS_Gen over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void benchmark_HSS_Enc(int iterations, RLWEPKEGenKey pke_gen_keys, RLWEContext ctx = default_ctx) {
        if (ctx.p == 0 && ctx.q == 0) {
            ctx = generate_context(8192, 72, 146);
            pke_gen_keys = PKE_Gen();
        }
        ZZ x = RandomBits_ZZ(1); // Random input value

        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            HSS_Enc(pke_gen_keys.pk, x, ctx, 0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for HSS_Enc over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void benchmark_RLWE_load(int iterations, HSSGenKeyHandle hss_gen_keys, RLWEContext ctx = default_ctx) {
        int b = 0; // Example bit
        if (ctx.p == 0 && ctx.q == 0) {
            ctx = generate_context(8192, 72, 146);
            hss_gen_keys = HSS_gen();
        }
        RLWEHSSGenKey rlwe_hss_gen_keys = *std::static_pointer_cast<RLWEHSSGenKey>(hss_gen_keys);
        EvalKeyHandle eval_key0 = rlwe_hss_gen_keys.eval_key0;
        ZZ x = RandomBits_ZZ(1); // Random input value
        InputHandle input = HSS_Enc(rlwe_hss_gen_keys.pke_keys.pk, x, ctx, 0);
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            load(b, i, input, eval_key0, ctx);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for RLWE_load over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void benchmark_RLWE_add_memory_values(int iterations, HSSGenKeyHandle hss_gen_keys, RLWEContext ctx = default_ctx) {
        int b = 0; // Example bit
        if (ctx.p == 0 && ctx.q == 0) {
            ctx = generate_context(8192, 72, 146);
            hss_gen_keys = HSS_gen();
        }
        RLWEHSSGenKey hss_gen_keys_struct = *std::static_pointer_cast<RLWEHSSGenKey>(hss_gen_keys);
        ZZ x1 = RandomBits_ZZ(1); // Random input value 1
        ZZ x2 = RandomBits_ZZ(1);
        InputHandle input1 = HSS_Enc(hss_gen_keys_struct.pke_keys.pk, x1, ctx, 0);
        InputHandle input2 = HSS_Enc(hss_gen_keys_struct.pke_keys.pk, x2, ctx, 0);
        MemoryHandle mem1 = load(b, 0, input1, hss_gen_keys_struct.eval_key0, ctx);
        MemoryHandle mem2 = load(b, 0, input2, hss_gen_keys_struct.eval_key0, ctx);
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            add_memory_values(b, i, mem1, mem2, hss_gen_keys_struct.eval_key0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for RLWE_add_memory_values over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void benchmark_RLWE_add_input_values(int iterations, HSSGenKeyHandle hss_gen_keys, RLWEContext ctx = default_ctx) {
        if (ctx.p == 0 && ctx.q == 0) {
            ctx = generate_context(8192, 72, 146);
            hss_gen_keys = HSS_gen();
        }
        RLWEHSSGenKey hss_gen_keys_struct = *std::static_pointer_cast<RLWEHSSGenKey>(hss_gen_keys);
        ZZ x1 = RandomBits_ZZ(1); // Random input value 1
        ZZ x2 = RandomBits_ZZ(1); // Random input value 2
        InputHandle input1 = HSS_Enc(hss_gen_keys_struct.pke_keys.pk, x1, ctx, 0);
        InputHandle input2 = HSS_Enc(hss_gen_keys_struct.pke_keys.pk, x2, ctx, 0);
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            add_input_values(input1, input2);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for RLWE_add_input_values over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void benchmark_RLWE_multiply(int iterations, HSSGenKeyHandle hss_gen_keys, RLWEContext ctx = default_ctx) {
        int b = 0;
        if (ctx.p == 0 && ctx.q == 0) {
            ctx = generate_context(8192, 72, 146);
            hss_gen_keys = HSS_gen();
        }
        RLWEHSSGenKey hss_gen_keys_struct = *std::static_pointer_cast<RLWEHSSGenKey>(hss_gen_keys);
        ZZ x1 = RandomBits_ZZ(1);
        ZZ x2 = RandomBits_ZZ(1);
        InputHandle input1 = HSS_Enc(hss_gen_keys_struct.pke_keys.pk, x1, ctx, 0);
        InputHandle input2 = HSS_Enc(hss_gen_keys_struct.pke_keys.pk, x2, ctx, 0);
        MemoryHandle mem1 = load(b, 0, input1, hss_gen_keys_struct.eval_key0, ctx);
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            multiply(b, i, input2, mem1, hss_gen_keys_struct.eval_key0, ctx);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for RLWE_multiply over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void benchmark_last_RLWE_multiply(int iterations, HSSGenKeyHandle hss_gen_keys, RLWEContext ctx = default_ctx) {
        int b = 0;
        if (ctx.p == 0 && ctx.q == 0) {
            ctx = generate_context(8192, 72, 146);
            hss_gen_keys = HSS_gen();
        }
        RLWEHSSGenKey hss_gen_keys_struct = *std::static_pointer_cast<RLWEHSSGenKey>(hss_gen_keys);
        ZZ x1 = RandomBits_ZZ(1);
        ZZ x2 = RandomBits_ZZ(1);
        InputHandle input1 = HSS_Enc(hss_gen_keys_struct.pke_keys.pk, x1, ctx, 0);
        InputHandle input2 = HSS_Enc(hss_gen_keys_struct.pke_keys.pk, x2, ctx, 0);
        MemoryHandle mem1 = load(b, 0, input1, hss_gen_keys_struct.eval_key0, ctx);
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            last_RLWE_multiply(b, i, input2, mem1, hss_gen_keys_struct.eval_key0, ctx);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for last_RLWE_multiply over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void benchmark_strassen_multiplication(int iterations, HSSGenKeyHandle hss_gen_keys, RLWEContext ctx = default_ctx) {
        int b = 0;
        if (ctx.p == 0 && ctx.q == 0) {
            ctx = generate_context(8192, 72, 146);
            hss_gen_keys = HSS_gen();
        }
        RLWEHSSGenKey hss_gen_keys_struct = *std::static_pointer_cast<RLWEHSSGenKey>(hss_gen_keys);
        ZZ x1 = RandomBits_ZZ(1);
        ZZ x2 = RandomBits_ZZ(1);
        ZZ x3 = RandomBits_ZZ(1);
        InputHandle input1 = HSS_Enc(hss_gen_keys_struct.pke_keys.pk, x1, ctx, 0);
        InputHandle input2 = HSS_Enc(hss_gen_keys_struct.pke_keys.pk, x2, ctx, 0);
        InputHandle input3 = HSS_Enc(hss_gen_keys_struct.pke_keys.pk, x3, ctx, 0);
        MemoryHandle mem1 = load(b, 0, input1, hss_gen_keys_struct.eval_key0, ctx);
        MemoryHandle mem2 = load(b, 0, input2, hss_gen_keys_struct.eval_key0, ctx);
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            strassen_multiplication(b, i, input3, mem1, mem2, hss_gen_keys_struct.eval_key0, ctx);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / (2 * iterations); // since every strassen multiplication
                                                                              // is equivalent to 2 multiplications
        std::cout << "Average time for strassen multiplication over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void benchmark_generate_UVC_input(int iterations, HSSGenKeyHandle hss_gen_keys, RLWEContext ctx = default_ctx) {
        if (ctx.p == 0 && ctx.q == 0) {
            ctx = generate_context(8192, 72, 146);
            hss_gen_keys = HSS_gen();
        }
        int k = 15; // example
        RLWEHSSGenKey hss_gen_keys_struct = *std::static_pointer_cast<RLWEHSSGenKey>(hss_gen_keys);
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            generate_UVC_input(hss_gen_keys_struct.pke_keys.pk, k, ctx);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for generate_UVC_input over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void benchmark_UVC_output(int iterations, HSSGenKeyHandle hss_gen_keys, RLWEContext ctx = default_ctx) {
        if (ctx.p == 0 && ctx.q == 0) {
            ctx = generate_context(8192, 72, 146);
            hss_gen_keys = HSS_gen();
        }
        int k = 15; // example
        RLWEHSSGenKey hss_gen_keys_struct = *std::static_pointer_cast<RLWEHSSGenKey>(hss_gen_keys);
        InputHandle input = generate_UVC_input(hss_gen_keys_struct.pke_keys.pk, k, ctx);
        MemoryHandle mem = load(0, 0, input, hss_gen_keys_struct.eval_key0, ctx);
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            UVC_output(mem, k);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for UVC_output over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void benchmark_all(int iterations) {
        ContextHandle ctx = generate_context(8192, 72, 146);
        HSSGenKeyHandle hss_gen_keys = HSS_gen();
        RLWEHSSGenKey hss_gen_keys_struct = *std::static_pointer_cast<RLWEHSSGenKey>(hss_gen_keys);
        RLWEPKEGenKey pke_gen_keys = hss_gen_keys_struct.pke_keys;
        benchmark_HSS_Gen(iterations);
        benchmark_HSS_Enc(iterations, pke_gen_keys, ctx);
        benchmark_RLWE_load(iterations, hss_gen_keys, ctx);
        benchmark_RLWE_add_memory_values(iterations, hss_gen_keys, ctx);
        benchmark_RLWE_add_input_values(iterations, hss_gen_keys, ctx);
        benchmark_RLWE_multiply(iterations, hss_gen_keys, ctx);
        benchmark_last_RLWE_multiply(iterations, hss_gen_keys, ctx);
        benchmark_strassen_multiplication(iterations, hss_gen_keys, ctx);
        benchmark_generate_UVC_input(iterations, hss_gen_keys, ctx);
        benchmark_UVC_output(iterations, hss_gen_keys, ctx);
    }

    void polynomial_mult_time(int iterations) {
        ZZ_pE poly1, poly2, result;
        random(poly1);
        random(poly2);

        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            result = poly1 * poly2;
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for polynomial multiplication over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void polynomial_add_time(int iterations) {
        ZZ_pE poly1, poly2, result;
        random(poly1);
        random(poly2);

        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            result = poly1 + poly2;
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for polynomial addition over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void integer_mult_time(int iterations) {
        ZZ_p a, b, result;
        random(a);
        random(b);

        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            result = a * b;
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for integer multiplication over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

    void integer_add_time(int iterations) {
        ZZ_p a, b, result;
        random(a);
        random(b);

        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            result = a + b;
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> avg = (end - start) / iterations;
        std::cout << "Average time for integer addition over " << iterations << " iterations: " << avg.count() << " seconds." << std::endl;
    }

}
