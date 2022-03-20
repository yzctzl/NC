#include "keystone_assembler.hpp"

#define NKG_CURRENT_SOURCE_FILE() u8".\\navicat-patcher\\keystone_assembler.cpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {

    keystone_assembler::keystone_assembler(ks_arch architecture, ks_mode mode) {
        auto err = ks_open(architecture, mode, m_keystone_engine.unsafe_addressof());
        if (err != KS_ERR_OK) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"ks_open failed.");
        }
    }

    void keystone_assembler::option(ks_opt_type option_type, size_t option_value) {
        auto err = ks_option(m_keystone_engine.get(), option_type, option_value);
        if (err != KS_ERR_OK) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"ks_option failed.");
        }
    }

    std::vector<uint8_t> keystone_assembler::assemble(std::string_view asm_string, uint64_t asm_address) const {
        resource_wrapper machine_code{ resource_traits::keystone::keystone_alloc{} };
        size_t machine_code_size = 0;
        size_t stat_count = 0;

        if (ks_asm(m_keystone_engine.get(), asm_string.data(), asm_address, machine_code.unsafe_addressof(), &machine_code_size, &stat_count) < 0) {
            auto err = ks_errno(m_keystone_engine.get());
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"ks_option failed.");
        }

        return std::vector<uint8_t>(machine_code.get(), machine_code.get() + machine_code_size);
    }

}

#undef NKG_CURRENT_SOURCE_LINE
#undef NKG_CURRENT_SOURCE_FILE
