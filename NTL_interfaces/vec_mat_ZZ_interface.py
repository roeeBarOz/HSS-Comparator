from cffi import FFI

ffi = FFI()

ffi.cdef("""
    void free_vec_mat_string(char* str);

    // Vector
    char* vec_zz_add(const char* a_str, const char* b_str);
    char* vec_zz_sub(const char* a_str, const char* b_str);
    char* vec_zz_mul_scalar(const char* vec_str, const char* scalar_str);
    char* vec_zz_inner_product(const char* a_str, const char* b_str);
    char* vec_zz_p_random(long length);
    char* vec_zz_get(const char* vec_str, long index);
    char* vec_zz_gaussian(long length, long k);
    char* vec_zz_prepend_one(const char* vec_str);
    char* vec_zz_create_e1(const char* val_str, long length);
    char* vec_zz_random_binary(long length);
    char* vec_add_scalar(const char* vec_str, const char* scalar_str);

    // Matrix
    char* mat_zz_add(const char* A_str, const char* B_str);
    char* mat_zz_sub(const char* A_str, const char* B_str);
    char* mat_zz_mul(const char* A_str, const char* B_str);
    char* mat_zz_mul_vec(const char* A_str, const char* v_str);
    char* mat_zz_mul_scalar(const char* A_str, const char* x_str);
    char* mat_zz_transpose(const char* A_str);
    char* mat_zz_inv(const char* A_str);
    char* mat_zz_determinant(const char* A_str);
    char* mat_zz_p_random(long rows, long cols);
    char* mat_zz_get_row(const char* matrix_str, long row_idx);
    char* mat_zz_negate(const char* matrix_str);
    char* mat_zz_concat_col_first(const char* col_vec_str, const char* matrix_str);
    char* mat_add_scalar(const char* A_str, const char* scalar_str);
    
    // HSS-specific Operations
    char* DDEC(const char* s, const char* C, const char* p, const char* q);
    char* OKDM(const char* x, const char* C, const char* p, const char* q);
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

# --- Python API ---

def vec_add(a_str: str, b_str: str) -> str:
    return _wrap_op(lib.vec_zz_add, a_str, b_str)

def vec_sub(a_str: str, b_str: str) -> str:
    return _wrap_op(lib.vec_zz_sub, a_str, b_str)

def vec_mul_scalar(vec_str: str, scalar_str: str) -> str:
    return _wrap_op(lib.vec_zz_mul_scalar, vec_str, scalar_str)

def vec_inner_product(a_str: str, b_str: str) -> str:
    return _wrap_op(lib.vec_zz_inner_product, a_str, b_str)

def vec_random(length: int) -> str:
    # Length is a long, pass directly
    res_c = lib.vec_zz_p_random(length)
    res_str = ffi.string(res_c).decode()
    lib.free_vec_mat_string(res_c)
    return res_str

def vec_get(vec_str: str, index: int) -> str:
    return _wrap_op(lib.vec_zz_get, vec_str, index)

def vec_gaussian(length: int, k: int = 2) -> str:
    """
    Generates an error vector using Centered Binomial Distribution.
    
    Args:
        length: Size of the vector.
        k: Distribution width (default 2). 
           Higher k = higher standard deviation = more noise.
    """
    res_c = lib.vec_zz_gaussian(length, k)
    res_str = ffi.string(res_c).decode()
    lib.free_vec_mat_string(res_c)
    return res_str

def vec_prepend_one(vec_str: str) -> str:
    """Creates (1, v) from v."""
    return _wrap_op(lib.vec_zz_prepend_one, vec_str)

def vec_create_e(val_str: str, length: int, k: int) -> str:
    """
    Creates a vector of 'length' where index k is 'val' and the rest are 0.
    Result: [0, ..., val, ..., 0] where val is at index k.
    """
    return _wrap_op(lib.vec_zz_create_e, val_str, length, k)

def vec_random_binary(length: int) -> str:
    """
    Generates a random vector with elements in {0, 1}.
    """
    res_c = lib.vec_zz_random_binary(length)
    res_str = ffi.string(res_c).decode()
    lib.free_vec_mat_string(res_c)
    return res_str

def vec_add_scalar(vec_str: str, scalar_str: str) -> str:
    """Computes v + scalar."""
    return _wrap_op(lib.vec_add_scalar, vec_str, scalar_str)

def mat_add(A_str: str, B_str: str) -> str:
    return _wrap_op(lib.mat_zz_add, A_str, B_str)

def mat_sub(A_str: str, B_str: str) -> str:
    return _wrap_op(lib.mat_zz_sub, A_str, B_str)

def mat_mul(A_str: str, B_str: str) -> str:
    return _wrap_op(lib.mat_zz_mul, A_str, B_str)

def mat_mul_vec(A_str: str, v_str: str) -> str:
    return _wrap_op(lib.mat_zz_mul_vec, A_str, v_str)

def mat_mul_scalar(A_str: str, x_str: str) -> str:
    return _wrap_op(lib.mat_zz_mul_scalar, A_str, x_str)

def mat_transpose(A_str: str) -> str:
    return _wrap_op(lib.mat_zz_transpose, A_str)

def mat_inv(A_str: str) -> str:
    return _wrap_op(lib.mat_zz_inv, A_str)

def mat_det(A_str: str) -> str:
    return _wrap_op(lib.mat_zz_determinant, A_str)

def mat_random(rows: int, cols: int) -> str:
    res_c = lib.mat_zz_p_random(rows, cols)
    res_str = ffi.string(res_c).decode()
    lib.free_vec_mat_string(res_c)
    return res_str

def mat_get_row(matrix_str: str, row_idx: int) -> str:
    return _wrap_op(lib.mat_zz_get_row, matrix_str, row_idx)

def mat_neg(matrix_str: str) -> str:
    """Computes -A."""
    return _wrap_op(lib.mat_zz_negate, matrix_str)

def mat_concat_col_first(col_vec_str: str, matrix_str: str) -> str:
    """
    Concatenates vector b to the LEFT of matrix A.
    Result = [b | A]
    """
    return _wrap_op(lib.mat_zz_concat_col_first, col_vec_str, matrix_str)

def mat_add_scalar(A_str: str, scalar_str: str) -> str:
    """Computes A + scalar."""
    return _wrap_op(lib.mat_add_scalar, A_str, scalar_str)

def DDEC(s_vec: str, C: str, p: str, q: str) -> str:
    """
    Computes the Distributed Decryption operation.
    """
    return _wrap_op(lib.DDEC, s_vec, C, p, q)

def OKDM(x_vec: str, C: str, p: str, q: str) -> str:
    """
    Computes the Oracle Key-Dependent Message operation.
    """
    return _wrap_op(lib.OKDM, x_vec, C, p, q)
    