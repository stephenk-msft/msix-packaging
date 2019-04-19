//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Exceptions.hpp"

#include <memory>
#include <vector>
#include <iostream>

#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/objects.h>
#include <openssl/evp.h>
#include <openssl/x509v3.h>
#include <openssl/pkcs7.h>
#include <openssl/pkcs12.h>
#include <openssl/pem.h>
#include <openssl/crypto.h>
#include <openssl/rsa.h>

#define ThrowOpenSSLError() PrintOpenSSLErr()

namespace MSIX
{
    struct unique_BIO_deleter {
        void operator()(BIO *b) const { if (b) BIO_free(b); };
    };
    
    struct unique_PKCS7_deleter {
        void operator()(PKCS7 *p) const { if (p) PKCS7_free(p); };
    };

    struct unique_PKCS12_deleter {
        void operator()(PKCS12 *p) const { if (p) PKCS12_free(p); };
    };

    struct unique_X509_deleter {
        void operator()(X509 *x) const { if (x) X509_free(x); };
    };
    
    struct unique_X509_STORE_deleter {
        void operator()(X509_STORE *xs) const { if (xs) X509_STORE_free(xs); };
    };

    struct unique_X509_STORE_CTX_deleter {
        void operator()(X509_STORE_CTX *xsc) const { if (xsc) { X509_STORE_CTX_cleanup(xsc); X509_STORE_CTX_free(xsc); } };
    };

    struct unique_OPENSSL_string_deleter {
        void operator()(char *os) const { if (os) OPENSSL_free(os); };
    };

    struct unique_STACK_X509_deleter {
        void operator()(STACK_OF(X509) *sx) const { if (sx) sk_X509_free(sx); };
    };

    struct unique_EVP_PKEY_deleter {
        void operator()(EVP_PKEY *pkey) const { if (pkey) EVP_PKEY_free(pkey); }
    };

    struct shared_BIO_deleter {
        void operator()(BIO *b) const { if (b) BIO_free(b); };
    };

    using unique_BIO = std::unique_ptr<BIO, unique_BIO_deleter>;
    using unique_PKCS7 = std::unique_ptr<PKCS7, unique_PKCS7_deleter>;
    using unique_PKCS12 = std::unique_ptr<PKCS12, unique_PKCS12_deleter>;
    using unique_X509 = std::unique_ptr<X509, unique_X509_deleter>;
    using unique_X509_STORE = std::unique_ptr<X509_STORE, unique_X509_STORE_deleter>;
    using unique_X509_STORE_CTX = std::unique_ptr<X509_STORE_CTX, unique_X509_STORE_CTX_deleter>;
    using unique_OPENSSL_string = std::unique_ptr<char, unique_OPENSSL_string_deleter>;
    using unique_STACK_X509 = std::unique_ptr<STACK_OF(X509), unique_STACK_X509_deleter>;
    using unique_EVP_PKEY = std::unique_ptr<EVP_PKEY, unique_EVP_PKEY_deleter>;
    
    typedef struct Asn1Sequence
    {
        std::uint8_t tag;
        std::uint8_t encoding;
        union 
        {   
            struct {
                std::uint8_t length;
                std::uint8_t content;
            } rle8;
            struct {
                std::uint8_t lengthHigh;
                std::uint8_t lengthLow;
                std::uint8_t content;
            } rle16;
            std::uint8_t content;
        };
    } Asn1Sequence;

    inline void PrintOpenSSLErr()
    {
        ERR_load_crypto_strings();

        std::cout << "OpenSSL Error:" << std::endl;

        unsigned long err = 0;
        do
        {
            const char* file{};
            int line{};
            const char* data{};
            int flags{};

            err = ERR_get_error_line_data(&file, &line, &data, &flags);

            if (err)
            {
                std::cout << "  at " << file << '[' << line << ']';
                if (flags & ERR_TXT_STRING)
                {
                    std::cout << " : " << data;
                }
                std::cout << std::endl;

                std::cout << "    " << ERR_error_string(err, nullptr) << std::endl;
            }
        } while (err);
    }

    template <typename T>
    inline void CheckOpenSSLAlloc(const T& t)
    {
        if (!t)
        {
            PrintOpenSSLErr();
        }
    }

    inline void CheckOpenSSLErr(int err)
    {
        if (!err)
        {
            PrintOpenSSLErr();
        }
    }

    // Support for our custom OIDs

    enum class CustomOpenSSLObjectName
    {
        spcIndirectDataContext,
        spcSipInfoObjID,
        spcSpOpusInfo,
        spcStatementType,
        individualCodeSigning,
    };

    struct CustomOpenSSLObjects;

    struct CustomOpenSSLObject
    {
        CustomOpenSSLObject(CustomOpenSSLObjectName name_, int nid_) :
            name(name_), nid(nid_) {}

        CustomOpenSSLObject(const CustomOpenSSLObject&) = default;
        CustomOpenSSLObject& operator=(const CustomOpenSSLObject&) = default;

        CustomOpenSSLObject(CustomOpenSSLObject&&) = default;
        CustomOpenSSLObject& operator=(CustomOpenSSLObject&&) = default;

        CustomOpenSSLObjectName GetName() const { return name; }

        int GetNID() const { return nid; }

        ASN1_OBJECT* GetObj() const { return OBJ_nid2obj(nid); }

    private:
        CustomOpenSSLObjectName name;
        int nid = NID_undef;
    };

    // This helper class can only support a single use at a time because OBJ_cleanup will destroy
    // any other simultaneous use.  A shared_ptr singleton model would be best if needed.
    struct CustomOpenSSLObjects
    {
        CustomOpenSSLObjects();
        ~CustomOpenSSLObjects();

        CustomOpenSSLObjects(const CustomOpenSSLObjects&) = delete;
        CustomOpenSSLObjects& operator=(const CustomOpenSSLObjects&) = delete;

        CustomOpenSSLObjects(CustomOpenSSLObjects&&) = delete;
        CustomOpenSSLObjects& operator=(CustomOpenSSLObjects&&) = delete;

        const CustomOpenSSLObject& Get(CustomOpenSSLObjectName name) const;

    private:
        // Not enough to bother with more complex search
        std::vector<CustomOpenSSLObject> objects;
    };
} // namespace MSIX
