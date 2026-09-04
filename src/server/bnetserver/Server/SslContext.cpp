/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "SslContext.h"
#include "Log.h"
#include "Config.h"
#include <openssl/ssl.h>

bool Battlenet::SslContext::Initialize()
{
    boost::system::error_code err;

#define LOAD_CHECK(fn) do { fn; \
    if (err) \
    { \
        TC_LOG_ERROR("server.ssl", #fn " failed: %s", err.message().c_str()); \
        return false; \
    } } while (0)

    std::string certificateChainFile = sConfigMgr->GetStringDefault("CertificatesFile", "./bnetserver.cert.pem");
    std::string privateKeyFile = sConfigMgr->GetStringDefault("PrivateKeyFile", "./bnetserver.key.pem");

    // WoW 6.2.4 Battle.net client speaks TLS 1.0/1.2 with old ciphers.
    // OpenSSL 3 defaults (TLS 1.3 + SECLEVEL 2) abort the handshake with "stream truncated".
    LOAD_CHECK(instance().set_options(
        boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2
        | boost::asio::ssl::context::no_sslv3
        | boost::asio::ssl::context::single_dh_use, err));

    SSL_CTX* native = instance().native_handle();
#if defined(SSL_OP_NO_TLSv1_3)
    SSL_CTX_set_options(native, SSL_OP_NO_TLSv1_3);
#endif
#if defined(TLS1_VERSION)
    SSL_CTX_set_min_proto_version(native, TLS1_VERSION);
#endif
#if defined(TLS1_2_VERSION)
    SSL_CTX_set_max_proto_version(native, TLS1_2_VERSION);
#endif
    SSL_CTX_set_security_level(native, 0);
    SSL_CTX_set_cipher_list(native, "ALL:@SECLEVEL=0");

    LOAD_CHECK(instance().use_certificate_chain_file(certificateChainFile, err));
    LOAD_CHECK(instance().use_private_key_file(privateKeyFile, boost::asio::ssl::context::pem, err));

#undef LOAD_CHECK

    return true;
}

boost::asio::ssl::context& Battlenet::SslContext::instance()
{
    static boost::asio::ssl::context context(boost::asio::ssl::context::sslv23);
    return context;
}
