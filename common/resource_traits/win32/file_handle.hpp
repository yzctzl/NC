#pragma once
#include <windows.h>

namespace nkg::resource_traits::win32 {

    struct file_handle {
        using handle_t = HANDLE;

        static inline const handle_t invalid_value = INVALID_HANDLE_VALUE;

        [[nodiscard]]
        static bool is_valid(const handle_t& handle) noexcept {
            return handle != invalid_value;
        }

        static void release(const handle_t& handle) {
            CloseHandle(handle);
        }
    };

}
