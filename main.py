import NTL_interfaces.ZZ_interface as zz

# 617-digit numbers (~2000 bits)
a = "1234567890123456789012345678901234567890123456789012345678901234567890"
b = "9876543210987654321098765432109876543210987654321098765432109876543210"

# Example usage of the ZZ_interface functions
print("Addition Result:", zz.zz_add(a, b))
print("Subtraction Result:", zz.zz_sub(a, b))
print("Multiplication Result:", zz.zz_mul(a, b))
print("Division Result:", zz.zz_div(b, a))
print("Modulus Result:", zz.zz_mod(b, a))
print("Power Result:", zz.zz_pow(a, "2"))
print("GCD Result:", zz.zz_gcd(a, b))
print("Negation Result:", zz.zz_neg(a))
# print("Inverse Result:", zz.zz_inv("123456789", b))
print("Random Result:", zz.zz_random(100))  # Random ZZ number with 100 bits
print("Random Prime Result:", zz.zz_random_prime(100, 7))  # Random prime ZZ number with 100 bits
print("Next Prime Result:", zz.zz_next_prime(a))  # Next prime after a
print("Equals Result:", zz.zz_equals(a, b))  # Check if a equals b
print("Less Than Result:", zz.zz_less_than(a, b))  # Check if a is less than b