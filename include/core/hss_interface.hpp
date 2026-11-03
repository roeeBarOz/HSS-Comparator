#ifndef HSS_INTERFACE_HPP
#define HSS_INTERFACE_HPP

#include "types.hpp"
#include <string>
#include <vector>
#include <chrono>
#include <numeric>

struct BenchmarkResult {
    std::string op_name;
    double avg_time_us;
    double min_time_us;
    size_t memory_bytes;
};

class IHSSScheme {
public:
    virtual ~IHSSScheme() = default;

    // --- Standardized Scheme Metadata ---
    virtual std::string get_name() const = 0;
    virtual std::string get_math_family() const = 0; // e.g., "DCR", "LWE", "RLWE", "FHE"

    // --- Core Cryptographic Interface (Implemented by Concrete Schemes) ---
    virtual void HSS_gen() = 0;
    virtual InputHandle HSS_Enc(const PublicKeyHandle& pk, const ZZ& x, const ContextHandle& ctx, int k) = 0;
    virtual MemoryHandle load(int party_id, int action_id, const InputHandle& input, 
                            const EvalKeyHandle& ek, const ContextHandle& ctx) = 0;
    virtual MemoryHandle multiply(int party_id, int action_id, const MemoryHandle& mem, 
                            const InputHandle& in, const EvalKeyHandle& ek, const ContextHandle& ctx) = 0;
    virtual MemoryHandle add_memory(int party_id, int action_id, const MemoryHandle& mem1, const MemoryHandle& mem2, 
                            const EvalKeyHandle& ek) = 0;
    virtual InputHandle add_inputs(const InputHandle& in1, const InputHandle& in2) = 0;

    // --- Metrics & Diagnostics ---
    virtual size_t get_share_size_bytes() const = 0;
    virtual size_t get_memory_footprint_bytes() const = 0;
    virtual double get_noise_budget_consumed() const = 0;

    // --- Universal Benchmark Runner Methods (Implemented ONLY in Base Class) ---

    BenchmarkResult benchmark_load(int party_id = 0, int iterations = 1000) {
        InputHandle in = sample_random_input();
        ContextHandle ctx = std::make_shared<IHSSContext>(); // Placeholder, replace with actual context initialization
        EvalKeyHandle ek = std::make_shared<IHSSEvalKey>(); // Placeholder, replace with actual eval key initialization
        MemoryHandle warmup = load(party_id, 0, in, ek, ctx);

        std::vector<double> timings_us;
        timings_us.reserve(iterations);

        for (int i = 0; i < iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            MemoryHandle res = load(party_id, 0, in, ek, ctx);
            auto end = std::chrono::high_resolution_clock::now();
            
            double elapsed = std::chrono::duration<double, std::micro>(end - start).count();
            timings_us.push_back(elapsed);
        }

        double sum = std::accumulate(timings_us.begin(), timings_us.end(), 0.0);
        double avg = sum / iterations;
        double min_val = *std::min_element(timings_us.begin(), timings_us.end());

        return BenchmarkResult{"Load", avg, min_val, warmup->get_size_bytes()};
    }

    BenchmarkResult benchmark_multiply(int party_id = 0, int iterations = 1000) {
        InputHandle in = sample_random_input();
        ContextHandle ctx = std::make_shared<IHSSContext>(); // Placeholder, replace with actual context initialization
        EvalKeyHandle ek = std::make_shared<IHSSEvalKey>(); // Placeholder, replace with actual eval key initialization
        MemoryHandle mem = load(party_id, 0, in, ek, ctx);
        MemoryHandle warmup = multiply(party_id, 0, mem, in, ek, ctx); // Placeholder, replace with actual eval key initialization

        std::vector<double> timings_us;
        timings_us.reserve(iterations);

        for (int i = 0; i < iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            MemoryHandle res = multiply(party_id, 0, mem, in, ek, ctx);
            auto end = std::chrono::high_resolution_clock::now();

            double elapsed = std::chrono::duration<double, std::micro>(end - start).count();
            timings_us.push_back(elapsed);
        }

        double sum = std::accumulate(timings_us.begin(), timings_us.end(), 0.0);
        double avg = sum / iterations;
        double min_val = *std::min_element(timings_us.begin(), timings_us.end());

        return BenchmarkResult{"Multiply", avg, min_val, warmup->get_size_bytes()};
    }

    BenchmarkResult benchmark_add_memory(int party_id = 0, int iterations = 1000) {
        InputHandle in1 = sample_random_input();
        InputHandle in2 = sample_random_input();
        EvalKeyHandle ek = std::make_shared<IHSSEvalKey>(); // Placeholder, replace with actual eval key initialization
        ContextHandle ctx = std::make_shared<IHSSContext>(); // Placeholder, replace with actual context initialization
        MemoryHandle mem1 = load(party_id, 0, in1, ek, ctx);
        MemoryHandle mem2 = load(party_id, 0, in2, ek, ctx);
        MemoryHandle warmup = add_memory(party_id, 0, mem1, mem2, ek);

        std::vector<double> timings_us;
        timings_us.reserve(iterations);

        for (int i = 0; i < iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            MemoryHandle res = add_memory(party_id, 0, mem1, mem2, ek);
            auto end = std::chrono::high_resolution_clock::now();

            double elapsed = std::chrono::duration<double, std::micro>(end - start).count();
            timings_us.push_back(elapsed);
        }

        double sum = std::accumulate(timings_us.begin(), timings_us.end(), 0.0);
        double avg = sum / iterations;
        double min_val = *std::min_element(timings_us.begin(), timings_us.end());

        return BenchmarkResult{"AddMemory", avg, min_val, warmup->get_size_bytes()};
    }

    BenchmarkResult benchmark_add_inputs(int iterations = 1000) {
        InputHandle in1 = sample_random_input();
        InputHandle in2 = sample_random_input();
        InputHandle warmup = add_inputs(in1, in2);

        std::vector<double> timings_us;
        timings_us.reserve(iterations);

        for (int i = 0; i < iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            InputHandle res = add_inputs(in1, in2, std::make_shared<IHSSEvalKey>(), std::make_shared<IHSSContext>());
            auto end = std::chrono::high_resolution_clock::now();

            double elapsed = std::chrono::duration<double, std::micro>(end - start).count();
            timings_us.push_back(elapsed);
        }

        double sum = std::accumulate(timings_us.begin(), timings_us.end(), 0.0);
        double avg = sum / iterations;
        double min_val = *std::min_element(timings_us.begin(), timings_us.end());

        return BenchmarkResult{"AddInputs", avg, min_val, warmup->get_size_bytes()};
    }
};

#endif // HSS_INTERFACE_HPP
