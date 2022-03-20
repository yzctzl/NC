#define _CRT_SECURE_NO_WARNINGS
#include "amd64_emulator.hpp"
#include "exceptions/key_exception.hpp"

#define NKG_CURRENT_SOURCE_FILE() u8".\\navicat-patcher\\amd64_emulator.cpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {

    void amd64_emulator::_unicorn_hookcode_cb_stub(uc_engine* uc, uint64_t address, uint32_t size, void* user_data) {
        auto hook_stub_ctx = reinterpret_cast<hook_stub_context_t*>(user_data);
        hook_stub_ctx->self->m_unicorn_hook_cbs_hookcode[hook_stub_ctx->unicorn_hook_handle](address, size);
    }

    void amd64_emulator::_unicorn_hookmem_cb_stub(uc_engine* uc, uc_mem_type type, uint64_t address, int size, int64_t value, void* user_data) {
        auto hook_stub_ctx = reinterpret_cast<hook_stub_context_t*>(user_data);
        hook_stub_ctx->self->m_unicorn_hook_cbs_hookmem[hook_stub_ctx->unicorn_hook_handle](type, address, static_cast<unsigned int>(size), value);
    }

    bool amd64_emulator::_unicorn_eventmem_cb_stub(uc_engine* uc, uc_mem_type type, uint64_t address, int size, int64_t value, void* user_data) {
        auto hook_stub_ctx = reinterpret_cast<hook_stub_context_t*>(user_data);
        return hook_stub_ctx->self->m_unicorn_hook_cbs_eventmem[hook_stub_ctx->unicorn_hook_handle](type, address, static_cast<unsigned int>(size), value);
    }

    amd64_emulator::amd64_emulator() {
        auto err = uc_open(UC_ARCH_X86, UC_MODE_64, m_unicorn_engine.unsafe_addressof());
        if (err != UC_ERR_OK) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_open failed.");
        }
    }

    void amd64_emulator::reg_read(int regid, void* value) {
        auto err = uc_reg_read(m_unicorn_engine.get(), regid, value);
        if (err != UC_ERR_OK) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_reg_read failed.");
        }
    }

    void amd64_emulator::reg_write(int regid, const void* value) {
        auto err = uc_reg_write(m_unicorn_engine.get(), regid, value);
        if (err != UC_ERR_OK) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_reg_write failed.");
        }
    }

    uint64_t amd64_emulator::msr_read(uint32_t rid) {
        uc_x86_msr msr;
        msr.rid = rid;

        auto err = uc_reg_read(m_unicorn_engine.get(), UC_X86_REG_MSR, &msr);
        if (err != UC_ERR_OK) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_reg_write failed.");
        }

        return msr.value;
    }

    void amd64_emulator::msr_write(uint32_t rid, uint64_t value) {
        uc_x86_msr msr;
        msr.rid = rid;
        msr.value = value;

        auto err = uc_reg_write(m_unicorn_engine.get(), UC_X86_REG_MSR, &msr);
        if (err != UC_ERR_OK) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_reg_write failed.");
        }
    }

    void amd64_emulator::mem_map(uint64_t address, size_t size, uint32_t perms) {
        auto err = uc_mem_map(m_unicorn_engine.get(), address, size, perms);
        if (err) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_mem_map failed.");
        }
    }

    void amd64_emulator::mem_unmap(uint64_t address, size_t size) {
        auto err = uc_mem_unmap(m_unicorn_engine.get(), address, size);
        if (err) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_mem_unmap failed.");
        }
    }

    void amd64_emulator::mem_read(uint64_t address, void* buf, size_t size) {
        auto err = uc_mem_read(m_unicorn_engine.get(), address, buf, size);
        if (err) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_mem_read failed.");
        }
    }

    std::vector<uint8_t> amd64_emulator::mem_read(uint64_t address, size_t size) {
        std::vector<uint8_t> ret_buf(size);

        auto err = uc_mem_read(m_unicorn_engine.get(), address, ret_buf.data(), ret_buf.size());
        if (err) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_mem_read failed.");
        }

        return ret_buf;
    }

    void amd64_emulator::mem_write(uint64_t address, const void* buf, size_t size) {
        auto err = uc_mem_write(m_unicorn_engine.get(), address, buf, size);
        if (err) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_mem_write failed.");
        }
    }

    void amd64_emulator::mem_write(uint64_t address, const std::vector<uint8_t>& buf) {
        mem_write(address, buf.data(), buf.size());
    }

    void amd64_emulator::hook_del(uc_hook hook_handle) {
        auto iter_of_hook_stub_ctxs = m_unicorn_hook_stub_ctxs.find(hook_handle);
        if (iter_of_hook_stub_ctxs == m_unicorn_hook_stub_ctxs.end()) {
            throw exceptions::key_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Target hook is not found.");
        }

        auto iter_of_hook_cbs_hookcode = m_unicorn_hook_cbs_hookcode.find(hook_handle);
        if (iter_of_hook_cbs_hookcode != m_unicorn_hook_cbs_hookcode.end()) {
            auto err = uc_hook_del(m_unicorn_engine.get(), hook_handle);
            if (err) {
                throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"hook_del failed.");
            }

            m_unicorn_hook_cbs_hookcode.erase(iter_of_hook_cbs_hookcode);
            m_unicorn_hook_stub_ctxs.erase(iter_of_hook_stub_ctxs);
            return;
        }

        auto iter_of_hook_cbs_hookmem = m_unicorn_hook_cbs_hookmem.find(hook_handle);
        if (iter_of_hook_cbs_hookmem != m_unicorn_hook_cbs_hookmem.end()) {
            auto err = uc_hook_del(m_unicorn_engine.get(), hook_handle);
            if (err) {
                throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"hook_del failed.");
            }

            m_unicorn_hook_cbs_hookmem.erase(iter_of_hook_cbs_hookmem);
            m_unicorn_hook_stub_ctxs.erase(iter_of_hook_stub_ctxs);
            return;
        }

        auto iter_of_hook_cbs_eventmem = m_unicorn_hook_cbs_eventmem.find(hook_handle);
        if (iter_of_hook_cbs_eventmem != m_unicorn_hook_cbs_eventmem.end()) {
            auto err = uc_hook_del(m_unicorn_engine.get(), hook_handle);
            if (err) {
                throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"hook_del failed.");
            }

            m_unicorn_hook_cbs_eventmem.erase(iter_of_hook_cbs_eventmem);
            m_unicorn_hook_stub_ctxs.erase(iter_of_hook_stub_ctxs);
            return;
        }

        __assume(false);
    }

    void amd64_emulator::emu_start(uint64_t begin_address, uint64_t end_address, uint64_t timeout, size_t count) {
        auto err = uc_emu_start(m_unicorn_engine.get(), begin_address, end_address, timeout, count);
        if (err) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"emu_start failed.");
        }
    }

    void amd64_emulator::emu_stop() {
        auto err = uc_emu_stop(m_unicorn_engine.get());
        if (err) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_emu_stop failed.");
        }
    }

    //void amd64_emulator::create_gdt_entry(uint64_t gdt_entry_address, uint32_t base, uint32_t limit, uint8_t access_byte, uint8_t flags) {
    //    struct {
    //        uint16_t limit0;
    //        uint16_t base0;
    //        uint8_t base1;
    //        uint8_t access_byte;
    //        uint8_t limit1 : 4;
    //        uint8_t flags : 4;
    //        uint8_t base2;
    //    } segment_descriptor;

    //    static_assert(sizeof(segment_descriptor) == 8);

    //    segment_descriptor.limit0 = limit & 0xffff;
    //    segment_descriptor.base0 = base & 0xffff;
    //    segment_descriptor.base1 = (base >> 16) & 0xff;
    //    segment_descriptor.access_byte = access_byte;
    //    segment_descriptor.limit1 = (limit >> 16) & 0xf;
    //    segment_descriptor.flags = flags & 0xf;
    //    segment_descriptor.base2 = (base >> 24) & 0xff;

    //    auto err = uc_mem_write(m_unicorn_engine.get(), gdt_entry_address, &segment_descriptor, sizeof(segment_descriptor));
    //    if (err != UC_ERR_OK) {
    //        throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_mem_write failed.");
    //    }
    //}
}

#undef NKG_CURRENT_SOURCE_LINE
#undef NKG_CURRENT_SOURCE_FILE
