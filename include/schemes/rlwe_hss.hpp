#ifndef RLWE_HSS_HPP
#define RLWE_HSS_HPP

#include "core/hss_interface.hpp"
#include <NTL/ZZ_pE.h>

class RLWEInputValue : public IHSSInputValue {
public:
    NTL::ZZ_pE M[2][2]; 

    size_t get_size_bytes() const override {
        return NTL::NumBytes(M[0][0]) + NTL::NumBytes(M[0][1]) + NTL::NumBytes(M[1][0]) + NTL::NumBytes(M[1][1]);
    }
};

class RLWEMemoryValue : public IHSSMemoryValue {
public:
    NTL::ZZ_pE v[2];

    size_t get_size_bytes() const override {
        return NTL::NumBytes(v[0]) + NTL::NumBytes(v[1]);
    }
};

class RLWEEvalKey : public IHSSEvalKey {
public:
    uint8_t prf_key[16]; // 128-bit key for AES
    NTL::ZZ_pE share_of_1;
    NTL::ZZ_pE share_of_sk;

    size_t get_size_bytes() const override {
        return 16 + NTL::NumBytes(share_of_1) + NTL::NumBytes(share_of_sk);
    }
};

class RLWEContext : public IHSSContext {
public:
    NTL::ZZ p;
    NTL::ZZ q;

    size_t get_size_bytes() const override {
        return NTL::NumBytes(p) + NTL::NumBytes(q);
    }
};

class RLWEPublicKey : public IHSSPublicKey {
public:
    NTL::ZZ_pE a;
    NTL::ZZ_pE b;

    size_t get_size_bytes() const override {
        return NTL::NumBytes(a) + NTL::NumBytes(b);
    }
};

class RLWEPKEGenKey : public IHSSPKEGenKey {
public:
    RLWEPublicKey pk;
    NTL::ZZ_pE sk;

    size_t get_size_bytes() const override {
        return pk.get_size_bytes() + NTL::NumBytes(sk);
    }
};

class RLWEHSSGenKey : public IHSSHSSGenKey {
public:
    RLWEPKEGenKey pke_keys;
    RLWEEvalKey eval_key0;
    RLWEEvalKey eval_key1;

    size_t get_size_bytes() const override {
        return pke_keys.get_size_bytes() + eval_key0.get_size_bytes() + eval_key1.get_size_bytes();
    }
};

class RLWEEncryption : public IHSSEncryption {
public:
    NTL::ZZ_pE c[2];

    size_t get_size_bytes() const override {
        return NTL::NumBytes(c[0]) + NTL::NumBytes(c[1]);
    }
};

class RLWEHSS : public IHSSScheme {
public:
    HSSGenKeyHandle HSS_gen() override;
    InputHandle HSS_Enc(const PublicKeyHandle& pk, const ZZ& x, const ContextHandle& ctx, int k) override;
    MemoryHandle load(int party_id, int action_id, const InputHandle& input, const EvalKeyHandle& ek, const ContextHandle& ctx) override;
    MemoryHandle multiply(int party_id, int action_id, const MemoryHandle& mem, const InputHandle& in, 
                        const EvalKeyHandle& ek, const ContextHandle& ctx) override;
    MemoryHandle add_memory_values(int party_id, int action_id, const MemoryHandle& mem1, const MemoryHandle& mem2, 
                        const EvalKeyHandle& ek) override;
    InputHandle add_input_values(const InputHandle& in1, const InputHandle& in2) override;
};

#endif // RLWE_HSS_HPP
