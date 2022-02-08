#pragma once
#include <string>
#include <filesystem>
#include <openssl/rsa.h>

#include "resource_wrapper.hpp"
#include "resource_traits/openssl/rsa.hpp"

#include "exception.hpp"

#define NKG_CURRENT_SOURCE_FILE() u8".\\common\\rsa_cipher.hpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {

    class rsa_cipher {
    public:
        class no_key_assigned_error : public ::nkg::exception {
        public:
            no_key_assigned_error(std::string_view file, int line, std::string_view message) noexcept :
                ::nkg::exception(file, line, message) {}
        };

        class backend_error : public ::nkg::exception {
        public:
            backend_error(std::string_view file, int line, std::string_view message) noexcept :
                ::nkg::exception(file, line, message) {}
        };

    private:
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

}

#undef NKG_CURRENT_SOURCE_FILE
#undef NKG_CURRENT_SOURCE_LINE
