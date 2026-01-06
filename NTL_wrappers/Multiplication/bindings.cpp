/*
 * bindings.cpp
 * This file wraps the 32-bit RNS AVX2 C++ library for Python.
 * It implements the "square-and-multiply" exponentiation loop
 * completely in C++, so Python is only called once.
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // For std::string
#include "rns_mont.h"      // Our C++ library header

namespace py = pybind11;

/**
 * @brief This wrapper class holds the RnsContext.
 *
 * Python will create one of these objects. The constructor is slow
 * (precomputation), but the 'mod_exp' call is fast.
 */
class PyRnsContext {
public:
    RnsContext* ctx;

    PyRnsContext(std::string mod_str) {
        // 1. Initialize the C++ RnsContext.
        //    This runs the expensive CRNSPrecomputation (Alg 2).
        ctx = new RnsContext(mod_str);
    }
    
    ~PyRnsContext() {
        // Clean up the context when the Python object is garbage collected
        delete ctx;
    }

    /**
     * @brief The main function exposed to Python.
     * Takes base, exponent, and modulus as strings.
     */
    std::string modular_exponentiation(std::string base_str, std::string exp_str) {
        
        mpz_t e;
        mpz_init_set_str(e, exp_str.c_str(), 10);
        
        RnsNumber* base = stringToRns(base_str, ctx);
        RnsNumber* r = stringToRns("1", ctx); // r = 1
        
        rnsPowerMod(r, base, e, ctx);
        
        std::string result_str = rnsToString(r, ctx);
        
        mpz_clear(e);
        destroyRnsNumber(base);
        destroyRnsNumber(r);
        return result_str;
    }
};

// This is the "module definition" that Python will import
PYBIND11_MODULE(rns_avx2_backend, m) {
    m.doc() = "Backend for RNS Montgomery AVX2 (32-bit port)";

    py::class_<PyRnsContext>(m, "RnsContext")
        // Binds the constructor: ctx = RnsContext("modulus_string")
        .def(py::init<std::string>())
        
        // Binds the method: ctx.mod_exp("base_str", "exp_str")
        .def("mod_exp", &PyRnsContext::modular_exponentiation);
}

