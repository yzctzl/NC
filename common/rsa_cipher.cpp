#include "rsa_cipher.hpp"
#include <mutex>

#include <openssl/pem.h>
#include <openssl/bio.h>

#include "resource_traits/openssl/bio.hpp"
#include "resource_traits/openssl/bignum.hpp"

#if (OPENSSL_VERSION_NUMBER & 0xf0000000) == 0x30000000     // for openssl 3.x.x
#include <openssl/encoder.h>
#include <openssl/decoder.h>
#include "resource_traits/openssl/encoder_ctx.hpp"
#include "resource_traits/openssl/decoder_ctx.hpp"
#endif

#include "cp_converter.hpp"

#include "exceptions/overflow_exception.hpp"

#pragma comment(lib, "libcrypto")
#pragma comment(lib, "crypt32") // required by libcrypto.lib
#pragma comment(lib, "ws2_32")  // required by libcrypto.lib

#define NKG_CURRENT_SOURCE_FILE() u8".\\common\\rsa_cipher.cpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {

#if (OPENSSL_VERSION_NUMBER & 0xf0000000) < 0x30000000     // for openssl < 3.0.0
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
#elif (OPENSSL_VERSION_NUMBER & 0xf0000000) == 0x30000000               // for openssl 3.x.x
    [[nodiscard]]
    EVP_PKEY* rsa_cipher::_read_private_key_from_bio(BIO* p_bio) {
        resource_wrapper new_rsa{ resource_traits::openssl::evp_pkey{} };

        resource_wrapper decoder_context
            { resource_traits::openssl::decoder_ctx{}, OSSL_DECODER_CTX_new_for_pkey(new_rsa.unsafe_addressof(), "PEM", "pkcs1", "RSA", OSSL_KEYMGMT_SELECT_PRIVATE_KEY, nullptr, nullptr) };

        if (!decoder_context.is_valid()) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"OSSL_DECODER_CTX_new_for_pkey failed.");
        }

        if (!OSSL_DECODER_from_bio(decoder_context.get(), p_bio)) {     // 1 on success, 0 on failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"OSSL_DECODER_from_bio failed.");
        }

        return new_rsa.transfer();
}

    [[nodiscard]]
    EVP_PKEY* rsa_cipher::_read_public_key_pem_from_bio(BIO* p_bio) {
        resource_wrapper new_rsa{ resource_traits::openssl::evp_pkey{} };

        resource_wrapper decoder_context
            { resource_traits::openssl::decoder_ctx{}, OSSL_DECODER_CTX_new_for_pkey(new_rsa.unsafe_addressof(), "PEM", "SubjectPublicKeyInfo", "RSA", OSSL_KEYMGMT_SELECT_PUBLIC_KEY, nullptr, nullptr) };

        if (!decoder_context.is_valid()) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"OSSL_DECODER_CTX_new_for_pkey failed.");
        }

        if (!OSSL_DECODER_from_bio(decoder_context.get(), p_bio)) {     // 1 on success, 0 on failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"OSSL_DECODER_from_bio failed.");
        }

        return new_rsa.transfer();
    }

    [[nodiscard]]
    EVP_PKEY* rsa_cipher::_read_public_key_pkcs1_from_bio(BIO* p_bio) {
        resource_wrapper new_rsa{ resource_traits::openssl::evp_pkey{} };

        resource_wrapper decoder_context
            { resource_traits::openssl::decoder_ctx{}, OSSL_DECODER_CTX_new_for_pkey(new_rsa.unsafe_addressof(), "PEM", "pkcs1", "RSA", OSSL_KEYMGMT_SELECT_PUBLIC_KEY, nullptr, nullptr) };

        if (!decoder_context.is_valid()) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"OSSL_DECODER_CTX_new_for_pkey failed.");
        }

        if (!OSSL_DECODER_from_bio(decoder_context.get(), p_bio)) {     // 1 on success, 0 on failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"OSSL_DECODER_from_bio failed.");
        }

        return new_rsa.transfer();
    }

    void rsa_cipher::_write_private_key_to_bio(EVP_PKEY* p_rsa, BIO* p_bio) {
        resource_wrapper encoder_context
            { resource_traits::openssl::encoder_ctx{}, OSSL_ENCODER_CTX_new_for_pkey(p_rsa, OSSL_KEYMGMT_SELECT_PRIVATE_KEY, "PEM", "pkcs1", nullptr) };

        if (!encoder_context.is_valid()) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"OSSL_ENCODER_CTX_new_for_pkey failed.");
        }

        if (!OSSL_ENCODER_to_bio(encoder_context.get(), p_bio)) {   // 1 on success, 0 on failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"OSSL_ENCODER_to_bio failed.");
        }
    }

    void rsa_cipher::_write_public_key_pem_to_bio(EVP_PKEY* p_rsa, BIO* p_bio) {
        resource_wrapper encoder_context
            { resource_traits::openssl::encoder_ctx{}, OSSL_ENCODER_CTX_new_for_pkey(p_rsa, OSSL_KEYMGMT_SELECT_PUBLIC_KEY, "PEM", "SubjectPublicKeyInfo", nullptr) };

        if (!encoder_context.is_valid()) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"OSSL_ENCODER_CTX_new_for_pkey failed.");
        }

        if (!OSSL_ENCODER_to_bio(encoder_context.get(), p_bio)) {   // 1 on success, 0 on failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"OSSL_ENCODER_to_bio failed.");
        }
    }

    void rsa_cipher::_write_public_key_pkcs1_to_bio(EVP_PKEY* p_rsa, BIO* p_bio) {
        resource_wrapper encoder_context
            { resource_traits::openssl::encoder_ctx{}, OSSL_ENCODER_CTX_new_for_pkey(p_rsa, OSSL_KEYMGMT_SELECT_PUBLIC_KEY, "PEM", "pkcs1", nullptr) };

        if (!encoder_context.is_valid()) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"OSSL_ENCODER_CTX_new_for_pkey failed.");
        }

        if (!OSSL_ENCODER_to_bio(encoder_context.get(), p_bio)) {   // 1 on success, 0 on failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"OSSL_ENCODER_to_bio failed.");
        }
    }
