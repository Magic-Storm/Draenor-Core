////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "SHA1.h"
#include "BigNumber.h"
#include <stdarg.h>

SHA1Hash::SHA1Hash()
{
    m_ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(m_ctx, EVP_sha1(), nullptr);
}
SHA1Hash::SHA1Hash(const SHA1Hash& other) : SHA1Hash() // copy
{
    EVP_MD_CTX_copy_ex(m_ctx, other.m_ctx);
    std::memcpy(m_digest, other.m_digest, SHA_DIGEST_LENGTH);
}
SHA1Hash::SHA1Hash(SHA1Hash&& other) : SHA1Hash() // move
{
    Swap(other);
}
SHA1Hash& SHA1Hash::operator=(SHA1Hash other) // assign
{
    Swap(other);
    return *this;
}

SHA1Hash::~SHA1Hash()
{
    EVP_MD_CTX_free(m_ctx);
}
void SHA1Hash::Swap(SHA1Hash& other) throw()
{
    std::swap(m_ctx, other.m_ctx);
    std::swap(m_digest, other.m_digest);
}

void SHA1Hash::UpdateData(const uint8* dta, int len)
{
    EVP_DigestUpdate(m_ctx, dta, len);
}

void SHA1Hash::UpdateData(const std::string& str)
{
    UpdateData((uint8 const*)str.c_str(), str.length());
}

void SHA1Hash::UpdateBigNumbers(BigNumber* bn0, ...)
{
    va_list v;
    BigNumber* bn;

    va_start(v, bn0);
    bn = bn0;
    while (bn)
    {
        UpdateData(bn->AsByteArray(), bn->GetNumBytes());
        bn = va_arg(v, BigNumber*);
    }
    va_end(v);
}

void SHA1Hash::Initialize()
{
    EVP_DigestInit(m_ctx, EVP_sha1());
}

void SHA1Hash::Finalize(void)
{
    uint32 length = SHA_DIGEST_LENGTH;
    EVP_DigestFinal_ex(m_ctx, m_digest, &length);
}
