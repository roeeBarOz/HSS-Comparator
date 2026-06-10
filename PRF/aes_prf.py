import ctypes
from ctypes import c_uint8, c_uint64, POINTER, cdll
import os

# Load the compiled shared library
lib = cdll.LoadLibrary('/home/roee/HSS-Comparator/aesni_ctr.so')

# Define the function prototype
lib.aesni_ctr_encrypt.argtypes = [
    POINTER(c_uint8), POINTER(c_uint8),
    POINTER(c_uint8), POINTER(c_uint8),
    c_uint64
]

BLOCK_SIZE = 16
KEY_SIZE = 16  # AES-128
NONCE_SIZE = 8

nonce = b'\x01\x02\x03\x04\x05\x06\x07\x08'  # Example nonce, must be 8 bytes

def aesni_ctr_encrypt(key: bytes, nonce: bytes, plaintext: bytes) -> bytes:
    assert len(key) == KEY_SIZE
    assert len(nonce) == NONCE_SIZE

    plaintext_buf = (c_uint8 * len(plaintext)).from_buffer_copy(plaintext)
    ciphertext_buf = (c_uint8 * len(plaintext))()

    lib.aesni_ctr_encrypt(
        (c_uint8 * KEY_SIZE).from_buffer_copy(key),
        (c_uint8 * NONCE_SIZE).from_buffer_copy(nonce),
        plaintext_buf,
        ciphertext_buf,
        c_uint64(len(plaintext))
    )

    return bytes(ciphertext_buf)

def apply(key: bytes, message: bytes) -> bytes:
    """
    Apply AES-CTR encryption to the message using the provided key.
    
    :param key: The AES key (16 bytes for AES-128).
    :param message: The plaintext message to encrypt.
    :return: The encrypted ciphertext.
    """
    output = aesni_ctr_encrypt(key, nonce, message)
    return output
