#pragma once
#include <openssl/bio.h>

namespace nkg::resource_traits::openssl {

    struct bio {
        using handle_t = BIO*;

        static constexpr handle_t invalid_value = nullptr;

        [[nodiscard]]
        static bool is_valid(const handle_t& handle) noexcept {
            return handle != invalid_value;
        }

        static void release(const handle_t& handle) noexcept {
            BIO_free(handle);
        }
    };

}
