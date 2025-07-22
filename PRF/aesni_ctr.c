// Compile with: gcc -Wall -O3 -maes -mssse3 -shared -fPIC -o aesni_ctr.so aesni_ctr.c

#include <stdint.h>
#include <string.h>
#include <wmmintrin.h>

// Encrypt a single block with AES-NI
void aesni_encrypt_block(const uint8_t *key, const uint8_t *input, uint8_t *output) {
    __m128i m = _mm_loadu_si128((__m128i*)input);
    __m128i k = _mm_loadu_si128((__m128i*)key);

    m = _mm_xor_si128(m, k);
    for (int i = 0; i < 9; i++) {
        m = _mm_aesenc_si128(m, k); // fake key schedule for simplicity
    }
    m = _mm_aesenclast_si128(m, k);

    _mm_storeu_si128((__m128i*)output, m);
}

// Encrypt using AES-NI in CTR mode (for multiple blocks)
void aesni_ctr_encrypt(const uint8_t *key, const uint8_t *nonce,
                       const uint8_t *plaintext, uint8_t *ciphertext,
                       uint64_t length) {
    uint8_t counter_block[16];
    uint8_t keystream_block[16];
    uint64_t counter = 0;

    for (uint64_t i = 0; i < length; i += 16) {
        memcpy(counter_block, nonce, 8);
        memcpy(counter_block + 8, &counter, 8);

        aesni_encrypt_block(key, counter_block, keystream_block);

        for (int j = 0; j < 16 && i + j < length; j++) {
            ciphertext[i + j] = plaintext[i + j] ^ keystream_block[j];
        }

        counter++;
    }
}
