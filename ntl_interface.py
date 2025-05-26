from cffi import FFI

ffi = FFI()

ffi.cdef("""
    void multiply_big_polynomials(const char** a_coeffs, int a_len,
                                  const char** b_coeffs, int b_len,
                                  char** result_coeffs, int* result_len);
""")

lib = ffi.dlopen("./libntl_wrapper.so")

def multiply_big_polynomials(a: list[str], b: list[str]) -> list[str]:
    a_c = ffi.new("char*[]", [ffi.new("char[]", x.encode()) for x in a])
    b_c = ffi.new("char*[]", [ffi.new("char[]", x.encode()) for x in b])
    max_len = len(a) + len(b) - 1
    result_c = ffi.new("char*[]", max_len)
    result_len = ffi.new("int*")

    lib.multiply_big_polynomials(a_c, len(a), b_c, len(b), result_c, result_len)
    
    return [ffi.string(result_c[i]).decode() for i in range(result_len[0])]
