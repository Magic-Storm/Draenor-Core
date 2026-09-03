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

    LOAD_CHECK(instance().set_options(
        boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2
        | boost::asio::ssl::context::no_sslv3
        | boost::asio::ssl::context::no_tlsv1
        | boost::asio::ssl::context::no_tlsv1_1
        | boost::asio::ssl::context::single_dh_use, err));
#if defined(SSL_OP_NO_TLSv1_3)
    ::SSL_CTX_set_options(instance().native_handle(), SSL_OP_NO_TLSv1_3);
#endif
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
