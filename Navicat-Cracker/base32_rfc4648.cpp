#include "base32_rfc4648.hpp"

#define NKG_CURRENT_SOURCE_FILE() u8".\\navicat-keygen\\base32_rfc4648.cpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {

    char base32_rfc4648::symbol(alphabet_index_t idx) {
        return alphabet[idx];
    }

    base32_rfc4648::alphabet_index_t base32_rfc4648::reverse_symbol(char c) {
        if ('A' <= c && c <= 'Z') {
            return c - 'A';
        } else if ('2' <= c && c <= '7') {
            return c - '2' + 26;
        } else {
            throw decoding_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Non-base32 digit is found");
        }
    }

    std::string base32_rfc4648::encode(const std::vector<uint8_t>& data) {
        return encode(data.data(), data.size());
    }

    std::string base32_rfc4648::encode(const void* data_ptr, size_t data_size) {
        std::string retval;

        if (data_size) {
            retval.reserve((data_size * 8 + 4) / 5);

            auto p = reinterpret_cast<const uint8_t*>(data_ptr);
            alphabet_index_t left_bits = 0;
            alphabet_index_t bit_buffer = 0;
            for (size_t i = 0; i < data_size; ++i) {
                bit_buffer = (bit_buffer << 8) | p[i];
                left_bits += 8;

                while (left_bits >= 5) {
                    alphabet_index_t idx = (bit_buffer >> (left_bits - 5)) & 0x1f;
                    retval.push_back(symbol(idx));
                    left_bits -= 5;
                }
            }

            if (left_bits > 0) {
                alphabet_index_t idx = (bit_buffer << (5 - left_bits)) & 0x1f;
                retval.push_back(symbol(idx));
            }

            switch (data_size % 5) {
                case 0:
                    break;
                case 1:
                    retval.append(6, padding_character);
                    break;
                case 2:
                    retval.append(4, padding_character);
                    break;
                case 3:
                    retval.append(3, padding_character);
                    break;
                case 4:
                    retval.append(1, padding_character);
                    break;
                default:
                    __assume(false);
            }
        }
            
        return retval;
    }

    std::vector<uint8_t> base32_rfc4648::decode(std::string_view b32_string) {
        if (b32_string.length() % 8 == 0) {
            std::vector<uint8_t> retval;

            size_t count_of_padding = std::distance(b32_string.crbegin(), std::find_if_not(b32_string.crbegin(), b32_string.crend(), [](char c) -> bool { return c == padding_character; }));
            switch (count_of_padding) {
                case 1:
                    retval.reserve(b32_string.length() / 8 * 5 - (5 - 4));
                    break;
                case 3:
                    retval.reserve(b32_string.length() / 8 * 5 - (5 - 3));
                    break;
                case 4:
                    retval.reserve(b32_string.length() / 8 * 5 - (5 - 2));
                    break;
                case 6:
                    retval.reserve(b32_string.length() / 8 * 5 - (5 - 1));
                    break;
                default:
                    throw decoding_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Incorrect padding");
            }

            size_t count_of_encoded = b32_string.length() - count_of_padding;

            alphabet_index_t left_bits = 0;
            alphabet_index_t bit_buffer = 0;
            for (size_t i = 0; i < count_of_encoded; ++i) {
                bit_buffer = (bit_buffer << 5) | reverse_symbol(b32_string[i]);
                left_bits += 5;

                while (left_bits >= 8) {
                    auto val = static_cast<uint8_t>((bit_buffer >> (left_bits - 8)) & 0xff);
                    retval.push_back(val);
                    left_bits -= 8;
                }
            }

            return retval;
        } else {
            throw decoding_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Incorrect padding");
        }
    }

}

#undef NKG_CURRENT_SOURCE_LINE
#undef NKG_CURRENT_SOURCE_FILE
