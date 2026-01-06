# setup.py
# This script builds the 32-bit AVX2 C++ backend.
# Run: pip install .

from setuptools import setup, Extension
import pybind11
import os

# --- MODIFIED ---
# These are the flags for AVX2.
cpp_args = [
    '-O3',         # Full optimization
    '-std=c++17',  # C++17 standard
    '-fPIC',       # Position Independent Code
    '-mavx',       # AVX
    '-mavx2',      # AVX2
    '-mfma'        # Fused Multiply-Add
]

# Linker flags: We need to link GMP for string conversions
linker_args = [
    '-lgmp'
]

# Common paths for libraries
common_include_paths = [
    pybind11.get_include(),
    '/usr/include',
    '/usr/local/include',
    '/opt/homebrew/include' # For macOS Homebrew
]
common_lib_paths = [
    '/usr/lib',
    '/usr/local/lib',
    '/opt/homebrew/lib'
]

ext_modules = [
    Extension(
        'rns_avx2_backend', # Name of the python module
        
        # Source files
        [
            'bindings.cpp',
            'rns_mont.cpp'  # Our C++ implementation
        ],
        
        include_dirs=common_include_paths,
        library_dirs=common_lib_paths,
        language='c++',
        extra_compile_args=cpp_args,
        extra_link_args=linker_args,
    ),
]

setup(
    name='rns_avx2_backend',
    version='0.3.0', # Version 3.0 (32-bit AVX2)
    description='Python bindings for RNS Montgomery AVX2 (32-bit port)',
    ext_modules=ext_modules,
)

