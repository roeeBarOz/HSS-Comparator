#include <NTL/ZZ_p.h>
#include <NTL/vector.h>
#include <chrono>
#include "../PRF/aesni_ctr.c"
#include "../include/schemes/paillier_hss.hpp"

using namespace NTL;

PaillierContext default_ctx = PaillierContext();
auto hss = std::make_shared<PaillierHSS>();
PaillierHelper helper;

extern "C" {

    std::vector<ZZ> PaillierHelper::decompose_sk(const ZZ& sk, const ZZ& Bsk) {
        std::vector<ZZ> digits;
        ZZ temp_sk = sk;
        while (temp_sk > 0) {
            digits.push_back(temp_sk % Bsk);
            temp_sk /= Bsk;
        }
        return digits;
    }

    uint8_t* PaillierHelper::generate_random_prf_key() {
        static uint8_t key[16]; // 128-bit key
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        for (int i = 0; i < 16; ++i) {
            key[i] = dis(gen);
        }
        return key;
    }

    PaillierPublicKey PKE_gen() {
        return PaillierPKEGenKey();
    }

    HSSGenKeyHandle PaillierHSS::HSS_gen(const PaillierContext& ctx) {
        PaillierPublicKey pk_sk = PKE_gen();
        ZZ sk = pk_sk.sk;
        std::vector<ZZ> sk_digits = helper.decompose_sk(sk, ctx.Bsk);

        ZZ Bsk_time_two_to_kappa = ctx.Bsk * power2_ZZ(ctx.kappa);
        std::vector<ZZ> eval_key1_digits, eval_key0_digits;
        ZZ temp;
        for (const auto& digit : sk_digits) {
            temp = RandomBnd(Bsk_time_two_to_kappa);
            eval_key1_digits.push_back(temp);
            eval_key0_digits.push_back(temp - digit);
        }
        uint8_t* prf_key = helper.generate_random_prf_key();
        PaillierEvalKey eval_key0(prf_key, eval_key0_digits);
        PaillierEvalKey eval_key1(prf_key, eval_key1_digits);

        std::vector<ZZ_p> encryptions_of_digits;
        for (const auto& digit : sk_digits) {
            ZZ_p encrypted_digit = helper.encrypt(pk_sk.pk_paillier, digit);
            encryptions_of_digits.push_back(encrypted_digit);
        }

        PaillierPKEGenKey pke_keys(pk_sk.pk_paillier, encryptions_of_digits);
        PaillierHSSGenKey hss_gen_key(pke_keys, eval_key0, eval_key1);
        HSSGenKeyHandle hss_gen_key_handle = std::make_shared<PaillierHSSGenKey>(hss_gen_key);

        return hss_gen_key_handle;
    }

    InputHandle PaillierHSS::HSS_Enc(const PublicKeyHandle &pk, const ZZ &x, const ContextHandle &ctx, int k){
        return InputHandle();
    }

    MemoryHandle PaillierHSS::load(int party_id, int action_id, const InputHandle &input, const EvalKeyHandle &ek, 
                                const ContextHandle &ctx){
        return MemoryHandle();
    }

    MemoryHandle PaillierHSS::multiply(int party_id, int action_id, const MemoryHandle &mem, const InputHandle &in, 
                                    const EvalKeyHandle &ek, const ContextHandle &ctx){
        return MemoryHandle();
    }

    MemoryHandle PaillierHSS::add_memory_values(int party_id, int action_id, const MemoryHandle &mem1, 
                                                const MemoryHandle &mem2, const EvalKeyHandle &ek){
        return MemoryHandle();
    }

    InputHandle PaillierHSS::add_input_values(const InputHandle &in1, const InputHandle &in2){
        return InputHandle();
    }

    ZZ_p PaillierHelper::DDLog(ZZ_p x, ZZ yd){
        return ZZ_p();
    }

    ZZ PaillierHelper::construct_yd(ZZ Bsk, PaillierMemoryValue &mem){
        return ZZ();
    }

}
