#pragma once
#include <keystone/keystone.h>

namespace nkg::resource_traits::keystone {

    struct keystone_handle {
        using handle_t = ks_engine*;

        static constexpr handle_t invalid_value = nullptr;

        [[nodiscard]]
        static bool is_valid(const handle_t& handle) noexcept {
            return handle != invalid_value;
        }

        static void release(const handle_t& handle) {
            ks_close(handle);
        }
    };

    struct keystone_alloc {
        using handle_t = unsigned char*;

        static constexpr handle_t invalid_value = nullptr;

        [[nodiscard]]
        static bool is_valid(const handle_t& handle) noexcept {
            return handle != invalid_value;
        }

        static void release(const handle_t& handle) noexcept {
            ks_free(handle);
        }
    };
}
