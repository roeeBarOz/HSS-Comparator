import Encryptions.Paillier as Paillier
import HSS_schemes.Paillier_HSS as hss

n = "1" * 1024  # Bit length for the primes
m = "4287"  # Example message to encrypt

pk, ek_0, ek_1 = hss.Setup(n)  # Setup HSS scheme with the given bit length
I1, I2 = hss.Input(pk, m)  # Input the message into the HSS scheme
m1 = hss.Load("0", pk, ek_0, I1, "1")  # Load the first input
m2 = hss.Load("1", pk, ek_1, I2, "2")  # Load the second input
a1 = hss.Add_Inputs("0", ek_0, I1, I2, "3")  # Add the two inputs
a2 = hss.Add_Memory_Values("0", ek_1, m1, m2, "3")  # Add the two inputs for the second key
a3 = hss.Mul("0", ek_0, I1, m1, "3")  # Multiply the first input with the memory value
hss.Output("0", ek_0, m1, pk[0][0], "3")  # Output the result of the first input
