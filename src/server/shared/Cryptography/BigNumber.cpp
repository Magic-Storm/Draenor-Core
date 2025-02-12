////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include <ace/Guard_T.h>

#include "Cryptography/BigNumber.h"
#include <openssl/bn.h>
#include <openssl/crypto.h>
#include <algorithm>

BigNumber::BigNumber()
    : _bn(BN_new())
{ }

BigNumber::BigNumber(const BigNumber& bn)
    : _bn(BN_dup(bn._bn))
{ }

BigNumber::BigNumber(uint32 val)
    : _bn(BN_new())
{
    BN_set_word(_bn, val);
}

BigNumber::~BigNumber()
{
    BN_free(_bn);
}

void BigNumber::SetDword(uint32 val)
{
    BN_set_word(_bn, val);
}

void BigNumber::SetQword(uint64 val)
{
    BN_set_word(_bn, (uint32)(val >> 32));
    BN_lshift(_bn, _bn, 32);
    BN_add_word(_bn, (uint32)(val & 0xFFFFFFFF));
}

void BigNumber::SetBinary(const uint8* bytes, int len)
{
    uint8* array = new uint8[len];

    for (int i = 0; i < len; i++)
        array[i] = bytes[len - 1 - i];

    BN_bin2bn(array, len, _bn);

    delete[] array;
}

void BigNumber::SetHexStr(const char* str)
{
    BN_hex2bn(&_bn, str);
}

void BigNumber::SetRand(int numbits)
{
    BN_rand(_bn, numbits, 0, 1);
}

BigNumber& BigNumber::operator=(const BigNumber& bn)
{
    if (this == &bn)
        return *this;
    BN_copy(_bn, bn._bn);
    return *this;
}

BigNumber BigNumber::operator+=(const BigNumber& bn)
{
    BN_add(_bn, _bn, bn._bn);
    return *this;
}

BigNumber BigNumber::operator-=(const BigNumber& bn)
{
    BN_sub(_bn, _bn, bn._bn);
    return *this;
}

BigNumber BigNumber::operator*=(const BigNumber& bn)
{
    BN_CTX *bnctx;

    bnctx = BN_CTX_new();
    BN_mul(_bn, _bn, bn._bn, bnctx);
    BN_CTX_free(bnctx);

    return *this;
}

BigNumber BigNumber::operator/=(const BigNumber& bn)
{
    BN_CTX *bnctx;

    bnctx = BN_CTX_new();
    BN_div(_bn, NULL, _bn, bn._bn, bnctx);
    BN_CTX_free(bnctx);

    return *this;
}

BigNumber BigNumber::operator%=(const BigNumber& bn)
{
    BN_CTX *bnctx;

    bnctx = BN_CTX_new();
    BN_mod(_bn, _bn, bn._bn, bnctx);
    BN_CTX_free(bnctx);

    return *this;
}

BigNumber BigNumber::Exp(const BigNumber& bn)
{
    BigNumber ret;
    BN_CTX *bnctx;

    bnctx = BN_CTX_new();
    BN_exp(ret._bn, _bn, bn._bn, bnctx);
    BN_CTX_free(bnctx);

    return ret;
}

BigNumber BigNumber::ModExp(const BigNumber& bn1, const BigNumber& bn2)
{
    BigNumber ret;
    BN_CTX *bnctx;

    bnctx = BN_CTX_new();
    BN_mod_exp(ret._bn, _bn, bn1._bn, bn2._bn, bnctx);
    BN_CTX_free(bnctx);

    return ret;
}

int BigNumber::GetNumBytes(void)
{
    return BN_num_bytes(_bn);
}

uint32 BigNumber::AsDword()
{
    return (uint32)BN_get_word(_bn);
}

bool BigNumber::isZero() const
{
    return BN_is_zero(_bn);
}


uint8* BigNumber::AsByteArray(int32 minSize, bool littleEndian)
{
    int length = (minSize >= GetNumBytes()) ? minSize : GetNumBytes();

    uint8* array = new uint8[length];

    // If we need more bytes than length of BigNumber set the rest to 0
    if (length > GetNumBytes())
        memset((void*)array, 0, length);

    int paddingOffset = length - GetNumBytes();

    BN_bn2bin(_bn, (unsigned char *)array + paddingOffset);

    // openssl's BN stores data internally in big endian format, reverse if little endian desired
    if (littleEndian)
        std::reverse(array, array + length);

    uint8* ret(array);
    return ret;
}

char * BigNumber::AsHexStr() const
{
    return BN_bn2hex(_bn);
}

char * BigNumber::AsDecStr() const
{
    return BN_bn2dec(_bn);
}

