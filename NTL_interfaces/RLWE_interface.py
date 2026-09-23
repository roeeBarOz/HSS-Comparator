from cffi import FFI

ffi = FFI()

ffi.cdef("""
        void free_string(char* str);
        void benchmark_all(int iterations);
""")

lib = ffi.dlopen("./libntl_wrappers.so")

def free_string(s: str) -> None:
    """Free a string allocated by the C library."""
    c_str = ffi.new("char[]", s.encode())
    lib.free_string(c_str)
    
def benchmark_all(iterations: int) -> None:
    """Benchmark all functions."""
    lib.benchmark_all(iterations)
