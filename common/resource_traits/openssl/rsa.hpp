#pragma once
#include <openssl/rsa.h>

namespace nkg::resource_traits::openssl {

    struct rsa {
        using handle_t = RSA*;

        static constexpr handle_t invalid_value = nullptr;

        [[nodiscard]]
        static bool is_valid(const handle_t& handle) noexcept {
            return handle != invalid_value;
        }

        static void release(const handle_t& handle) noexcept {
            RSA_free(handle);
        }
    };

}
