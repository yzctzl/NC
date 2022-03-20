#pragma once
#include <string>
#include <vector>
#include <keystone/keystone.h>

#include "resource_wrapper.hpp"
#include "resource_traits/keystone/keystone_handle.hpp"

#include "exception.hpp"

namespace nkg {

    class keystone_assembler {
    public:
        class backend_error : public ::nkg::exception {
        public:
            using error_code_t = ks_err;

        private:
            error_code_t m_error_code;
            std::string m_error_string;

        public:
            backend_error(std::string_view file, int line, error_code_t keystone_err, std::string_view message) noexcept :
                ::nkg::exception(file, line, message), m_error_code(keystone_err), m_error_string(ks_strerror(keystone_err)) {}

            [[nodiscard]]
            virtual bool error_code_exists() const noexcept override {
                return true;
            }

            [[nodiscard]]
            virtual intptr_t error_code() const noexcept override {
                return m_error_code;
            }

            [[nodiscard]]
            virtual const std::string& error_string() const noexcept override {
                return m_error_string;
            }
        };

    private:
        resource_wrapper<resource_traits::keystone::keystone_handle> m_keystone_engine;

    public:
        keystone_assembler(ks_arch architecture, ks_mode mode);

        void option(ks_opt_type option_type, size_t option_value);

        std::vector<uint8_t> assemble(std::string_view asm_string, uint64_t asm_address = 0) const;
    };

}
