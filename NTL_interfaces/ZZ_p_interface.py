from cffi import FFI

ffi = FFI()

ffi.cdef("""
        void zz_p_init(const char* modulus_str);
        void zz_p_free_string(char* str);
        char* zz_p_add(const char* a, const char* b);
        char* zz_p_sub(const char* a, const char* b);
        char* zz_p_mul(const char* a, const char* b);
        char* zz_p_div(const char* a, const char* b);
        char* zz_p_pow(const char* base_str, long exponent);
        char* zz_p_neg(const char* a);
        char* zz_p_inv(const char* a);
        int zz_p_eq(const char* a, const char* b)
""")

lib = ffi.dlopen("./libntl_wrappers.so")

def zz_p_init(s: str) -> None:
    """Set the modulus for next operations."""
    s_c = ffi.new("char[]", s.encode())
    lib.zz_p_init(s_c)

def zz_p_free_string(s: str) -> None:
    """Free the allocated String."""
    s_c = ffi.new("char[]", s.encode())
    lib.zz_p_free_string(s_c)

def zz_p_add(a: str, b: str) -> str:
    """Add two ZZ_p numbers."""
    a_c = ffi.new("char[]", a.encode())
    b_c = ffi.new("char[]", b.encode())
    result_c = lib.zz_p_add(a_c, b_c)
    result = ffi.string(result_c).decode()
    return result
    
def zz_p_sub(a: str, b: str) -> str:
    """Subtract two ZZ_p numbers."""
    a_c = ffi.new("char[]", a.encode())
    b_c = ffi.new("char[]", b.encode())
    result_c = lib.zz_p_sub(a_c, b_c)
    result = ffi.string(result_c).decode()
    return result


def zz_p_mul(a: str, b: str) -> str:
    """Multiply two ZZ_p numbers."""
    a_c = ffi.new("char[]", a.encode())
    b_c = ffi.new("char[]", b.encode())
    result_c = lib.zz_p_mul(a_c, b_c)
    result = ffi.string(result_c).decode()
    return result


def zz_p_div(numerator: str, denumerator: str) -> str:
    """Divide two ZZ_p numbers."""
    a_c = ffi.new("char[]", numerator.encode())
    b_c = ffi.new("char[]", denumerator.encode())
    result_c = lib.zz_p_div(a_c, b_c)
    result = ffi.string(result_c).decode()
    return result

def zz_p_pow(base: str, exponent: str) -> str:
    """Compute base to the power of exponent."""
    a_c = ffi.new("char[]", base.encode())
    b_c = ffi.new("char[]", exponent.encode())
    result_c = lib.zz_p_pow(a_c, b_c)
    result = ffi.string(result_c).decode()
    return result

def zz_p_neg(a: str) -> str:
    """Negate a ZZ_p number."""
    a_c = ffi.new("char[]", a.encode())
    return ffi.string(lib.zz_p_neg(a_c)).decode()

def zz_p_inv(a: str) -> str:
    """Compute the modular inverse of a ZZ_p number."""
    a_c = ffi.new("char[]", a.encode())
    return ffi.string(lib.zz_p_inv(a_c)).decode()

def zz_p_eq(a: str, b: str) -> bool:
    """Check if two ZZ_p numbers are equal."""
    a_c = ffi.new("char[]", a.encode())
    b_c = ffi.new("char[]", b.encode())
    result = lib.zz__p_eq(a_c, b_c)
    return result != 0
