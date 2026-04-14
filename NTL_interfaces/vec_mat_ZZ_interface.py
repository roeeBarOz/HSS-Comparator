from cffi import FFI

ffi = FFI()

ffi.cdef("""
    void free_vec_mat_string(char* str);
    void benchmark_ntl_mul(long size, long iterations, const char* p_str);    
    void benchmark_ntl_add_mat(long size, long iterations, const char* p_str);
    void benchmark_ntl_setup(long n, long m, long q_length, long p_length, long times);
    void Setup(const char* lambda, long n, long m, long q_length, long p_length);
    void benchmark_ntl_gen_okdm_chunk(const uint8_t* seed_A, const char* b, const char* message,
                         long start_row, long num_rows, long m, long iterations);
    void benchmark_ntl_add_memory_values(int b, const char* val1, const char* val2, const char* q,
                         const uint8_t prf_key, long step_index, long row_length, long iterations);
""")

# Load the library (ensure you compile all cpp files into this .so)
lib = ffi.dlopen("./libntl_wrappers.so")

def _wrap_op(func, *args):
    """Helper to handle C string conversion and freeing."""
    c_args = [ffi.new("char[]", str(arg).encode()) if isinstance(arg, str) else arg for arg in args]
    res_c = func(*c_args)
    res_str = ffi.string(res_c).decode()
    lib.free_vec_mat_string(res_c)
    return res_str

def _wrap_op_no_ret(func, *args):
    """Helper for functions that return void."""
    c_args = [ffi.new("char[]", str(arg).encode()) if isinstance(arg, str) else arg for arg in args]
    func(*c_args)

# --- Python API ---

def benchmark_ntl_mul(size: int, iterations: int, p_str: str):
    """
    Benchmark NTL vector-matrix multiplication for given size, iterations, and modulus p (as string).
    
    Parameters:
        size: The dimension of the square matrix and vector.
        iterations: The number of times to repeat the multiplication for benchmarking.
        p_str: The modulus p as a string.
    """
    return _wrap_op_no_ret(lib.benchmark_ntl_mul, size, iterations, p_str)

def benchmark_ntl_add_mat(size: int, iterations: int, p_str: str):
    """
    Benchmark NTL matrix addition for given size, iterations, and modulus p (as string).
    
    Parameters:
        size: The dimension of the square matrices.
        iterations: The number of times to repeat the addition for benchmarking.
        p_str: The modulus p as a string.
    """
    return _wrap_op_no_ret(lib.benchmark_ntl_add_mat, size, iterations, p_str)

def benchmark_ntl_setup(n: int, m: int, q_length: int, p_length: int, times: int):
    """
    Benchmark NTL LWE-setup for given parameters.
    
    Parameters:
        n: The dimension of the secret vector, and the number of columns in the matrix A.
        m: The number of rows in the matrix A.
        q_length: The bit-length of the modulus q.
        p_length: The bit-length of the modulus p.
        times: The number of times to repeat the setup for benchmarking.
    """
    return _wrap_op_no_ret(lib.benchmark_ntl_setup, n, m, q_length, p_length, times)

def Setup(lambda_str: str, n: int, m: int, q_length: int, p_length: int):
    """
    LWE-Setup NTL for given parameters.
    
    Parameters:
        lambda_str: The security parameter as a string (e.g., "128").
        n: The dimension of the secret vector, and the number of columns in the matrix A.
        m: The number of rows in the matrix A.
        q_length: The bit-length of the modulus q.
        p_length: The bit-length of the modulus p.
    """
    return _wrap_op_no_ret(lib.Setup, lambda_str, n, m, q_length, p_length)

def benchmark_ntl_gen_okdm_chunk(seed_A: str, b: str, message: str, start_row: int, num_rows: int, m: int, iterations: int):
    """
    Benchmark NTL OKDM chunk generation for given parameters.
    A chunk is a set of vectors generated from the same seed_A, and saved into a file.
    This function benchmarks the generation of such a chunk, which is used in the OKDM protocol.
    
    Parameters:
        seed_A: The seed for generating the matrix A (as a string).
        b: The vector b (as a string).
        message: The plaintext message (as a string).
        start_row: The starting row index for the chunk generation.
        num_rows: The number of rows in the chunk to generate.
        m: The number of columns in the matrix A.
        iterations: The number of times to repeat the chunk generation for benchmarking.
    """
    # Convert seed_A to bytes if it's a list of integers
    if isinstance(seed_A, list):
        seed_A = bytes(seed_A)
    return _wrap_op_no_ret(lib.benchmark_ntl_gen_okdm_chunk, seed_A, b, message, start_row, num_rows, m, iterations)

def benchmark_ntl_add_memory_values(b: int, val1: str, val2: str, q: str, prf_key: str, step_index: int, row_length: int, iterations: int):
    """
    Benchmark NTL LWE-memory value addition for given parameters.
    
    Parameters:
        b: Identifier of the player (0 or 1).
        val1: The first memory value (as a string).
        val2: The second memory value (as a string).
        q: The modulus q (as a string).
        prf_key: The PRF key used for masking (as a string).
        step_index: The index of the current step in the protocol (used for PRF masking).
        row_length: The length of the memory value vectors.
        iterations: The number of times to repeat the addition for benchmarking.
    """
    return _wrap_op_no_ret(lib.benchmark_ntl_add_memory_values, b, val1, val2, q, prf_key, step_index, row_length, iterations)