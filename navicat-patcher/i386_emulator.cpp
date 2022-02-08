#define _CRT_SECURE_NO_WARNINGS
#include "i386_emulator.hpp"
#include "exceptions/key_exception.hpp"

#define NKG_CURRENT_SOURCE_FILE() u8".\\navicat-patcher\\i386_emulator.cpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {

    void i386_emulator::_unicorn_hookcode_cb_stub(uc_engine* uc, uint64_t address, uint32_t size, void* user_data) {
        auto hook_stub_ctx = 
            reinterpret_cast<hook_stub_context_t*>(user_data);

        auto& hook_callback = 
            std::any_cast<std::function<hookcode_cb_t>&>(hook_stub_ctx->self->m_unicorn_hook_callbacks[hook_stub_ctx->unicorn_hook_handle]);

        hook_callback(static_cast<uint32_t>(address), size);
    }

    void i386_emulator::_unicorn_hookmem_cb_stub(uc_engine* uc, uc_mem_type type, uint64_t address, int size, int64_t value, void* user_data) {
        auto hook_stub_ctx = 
            reinterpret_cast<hook_stub_context_t*>(user_data);

        auto& hook_callback =
            std::any_cast<std::function<hookmem_cb_t>&>(hook_stub_ctx->self->m_unicorn_hook_callbacks[hook_stub_ctx->unicorn_hook_handle]);

        hook_callback(type, static_cast<uint32_t>(address), static_cast<unsigned int>(size), static_cast<int32_t>(value));
    }

    bool i386_emulator::_unicorn_eventmem_cb_stub(uc_engine* uc, uc_mem_type type, uint64_t address, int size, int64_t value, void* user_data) {
        auto hook_stub_ctx = 
            reinterpret_cast<hook_stub_context_t*>(user_data);

        auto& hook_callback =
            std::any_cast<std::function<eventmem_cb_t>&>(hook_stub_ctx->self->m_unicorn_hook_callbacks[hook_stub_ctx->unicorn_hook_handle]);

        return hook_callback(type, static_cast<uint32_t>(address), static_cast<unsigned int>(size), static_cast<int32_t>(value));
    }

    i386_emulator::i386_emulator() {
        auto err = uc_open(UC_ARCH_X86, UC_MODE_32, m_unicorn_engine.unsafe_addressof());
        if (err != UC_ERR_OK) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_open failed.");
        }
    }

    void i386_emulator::reg_read(int regid, void* value) {
        auto err = uc_reg_read(m_unicorn_engine.get(), regid, value);
        if (err != UC_ERR_OK) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_reg_read failed.");
        }
    }

    void i386_emulator::reg_write(int regid, const void* value) {
        auto err = uc_reg_write(m_unicorn_engine.get(), regid, value);
        if (err != UC_ERR_OK) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_reg_write failed.");
        }
    }

    void i386_emulator::mem_map(uint32_t address, size_t size, uint32_t perms) {
        auto err = uc_mem_map(m_unicorn_engine.get(), address, size, perms);
        if (err) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_mem_map failed.");
        }
    }

    void i386_emulator::mem_unmap(uint32_t address, size_t size) {
        auto err = uc_mem_unmap(m_unicorn_engine.get(), address, size);
        if (err) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_mem_unmap failed.");
        }
    }

    void i386_emulator::mem_read(uint32_t address, void* buf, size_t size) {
        auto err = uc_mem_read(m_unicorn_engine.get(), address, buf, size);
        if (err) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_mem_read failed.");
        }
    }

    std::vector<uint8_t> i386_emulator::mem_read(uint32_t address, size_t size) {
        std::vector<uint8_t> ret_buf(size);

        auto err = uc_mem_read(m_unicorn_engine.get(), address, ret_buf.data(), ret_buf.size());
        if (err) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_mem_read failed.");
        }

        return ret_buf;
    }

    void i386_emulator::mem_write(uint32_t address, const void* buf, size_t size) {
        auto err = uc_mem_write(m_unicorn_engine.get(), address, buf, size);
        if (err) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_mem_write failed.");
        }
    }

    void i386_emulator::mem_write(uint32_t address, const std::vector<uint8_t>& buf) {
        mem_write(address, buf.data(), buf.size());
    }

    void i386_emulator::hook_del(uc_hook hook_handle) {
        auto iter_of_hook_stub_ctxs = m_unicorn_hook_stub_ctxs.find(hook_handle);
        if (iter_of_hook_stub_ctxs == m_unicorn_hook_stub_ctxs.end()) {
            throw exceptions::key_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Target hook is not found.");
        }

        auto iter_of_hook_callbacks = m_unicorn_hook_callbacks.find(hook_handle);
        if (iter_of_hook_callbacks != m_unicorn_hook_callbacks.end()) {
            auto err = uc_hook_del(m_unicorn_engine.get(), hook_handle);
            if (err) {
                throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"hook_del failed.");
            }

            m_unicorn_hook_callbacks.erase(iter_of_hook_callbacks);
            m_unicorn_hook_stub_ctxs.erase(iter_of_hook_stub_ctxs);
            return;
        }

        __assume(false);
    }

    void i386_emulator::create_gdt_entry(uint32_t gdt_entry_address, uint32_t base, uint32_t limit, uint8_t access_byte, uint8_t flags) {
        struct {
            uint64_t limit0 : 16;
            uint64_t base0 : 24;
            uint64_t access_byte : 8;
            uint64_t limit1 : 4;
            uint64_t flags : 4;
            uint64_t base1 : 8;
        } gdt_entry;

        gdt_entry.limit0 = limit & 0xffff;
        gdt_entry.base0 = base & 0xffffff;
        gdt_entry.access_byte = access_byte;
        gdt_entry.flags = flags & 0xf;
        gdt_entry.base1 = (base & 0xff000000) >> 24;

        mem_write(gdt_entry_address, &gdt_entry, sizeof(gdt_entry));
    }

    void i386_emulator::emu_start(uint32_t begin_address, uint32_t end_address, uint64_t timeout, size_t count) {
        auto err = uc_emu_start(m_unicorn_engine.get(), begin_address, end_address, timeout, count);
        if (err) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"emu_start failed.");
        }
    }

    void i386_emulator::emu_stop() {
        auto err = uc_emu_stop(m_unicorn_engine.get());
        if (err) {
            throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_emu_stop failed.");
        }
    }
}

#undef NKG_CURRENT_SOURCE_LINE
#undef NKG_CURRENT_SOURCE_FILE
