////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "HMACSHA1.h"
#include "BigNumber.h"
#include "Common.h"
#include "BigNumber.h"
#include "Errors.h"

#if defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER < 0x10100000L
HMAC_CTX* HMAC_CTX_new()
{
    HMAC_CTX* ctx = new HMAC_CTX();
    HMAC_CTX_init(ctx);
    return ctx;
}

void HMAC_CTX_free(HMAC_CTX* ctx)
{
    HMAC_CTX_cleanup(ctx);
    delete ctx;
}
#endif

HmacHash::HmacHash(uint32 len, uint8 *seed) : ctx(HMAC_CTX_new())
{
    ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, seed, len, EVP_sha1(), nullptr);
}

HmacHash::~HmacHash()
{
    HMAC_CTX_free(ctx);
}

void HmacHash::UpdateData(const std::string& str)
{
    HMAC_Update(ctx, (uint8 const*)str.c_str(), str.length());
}

void HmacHash::UpdateData(const uint8* data, size_t len)
{
    HMAC_Update(ctx, data, len);
}

void HmacHash::Finalize()
{
    uint32 length = 0;
    HMAC_Final(ctx, (uint8*)m_digest, &length);
    ASSERT(length == SHA_DIGEST_LENGTH);
}

uint8* HmacHash::ComputeHash(BigNumber* bn)
{
    HMAC_Update(ctx, bn->AsByteArray(), bn->GetNumBytes());
    Finalize();
    return (uint8*)m_digest;
}

//////////////////////////////////////////////////////////////////////////

HmacHash256::HmacHash256(uint32 len, uint8 *seed)
{
    ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, seed, len, EVP_sha256(), nullptr);
}

HmacHash256::~HmacHash256()
{
    HMAC_CTX_free(ctx);
}

void HmacHash256::UpdateData(const std::string &str)
{
    HMAC_Update(ctx, (uint8 const*)str.c_str(), str.length());
}

void HmacHash256::UpdateData(const uint8* data, size_t len)
{
    HMAC_Update(ctx, data, len);
}

void HmacHash256::Finalize()
{
    uint32 length = 0;
    HMAC_Final(ctx, (uint8*)m_digest, &length);
    ASSERT(length == SHA256_DIGEST_LENGTH);
}

uint8 *HmacHash256::ComputeHash(BigNumber* bn)
{
    HMAC_Update(ctx, bn->AsByteArray(), bn->GetNumBytes());
    Finalize();
    return (uint8*)m_digest;
}

uint8 *HmacHash256::ComputeHash(uint8 *seed, uint32 len)
{
    HMAC_Update(ctx, seed, len);
    Finalize();
    return (uint8*)m_digest;
}