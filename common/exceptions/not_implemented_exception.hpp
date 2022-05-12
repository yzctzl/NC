#pragma once
#include "../exception.hpp"

namespace nkg::exceptions {

    class not_implemented_exception : public ::nkg::exception {
        using ::nkg::exception::exception;
    };

}
