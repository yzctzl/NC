#pragma once
#include "../exception.hpp"

namespace nkg::exceptions {

    class operation_canceled_exception : public ::nkg::exception {
        using ::nkg::exception::exception;
    };

}
