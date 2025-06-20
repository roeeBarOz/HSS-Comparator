import Encryptions.Paillier as Paillier

n = "1" * 1024  # Bit length for the primes
pk, sk = Paillier.Gen(n)  # Generate public and secret keys
print("Public Key:", pk)
print("Secret Key:", sk)

m = "4287"  # Example message to encrypt
ciphertext = Paillier.Enc(n, m, pk)  # Encrypt the message
print("Ciphertext:", ciphertext)

decrypted_message = Paillier.Dec(pk, ciphertext, sk)  # Decrypt the message
print("Decrypted Message:", decrypted_message)
assert decrypted_message == m, "Decryption failed!"