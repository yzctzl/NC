#pragma once
#include "../exception.hpp"

namespace nkg::exceptions {

    class index_exception : public ::nkg::exception {
        using ::nkg::exception::exception;
    };

}
