#pragma once
#include "../exception.hpp"
#include <windows.h>

namespace nkg::exceptions {

    class win32_exception final : public ::nkg::exception {
    public:
        using error_code_t = decltype(GetLastError());

    private:
        error_code_t m_error_code;
        std::string m_error_string;

    public:

        win32_exception(std::string_view file, int line, error_code_t win32_error_code, std::string_view message) noexcept;

        [[nodiscard]]
        virtual bool error_code_exists() const noexcept override {
            return true;
        }

        [[nodiscard]]
        virtual intptr_t error_code() const noexcept override {
            return m_error_code;
        }

        [[nodiscard]]
        virtual const std::string& error_string() const noexcept override {
            return m_error_string;
        }
    };

}
