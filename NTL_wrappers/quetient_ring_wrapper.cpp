#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pE.h>
#include <NTL/vector.h>
#include <cstdlib>
#include <sstream>

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

    void init_modulus(const ZZ* r, long degree, ZZ_pX* modulus) {
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

    vec_ZZ_pX* CRT_precomputation() {
        long n = 8192;
        ZZ r = ZZ(8191);
        ZZ_pX modulus_poly;
        SetCoeff(modulus_poly, n, 1); // x^n
        SetCoeff(modulus_poly, 0, 1); // x^n + 1
        init_modulus(&r, n, &modulus_poly);

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

}
