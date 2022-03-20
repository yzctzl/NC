#pragma once
#include <string>
#include <vector>
#include "exception.hpp"

namespace nkg {

    struct base64_rfc4648 {

        class backend_error : public ::nkg::exception {
        public:
            backend_error(std::string_view file, int line, std::string_view message) noexcept :
                ::nkg::exception(file, line, message) {}
        };

        static std::string encode(const std::vector<std::uint8_t>& data);
        static std::vector<uint8_t> decode(std::string_view str_b64);
    };

}
