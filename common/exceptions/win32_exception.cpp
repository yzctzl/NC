#include "win32_exception.hpp"
#include "../resource_wrapper.hpp"
#include "../resource_traits/win32/local_alloc.hpp"
#include "../cp_converter.hpp"

namespace nkg::exceptions {

    win32_exception::win32_exception(std::string_view file, int line, error_code_t win32_error_code, std::string_view message) noexcept :
        ::nkg::exception(file, line, message)
    {
        m_error_code = win32_error_code;

        ::nkg::resource_wrapper error_string{ ::nkg::resource_traits::win32::local_alloc{} };
        FormatMessageW(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
            NULL,
            win32_error_code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            error_string.template unsafe_addressof<wchar_t>(),
            0,
            NULL
        );
        if (error_string.is_valid()) {
            m_error_string = ::nkg::cp_converter<-1, CP_UTF8>::convert(error_string.template as<wchar_t*>());
        } else {
            std::terminate();
        }
    }

}
