#ifndef PAILLIER_HSS_HPP
#define PAILLIER_HSS_HPP

#include "../core/hss_interface.hpp"

using namespace NTL;

class PaillierMemoryValue : public IHSSMemoryValue {
public:
    std::vector<ZZ_p> m;

    size_t get_size_bytes() const override {
        size_t size = 0;
        for (const auto& val : m) {
            size += NumBytes(val);
        }
        return size;
    }
};

class PaillierInputValue : public IHSSInputValue {
public:
    std::vector<ZZ_p> i;

    size_t get_size_bytes() const override {
        size_t size = 0;
        for (const auto& val : i) {
            size += NumBytes(val);
        }
        return size;
    }
};

class PaillierEvalKey : public IHSSEvalKey {
public:
    uint8_t prf_key[16]; // 128-bit PRF key
    std::vector<ZZ> digits; // Precomputed digits for efficient multiplication

    size_t get_size_bytes() const override {
        size_t size = sizeof(prf_key);
        for (const auto& digit : digits) {
            size += NumBytes(digit);
        }
        return size;
    }
};

class PaillierContext : public IHSSContext {
public:
    ZZ Bsk;
    ZZ kappa;
    ZZ l;

    size_t get_size_bytes() const override {
        return NumBytes(Bsk) + NumBytes(kappa) + NumBytes(l);
    }
};

class PaillierPublicKey : public IHSSPublicKey {
public:
    ZZ N;
    ZZ sk;

    size_t get_size_bytes() const override {
        return NumBytes(N) + NumBytes(sk);
    }
};

class PaillierPKEGenKey : public IHSSPKEGenKey {
public:
    PaillierPublicKey pk;
    std::vector<PaillierEncryption> encrypted_digits;

    size_t get_size_bytes() const override {
        size_t size = pk.get_size_bytes();
        for (const auto& digit : encrypted_digits) {
            size += digit.get_size_bytes();
        }
        return size;
    }
};

class PaillierHSSGenKey : public IHSSHSSGenKey {
public:
    PaillierPKEGenKey pke_keys;
    PaillierEvalKey eval_key0;
    PaillierEvalKey eval_key1;

    size_t get_size_bytes() const override {
        return pke_keys.get_size_bytes() + eval_key0.get_size_bytes() + eval_key1.get_size_bytes();
    }
};

class PaillierEncryption : public IHSSEncryption {
public:
    ZZ_p c;

    size_t get_size_bytes() const override {
        return NumBytes(c);
    }
};

class PaillierOutputValue : public IHSSOutputValue {
public:
    ZZ m;

    size_t get_size_bytes() const override {
        return NumBytes(m);
    }
};

class PaillierHSS : public IHSS {
public:

    PaillierHSS() = default;
    ~PaillierHSS() override = default;

    std::string get_name() const override {
        return "PaillierHSS";
    }

    std::string get_math_family() const override {
        return "DCR";
    }

    HSSGenKeyHandle HSS_gen() override;
    InputHandle HSS_Enc(const PublicKeyHandle& pk, const ZZ& x, const ContextHandle& ctx, int k) override;
    MemoryHandle load(int party_id, int action_id, const InputHandle& input, const EvalKeyHandle& ek, const ContextHandle& ctx) override;
    MemoryHandle multiply(int party_id, int action_id, const MemoryHandle& mem, const InputHandle& in, const EvalKeyHandle& ek, const ContextHandle& ctx) override;
    MemoryHandle add_memory_values(int party_id, int action_id, const MemoryHandle& mem1, const MemoryHandle& mem2, const EvalKeyHandle& ek) override;
    InputHandle add_input_values(const InputHandle& in1, const InputHandle& in2) override;
}

class PaillierHelper {
public:
    ZZ_p DDLog(ZZ_p x, ZZ yd);
    ZZ construct_yd(ZZ Bsk, PaillierMemoryValue& mem);
}

#endif // PAILLIER_HSS_HPP
