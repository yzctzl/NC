#include "rsa_cipher.hpp"

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/bio.h>

#include "resource_traits/openssl/bio.hpp"
#include "resource_traits/openssl/bignum.hpp"

#include "cp_converter.hpp"

#include "exceptions/overflow_exception.hpp"
#include "exceptions/openssl_exception.hpp"

#pragma comment(lib, "libcrypto")
#pragma comment(lib, "crypt32") // required by libcrypto.lib
#pragma comment(lib, "ws2_32")  // required by libcrypto.lib

#define NKG_CURRENT_SOURCE_FILE() u8".\\common\\rsa_cipher.cpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {

    RSA* rsa_cipher::_read_private_key_from_bio(BIO* p_bio) {
        resource_wrapper new_rsa
            { resource_traits::openssl::rsa{}, PEM_read_bio_RSAPrivateKey(p_bio, nullptr, nullptr, nullptr) };

        if (new_rsa.is_valid()) {
            return new_rsa.transfer();
        } else {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"PEM_read_bio_RSAPrivateKey failed.")
                .push_hint(u8"Are you sure that you DO provide a valid RSA private key file?");
        }
    }

    RSA* rsa_cipher::_read_public_key_pem_from_bio(BIO* p_bio) {
        resource_wrapper new_rsa
            { resource_traits::openssl::rsa{}, PEM_read_bio_RSA_PUBKEY(p_bio, nullptr, nullptr, nullptr) };

        if (new_rsa.is_valid()) {
            return new_rsa.transfer();
        } else {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"PEM_read_bio_RSA_PUBKEY failed.")
                .push_hint(u8"Are you sure that you DO provide a valid RSA public key file with PEM format?");
        }
    }

    RSA* rsa_cipher::_read_public_key_pkcs1_from_bio(BIO* p_bio) {
        resource_wrapper new_rsa
            { resource_traits::openssl::rsa{}, PEM_read_bio_RSAPublicKey(p_bio, nullptr, nullptr, nullptr) };

        if (new_rsa.is_valid()) {
            return new_rsa.transfer();
        } else {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"PEM_read_bio_RSAPublicKey failed.")
                .push_hint(u8"Are you sure that you DO provide a valid RSA public key file with PKCS1 format?");
        }
    }

    void rsa_cipher::_write_private_key_to_bio(RSA* p_rsa, BIO* p_bio) {
        auto r = PEM_write_bio_RSAPrivateKey(p_bio, p_rsa, nullptr, nullptr, 0, nullptr, nullptr);
        if (r == 0) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"PEM_write_bio_RSAPrivateKey failed.");
        };
    }

    void rsa_cipher::_write_public_key_pem_to_bio(RSA* p_rsa, BIO* p_bio) {
        auto r = PEM_write_bio_RSA_PUBKEY(p_bio, p_rsa);
        if (r == 0) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"PEM_write_bio_RSA_PUBKEY failed.");
        }
    }

    void rsa_cipher::_write_public_key_pkcs1_to_bio(RSA* p_rsa, BIO* p_bio) {
        auto r = PEM_write_bio_RSAPublicKey(p_bio, p_rsa);
        if (r == 0) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"PEM_write_bio_RSAPublicKey failed.");
        }
    }

    rsa_cipher::rsa_cipher() : m_rsa(RSA_new()) {
        if (!m_rsa.is_valid()) {
            throw exceptions::openssl_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), ERR_get_error(), u8"RSA_new failed.");
        }
    }

    [[nodiscard]]
    size_t rsa_cipher::bits() const {
#if (OPENSSL_VERSION_NUMBER & 0xffff0000) == 0x10000000     // openssl 1.0.x
        if (m_rsa->n == nullptr) {
            throw no_key_assigned_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"RSA modulus has not been set.");
        }
        return BN_num_bits(m_rsa->n);
#elif (OPENSSL_VERSION_NUMBER & 0xffff0000) == 0x10100000     // openssl 1.1.x
        return RSA_bits(m_rsa.get());
