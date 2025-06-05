#include <NTL/ZZ_p.h>
#include <sstream>
#include <cstdlib>

using namespace NTL;

extern "C" {

    // Initialize ZZ_p modulus from string
    void zz_p_init(const char* modulus_str) {
        ZZ p = to_ZZ(modulus_str);
        ZZ_p::init(p);
    }

    // Allocate and initialize a ZZ_p from string
    ZZ_p* zz_p_from_string(const char* val_str) {
        ZZ val = to_ZZ(val_str);
        ZZ_p* result = new ZZ_p(conv<ZZ_p>(val));
        return result;
    }

    // Convert ZZ_p to string (returns malloc'd C-string)
    char* zz_p_to_string(const ZZ_p* val) {
        std::ostringstream oss;
        oss << *val;
        std::string str = oss.str();
        return strdup(str.c_str());
    }

    // Free ZZ_p
    void zz_p_free(ZZ_p* val) {
        delete val;
    }

    // Free C string
    void zz_p_free_string(char* str) {
        free(str);
    }

    // Arithmetic operations

    char* zz_p_add(const char* a, const char* b) {
        ZZ_p* x = zz_p_from_string(a);
        ZZ_p* y = zz_p_from_string(b);
        ZZ_p* res = new ZZ_p(*x + *y);
        zz_p_free(x);
        zz_p_free(y);
        return zz_p_to_string(res);
    }

    char* zz_p_sub(const char* a, const char* b) {
        ZZ_p* x = zz_p_from_string(a);
        ZZ_p* y = zz_p_from_string(b);
        ZZ_p* res = new ZZ_p(*x - *y);
        zz_p_free(x);
        zz_p_free(y);
        return zz_p_to_string(res);
    }

    char* zz_p_mul(const char* a, const char* b) {
        ZZ_p* x = zz_p_from_string(a);
        ZZ_p* y = zz_p_from_string(b);
        ZZ_p* res = new ZZ_p(*x * *y);
        zz_p_free(x);
        zz_p_free(y);
        return zz_p_to_string(res);
    }

    char* zz_p_div(const char* a, const char* b) {
        ZZ_p* x = zz_p_from_string(a);
        ZZ_p* y = zz_p_from_string(b);
        if (IsZero(*y)) {
            zz_p_free(x);
            zz_p_free(y);
            return nullptr;  // Division by zero
        }
        ZZ_p* res = new ZZ_p(*x / *y);
        zz_p_free(x);
        zz_p_free(y);
        return zz_p_to_string(res);
    }

    char* zz_p_neg(const char* a) {
        ZZ_p* x = zz_p_from_string(a);
        ZZ_p* res = new ZZ_p(-(*x));
        zz_p_free(x);
        return zz_p_to_string(res);
    }

    char* zz_p_inv(const char* a) {
        ZZ_p* x = zz_p_from_string(a);
        if (IsZero(*x)) {
            zz_p_free(x);
            return nullptr; // No inverse
        }
        ZZ_p* res = new ZZ_p(inv(*x));
        zz_p_free(x);
        return zz_p_to_string(res);
    }

    // Equality check (returns 1 if equal, else 0)
    int zz_p_eq(const char* a, const char* b) {
        ZZ_p* x = zz_p_from_string(a);
        ZZ_p* y = zz_p_from_string(b);
        int result = (*x == *y) ? 1 : 0;
        zz_p_free(x);
        zz_p_free(y);
        return result;
    }
}
