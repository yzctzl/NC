#pragma once
#include <winsock2.h>
#include <windows.h>
#include <unicorn/unicorn.h>

#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

#include "resource_wrapper.hpp"
#include "resource_traits/unicorn/unicorn_handle.hpp"

#include "exception.hpp"

#define NKG_CURRENT_SOURCE_FILE() u8".\\navicat-patcher\\i386_emulator.hpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {

    class i386_emulator {
    public:
        class backend_error : public ::nkg::exception {
        public:
            using error_code_t = uc_err;

        private:
            error_code_t m_error_code;
            std::string m_error_string;

        public:
            backend_error(std::string_view file, int line, error_code_t unicorn_err, std::string_view message) noexcept :
                ::nkg::exception(file, line, message), m_error_code(unicorn_err), m_error_string(uc_strerror(unicorn_err)) {}

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

        using hookcode_cb_t = void(uint32_t address, size_t size);
        using hookmem_cb_t = void(uc_mem_type type, uint32_t address, size_t size, int32_t value);
        using eventmem_cb_t = bool(uc_mem_type type, uint32_t address, size_t size, int32_t value);

    private:
        struct hook_stub_context_t {
            i386_emulator* self;
            uc_hook unicorn_hook_handle;
        };

        resource_wrapper<resource_traits::unicorn::unicorn_handle>    m_unicorn_engine;
        std::unordered_map<std::string, std::any>                     m_unicorn_user_ctx;

        std::unordered_map<uc_hook, std::unique_ptr<hook_stub_context_t>>    m_unicorn_hook_stub_ctxs;
        std::unordered_map<uc_hook, std::any>                                m_unicorn_hook_callbacks;

        static void _unicorn_hookcode_cb_stub(uc_engine* uc, uint64_t address, uint32_t size, void* user_data);
        static void _unicorn_hookmem_cb_stub(uc_engine* uc, uc_mem_type type, uint64_t address, int size, int64_t value, void* user_data);
        static bool _unicorn_eventmem_cb_stub(uc_engine* uc, uc_mem_type type, uint64_t address, int size, int64_t value, void* user_data);

    public:
        i386_emulator();

        void reg_read(int regid, void* value);

        void reg_write(int regid, const void* value);

        void mem_map(uint32_t address, size_t size, uint32_t perms);

        void mem_unmap(uint32_t address, size_t size);

        void mem_read(uint32_t address, void* buf, size_t size);

        std::vector<uint8_t> mem_read(uint32_t address, size_t size);

        void mem_write(uint32_t address, const void* buf, size_t size);

        void mem_write(uint32_t address, const std::vector<uint8_t>& buf);

        template<int hook_type, typename callable_t>
        uc_hook hook_add(callable_t&& hook_callback, uint32_t begin_address = 1, uint32_t end_address = 0) {
            uc_err err;

            auto hook_stub_ctx = std::make_unique<hook_stub_context_t>();
            hook_stub_ctx->self = this;
            hook_stub_ctx->unicorn_hook_handle = 0;

            if constexpr (hook_type == UC_HOOK_CODE) {
                err = uc_hook_add(m_unicorn_engine.get(), &hook_stub_ctx->unicorn_hook_handle, hook_type, _unicorn_hookcode_cb_stub, hook_stub_ctx.get(), begin_address, end_address);
                if (err != UC_ERR_OK) {
                    throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_hook_add failed.");
                }

                m_unicorn_hook_callbacks.emplace(std::make_pair(hook_stub_ctx->unicorn_hook_handle, std::function<hookcode_cb_t>{ std::forward<callable_t>(hook_callback) }));
            } else if constexpr ((hook_type & ~UC_HOOK_MEM_VALID) == 0) {
                err = uc_hook_add(m_unicorn_engine.get(), &hook_stub_ctx->unicorn_hook_handle, hook_type, _unicorn_hookmem_cb_stub, hook_stub_ctx.get(), begin_address, end_address);
                if (err != UC_ERR_OK) {
                    throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_hook_add failed.");
                }

                m_unicorn_hook_callbacks.emplace(std::make_pair(hook_stub_ctx->unicorn_hook_handle, std::function<hookmem_cb_t>{ std::forward<callable_t>(hook_callback) }));
            } else if constexpr ((hook_type & ~UC_HOOK_MEM_UNMAPPED) == 0) {
                err = uc_hook_add(m_unicorn_engine.get(), &hook_stub_ctx->unicorn_hook_handle, hook_type, _unicorn_eventmem_cb_stub, hook_stub_ctx.get(), begin_address, end_address);
                if (err != UC_ERR_OK) {
                    throw backend_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), err, u8"uc_hook_add failed.");
                }

                m_unicorn_hook_callbacks.emplace(std::make_pair(hook_stub_ctx->unicorn_hook_handle, std::function<eventmem_cb_t>{ std::forward<callable_t>(hook_callback) }));
            } else {
                static_assert(
                    hook_type == UC_HOOK_CODE ||
                    (hook_type & ~UC_HOOK_MEM_VALID) == 0 ||
                    (hook_type & ~UC_HOOK_MEM_UNMAPPED) == 0, "Unsupported hook type.");
            }

            return m_unicorn_hook_stub_ctxs.emplace(std::make_pair(hook_stub_ctx->unicorn_hook_handle, std::move(hook_stub_ctx))).first->first;
        }

        void hook_del(uc_hook hook_handle);

        void create_gdt_entry(uint32_t gdt_entry_address, uint32_t base, uint32_t limit, uint8_t access_byte, uint8_t flags);

        void emu_start(uint32_t begin_address, uint32_t end_address = 0, uint64_t timeout = 0, size_t count = 0);

        void emu_stop();

        template<typename val_t>
        void context_set(const std::string& name, val_t&& value) {
            m_unicorn_user_ctx[name] = std::forward<val_t>(value);
        }

        template<typename val_t>
        val_t context_get(const std::string& name) {
            return std::any_cast<val_t>(m_unicorn_user_ctx[name]);
        }
    };

}

#undef NKG_CURRENT_SOURCE_LINE
#undef NKG_CURRENT_SOURCE_FILE
