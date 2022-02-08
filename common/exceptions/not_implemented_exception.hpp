#pragma once
#include "../exception.hpp"

namespace nkg::exceptions {

    class not_implemented_exception : public ::nkg::exception {
    public:
        not_implemented_exception(std::string_view file, int line, std::string_view message) noexcept :
            ::nkg::exception(file, line, message) {}
    };

}