#else
#error "rsa_cipher.cpp: Unexpected OpenSSL version."
#endif

    rsa_cipher::rsa_cipher() = default;

    [[nodiscard]]
    size_t rsa_cipher::bits() const {
        if (m_rsa.get()) {
#if (OPENSSL_VERSION_NUMBER & 0xfff00000) == 0x10000000         // openssl 1.0.x
            return BN_num_bits(m_rsa->n);
#elif (OPENSSL_VERSION_NUMBER & 0xfff00000) == 0x10100000       // openssl 1.1.x
            return RSA_bits(m_rsa.get());
#elif (OPENSSL_VERSION_NUMBER & 0xf0000000) == 0x30000000       // openssl 3.x.x
            return EVP_PKEY_get_bits(m_rsa.get());
#else
#error "rsa_cipher.cpp: uexpected OpenSSL version"
#endif
        } else {
            throw no_key_assigned_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"RSA key has not been assigned yet.");
        }
    }

    void rsa_cipher::generate_key(int bits, unsigned int e) {
        resource_wrapper bn_e{ resource_traits::openssl::bignum{}, BN_new() };

        if (bn_e.is_valid() == false) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), ERR_get_error(), u8"BN_new failed.");
        }

        if (BN_set_word(bn_e.get(), e) == 0) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BN_set_word failed.");
        }

