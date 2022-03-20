#pragma once
#include <limits>
#include <string>
#include <vector>
#include <algorithm>
#include "exception.hpp"

#define NKG_CURRENT_SOURCE_FILE() u8".\\navicat-keygen\\base32_rfc4648.hpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {

    struct base32_rfc4648 {
        using alphabet_index_t = size_t;

        static constexpr const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
        static constexpr const char padding_character = '=';

        class decoding_error : public ::nkg::exception {
        public:
            decoding_error(std::string_view file, int line, std::string_view message) noexcept :
                ::nkg::exception(file, line, message) {}
        };

        static char symbol(alphabet_index_t idx);

        static alphabet_index_t reverse_symbol(char c);

        static std::string encode(const std::vector<uint8_t>& data);

        static std::string encode(const void* data_ptr, size_t data_size);

        static std::vector<uint8_t> decode(std::string_view b32_string);
    };

}

#undef NKG_CURRENT_SOURCE_LINE
#undef NKG_CURRENT_SOURCE_FILE