#else
#error "rsa_cipher.cpp: uexpected OpenSSL version"
#endif
    }

    void rsa_cipher::generate_key(int bits, unsigned int e) {
        resource_wrapper bn_e{ resource_traits::openssl::bignum{}, BN_new() };

        if (bn_e.is_valid() == false) {
            throw exceptions::openssl_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), ERR_get_error(), u8"BN_new failed.");
        }

        if (BN_set_word(bn_e.get(), e) == 0) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BN_set_word failed.");
        }

        if (RSA_generate_key_ex(m_rsa.get(), bits, bn_e.get(), nullptr) == 0) {
            throw exceptions::openssl_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), ERR_get_error(), u8"RSA_generate_key_ex failed.");
        }
    }

    void rsa_cipher::export_private_key_file(std::wstring_view file_path) const {
        resource_wrapper bio_file
            { resource_traits::openssl::bio{}, BIO_new_file(cp_converter<-1, CP_UTF8>::convert(file_path).c_str(), "w")};

        if (bio_file.is_valid()) {
            _write_private_key_to_bio(m_rsa.get(), bio_file.get());
        } else {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_new_file failed.");
        }
    }

    void rsa_cipher::export_private_key_file(const std::filesystem::path& file_path) const {
        export_private_key_file(static_cast<std::wstring_view>(file_path.native()));
    }

    void rsa_cipher::export_public_key_file_pem(std::wstring_view file_path) const {
        resource_wrapper bio_file
            { resource_traits::openssl::bio{}, BIO_new_file(cp_converter<-1, CP_UTF8>::convert(file_path).c_str(), "w")};

        if (bio_file.is_valid()) {
            _write_public_key_pem_to_bio(m_rsa.get(), bio_file.get());
        } else {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_new_file failed.");
        }
    }

    void rsa_cipher::export_public_key_file_pem(const std::filesystem::path& file_path) const {
        export_public_key_file_pem(static_cast<std::wstring_view>(file_path.native()));
    }

    void rsa_cipher::export_public_key_file_pkcs1(std::wstring_view file_path) const {
        resource_wrapper bio_file
            { resource_traits::openssl::bio{}, BIO_new_file(cp_converter<-1, CP_UTF8>::convert(file_path).c_str(), "w")};

        if (bio_file.is_valid()) {
            _write_public_key_pkcs1_to_bio(m_rsa.get(), bio_file.get());
        } else {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_new_file failed.");
        }
    }

    void rsa_cipher::export_public_key_file_pkcs1(const std::filesystem::path& file_path) const {
        export_public_key_file_pkcs1(static_cast<std::wstring_view>(file_path.native()));
    }

    void rsa_cipher::import_private_key_file(std::wstring_view file_path) {
        resource_wrapper bio_file
            { resource_traits::openssl::bio{}, BIO_new_file(cp_converter<-1, CP_UTF8>::convert(file_path).c_str(), "r") };

        if (bio_file.is_valid()) {
            m_rsa.set(_read_private_key_from_bio(bio_file.get()));
        } else {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_new_file failed.");
        }
    }

    void rsa_cipher::import_private_key_file(const std::filesystem::path& file_path) {
        import_private_key_file(static_cast<std::wstring_view>(file_path.native()));
    }

    void rsa_cipher::import_public_key_file_pem(std::wstring_view file_path) {
        resource_wrapper bio_file
            { resource_traits::openssl::bio{}, BIO_new_file(cp_converter<-1, CP_UTF8>::convert(file_path).c_str(), "r") };

        if (bio_file.is_valid()) {
            m_rsa.set(_read_public_key_pem_from_bio(bio_file.get()));
        } else {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_new_file failed.");
        }
    }

    void rsa_cipher::import_public_key_file_pem(const std::filesystem::path& file_path) {
        import_public_key_file_pem(static_cast<std::wstring_view>(file_path.native()));
    }

    void rsa_cipher::import_public_key_file_pkcs1(std::wstring_view file_path) {
        resource_wrapper bio_file
            { resource_traits::openssl::bio{}, BIO_new_file(cp_converter<-1, CP_UTF8>::convert(file_path).c_str(), "r") };

        if (bio_file.is_valid()) {
            m_rsa.set(_read_public_key_pkcs1_from_bio(bio_file.get()));
        } else {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_new_file failed.");
        }
    }

    void rsa_cipher::import_public_key_file_pkcs1(const std::filesystem::path& file_path) {
        import_public_key_file_pkcs1(static_cast<std::wstring_view>(file_path.native()));
    }

    [[nodiscard]]
    std::string rsa_cipher::export_private_key_string() const {
        resource_wrapper bio_memory{ resource_traits::openssl::bio{}, BIO_new(BIO_s_mem()) };

        if (bio_memory.is_valid()) {
            _write_private_key_to_bio(m_rsa.get(), bio_memory.get());

            const char* pch = nullptr;
            long lch = BIO_get_mem_data(bio_memory.get(), &pch);

            return std::string(pch, lch);
        } else {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_new failed.");
        }
    }

    [[nodiscard]]
    std::string rsa_cipher::export_public_key_string_pem() const {
        resource_wrapper bio_memory{ resource_traits::openssl::bio{}, BIO_new(BIO_s_mem()) };

        if (bio_memory.is_valid()) {
            _write_public_key_pem_to_bio(m_rsa.get(), bio_memory.get());

            const char* pch = nullptr;
            long lch = BIO_get_mem_data(bio_memory.get(), &pch);

            return std::string(pch, lch);
        } else {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_new failed.");
        }
    }

    [[nodiscard]]
    std::string rsa_cipher::export_public_key_string_pkcs1() const {
        resource_wrapper bio_memory{ resource_traits::openssl::bio{}, BIO_new(BIO_s_mem()) };

        if (bio_memory.is_valid()) {
            _write_public_key_pkcs1_to_bio(m_rsa.get(), bio_memory.get());

            const char* pch = nullptr;
            long lch = BIO_get_mem_data(bio_memory.get(), &pch);

            return std::string(pch, lch);
        } else {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_new failed.");
        }
    }

    void rsa_cipher::import_private_key_string(std::string_view key_string) {
        resource_wrapper bio_memory{ resource_traits::openssl::bio{}, BIO_new(BIO_s_mem()) };

        if (bio_memory.is_valid() == false) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_new failed.");
        }

        if (BIO_puts(bio_memory.get(), key_string.data()) <= 0) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_puts failed.");
        }

        m_rsa.set(_read_private_key_from_bio(bio_memory.get()));
    }

    void rsa_cipher::import_public_key_string_pem(std::string_view key_string) {
        resource_wrapper bio_memory{ resource_traits::openssl::bio{}, BIO_new(BIO_s_mem()) };

        if (bio_memory.is_valid() == false) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_new failed.");
        }

        if (BIO_puts(bio_memory.get(), key_string.data()) <= 0) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_puts failed.");
        }

        m_rsa.set(_read_public_key_pem_from_bio(bio_memory.get()));
    }

    void rsa_cipher::import_public_key_string_pkcs1(std::string_view key_string) {
        resource_wrapper bio_memory{ resource_traits::openssl::bio{}, BIO_new(BIO_s_mem()) };

        if (bio_memory.is_valid() == false) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_new failed.");
        }

        if (BIO_puts(bio_memory.get(), key_string.data()) <= 0) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_puts failed.");
        }

        m_rsa.set(_read_public_key_pkcs1_from_bio(bio_memory.get()));
    }

    size_t rsa_cipher::public_encrypt(const void* plaintext, size_t plaintext_size, void* ciphertext, int padding) const {
        if (plaintext_size <= INT_MAX) {
            int bytes_written = 
                RSA_public_encrypt(static_cast<int>(plaintext_size), reinterpret_cast<const unsigned char*>(plaintext), reinterpret_cast<unsigned char*>(ciphertext), m_rsa.get(), padding);
            
            if (bytes_written != -1) {
                return bytes_written;
            } else {
                throw exceptions::openssl_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), ERR_get_error(), u8"RSA_public_encrypt failed.");
            }
        } else {
            throw exceptions::overflow_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"plaintext_size > INT_MAX");
        }
    }

    size_t rsa_cipher::private_encrypt(const void* plaintext, size_t plaintext_size, void* ciphertext, int padding) const {
        if (plaintext_size <= INT_MAX) {
            int bytes_written = 
                RSA_private_encrypt(static_cast<int>(plaintext_size), reinterpret_cast<const unsigned char*>(plaintext), reinterpret_cast<unsigned char*>(ciphertext), m_rsa.get(), padding);
            
            if (bytes_written != -1) {
                return bytes_written;
            } else {
                throw exceptions::openssl_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), ERR_get_error(), u8"RSA_public_encrypt failed.");
            }
        } else {
            throw exceptions::overflow_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"plaintext_size > INT_MAX");
        }
    }

    size_t rsa_cipher::public_decrypt(const void* ciphertext, size_t ciphertext_size, void* plaintext, int padding) const {
        if (ciphertext_size <= INT_MAX) {
            int bytes_written =
                RSA_public_decrypt(static_cast<int>(ciphertext_size), reinterpret_cast<const unsigned char*>(ciphertext), reinterpret_cast<unsigned char*>(plaintext), m_rsa.get(), padding);

            if (bytes_written != -1) {
                return bytes_written;
            } else {
                throw exceptions::openssl_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), ERR_get_error(), u8"RSA_public_decrypt failed.")
                    .push_hint(u8"Are your sure you DO provide a correct public key?");
            }
        } else {
            throw exceptions::overflow_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"ciphertext_size > INT_MAX");
        }
    }

    size_t rsa_cipher::private_decrypt(const void* ciphertext, size_t ciphertext_size, void* plaintext, int padding) const {
        if (ciphertext_size <= INT_MAX) {
            int bytes_written =
                RSA_private_decrypt(static_cast<int>(ciphertext_size), reinterpret_cast<const unsigned char*>(ciphertext), reinterpret_cast<unsigned char*>(plaintext), m_rsa.get(), padding);

            if (bytes_written != -1) {
                return bytes_written;
            } else {
                throw exceptions::openssl_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), ERR_get_error(), u8"RSA_public_decrypt failed.")
                    .push_hint(u8"Are your sure you DO provide a correct private key?");
            }
        } else {
            throw exceptions::overflow_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"ciphertext_size > INT_MAX");
        }
    }
}

#undef NKG_CURRENT_SOURCE_FILE
#undef NKG_CURRENT_SOURCE_LINE
