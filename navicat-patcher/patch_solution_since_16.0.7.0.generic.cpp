#include "patch_solution_since_16.0.7.0.hpp"
#include <regex>

namespace nkg {

    std::string patch_solution_since<16, 0, 7, 0>::_build_encoded_key(const rsa_cipher& cipher) {
        auto encoded_key = cipher.export_public_key_string_pem();

        encoded_key = std::regex_replace(encoded_key, std::regex("-----BEGIN PUBLIC KEY-----"), "");
        encoded_key = std::regex_replace(encoded_key, std::regex("-----END PUBLIC KEY-----"), "");
        encoded_key = std::regex_replace(encoded_key, std::regex("\n"), "");

        return encoded_key;
    }

}
