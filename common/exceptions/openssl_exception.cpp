#include "openssl_exception.hpp"
#include <mutex>

#pragma comment(lib, "libcrypto")
#pragma comment(lib, "crypt32") // required by libcrypto.lib
#pragma comment(lib, "ws2_32")  // required by libcrypto.lib

namespace nkg::exceptions {

    openssl_exception::openssl_exception(std::string_view file, int line, error_code_t openssl_error_code, std::string_view message) noexcept :
        ::nkg::exception(file, line, message) 
    {
        static std::once_flag onceflag_load_crypto_strings;
        std::call_once(onceflag_load_crypto_strings, []() { ERR_load_crypto_strings(); });

        m_error_code = openssl_error_code;
        m_error_string = ERR_reason_error_string(m_error_code);
    }

}
