#pragma once
#include <openssl/bn.h>

namespace nkg::resource_traits::openssl {

    struct bignum {
        using handle_t = BIGNUM*;

        static constexpr handle_t invalid_value = nullptr;

        [[nodiscard]]
        static bool is_valid(const handle_t& handle) noexcept {
            return handle != invalid_value;
        }

        static void release(const handle_t& handle) noexcept {
            BN_free(handle);
        }
    };

}
