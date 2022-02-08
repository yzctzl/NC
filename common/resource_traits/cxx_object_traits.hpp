#pragma once

namespace nkg::resource_traits {

    template<typename object_t>
    struct cxx_object_traits {
        using handle_t = object_t*;

        static constexpr handle_t invalid_value = nullptr;

        [[nodiscard]]
        static bool is_valid(const handle_t& handle) noexcept {
            return handle != invalid_value;
        }

        static void release(const handle_t& handle) {
            delete handle;
        }
    };

}
