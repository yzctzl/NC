#pragma once
#include <windows.h>

namespace nkg::resource_traits::win32 {

    struct map_view_ptr {
        using handle_t = PVOID;

        static constexpr handle_t invalid_value = NULL;

        [[nodiscard]]
        static bool is_valid(const handle_t& handle) noexcept {
            return handle != invalid_value;
        }

        static void release(const handle_t& handle) {
            UnmapViewOfFile(handle);
        }
    };

}
