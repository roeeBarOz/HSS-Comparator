#include <NTL/ZZ.h>
#include <cstdlib>
#include <sstream>

using namespace NTL;

extern "C" {

    ZZ* from_string(const char* str) {
        ZZ* result = new ZZ;
        *result = to_ZZ(str);
        return result;
    }

    void free_ZZ(ZZ* zz) {
        delete zz;
    }

    char* to_string(const ZZ* zz) {
        std::ostringstream oss;
        oss << *zz;
        std::string str = oss.str();
        return strdup(str.c_str());
    }

    void free_string(char* str) {
        free(str);
    }

    // Add two ZZs, return new ZZ*
    char* zz_add(const char* a, const char* b) {
        ZZ* zz_a = from_string(a);
        ZZ* zz_b = from_string(b);
        ZZ* res = new ZZ();
        *res = *zz_a + *zz_b;
        free_ZZ(zz_a);
        free_ZZ(zz_b);
        return to_string(res);
    }

    // Subtract two ZZs
    char* zz_sub(const char* a, const char* b) {
        ZZ* zz_a = from_string(a);
        ZZ* zz_b = from_string(b);
        ZZ* res = new ZZ();
        *res = *zz_a - *zz_b;
        free_ZZ(zz_a);
        free_ZZ(zz_b);
        return to_string(res);
    }

    // Multiply two ZZs
    char* zz_mul(const char* a, const char* b) {
        ZZ* zz_a = from_string(a);
        ZZ* zz_b = from_string(b);
        ZZ* res = new ZZ();
        *res = (*zz_a) * (*zz_b);
        free_ZZ(zz_a);
        free_ZZ(zz_b);
        return to_string(res);
    }

    // Divide two ZZs (integer division)
    char* zz_div(const char* a, const char* b) {
        ZZ* zz_a = from_string(a);
        ZZ* zz_b = from_string(b);
        ZZ* res = new ZZ();
        *res = (*zz_a) / (*zz_b);
        free_ZZ(zz_a);
        free_ZZ(zz_b);
        return to_string(res);
    }

    char* zz_mod(const char* a, const char* b) {
        ZZ* zz_a = from_string(a);
        ZZ* zz_b = from_string(b);
        ZZ* res = new ZZ();
        *res = (*zz_a) % (*zz_b);
        free_ZZ(zz_a);
        free_ZZ(zz_b);
        return to_string(res);
    }

    char* zz_pow(const char* base, const char* exp) {
        ZZ* zz_base = from_string(base);
        ZZ* zz_exp = from_string(exp);
        ZZ* res = new ZZ();
        *res = power(*zz_base, conv<long>(*zz_exp));
        free_ZZ(zz_base);
        free_ZZ(zz_exp);
        return to_string(res);
    }

    char* zz_gcd(const char* a, const char* b) {
        ZZ* zz_a = from_string(a);
        ZZ* zz_b = from_string(b);
        ZZ* res = new ZZ();
        *res = GCD(*zz_a, *zz_b);
        free_ZZ(zz_a);
        free_ZZ(zz_b);
        return to_string(res);
    }

    char* zz_neg(const char* a) {
        ZZ* zz_a = from_string(a);
        ZZ* res = new ZZ();
        *res = -(*zz_a);
        free_ZZ(zz_a);
        return to_string(res);
    }

    char* zz_abs(const char* a) {
        ZZ* zz_a = from_string(a);
        ZZ* res = new ZZ();
        *res = abs(*zz_a);
        free_ZZ(zz_a);
        return to_string(res);
    }

    char* zz_inv(const char* a, const char* mod) {
        ZZ* zz_a = from_string(a);
        ZZ* zz_mod = from_string(mod);
        if (IsZero(*zz_a) || IsZero(*zz_mod)) {
            free_ZZ(zz_a);
            free_ZZ(zz_mod);
            return nullptr; // Cannot invert zero
        }
        ZZ* res = new ZZ();
        *res = InvMod(*zz_a, *zz_mod); // Inverse modulo itself is not defined, but for demonstration
        free_ZZ(zz_a);
        return to_string(res);
    }

    char* zz_sqrt(const char* a) {
        ZZ* zz_a = from_string(a);
        if (IsZero(*zz_a)) {
            free_ZZ(zz_a);
            return strdup("0"); // Square root of zero is zero
        }
        ZZ* res = new ZZ();
        *res = SqrRoot(*zz_a);
        free_ZZ(zz_a);
        return to_string(res);
    }

    char* zz_random(int bits) {
        ZZ* res = new ZZ();
        RandomBits(*res, bits);
        return to_string(res);
    }

    char* zz_random_prime(int bits, int certainty) {
        ZZ* res = new ZZ();
        RandomPrime(*res, bits, certainty);
        return to_string(res);
    }

    char* zz_next_prime(const char* a) {
        ZZ* zz_a = from_string(a);
        ZZ* res = new ZZ();
        *res = NextPrime(*zz_a);
        free_ZZ(zz_a);
        return to_string(res);
    }

    // Compare equality (return 1 if equal, 0 else)
    int zz_eq(const char* a, const char* b) {
        ZZ* zz_a = from_string(a);
        ZZ* zz_b = from_string(b);
        int result = (*zz_a == *zz_b) ? 1 : 0;
        free_ZZ(zz_a);
        free_ZZ(zz_b);
        return result;
    }

    // Compare less than (return 1 if a < b, else 0)
    int zz_lt(const char* a, const char* b) {
        ZZ* zz_a = from_string(a);
        ZZ* zz_b = from_string(b);
        int result = (*zz_a < *zz_b) ? 1 : 0;
        free_ZZ(zz_a);
        free_ZZ(zz_b);
        return result;
    }

}