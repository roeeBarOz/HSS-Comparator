#include <NTL/ZZX.h>
#include <NTL/tools.h>
#include <sstream>

extern "C" {

    void multiply_big_polynomials(const char** a_coeffs, int a_len,
                                const char** b_coeffs, int b_len,
                                char** result_coeffs, int* result_len) {
        std::ostringstream oss;
        NTL::ZZX A, B, C;

        for (int i = 0; i < a_len; ++i)
            SetCoeff(A, i, NTL::to_ZZ(a_coeffs[i]));

        for (int i = 0; i < b_len; ++i)
            SetCoeff(B, i, NTL::to_ZZ(b_coeffs[i]));

        C = A * B;
        *result_len = deg(C) + 1;

        for (int i = 0; i < *result_len; ++i) {
            oss << coeff(C, i);
            std::string coeff_str = oss.str();
            result_coeffs[i] = strdup(coeff_str.c_str()); // allocate and copy
        }
    }

}