#if (OPENSSL_VERSION_NUMBER & 0xf0000000) < 0x30000000      // for openssl < 3.0.0
        resource_wrapper new_rsa{ resource_traits::openssl::rsa{}, RSA_new() };
        if (!new_rsa.is_valid()) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), ERR_get_error(), u8"RSA_new failed.");
        }

        if (RSA_generate_key_ex(new_rsa.get(), bits, bn_e.get(), nullptr) == 0) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), ERR_get_error(), u8"RSA_generate_key_ex failed.");
        }

        m_rsa = std::move(new_rsa);
#elif (OPENSSL_VERSION_NUMBER & 0xf0000000) == 0x30000000               // for openssl 3.x.x
        resource_wrapper evp_pkey_context{ resource_traits::openssl::evp_pkey_ctx{}, EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr) };
        if (!evp_pkey_context.is_valid()) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_CTX_new_id failed.");
        }

        if (EVP_PKEY_keygen_init(evp_pkey_context.get()) <= 0) {     // 1 for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_keygen_init failed.");
        }

        if (EVP_PKEY_CTX_set_rsa_keygen_bits(evp_pkey_context.get(), bits) <= 0) {   // return a positive value for success and 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_CTX_set_rsa_keygen_bits failed.");
        }

        if (EVP_PKEY_CTX_set1_rsa_keygen_pubexp(evp_pkey_context.get(), bn_e.get()) <= 0) {     // return a positive value for success and 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_CTX_set1_rsa_keygen_pubexp failed.");
        }

        resource_wrapper new_rsa{ resource_traits::openssl::evp_pkey{} };

        if (EVP_PKEY_keygen(evp_pkey_context.get(), new_rsa.unsafe_addressof()) <= 0) {   // 1 for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_keygen failed.");
        }

        m_rsa = std::move(new_rsa);
#else
#error "rsa_cipher.cpp: Unexpected OpenSSL version."
#endif
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
#if (OPENSSL_VERSION_NUMBER & 0xf0000000) < 0x30000000      // for openssl < 3.0.0
        if (plaintext_size <= INT_MAX) {
            int bytes_written = 
                RSA_public_encrypt(static_cast<int>(plaintext_size), reinterpret_cast<const unsigned char*>(plaintext), reinterpret_cast<unsigned char*>(ciphertext), m_rsa.get(), padding);
            
            if (bytes_written != -1) {
                return bytes_written;
            } else {
                throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), ERR_get_error(), u8"RSA_public_encrypt failed.");
            }
        } else {
            throw exceptions::overflow_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"plaintext_size > INT_MAX");
        }
#elif (OPENSSL_VERSION_NUMBER & 0xf0000000) == 0x30000000   // for openssl 3.x.x
        resource_wrapper evp_pkey_context{ resource_traits::openssl::evp_pkey_ctx{}, EVP_PKEY_CTX_new(m_rsa.get(), nullptr) };
        if (!evp_pkey_context.is_valid()) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_CTX_new failed.");
        }

        if (EVP_PKEY_encrypt_init(evp_pkey_context.get()) <= 0) {    // return 1 for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_encrypt_init failed.");
        }

        if (EVP_PKEY_CTX_set_rsa_padding(evp_pkey_context.get(), padding) <= 0) {   // return a positive value for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_CTX_set_rsa_padding failed.");
        }

        size_t ciphertext_size = 0;
        if (EVP_PKEY_encrypt(evp_pkey_context.get(), nullptr, &ciphertext_size, reinterpret_cast<const unsigned char*>(plaintext), plaintext_size) <= 0) {   // return 1 for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_encrypt failed.");
        }

        if (EVP_PKEY_encrypt(evp_pkey_context.get(), reinterpret_cast<unsigned char*>(ciphertext), &ciphertext_size, reinterpret_cast<const unsigned char*>(plaintext), plaintext_size) <= 0) {   // return 1 for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_encrypt failed.");
        }

        return ciphertext_size;
