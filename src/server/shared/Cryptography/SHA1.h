////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _AUTH_SHA1_H
#define _AUTH_SHA1_H

#include "Define.h"
#include <string>
#include <openssl/sha.h>
#include <openssl/evp.h>

class BigNumber;

class SHA1Hash
{
public:
    SHA1Hash();
    SHA1Hash(SHA1Hash const& other);     // copy
    SHA1Hash(SHA1Hash&& other);          // move
    SHA1Hash& operator=(SHA1Hash other); // assign
    ~SHA1Hash();

    void Swap(SHA1Hash& other) throw();
    friend void Swap(SHA1Hash& left, SHA1Hash& right) { left.Swap(right); }
    void UpdateBigNumbers(BigNumber* bn0, ...);

    void UpdateData(const uint8* dta, int len);
    void UpdateData(const std::string& str);

    void Initialize();
    void Finalize();

    uint8* GetDigest(void) { return m_digest; }
    int GetLength() const { return SHA_DIGEST_LENGTH; }

private:
    EVP_MD_CTX* m_ctx;
    uint8 m_digest[SHA_DIGEST_LENGTH];
};
#endif
