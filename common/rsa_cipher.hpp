#pragma once
#include <string>
#include <filesystem>

#include <openssl/err.h>
#include <openssl/rsa.h>

#include "resource_wrapper.hpp"
#if (OPENSSL_VERSION_NUMBER & 0xf0000000) < 0x30000000      // for openssl < 3.0.0
#include "resource_traits/openssl/rsa.hpp"
#elif (OPENSSL_VERSION_NUMBER & 0xf0000000) == 0x30000000   // for openssl 3.x.x
#include "resource_traits/openssl/evp_pkey_ctx.hpp"
#include "resource_traits/openssl/evp_pkey.hpp"
#else
#error "rsa_cipher.hpp: Unexpected OpenSSL version."
#endif

#include "exception.hpp"

#define NKG_CURRENT_SOURCE_FILE() u8".\\common\\rsa_cipher.hpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {

    class rsa_cipher {
    public:
        class backend_error;
        class no_key_assigned_error;

    private:
#if (OPENSSL_VERSION_NUMBER & 0xf0000000) < 0x30000000                  // for openssl < 3.0.0
        resource_wrapper<resource_traits::openssl::rsa> m_rsa;

        [[nodiscard]]
        static RSA* _read_private_key_from_bio(BIO* p_bio);

        [[nodiscard]]
        static RSA* _read_public_key_pem_from_bio(BIO* p_bio);

        [[nodiscard]]
        static RSA* _read_public_key_pkcs1_from_bio(BIO* p_bio);

        static void _write_private_key_to_bio(RSA* p_rsa, BIO* p_bio);

        static void _write_public_key_pem_to_bio(RSA* p_rsa, BIO* p_bio);

        static void _write_public_key_pkcs1_to_bio(RSA* p_rsa, BIO* p_bio);
#elif (OPENSSL_VERSION_NUMBER & 0xf0000000) == 0x30000000               // for openssl 3.x.x
        resource_wrapper<resource_traits::openssl::evp_pkey> m_rsa;

        [[nodiscard]]
        static EVP_PKEY* _read_private_key_from_bio(BIO* p_bio);

        [[nodiscard]]
        static EVP_PKEY* _read_public_key_pem_from_bio(BIO* p_bio);

        [[nodiscard]]
        static EVP_PKEY* _read_public_key_pkcs1_from_bio(BIO* p_bio);

        static void _write_private_key_to_bio(EVP_PKEY* p_rsa, BIO* p_bio);

        static void _write_public_key_pem_to_bio(EVP_PKEY* p_rsa, BIO* p_bio);

        static void _write_public_key_pkcs1_to_bio(EVP_PKEY* p_rsa, BIO* p_bio);
#else
#error "rsa_cipher.hpp: Unexpected OpenSSL version."
#endif

    public:
        rsa_cipher();

        [[nodiscard]]
        size_t bits() const;

        void generate_key(int bits, unsigned int e = RSA_F4);

        void export_private_key_file(std::wstring_view file_path) const;

        void export_private_key_file(const std::filesystem::path& file_path) const;

        void export_public_key_file_pem(std::wstring_view file_path) const;

        void export_public_key_file_pem(const std::filesystem::path& file_path) const;

        void export_public_key_file_pkcs1(std::wstring_view file_path) const;

        void export_public_key_file_pkcs1(const std::filesystem::path& file_path) const;

        void import_private_key_file(std::wstring_view file_path);

        void import_private_key_file(const std::filesystem::path& file_path);

        void import_public_key_file_pem(std::wstring_view file_path);

        void import_public_key_file_pem(const std::filesystem::path& file_path);

        void import_public_key_file_pkcs1(std::wstring_view file_path);

        void import_public_key_file_pkcs1(const std::filesystem::path& file_path);

        [[nodiscard]]
        std::string export_private_key_string() const;

        [[nodiscard]]
        std::string export_public_key_string_pem() const;

        [[nodiscard]]
        std::string export_public_key_string_pkcs1() const;

        void import_private_key_string(std::string_view key_string);

        void import_public_key_string_pem(std::string_view key_string);

        void import_public_key_string_pkcs1(std::string_view key_string);

        size_t public_encrypt(const void* plaintext, size_t plaintext_size, void* ciphertext, int padding) const;

        size_t private_encrypt(const void* plaintext, size_t plaintext_size, void* ciphertext, int padding) const;

        size_t public_decrypt(const void* ciphertext, size_t ciphertext_size, void* plaintext, int padding) const;

        size_t private_decrypt(const void* ciphertext, size_t ciphertext_size, void* plaintext, int padding) const;
    };

    class rsa_cipher::backend_error : public ::nkg::exception {
    public:
        using error_code_t = decltype(ERR_get_error());

    private:
        error_code_t m_error_code;
        std::string m_error_string;

    public:
        backend_error(std::string_view file, int line, std::string_view message) noexcept;

        backend_error(std::string_view file, int line, error_code_t openssl_errno, std::string_view message) noexcept;

        [[nodiscard]]
        virtual bool error_code_exists() const noexcept override {
            return m_error_code != 0;
        }

        [[nodiscard]]
        virtual intptr_t error_code() const noexcept override {
            if (error_code_exists()) { return m_error_code; } else { trap_then_terminate(); }
        }

        [[nodiscard]]
        virtual const std::string& error_string() const noexcept override {
            if (error_code_exists()) { return m_error_string; } else { trap_then_terminate(); }
        }
    };

    class rsa_cipher::no_key_assigned_error : public ::nkg::exception {
        using ::nkg::exception::exception;
    };

}

#undef NKG_CURRENT_SOURCE_FILE
#undef NKG_CURRENT_SOURCE_LINE
