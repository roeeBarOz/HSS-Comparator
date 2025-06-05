from cffi import FFI

ffi = FFI()

ffi.cdef("""
        void free_string(char* str);
        char* zz_add(const char* a, const char* b);
        char* zz_sub(const char* a, const char* b);
        char* zz_mul(const char* a, const char* b);
        char* zz_div(const char* a, const char* b);
        char* zz_mod(const char* a, const char* b);
        char* zz_pow(const char* base, const char* exp);
        char* zz_gcd(const char* a, const char* b);
        char* zz_neg(const char* a);
        char* zz_abs(const char* a);
        char* zz_inv(const char* a, const char* mod);
        char* zz_sqrt(const char* a);
        char* zz_random(int bits);
        char* zz_random_prime(int bits, int certainty);
        char* zz_next_prime(const char* a);
        int zz_eq(const char* a, const char* b);
        int zz_lt(const char* a, const char* b);
        char* zz_lcm(const char* a, const char* b);
""")

lib = ffi.dlopen("./libntl_wrappers.so")

def free_string(s: str) -> None:
    """Free a string allocated by the C library."""
    c_str = ffi.new("char[]", s.encode())
    lib.free_string(c_str)

def zz_add(a: str, b: str) -> str:
    """Add two ZZ numbers."""
    a_c = ffi.new("char[]", a.encode())
    b_c = ffi.new("char[]", b.encode())
    result_c = lib.zz_add(a_c, b_c)
    result = ffi.string(result_c).decode()
    return result

def zz_sub(a: str, b: str) -> str:
    """Subtract two ZZ numbers."""
    a_c = ffi.new("char[]", a.encode())
    b_c = ffi.new("char[]", b.encode())
    result_c = lib.zz_sub(a_c, b_c)
    result = ffi.string(result_c).decode()
    return result

def zz_mul(a: str, b: str) -> str:
    """Multiply two ZZ numbers."""
    a_c = ffi.new("char[]", a.encode())
    b_c = ffi.new("char[]", b.encode())
    result_c = lib.zz_mul(a_c, b_c)
    result = ffi.string(result_c).decode()
    return result

def zz_div(a: str, b: str) -> str:
    """Divide two ZZ numbers."""
    a_c = ffi.new("char[]", a.encode())
    b_c = ffi.new("char[]", b.encode())
    result_c = lib.zz_div(a_c, b_c)
    result = ffi.string(result_c).decode()
    return result

def zz_mod(a: str, b: str) -> str:
    """Compute the modulus of two ZZ numbers."""
    a_c = ffi.new("char[]", a.encode())
    b_c = ffi.new("char[]", b.encode())
    result_c = lib.zz_mod(a_c, b_c)
    result = ffi.string(result_c).decode()
    return result

def zz_pow(base: str, exp: str) -> str:
    """Raise a ZZ number to a power."""
    base_c = ffi.new("char[]", base.encode())
    exp_c = ffi.new("char[]", exp.encode())
    result_c = lib.zz_pow(base_c, exp_c)
    result = ffi.string(result_c).decode()
    return result

def zz_gcd(a: str, b: str) -> str:
    """Compute the GCD of two ZZ numbers."""
    a_c = ffi.new("char[]", a.encode())
    b_c = ffi.new("char[]", b.encode())
    result_c = lib.zz_gcd(a_c, b_c)
    result = ffi.string(result_c).decode()
    return result

def zz_neg(a: str) -> str:
    """Negate a ZZ number."""
    a_c = ffi.new("char[]", a.encode())
    result_c = lib.zz_neg(a_c)
    result = ffi.string(result_c).decode()
    return result

def zz_abs(a: str) -> str:
    """Compute the absolute value of a ZZ number."""
    a_c = ffi.new("char[]", a.encode())
    result_c = lib.zz_abs(a_c)
    result = ffi.string(result_c).decode()
    return result

def zz_inv(a: str, mod: str) -> str:
    """Compute the modular inverse of a ZZ number."""
    a_c = ffi.new("char[]", a.encode())
    mod_c = ffi.new("char[]", mod.encode())
    result_c = lib.zz_inv(a_c, mod_c)
    result = ffi.string(result_c).decode()
    return result

def zz_sqrt(a: str) -> str:
    """Compute the square root of a ZZ number."""
    a_c = ffi.new("char[]", a.encode())
    result_c = lib.zz_sqrt(a_c)
    result = ffi.string(result_c).decode()
    return result

def zz_random(bits: int) -> str:
    """Generate a random ZZ number with a specified number of bits."""
    result_c = lib.zz_random(bits)
    result = ffi.string(result_c).decode()
    return result

def zz_random_prime(bits: int, certainty: int) -> str:
    """Generate a random prime ZZ number with a specified number of bits."""
    result_c = lib.zz_random_prime(bits, certainty)
    result = ffi.string(result_c).decode()
    return result

def zz_next_prime(a: str) -> str:
    """Find the next prime ZZ number greater than or equal to a."""
    a_c = ffi.new("char[]", a.encode())
    result_c = lib.zz_next_prime(a_c)
    result = ffi.string(result_c).decode()
    return result

def zz_eq(a: str, b: str) -> bool:
    """Check if two ZZ numbers are equal."""
    a_c = ffi.new("char[]", a.encode())
    b_c = ffi.new("char[]", b.encode())
    result = lib.zz_eq(a_c, b_c)
    return result != 0

def zz_lt(a: str, b: str) -> bool:
    """Check if a ZZ number is less than another."""
    a_c = ffi.new("char[]", a.encode())
    b_c = ffi.new("char[]", b.encode())
    result = lib.zz_lt(a_c, b_c)
    return result != 0

def zz_lcm(a: str, b: str) -> str:
    """Compute the least common multiple of two ZZ numbers."""
    a_c = ffi.new("char[]", a.encode())
    b_c = ffi.new("char[]", b.encode())
    return lib.zz_lcm(a_c, b_c)