#else
#error "rsa_cipher.cpp: Unexpected OpenSSL version."
#endif
    }

    size_t rsa_cipher::private_encrypt(const void* plaintext, size_t plaintext_size, void* ciphertext, int padding) const {
#if (OPENSSL_VERSION_NUMBER & 0xf0000000) < 0x30000000      // for openssl < 3.0.0
        if (plaintext_size <= INT_MAX) {
            int bytes_written = 
                RSA_private_encrypt(static_cast<int>(plaintext_size), reinterpret_cast<const unsigned char*>(plaintext), reinterpret_cast<unsigned char*>(ciphertext), m_rsa.get(), padding);
            
            if (bytes_written != -1) {
                return bytes_written;
            } else {
                throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), ERR_get_error(), u8"RSA_public_encrypt failed.");
            }
        } else {
            throw exceptions::overflow_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"plaintext_size > INT_MAX");
        }
#elif (OPENSSL_VERSION_NUMBER & 0xf0000000) == 0x30000000   // for openssl 3.x.x
        resource_wrapper evp_pkey_context{ resource_traits::openssl::evp_pkey_ctx{}, EVP_PKEY_CTX_new(m_rsa.get(), nullptr) };
        if (!evp_pkey_context.is_valid()) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_CTX_new failed.");
        }

        if (EVP_PKEY_sign_init(evp_pkey_context.get()) <= 0) {    // return 1 for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_sign_init failed.");
        }

        if (EVP_PKEY_CTX_set_rsa_padding(evp_pkey_context.get(), padding) <= 0) {   // return a positive value for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_CTX_set_rsa_padding failed.");
        }

        size_t ciphertext_size = 0;
        if (EVP_PKEY_sign(evp_pkey_context.get(), nullptr, &ciphertext_size, reinterpret_cast<const unsigned char*>(plaintext), plaintext_size) <= 0) {   // return 1 for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_sign failed.");
        }

        if (EVP_PKEY_sign(evp_pkey_context.get(), reinterpret_cast<unsigned char*>(ciphertext), &ciphertext_size, reinterpret_cast<const unsigned char*>(plaintext), plaintext_size) <= 0) {   // return 1 for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_sign failed.");
        }

        return ciphertext_size;
#else
#error "rsa_cipher.cpp: Unexpected OpenSSL version."
#endif
    }

    size_t rsa_cipher::public_decrypt(const void* ciphertext, size_t ciphertext_size, void* plaintext, int padding) const {
#if (OPENSSL_VERSION_NUMBER & 0xf0000000) < 0x30000000      // for openssl < 3.0.0
        if (ciphertext_size <= INT_MAX) {
            int bytes_written =
                RSA_public_decrypt(static_cast<int>(ciphertext_size), reinterpret_cast<const unsigned char*>(ciphertext), reinterpret_cast<unsigned char*>(plaintext), m_rsa.get(), padding);

            if (bytes_written != -1) {
                return bytes_written;
            } else {
                throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), ERR_get_error(), u8"RSA_public_decrypt failed.")
                    .push_hint(u8"Are your sure you DO provide a correct public key?");
            }
        } else {
            throw exceptions::overflow_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"ciphertext_size > INT_MAX");
        }
#elif (OPENSSL_VERSION_NUMBER & 0xf0000000) == 0x30000000   // for openssl 3.x.x
        resource_wrapper evp_pkey_context{ resource_traits::openssl::evp_pkey_ctx{}, EVP_PKEY_CTX_new(m_rsa.get(), nullptr) };
        if (!evp_pkey_context.is_valid()) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_CTX_new failed.");
        }

        if (EVP_PKEY_verify_recover_init(evp_pkey_context.get())) {     // return 1 for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_verify_recover_init failed.");
        }

        if (EVP_PKEY_CTX_set_rsa_padding(evp_pkey_context.get(), padding) <= 0) {   // return a positive value for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_CTX_set_rsa_padding failed.");
        }

        size_t plaintext_size = 0;
        if (EVP_PKEY_verify_recover(evp_pkey_context.get(), nullptr, &plaintext_size, reinterpret_cast<const unsigned char*>(ciphertext), ciphertext_size) <= 0) {   // return 1 for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_verify_recover failed.")
                .push_hint(u8"Are your sure you DO provide a correct public key?");
        }

        if (EVP_PKEY_verify_recover(evp_pkey_context.get(), reinterpret_cast<unsigned char*>(plaintext), &plaintext_size, reinterpret_cast<const unsigned char*>(ciphertext), ciphertext_size) <= 0) {   // return 1 for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_verify_recover failed.");
        }

        return plaintext_size;
