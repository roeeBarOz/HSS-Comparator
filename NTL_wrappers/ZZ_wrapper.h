#ifndef NTL_LIB_H
#define NTL_LIB_H
#include <NTL/ZZ.h>

#ifdef __cplusplus
extern "C" {
    #endif

    NTL::ZZ* from_string(const char* str);
    void free_ZZ(NTL::ZZ* zz);
    char* to_string(const NTL::ZZ* zz);
    void free_string(char* str);

    // Arithmetic
    char* zz_add(const char* a, const char* b);
    char* zz_sub(const char* a, const char* b);
    char* zz_mul(const char* a, const char* b);
    char* zz_div(const char* a, const char* b);
    char* zz_mod(const char* a, const char* b);
    char* zz_pow(const char* base, const char* exp);
    char* zz_neg(const char* a);
    char* zz_abs(const char* a);
    char* zz_inv(const char* a, const char* mod);
    char* zz_lcm(const char* a, const char* b);

    // Comparisons
    int zz_eq(const char* a, const char* b);
    int zz_lt(const char* a, const char* b);

    // Number theory
    char* zz_gcd(const char* a, const char* b);
    char* zz_sqrt(const char* a);
    char* zz_next_prime(const char* a);
    char* zz_random_prime(int bits, int certainty);
    char* zz_random(int bits);
    char* zz_random_invertible_mod_n(const char* n);
    char* zz_random_smaller_than_n(const char* n);

    #ifdef __cplusplus
}
#endif

#endif // NTL_LIB_H
