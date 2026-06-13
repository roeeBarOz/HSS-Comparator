from cffi import FFI

ffi = FFI()

ffi.cdef("""
    void free_vec_mat_string(char* str);
    void benchmark_ntl_mul(long size, long iterations, const char* p_str);    
    void benchmark_ntl_add_mat(long size, long iterations, const char* p_str);
    void benchmark_ntl_setup(long n, long m, long Bmsg_length, long P_size, long times);
    void Setup(const char* lambda, long n, long m, long Bmsg_length, long P_size);
    void run_benchmark_OKDM(long n, long m, const char* q_str, int iterations);
    void run_benchmark_DDEC(long n, long m, const char* q_str, const char* p_str, int iterations);
    void benchmark_ntl_gen_okdm_chunk(const uint8_t* seed_A, const char* b, const char* message,
                         long start_row, long num_rows, long m, long iterations);
    void benchmark_ntl_add_memory_values(int b, const char* val1, const char* val2, const char* q,
                         const uint8_t prf_key, long step_index, long row_length, long iterations);
    void run_benchmark_last_mul(long n, long m, const char* q_str, const char* p_str, int iterations);
    void run_benchmark_last_mem_add(int b, long n, const char* q_str, const char* p_str, int iterations);
""")

# Load the library (ensure you compile all cpp files into this .so)
lib = ffi.dlopen("/home/roee/HSS-Comparator/libntl_wrappers.so")

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

def benchmark_ntl_setup(n: int, m: int, Bmsg_length: int, P_size: int, times: int):
    """
    Benchmark NTL LWE-setup for given parameters.
    
    Parameters:
        n: The dimension of the secret vector, and the number of columns in the matrix A.
        m: The number of rows in the matrix A.
        Bmsg_length: The bit-length of the message.
        P_size: The size of the polynomial.
        times: The number of times to repeat the setup for benchmarking.
    """
    return _wrap_op_no_ret(lib.benchmark_ntl_setup, n, m, Bmsg_length, P_size, times)

def Setup(lambda_str: str, n: int, m: int, Bmsg_length: int, P_size: int):
    """
    LWE-Setup NTL for given parameters.
    
    Parameters:
        lambda_str: The security parameter as a string (e.g., "128").
        n: The dimension of the secret vector, and the number of columns in the matrix A.
        m: The number of rows in the matrix A.
        Bmsg_length: The bit-length of the message.
        P_size: The size of the polynomial.
    """
    return _wrap_op_no_ret(lib.Setup, lambda_str, n, m, Bmsg_length, P_size)

def run_benchmark_OKDM(n: int, m: int, q_len: str, iterations: int):
    """
    Benchmark NTL OKDM for given parameters.
    
    Parameters:
        n: The dimension of the secret vector, and the number of columns in the matrix A.
        m: The number of rows in the matrix A.
        q_len: The bit-length of the modulus q.
        iterations: The number of times to repeat the OKDM for benchmarking.
    """
    return _wrap_op_no_ret(lib.run_benchmark_OKDM, n, m, q_len, iterations)

def run_benchmark_DDEC(n: int, m: int, q_len: str, p_len: str, iterations: int):
    """
    Benchmark NTL DDEC for given parameters.
    
    Parameters:
        n: The dimension of the secret vector, and the number of columns in the matrix A.
        m: The number of rows in the matrix A.
        q_len: The bit-length of the modulus q.
        p_len: The bit-length of the modulus p.
        iterations: The number of times to repeat the DDEC for benchmarking.
    """
    return _wrap_op_no_ret(lib.run_benchmark_DDEC, n, m, q_len, p_len, iterations)

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

def run_benchmark_last_mul(n: int, m: int, q_len: str, p_len: str, iterations: int):
    """
    Benchmark the last multiplication step in the protocol for given parameters.
    
    Parameters:
        n: The dimension of the secret vector, and the number of columns in the matrix A.
        m: The number of rows in the matrix A.
        q_len: The bit-length of the modulus q.
        p_len: The bit-length of the modulus p.
        iterations: The number of times to repeat the last multiplication for benchmarking.
    """
    return _wrap_op_no_ret(lib.run_benchmark_last_mul, n, m, q_len, p_len, iterations)

def run_benchmark_last_mem_add(b: int, n: int, q_len: str, p_len: str, iterations: int):
    """
    Benchmark the last memory addition step in the protocol for given parameters.
    
    Parameters:
        b: Identifier of the player (0 or 1).
        n: The dimension of the secret vector, and the number of columns in the matrix A.
        q_len: The bit-length of the modulus q.
        p_len: The bit-length of the modulus p.
        iterations: The number of times to repeat the last memory addition for benchmarking.
    """
    return _wrap_op_no_ret(lib.run_benchmark_last_mem_add, b, n, q_len, p_len, iterations)