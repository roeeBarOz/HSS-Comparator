#ifndef NTL_LIB_H
#define NTL_LIB_H
#include <NTL/ZZ.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pE.h>
#include <NTL/vector.h>

using namespace NTL;

#ifdef __cplusplus
extern "C" {
    #endif
    
    struct Context {
        ZZ q;
        ZZ p;
    };

    struct public_key {
        ZZ_pE a;
        ZZ_pE b;
    };

    struct PKE_Gen_keys {
        public_key pk;
        ZZ_pE sk;
    };

    struct eval_key {
        uint8_t prf_key[16]; // 128-bit key for AES
        ZZ_pE share_of_1;
        ZZ_pE share_of_sk;
    };

    struct HSS_Gen_keys {
        PKE_Gen_keys pke_keys;
        eval_key eval_key0;
        eval_key eval_key1;
    };

    struct encryption {
        ZZ_pE c_0;
        ZZ_pE c_1;
    };

    struct Input_Value {
        ZZ_pE c_00;
        ZZ_pE c_01;
        ZZ_pE c_10;
        ZZ_pE c_11;
    };

    struct Memory_Value {
        ZZ_pE mem_0;
        ZZ_pE mem_1;
    };

    Input_Value OKDM(const public_key& pk, const ZZ& x, const ZZ& p, const ZZ& q);
    Memory_Value DDEC(const Input_Value& input, const Memory_Value& memory, const Context& ctx);
    ZZ_pE round_poly(const ZZ_pE& value, const ZZ& p, const ZZ& q);
    encryption Enc(const public_key& pk, const ZZ_p& x);
    Memory_Value load(int b, int id, const Input_Value& input, const eval_key& ek, const Context& ctx);
    Memory_Value add_memory_values(int b, int id, const Memory_Value& mem0, const Memory_Value& mem1, const eval_key& ek);
    Input_Value add_input_values(const Input_Value& input0, const Input_Value& input1);
    Memory_Value multiply(int b, int id, const Input_Value& input, const Memory_Value& memory, const eval_key& ek, const Context& ctx);

    void polynomial_mult_time(int iterations);
    void polynomial_add_time(int iterations);
    void integer_mult_time(int iterations);
    void integer_add_time(int iterations);
    #ifdef __cplusplus
}
#endif

#endif // NTL_LIB_H