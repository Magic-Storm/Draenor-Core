/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * Ported from dio85/TCWoD d740184 (OpenSSL 3 STORE-based cert load), adapted for Draenor-Core.
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
#include "Config.h"
#include "Log.h"
#include <boost/asio/error.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/store.h>
#include <openssl/ui.h>
#include <openssl/x509.h>

bool Battlenet::SslContext::_usesDevWildcardCertificate = false;

namespace
{
    struct UiMethodDeleter
    {
        void operator()(UI_METHOD* m) const { if (m) UI_destroy_method(m); }
    };

    struct StoreDeleter
    {
        void operator()(OSSL_STORE_CTX* s) const { if (s) OSSL_STORE_close(s); }
    };

    std::unique_ptr<UI_METHOD, UiMethodDeleter> CreatePasswordUiMethodFromPemCallback(::pem_password_cb* callback)
    {
        return std::unique_ptr<UI_METHOD, UiMethodDeleter>(UI_UTIL_wrap_read_pem_callback(callback, 0));
    }

    boost::system::error_code GetLastOpenSSLError()
    {
        unsigned long ossl_error = ::ERR_get_error();
        if (ERR_SYSTEM_ERROR(ossl_error))
            return boost::system::error_code(static_cast<int>(::ERR_GET_REASON(ossl_error)), boost::asio::error::get_system_category());

        return boost::system::error_code(static_cast<int>(ossl_error), boost::asio::error::get_ssl_category());
    }

    std::unique_ptr<OSSL_STORE_CTX, StoreDeleter> OpenOpenSSLStore(boost::filesystem::path const& storePath,
        UI_METHOD const* passwordCallback, void* passwordCallbackData)
    {
        std::string uri;
        uri.reserve(6 + storePath.size());

        uri += "file:";
        std::string genericPath = storePath.generic_string();
        if (!genericPath.empty() && genericPath[0] != '/')
            uri += '/';

        uri += genericPath;

        TC_LOG_INFO("server.ssl", "Opening OpenSSL store with URI: %s", uri.c_str());

        std::unique_ptr<OSSL_STORE_CTX, StoreDeleter> store(
            OSSL_STORE_open(uri.c_str(), passwordCallback, passwordCallbackData, nullptr, nullptr));
        if (!store)
        {
            boost::system::error_code err = GetLastOpenSSLError();
            TC_LOG_ERROR("server.ssl", "OSSL_STORE_open failed: %s", err.message().c_str());
        }

        return store;
    }
}

bool Battlenet::SslContext::Initialize()
{
    boost::system::error_code err;
#define LOAD_CHECK(fn) do { fn; \
    if (err) \
    { \
        TC_LOG_ERROR("server.ssl", #fn " failed: %s", err.message().c_str()); \
        return false; \
    } } while (0)

    // WoW 6.2.4 Battle.net client speaks TLS 1.0/1.2 with old ciphers.
    LOAD_CHECK(instance().set_options(
        boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2
        | boost::asio::ssl::context::no_sslv3
        | boost::asio::ssl::context::single_dh_use, err));

    SSL_CTX* nativeContext = instance().native_handle();
#if defined(SSL_OP_NO_TLSv1_3)
    SSL_CTX_set_options(nativeContext, SSL_OP_NO_TLSv1_3);
#endif
#if defined(TLS1_VERSION)
    SSL_CTX_set_min_proto_version(nativeContext, TLS1_VERSION);
#endif
#if defined(TLS1_2_VERSION)
    SSL_CTX_set_max_proto_version(nativeContext, TLS1_2_VERSION);
#endif
    SSL_CTX_set_security_level(nativeContext, 0);
    SSL_CTX_set_cipher_list(nativeContext, "ALL:@SECLEVEL=0");

    std::string certificateChainFile = sConfigMgr->GetStringDefault("CertificatesFile", "./bnetserver.cert.pem");

    auto passwordCallback = [](std::size_t /*max_length*/, boost::asio::ssl::context::password_purpose /*purpose*/) -> std::string
    {
        return sConfigMgr->GetStringDefault("PrivateKeyPassword", "");
    };

    LOAD_CHECK(instance().set_password_callback(passwordCallback, err));

    std::unique_ptr<UI_METHOD, UiMethodDeleter> password_ui_method =
        CreatePasswordUiMethodFromPemCallback(SSL_CTX_get_default_passwd_cb(nativeContext));

    std::unique_ptr<OSSL_STORE_CTX, StoreDeleter> store = OpenOpenSSLStore(boost::filesystem::absolute(certificateChainFile),
        password_ui_method.get(), SSL_CTX_get_default_passwd_cb_userdata(nativeContext));

    if (!store)
    {
        err = GetLastOpenSSLError();
        TC_LOG_ERROR("server.ssl", "OSSL_STORE_open failed: %s", err.message().c_str());
        return false;
    }

    EVP_PKEY* key = nullptr;
    STACK_OF(X509)* certs = sk_X509_new_null();
    while (!OSSL_STORE_eof(store.get()))
    {
        OSSL_STORE_INFO* info = OSSL_STORE_load(store.get());
        if (!info)
            continue;

        switch (OSSL_STORE_INFO_get_type(info))
        {
            case OSSL_STORE_INFO_PKEY:
                if (key)
                    EVP_PKEY_free(key);
                key = OSSL_STORE_INFO_get1_PKEY(info);
                break;
            case OSSL_STORE_INFO_CERT:
                sk_X509_push(certs, OSSL_STORE_INFO_get1_CERT(info));
                break;
            default:
                break;
        }

        OSSL_STORE_INFO_free(info);
    }

    if (sk_X509_num(certs) > 0)
    {
        X509* cert = sk_X509_shift(certs);

        _usesDevWildcardCertificate = false;
        X509_NAME const* nm = X509_get_subject_name(cert);
        int lastpos = -1;
        while (true)
        {
            lastpos = X509_NAME_get_index_by_NID(nm, NID_commonName, lastpos);
            if (lastpos == -1)
                break;

            X509_NAME_ENTRY const* e = X509_NAME_get_entry(nm, lastpos);
            if (!e)
                continue;

            ASN1_STRING const* text = X509_NAME_ENTRY_get_data(e);
            if (!text)
                continue;

            unsigned char* utf8TextRaw = nullptr;
            int utf8Length = ASN1_STRING_to_UTF8(&utf8TextRaw, text);
            if (utf8Length >= 0)
            {
                std::string cn(reinterpret_cast<char const*>(utf8TextRaw), static_cast<size_t>(utf8Length));
                OPENSSL_free(utf8TextRaw);
                if (cn == "*.*")
                {
                    _usesDevWildcardCertificate = true;
                    break;
                }
            }
        }

        SSL_CTX_use_cert_and_key(nativeContext, cert, key, certs, 1);
        X509_free(cert);
    }

    sk_X509_free(certs);

    if (!key)
    {
        std::string privateKeyFile = sConfigMgr->GetStringDefault("PrivateKeyFile", "./bnetserver.key.pem");
        LOAD_CHECK(instance().use_private_key_file(privateKeyFile, boost::asio::ssl::context::pem, err));
    }
    else
        EVP_PKEY_free(key);

#undef LOAD_CHECK

    if (_usesDevWildcardCertificate)
        TC_LOG_INFO("server.ssl", "Loaded development wildcard (CN=*.*) certificate chain from %s", certificateChainFile.c_str());

    return true;
}

boost::asio::ssl::context& Battlenet::SslContext::instance()
{
    static boost::asio::ssl::context context(boost::asio::ssl::context::tls);
    return context;
}
