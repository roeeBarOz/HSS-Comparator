#ifndef NTL_LIB_H
#define NTL_LIB_H
#include <NTL/ZZ.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pE.h>
#include <NTL/vector.h>

#ifdef __cplusplus
extern "C" {
    #endif
    
    struct Context {
        NTL::ZZ q;
        NTL::ZZ p;
    };

    struct public_key {
        NTL::ZZ_pE a;
        NTL::ZZ_pE b;
    };

    struct PKE_Gen_keys {
        public_key pk;
        NTL::ZZ_pE sk;
    };

    struct eval_key {
        NTL::ZZ prf_key;
        NTL::ZZ_pE share_of_1;
        NTL::ZZ_pE share_of_sk;
    };

    struct HSS_Gen_keys {
        PKE_Gen_keys pke_keys;
        eval_key eval_key0;
        eval_key eval_key1;
    };

    struct encryption {
        NTL::ZZ_pE c_0;
        NTL::ZZ_pE c_1;
    };

    struct Input_Value {
        NTL::ZZ_pE c_00;
        NTL::ZZ_pE c_01;
        NTL::ZZ_pE c_10;
        NTL::ZZ_pE c_11;
    };

    struct Memory_Value {
        NTL::ZZ_pE mem_0;
        NTL::ZZ_pE mem_1;
    };

    #ifdef __cplusplus
}
#endif

#endif // NTL_LIB_H