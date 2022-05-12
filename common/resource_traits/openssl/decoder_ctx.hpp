#pragma once
#include <openssl/decoder.h>

namespace nkg::resource_traits::openssl {

    struct decoder_ctx {
        using handle_t = OSSL_DECODER_CTX*;

        static constexpr handle_t invalid_value = nullptr;

        [[nodiscard]]
        static bool is_valid(const handle_t& handle) noexcept {
            return handle != invalid_value;
        }

        static void release(const handle_t& handle) noexcept {
            OSSL_DECODER_CTX_free(handle);
        }
    };

}
