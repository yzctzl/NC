#include "base64_rfc4648.hpp"

#include <openssl/bio.h>
#include <openssl/evp.h>

#include "resource_wrapper.hpp"
#include "resource_traits/openssl/bio.hpp"
#include "resource_traits/openssl/bio_chain.hpp"

#pragma comment(lib, "libcrypto")
#pragma comment(lib, "crypt32") // required by libcrypto.lib
#pragma comment(lib, "ws2_32")  // required by libcrypto.lib

#define NKG_CURRENT_SOURCE_FILE() u8".\\navicat-keygen\\base64_rfc4648.cpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {

    std::string base64_rfc4648::encode(const std::vector<std::uint8_t>& data) {
        resource_wrapper bio_b64{ resource_traits::openssl::bio_chain{}, BIO_new(BIO_f_base64()) };
        if (bio_b64.is_valid() == false) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_new failed.");
        }

        BIO_set_flags(bio_b64.get(), BIO_FLAGS_BASE64_NO_NL);

        resource_wrapper bio_memory{ resource_traits::openssl::bio{}, BIO_new(BIO_s_mem()) };
        if (bio_memory.is_valid() == false) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_new failed.");
        }

        BIO_push(bio_b64.get(), bio_memory.get());

        for (size_t written_size = 0, left_size = data.size(); left_size != 0;) {
            int size_to_write = static_cast<int>(std::min(left_size, static_cast<size_t>(INT_MAX)));

            int r = BIO_write(bio_b64.get(), data.data() + written_size, size_to_write);
            if (r > 0) {
                written_size += r;
                left_size -= r;
            } else {
                throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_write failed.");
            }
        }

        BIO_flush(bio_b64.get());

        const char* pch = nullptr;
        long lch = BIO_get_mem_data(bio_memory.get(), &pch);

        bio_memory.discard();   // the bio_chain `bio_b64` will free it

        return std::string(pch, lch);
    }

    std::vector<uint8_t> base64_rfc4648::decode(std::string_view b64_string) {
        resource_wrapper bio_b64{ resource_traits::openssl::bio_chain{}, BIO_new(BIO_f_base64()) };
        if (bio_b64.is_valid() == false) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_new failed.");
        }

        BIO_set_flags(bio_b64.get(), BIO_FLAGS_BASE64_NO_NL);

        resource_wrapper bio_memory{ resource_traits::openssl::bio{}, BIO_new(BIO_s_mem()) };
        if (bio_memory.is_valid() == false) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_new failed.");
        }

        BIO_push(bio_b64.get(), bio_memory.get());

        for (size_t written_length = 0, left_length = b64_string.length(); left_length != 0;) {
            int length_to_write = static_cast<int>(std::min(left_length, static_cast<size_t>(INT_MAX)));

            int r = BIO_write(bio_memory.get(), b64_string.data() + written_length, length_to_write);
            if (r > 0) {
                written_length += r;
                left_length -= r;
            } else {
                throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"BIO_write failed.");
            }
        }

        std::vector<uint8_t> retval;
        retval.reserve(b64_string.length() * 3 / 4 + 1);

        for (uint8_t buf[256];;) {
            auto len = BIO_read(bio_b64.get(), buf, sizeof(buf));
            if (len > 0) {
                retval.insert(retval.end(), buf, buf + len);
            } else {
                break;
            }
        }

        bio_memory.discard();   // the bio_chain `bio_b64` will free it

        return retval;
    }

}

#undef NKG_CURRENT_SOURCE_FILE
#undef NKG_CURRENT_SOURCE_LINE
