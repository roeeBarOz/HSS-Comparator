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

    Input_Value OKDM(public_key pk, ZZ x, ZZ p, ZZ q);
    Memory_Value DDEC(Input_Value input, Memory_Value memory, Context ctx);
    ZZ_pE round_poly(ZZ_pE value, ZZ p, ZZ q);
    encryption Enc(public_key pk, ZZ_p x);

    #ifdef __cplusplus
}
#endif

#endif // NTL_LIB_H