#else
#error "rsa_cipher.cpp: Unexpected OpenSSL version."
#endif
    }

    size_t rsa_cipher::private_decrypt(const void* ciphertext, size_t ciphertext_size, void* plaintext, int padding) const {
#if (OPENSSL_VERSION_NUMBER & 0xf0000000) < 0x30000000      // for openssl < 3.0.0
        if (ciphertext_size <= INT_MAX) {
            int bytes_written =
                RSA_private_decrypt(static_cast<int>(ciphertext_size), reinterpret_cast<const unsigned char*>(ciphertext), reinterpret_cast<unsigned char*>(plaintext), m_rsa.get(), padding);

            if (bytes_written != -1) {
                return bytes_written;
            } else {
                throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), ERR_get_error(), u8"RSA_public_decrypt failed.")
                    .push_hint(u8"Are your sure you DO provide a correct private key?");
            }
        } else {
            throw exceptions::overflow_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"ciphertext_size > INT_MAX");
        }
#elif (OPENSSL_VERSION_NUMBER & 0xf0000000) == 0x30000000   // for openssl 3.x.x
        resource_wrapper evp_pkey_context{ resource_traits::openssl::evp_pkey_ctx{}, EVP_PKEY_CTX_new(m_rsa.get(), nullptr) };
        if (!evp_pkey_context.is_valid()) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_CTX_new failed.");
        }

        if (EVP_PKEY_decrypt_init(evp_pkey_context.get()) <= 0) {    // return 1 for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_decrypt_init failed.");
        }

        if (EVP_PKEY_CTX_set_rsa_padding(evp_pkey_context.get(), padding) <= 0) {   // return a positive value for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_CTX_set_rsa_padding failed.");
        }

        size_t plaintext_size = 0;
        if (EVP_PKEY_decrypt(evp_pkey_context.get(), nullptr, &plaintext_size, reinterpret_cast<const unsigned char*>(ciphertext), ciphertext_size) <= 0) {   // return 1 for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_decrypt failed.")
                .push_hint(u8"Are your sure you DO provide a correct private key?");
        }

        if (EVP_PKEY_decrypt(evp_pkey_context.get(), reinterpret_cast<unsigned char*>(plaintext), &plaintext_size, reinterpret_cast<const unsigned char*>(ciphertext), ciphertext_size) <= 0) {   // return 1 for success, 0 or a negative value for failure
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"EVP_PKEY_decrypt failed.");
        }

        return plaintext_size;
#else
#error "rsa_cipher.cpp: Unexpected OpenSSL version."
#endif
    }

    rsa_cipher::backend_error::backend_error(std::string_view file, int line, std::string_view message) noexcept:
        ::nkg::exception::exception(file, line, message), m_error_code(0) {}

    rsa_cipher::backend_error::backend_error(std::string_view file, int line, error_code_t openssl_errno, std::string_view message) noexcept:
        ::nkg::exception::exception(file, line, message), m_error_code(openssl_errno)
    {
        static std::once_flag onceflag_load_crypto_strings;
        std::call_once(onceflag_load_crypto_strings, []() { ERR_load_crypto_strings(); });

        m_error_string = ERR_reason_error_string(m_error_code);
    }
}

#undef NKG_CURRENT_SOURCE_FILE
#undef NKG_CURRENT_SOURCE_LINE
