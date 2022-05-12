#pragma once
#include "../exception.hpp"

namespace nkg::exceptions {

    class overflow_exception : public ::nkg::exception {
        using ::nkg::exception::exception;
    };

}
