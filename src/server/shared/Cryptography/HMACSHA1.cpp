////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "HMACSHA1.h"
#include "BigNumber.h"
#include "Errors.h"
#include <cstring>

HmacHash::HmacHash(uint32 len, uint8* seed)
    : _ctx(EVP_MD_CTX_new()), _key(EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, nullptr, seed, static_cast<int>(len)))
{
    EVP_DigestSignInit(_ctx, nullptr, EVP_sha1(), nullptr, _key);
    memset(m_digest, 0, SHA_DIGEST_LENGTH);
}

HmacHash::~HmacHash()
{
    EVP_MD_CTX_free(_ctx);
    EVP_PKEY_free(_key);
}

void HmacHash::UpdateData(std::string const& str)
{
    EVP_DigestSignUpdate(_ctx, reinterpret_cast<uint8 const*>(str.c_str()), str.length());
}

void HmacHash::UpdateData(uint8 const* data, size_t len)
{
    EVP_DigestSignUpdate(_ctx, data, len);
}

void HmacHash::Finalize()
{
    size_t length = SHA_DIGEST_LENGTH;
    EVP_DigestSignFinal(_ctx, m_digest, &length);
    ASSERT(length == SHA_DIGEST_LENGTH);
}

uint8* HmacHash::ComputeHash(BigNumber* bn)
{
    UpdateData(bn->AsByteArray().get(), bn->GetNumBytes());
    Finalize();
    return m_digest;
}

//////////////////////////////////////////////////////////////////////////

HmacHash256::HmacHash256(uint32 len, uint8* seed)
    : _ctx(EVP_MD_CTX_new()), _key(EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, nullptr, seed, static_cast<int>(len)))
{
    EVP_DigestSignInit(_ctx, nullptr, EVP_sha256(), nullptr, _key);
    memset(m_digest, 0, SHA256_DIGEST_LENGTH);
}

HmacHash256::~HmacHash256()
{
    EVP_MD_CTX_free(_ctx);
    EVP_PKEY_free(_key);
}

void HmacHash256::UpdateData(std::string const& str)
{
    EVP_DigestSignUpdate(_ctx, reinterpret_cast<uint8 const*>(str.c_str()), str.length());
}

void HmacHash256::UpdateData(uint8 const* data, size_t len)
{
    EVP_DigestSignUpdate(_ctx, data, len);
}

void HmacHash256::Finalize()
{
    size_t length = SHA256_DIGEST_LENGTH;
    EVP_DigestSignFinal(_ctx, m_digest, &length);
    ASSERT(length == SHA256_DIGEST_LENGTH);
}

uint8* HmacHash256::ComputeHash(BigNumber* bn)
{
    UpdateData(bn->AsByteArray().get(), bn->GetNumBytes());
    Finalize();
    return m_digest;
}

uint8* HmacHash256::ComputeHash(uint8* seed, uint32 len)
{
    UpdateData(seed, len);
    Finalize();
    return m_digest;
}
