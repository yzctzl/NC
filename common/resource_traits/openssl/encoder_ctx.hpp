#pragma once
#include <openssl/encoder.h>

namespace nkg::resource_traits::openssl {

    struct encoder_ctx {
        using handle_t = OSSL_ENCODER_CTX*;

        static constexpr handle_t invalid_value = nullptr;

        [[nodiscard]]
        static bool is_valid(const handle_t& handle) noexcept {
            return handle != invalid_value;
        }

        static void release(const handle_t& handle) noexcept {
            OSSL_ENCODER_CTX_free(handle);
        }
    };

}
