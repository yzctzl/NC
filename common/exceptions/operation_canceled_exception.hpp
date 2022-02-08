#pragma once
#include "../exception.hpp"

namespace nkg::exceptions {

    class operation_canceled_exception : public ::nkg::exception {
    public:
        operation_canceled_exception(std::string_view file, int line, std::string_view message) noexcept :
            ::nkg::exception(file, line, message) {}
    };

}
