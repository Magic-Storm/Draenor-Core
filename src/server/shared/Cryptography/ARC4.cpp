////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "ARC4.h"
#include "Errors.h"
#include <openssl/sha.h>

ARC4::ARC4(uint32 len) : _ctx(EVP_CIPHER_CTX_new())
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    _cipher = EVP_CIPHER_fetch(nullptr, "RC4", nullptr);
#else
    EVP_CIPHER const* _cipher = EVP_rc4();
#endif

    EVP_CIPHER_CTX_init(_ctx);
    EVP_EncryptInit_ex(_ctx, EVP_rc4(), nullptr, nullptr, nullptr);
    EVP_CIPHER_CTX_set_key_length(_ctx, len);
}

ARC4::ARC4(uint8* seed, uint32 len) : _ctx(EVP_CIPHER_CTX_new())
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    _cipher = EVP_CIPHER_fetch(nullptr, "RC4", nullptr);
#else
    EVP_CIPHER const* _cipher = EVP_rc4();
#endif
    EVP_CIPHER_CTX_init(_ctx);
    EVP_EncryptInit_ex(_ctx, EVP_rc4(), nullptr, nullptr, nullptr);
    EVP_CIPHER_CTX_set_key_length(_ctx, len);
    EVP_EncryptInit_ex(_ctx, nullptr, nullptr, seed, nullptr);
}

ARC4::~ARC4()
{
    EVP_CIPHER_CTX_free(_ctx);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_CIPHER_free(_cipher);
#endif
}

void ARC4::Init(uint8* seed)
{
    EVP_EncryptInit_ex(_ctx, nullptr, nullptr, seed, nullptr);
}

void ARC4::UpdateData(int len, uint8* data)
{
    int outlen = 0;
    EVP_EncryptUpdate(_ctx, data, &outlen, data, len);
    EVP_EncryptFinal_ex(_ctx, data, &outlen);
}
