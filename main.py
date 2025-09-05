# import Measurements.Measure_paillier_hss as measure
import Encryptions.Paillier as paillier
import NTL_interfaces.ZZ_interface as zz
# measure.measure(
#     sizes=[512, 1024, 2048],
#     times=10,
#     message="123456789123456789123456789"
# )
message = zz.zz_random(1000)
pk, sk = paillier.Gen("1" * 2048)
cipher = paillier.Enc(message, pk)
dec_message = paillier.Dec(pk, cipher, sk)
print("Original message: ", message)
print("Decrypted message:", dec_message)
assert message == dec_message
