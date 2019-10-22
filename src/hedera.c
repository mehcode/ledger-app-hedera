#include "hedera.h"
#include "debug.h"

static uint8_t seed[32];
static cx_ecfp_private_key_t pk;
static uint32_t path[5];

void hedera_derive_keypair(
    uint32_t index,
    /* out */ cx_ecfp_private_key_t* secret, 
    /* out */ cx_ecfp_public_key_t* public
) {
    path[0] = 44 | 0x80000000;
    path[1] = 3030 | 0x80000000;
    path[2] = index | 0x80000000;
    path[3] = 0x80000000;
    path[4] = 0x80000000;

    os_perso_derive_node_bip32_seed_key(HDW_ED25519_SLIP10, CX_CURVE_Ed25519, path, 5, seed, NULL, NULL, 0);

    cx_ecfp_init_private_key(CX_CURVE_Ed25519, seed, sizeof(seed), &pk);

    if (public) {
        cx_ecfp_init_public_key(CX_CURVE_Ed25519, NULL, 0, public);
        cx_ecfp_generate_pair(CX_CURVE_Ed25519, public, &pk, 1);
    }

    if (secret) {
        *secret = pk;
    }

    os_memset(seed, 0, sizeof(seed));
    os_memset(&pk, 0, sizeof(pk));
}

uint16_t hedera_sign(
    uint32_t index,
    const uint8_t* tx,
    uint8_t tx_len,
    /* out */ uint8_t* result
) {
    // Get Keys
    cx_ecfp_private_key_t private_key;
    hedera_derive_keypair(index, &private_key, NULL);

    // Sign Transaction
    // <cx.h> 2283
    // Claims to want Hashes, but other apps use the message itself
    // and complain that the documentation is wrong
    cx_eddsa_sign(
        &private_key,                    // private key
        0,                               // mode (UNSUPPORTED)
        CX_SHA512,                       // hashID
        tx,                              // hash (really message)
        tx_len,                          // hash length (really message length)
        NULL,                            // context (UNUSED)
        0,                               // context length (0)
        result,                          // signature
        64,                              // signature length
        NULL                             // info
    );

    // Clear private key
    os_memset(&private_key, 0, sizeof(private_key));

    return 64;
}
