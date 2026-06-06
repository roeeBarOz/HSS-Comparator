#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pE.h>
#include <NTL/vector.h>
#include <cstdlib>
#include <random>
#include <algorithm>
#include <sstream>
#include "quetient_ring_wrapper.h"

using namespace NTL;

vec_ZZ_pX* precomp = nullptr;

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

    vec_ZZ_pX* find_roots(const ZZ_pX* poly, long n) {
        vec_ZZ_pX* roots = new vec_ZZ_pX;
        ZZ_p omega;
        ZZ r = ZZ_p::modulus();
        ZZ exp = (r - 1) / (2 * n);

        // runs twice on average, since the probability of finding a primitive 2n-th root of unity is 1/2
        while (true) {
            ZZ_p g;
            random(g);
            power(omega, g, exp);

            ZZ_p check;
            power(check, omega, n);
            if (check == -1) {
                break;
            }
        }

        roots->SetLength(n);
        ZZ_p current_root = omega;
        ZZ_p omega_sqr = omega * omega; // Multiply by omega^2 to jump to the next odd power
        
        for (long i = 0; i < n; ++i) {
            ZZ_pX F_i;
            SetCoeff(F_i, 1, 1);             // Coefficient of x^1 is 1
            SetCoeff(F_i, 0, -current_root); // Coefficient of x^0 is -root
            
            roots->operator[](i) = F_i;
            current_root *= omega_sqr;       // Advance to the next odd power
        }

        return roots;
    }

    // Precompute the P_i(x) polynomials for CRT reconstruction
    vec_ZZ_pX* CRT_precomputation() {
        long n = 8192;
        ZZ r = ZZ(8191);
        ZZ_pX modulus_poly;
        SetCoeff(modulus_poly, n, 1); // x^n
        SetCoeff(modulus_poly, 0, 1); // x^n + 1
        init_modulus(&r, &modulus_poly);

        vec_ZZ_pX* roots = find_roots(&modulus_poly, n);

        // compute M(x) = prod_(i=1 to n) F_i(x)
        ZZ_pX M;
        SetCoeff(M, 0, 1); // M(x) starts as 1
        for (long i = 0; i < n; ++i) {
            M *= roots->operator[](i);
        }

        precomp = new vec_ZZ_pX;
        precomp->SetLength(n);
        // denote by M_i(x) = M(x) / F_i(x)
        // compute M_i(x) and its inverse Y_i(x) = M_i(x)^(-1) mod F_i(x)
        // compute P_i(x) = M_i(x) * Y_i(x) mod A(x)
        for (long i = 0; i < n; ++i) {
            ZZ_pX F_i = roots->operator[](i);
            ZZ_pX M_i = M / F_i;
            ZZ_pX Y_i;
            InvMod(Y_i, M_i, F_i); // Y_i = M_i^(-1) mod F_i
            ZZ_pX P_i = (M_i * Y_i) % modulus_poly; // P_i = M_i * Y_i mod A(x)
            precomp->operator[](i) = P_i; // Store P_i in the precomputation vector
        }
        return precomp;
    }

    // add the inputs together, each multiplied by the corresponding P_i, and reduce the result modulo A(x) to get the final reconstructed polynomial
    ZZ_pX* CRT_reconstruction(const vec_ZZ* values) {
        long n = precomp->length();
        ZZ_pX result;
        SetCoeff(result, 0, 0); // Initialize result to 0

        for (long i = 0; i < n; ++i) {
            ZZ_pX P_i = precomp->operator[](i);
            ZZ v_i = values->operator[](i);
            result += conv<ZZ_p>(v_i) * P_i; // reconstruct the polynomial using the precomputed P_i and the given values v_i
        }

        return &result; // Return the reconstructed polynomial
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
            // to_ZZ_p safely handles negative numbers, converting -1 to q-1
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

    PKE_Gen_keys PKE_Gen() {

        ZZ_pE a, b, s_hat;

        // 1. Generate 'a' uniformly at random from the ring
        random(a);

        // 2. Generate the raw polynomials for s_hat and e.
        // (You will populate these using the custom samplers we discussed).
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
        keys.sk = s_hat;

        return keys;
    }

    HSS_Gen_keys HSS_Gen() {
        // Implementation for HSS key generation
        PKE_Gen_keys pke_gen_output = PKE_Gen();
        ZZ_pE s_0_0, s_0_1, s_1_0, s_1_1;
        random(s_0_0);
        random(s_0_1);
        s_1_0 = s_0_0 - 1;
        s_1_1 = s_0_1 - pke_gen_output.sk;
        ZZ k = RandomBits_ZZ(256);
        eval_key ek_0, ek_1;
        ek_0.prf_key = k;
        ek_1.prf_key = k;
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

    Input_Value OKDM(public_key pk, ZZ x, ZZ p, ZZ q) {
        Input_Value result;
        ZZ_pX raw_v, raw_e0, raw_e1;
        ZZ_pE v, e0, e1, poly_x;

        x = x * q / p; // Scale x to the modulus q
        poly_x = conv<ZZ_pE>(conv<ZZ_pX>(x)); // Convert x to a polynomial in the ring

        for (long i = 0; i < 2; i++) {
            generate_distributions(raw_v, raw_e0, 8192, 64, 8);
            generate_distributions(raw_v, raw_e1, 8192, 64, 8);

            conv(v, raw_v);
            conv(e0, raw_e0);
            conv(e1, raw_e1);

            if (i == 0) {
                result.c_00 = pk.b * v + e0 + poly_x; 
                result.c_01 = -pk.a * v + e1; 
            } else {
                result.c_10 = pk.b * v + e0; 
                result.c_11 = -pk.a * v + e1 + poly_x; 
            }
        }

        return result;
    }

    ZZ_pE round(ZZ_pE value, ZZ p, ZZ q) {
        ZZ_pX raw_value = conv<ZZ_pX>(value);
        ZZ_pX rounded_value;
        for (long i = 0; i < raw_value.rep.length(); i++) {
            ZZ coeff = rep(raw_value.rep[i]);
            coeff = (coeff * p + q / 2) / q; // Scale down and round
            SetCoeff(rounded_value, i, conv<ZZ_p>(coeff));
        }
        return conv<ZZ_pE>(rounded_value);
    }

    Memory_Value DDEC(int b, eval_key ek, Input_Value input, Memory_Value memory, Context ctx) {
        Memory_Value result;

        ZZ_pE term0, term1;
        term0 = round(input.c_00 * memory.mem_0 + input.c_01 * memory.mem_1, ctx.p, ctx.q);
        term1 = round(input.c_10 * memory.mem_0 + input.c_11 * memory.mem_1, ctx.p, ctx.q);

        // TODO: Add the PRF term using ek.prf_key to mask the result, ensuring security against chosen-ciphertext attacks.
        ZZ_pE prf_term;

        if (b == 0) {
            result.mem_0 = term0 + prf_term; 
            result.mem_1 = term1 + prf_term;
        } else {
            result.mem_0 = term0 - prf_term; 
            result.mem_1 = term1 - prf_term;
        }
        return result;
    }

}
