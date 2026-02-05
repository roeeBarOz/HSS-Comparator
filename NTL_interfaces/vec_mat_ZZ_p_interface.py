from cffi import FFI

ffi = FFI()

ffi.cdef("""
    void free_vec_mat_string(char* str);

    // Vector
    char* vec_zz_p_add(const char* a_str, const char* b_str);
    char* vec_zz_p_sub(const char* a_str, const char* b_str);
    char* vec_zz_p_mul_scalar(const char* vec_str, const char* scalar_str);
    char* vec_zz_p_inner_product(const char* a_str, const char* b_str);
    char* vec_zz_p_random(long length);
    char* vec_zz_p_get(const char* vec_str, long index);

    // Matrix
    char* mat_zz_p_add(const char* A_str, const char* B_str);
    char* mat_zz_p_sub(const char* A_str, const char* B_str);
    char* mat_zz_p_mul(const char* A_str, const char* B_str);
    char* mat_zz_p_mul_vec(const char* A_str, const char* v_str);
    char* mat_zz_p_mul_scalar(const char* A_str, const char* x_str);
    char* mat_zz_p_transpose(const char* A_str);
    char* mat_zz_p_inv(const char* A_str);
    char* mat_zz_p_determinant(const char* A_str);
    char* mat_zz_p_random(long rows, long cols);
    char* mat_zz_p_get_row(const char* matrix_str, long row_idx);
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
    return _wrap_op(lib.vec_zz_p_add, a_str, b_str)

def vec_sub(a_str: str, b_str: str) -> str:
    return _wrap_op(lib.vec_zz_p_sub, a_str, b_str)

def vec_mul_scalar(vec_str: str, scalar_str: str) -> str:
    return _wrap_op(lib.vec_zz_p_mul_scalar, vec_str, scalar_str)

def vec_inner_product(a_str: str, b_str: str) -> str:
    return _wrap_op(lib.vec_zz_p_inner_product, a_str, b_str)

def vec_random(length: int) -> str:
    # Length is a long, pass directly
    res_c = lib.vec_zz_p_random(length)
    res_str = ffi.string(res_c).decode()
    lib.free_vec_mat_string(res_c)
    return res_str

def vec_get(vec_str: str, index: int) -> str:
    return _wrap_op(lib.vec_zz_p_get, vec_str, index)

def mat_add(A_str: str, B_str: str) -> str:
    return _wrap_op(lib.mat_zz_p_add, A_str, B_str)

def mat_sub(A_str: str, B_str: str) -> str:
    return _wrap_op(lib.mat_zz_p_sub, A_str, B_str)

def mat_mul(A_str: str, B_str: str) -> str:
    return _wrap_op(lib.mat_zz_p_mul, A_str, B_str)

def mat_mul_vec(A_str: str, v_str: str) -> str:
    return _wrap_op(lib.mat_zz_p_mul_vec, A_str, v_str)

def mat_mul_scalar(A_str: str, x_str: str) -> str:
    return _wrap_op(lib.mat_zz_p_mul_scalar, A_str, x_str)

def mat_transpose(A_str: str) -> str:
    return _wrap_op(lib.mat_zz_p_transpose, A_str)

def mat_inv(A_str: str) -> str:
    return _wrap_op(lib.mat_zz_p_inv, A_str)

def mat_det(A_str: str) -> str:
    return _wrap_op(lib.mat_zz_p_determinant, A_str)

def mat_random(rows: int, cols: int) -> str:
    res_c = lib.mat_zz_p_random(rows, cols)
    res_str = ffi.string(res_c).decode()
    lib.free_vec_mat_string(res_c)
    return res_str

def mat_get_row(matrix_str: str, row_idx: int) -> str:
    return _wrap_op(lib.mat_zz_p_get_row, matrix_str, row_idx)