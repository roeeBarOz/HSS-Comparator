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

    ZZ_p PaillierHelper::encrypt(const PaillierPublicKey& pk, const ZZ& m, const ZZ& g) {
        ZZ_p r = conv<ZZ_p>(RandomBnd(pk.pk_paillier));
        r = r ^ pk.pk_paillier;
        ZZ N_squared = pk.pk_paillier * pk.pk_paillier;
        if (g == pk.pk_paillier + 1) {
            // in this case: (1 + N)^m = 1 + mN mod N^2
            return (1 + m * pk.pk_paillier) * r % N_squared;
        }
        else {
            return PowerMod(g, m, N_squared) * r;
        }
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

    InputHandle PaillierHSS::HSS_Enc(const PKEGenKeyHandle &pke_keys, const ZZ &x, const ContextHandle &ctx, int k){
        std::vector<ZZ_p> input;
        ZZ_p X = helper.encrypt(pke_keys->pk, x, pke_keys->pk->pk_paillier + 1);
        input.push_back(X);
        for (ZZ_p encrypted_digit : pke_keys->encrypted_digits) {
            // X^(i) = r_i^N * (D^(i))^x mod N^2
            input.push_back(helper.encrypt(pke_keys->pk, x, conv<ZZ>(encrypted_digit)));
        }
        PaillierInputValue input_value(input);
        InputHandle input_handle = std::make_shared<PaillierInputValue>(input_value);
        return input_handle;
    }

    MemoryHandle PaillierHSS::load(int party_id, int action_id, const InputHandle &input, const EvalKeyHandle &ek, 
                                const ContextHandle &ctx){
        std::vector<ZZ_p> mem;
        // push a secret share of 1 into mem
        // do it by using the PRF function
        PaillierEvalKey eval_key = *std::dynamic_pointer_cast<PaillierEvalKey>(ek);
        for (const ZZ_p& val : eval_key.digits) {
            mem.push_back(val);
        }
        return hss.multiply(party_id, action_id, std::make_shared<PaillierMemoryValue>(PaillierMemoryValue(mem)), input, ek, ctx);
    }

    MemoryHandle PaillierHSS::multiply(int party_id, int action_id, const MemoryHandle &mem, const InputHandle &in, 
                                    const EvalKeyHandle &ek, const ContextHandle &ctx){
        PaillierContext context = *std::dynamic_pointer_cast<PaillierContext>(ctx);
        PaillierEvalKey eval_key = *std::dynamic_pointer_cast<PaillierEvalKey>(ek);
        ZZ share_of_yd = helper.construct_yd(context.Bsk, *std::dynamic_pointer_cast<PaillierMemoryValue>(mem));
        std::vector<ZZ_p> exponent_outputs, ddlog_outputs;
        if (ZZ_p::modulus() == context.pk_paillier) {
            ZZ_p::init(context.pk_paillier * context.pk_paillier);
        }
        for (const ZZ_p& val : std::dynamic_pointer_cast<PaillierInputValue>(in)->i) {
            exponent_outputs.push_back(val ^ share_of_yd);
        }
        int index = 0;
        ZZ_p::init(context.pk_paillier);
        for (const ZZ_p& val : exponent_outputs) {
            // TODO: Add PRF evaluation
            if (index == 0) {
                ddlog_outputs.push_back(helper.DDLog(val, context.pk_paillier) % context.pk_paillier);
            }
            ddlog_outputs.push_back(helper.DDLog(val, context.pk_paillier));
            index += 1;
        }
        PaillierMemoryValue mem_value(ddlog_outputs);
        return std::make_shared<PaillierMemoryValue>(mem_value);
    }

    MemoryHandle PaillierHSS::add_memory_values(int party_id, int action_id, const MemoryHandle &mem1, 
                                                const MemoryHandle &mem2, const EvalKeyHandle &ek){
        PaillierMemoryValue mem1_value = *std::dynamic_pointer_cast<PaillierMemoryValue>(mem1);
        PaillierMemoryValue mem2_value = *std::dynamic_pointer_cast<PaillierMemoryValue>(mem2);
        std::vector<ZZ_p> result;
        for (size_t i = 0; i < mem1_value.m.size(); ++i) {
            result.push_back(mem1_value.m[i] + mem2_value.m[i]);
        }
        PaillierMemoryValue result_value(result);
        return std::make_shared<PaillierMemoryValue>(result_value);
    }

    InputHandle PaillierHSS::add_input_values(const InputHandle &in1, const InputHandle &in2, const ContextHandle& ctx){
        PaillierContext context = *std::dynamic_pointer_cast<PaillierContext>(ctx);
        PaillierInputValue in1_value = *std::dynamic_pointer_cast<PaillierInputValue>(in1);
        PaillierInputValue in2_value = *std::dynamic_pointer_cast<PaillierInputValue>(in2);
        std::vector<ZZ_p> result;
        if (ZZ_p::modulus() == context.pk_paillier) {
            ZZ_p::init(context.pk_paillier * context.pk_paillier);
        }
        for (size_t i = 0; i < in1_value.i.size(); ++i) {
            result.push_back(in1_value.i[i] + in2_value.i[i]);
        }
        PaillierInputValue result_value(result);
        return std::make_shared<PaillierInputValue>(result_value);
    }

    ZZ_p PaillierHelper::DDLog(ZZ_p x, ZZ N){
        ZZ_p h = x % N;
        ZZ_p h_tag = x / N;
        return h * inv(h_tag);
    }

    ZZ PaillierHelper::construct_yd(ZZ Bsk, PaillierMemoryValue &mem){
        ZZ yd = ZZ(0);
        ZZ Bsk_accumulator = ZZ(1);
        std::vector<ZZ_p> yd_values;
        for (size_t i = 1; i < mem.m.size(); ++i) {
            yd_values.push_back(mem.m[i]);
        }
        for (size_t i = 0; i < yd_values.size(); ++i) {
            yd += conv<ZZ>(yd_values[i]) * Bsk_accumulator;
            Bsk_accumulator *= Bsk;
        }
        return yd;
    }

}
