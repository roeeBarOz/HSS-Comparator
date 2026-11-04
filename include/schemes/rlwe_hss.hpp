#ifndef RLWE_HSS_HPP
#define RLWE_HSS_HPP

#include "../core/hss_interface.hpp"
#include <NTL/ZZ_pE.h>

using namespace NTL;

size_t get_size_of_ZZ_pE(const ZZ_pE& polynomial) {
    // Get the underlying polynomial representative
    const ZZ_pX& poly = rep(polynomial);
    ZZ_p coeff = conv<ZZ_p>(0);
    size_t total_bytes = 0;
    
    // deg() returns -1 for a zero polynomial
    long degree = deg(poly);
    if (degree < 0) {
        return 0; 
    }

    // Accumulate the byte requirements for each active coefficient
    for (long i = 0; i <= degree; ++i) {
        GetCoeff(coeff, poly, i); // Get the i-th coefficient of the polynomial
        
        // Extract the BigInt (ZZ) representative of the coefficient field element
        const ZZ& big_int = rep(coeff);
        
        // Count base-256 digits (bytes) needed for this BigInt
        total_bytes += NumBytes(big_int);
    }
    
    return total_bytes;
}

class RLWEInputValue : public IHSSInputValue {
public:
    ZZ_pE M[2][2]; 

    size_t get_size_bytes() const override {
        return get_size_of_ZZ_pE(M[0][0]) + get_size_of_ZZ_pE(M[0][1]) + get_size_of_ZZ_pE(M[1][0]) + get_size_of_ZZ_pE(M[1][1]);
    }
};

class RLWEMemoryValue : public IHSSMemoryValue {
public:
    ZZ_pE v[2];

    RLWEMemoryValue(ZZ_pE v0 = conv<ZZ_pE>(0), ZZ_pE v1 = conv<ZZ_pE>(0)) {
        v[0] = v0;
        v[1] = v1;
    }

    size_t get_size_bytes() const override {
        return get_size_of_ZZ_pE(v[0]) + get_size_of_ZZ_pE(v[1]);
    }
};

class RLWEEvalKey : public IHSSEvalKey {
public:
    uint8_t prf_key[16]; // 128-bit key for AES
    ZZ_pE share_of_1;
    ZZ_pE share_of_sk;

    size_t get_size_bytes() const override {
        return 16 + get_size_of_ZZ_pE(share_of_1) + get_size_of_ZZ_pE(share_of_sk);
    }
};

class RLWEContext : public IHSSContext {
public:
    ZZ p;
    ZZ q;

    RLWEContext() : p(0), q(0) {}

    size_t get_size_bytes() const override {
        return NumBytes(p) + NumBytes(q);
    }
};

class RLWEPublicKey : public IHSSPublicKey {
public:
    ZZ_pE a;
    ZZ_pE b;

    size_t get_size_bytes() const override {
        return get_size_of_ZZ_pE(a) + get_size_of_ZZ_pE(b);
    }
};

class RLWEPKEGenKey : public IHSSPKEGenKey {
public:
    RLWEPublicKey pk;
    ZZ_pE sk;

    size_t get_size_bytes() const override {
        return pk.get_size_bytes() + get_size_of_ZZ_pE(sk);
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
    ZZ_pE c[2];

    size_t get_size_bytes() const override {
        return get_size_of_ZZ_pE(c[0]) + get_size_of_ZZ_pE(c[1]);
    }
};

class RLWEOutputValue : public IHSSOutputValue {
public:
    ZZ_pE out;

    size_t get_size_bytes() const override {
        return get_size_of_ZZ_pE(out);
    }
};

class RLWEHSS : public IHSSScheme {
public:

    RLWEHSS() = default;
    ~RLWEHSS() = default;

    // --- Standardized Scheme Metadata ---
    std::string get_name() const override {
        return "RLWE_HSS";
    }
    std::string get_math_family() const override {
        return "RLWE";
    }

    HSSGenKeyHandle HSS_gen() override;
    InputHandle HSS_Enc(const PublicKeyHandle& pk, const ZZ& x, const ContextHandle& ctx, int k) override;
    MemoryHandle load(int party_id, int action_id, const InputHandle& input, const EvalKeyHandle& ek, const ContextHandle& ctx) override;
    MemoryHandle multiply(int party_id, int action_id, const MemoryHandle& mem, const InputHandle& in, 
                        const EvalKeyHandle& ek, const ContextHandle& ctx) override;
    MemoryHandle add_memory_values(int party_id, int action_id, const MemoryHandle& mem1, const MemoryHandle& mem2, 
                        const EvalKeyHandle& ek) override;
    InputHandle add_input_values(const InputHandle& in1, const InputHandle& in2) override;
    
};

class RLWEHelper {
public:
    RLWEMemoryValue RLWE_DDEC(const RLWEInputValue& input, const RLWEMemoryValue& value_of_s, const RLWEContext& ctx);
    RLWEInputValue RLWE_OKDM(const RLWEPublicKey& pk, const ZZ& x, const ZZ& p, const ZZ& q, int k);
    ZZ_pE round_poly(const ZZ_pE& value, const ZZ& p, const ZZ& q);
    RLWEEncryption Enc(const RLWEPublicKey& pk, const ZZ_pX& x);
};
#endif // RLWE_HSS_HPP
