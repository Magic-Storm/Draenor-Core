////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _AUTH_SARC4_H
#define _AUTH_SARC4_H

#include "Define.h"
#include <openssl/evp.h>
#include <array>

class ARC4
{
    public:
        ARC4(uint32 len);
        ARC4(uint8* seed, uint32 len);
        ~ARC4();
        void Init(uint8* seed);
        void UpdateData(int len, uint8* data);
    private:
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        EVP_CIPHER* _cipher;
#endif
        EVP_CIPHER_CTX* _ctx;
};

#endif
