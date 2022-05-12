#define _CRT_SECURE_NO_WARNINGS
#include "amd64_emulator.hpp"
#include "keystone_assembler.hpp"
#include "patch_solution_since_16.0.7.0.hpp"
#include <algorithm>
#include <fmt/format.h>

namespace nkg {

    patch_solution_since<16, 0, 7, 0>::patch_solution_since(image_interpreter& libcc_interpreter) :
        m_libcc_interpreter(libcc_interpreter),
        m_va_CSRegistrationInfoFetcher_WIN_vtable(0),
        m_va_CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey(0),
        m_va_iat_entry_malloc(0) {}

    bool patch_solution_since<16, 0, 7, 0>::find_patch() {
        auto CSRegistrationInfoFetcher_WIN_type_descriptor_name = 
            m_libcc_interpreter.search_section<const uint8_t*>(
                ".data", 
                [](const uint8_t* p, size_t s) {
                    if (s < sizeof(".?AVCSRegistrationInfoFetcher_WIN@@")) {
                        return false;
                    }

                    return strcmp(reinterpret_cast<const char*>(p), ".?AVCSRegistrationInfoFetcher_WIN@@") == 0;
                }
            );

        if (CSRegistrationInfoFetcher_WIN_type_descriptor_name == nullptr) {
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: RTTI info for CSRegistrationInfoFetcher_WIN is not found. (failure label 0)\n");
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: This patch solution will be suppressed.\n");
            return false;
        }

        auto CSRegistrationInfoFetcher_WIN_rtti_type_descriptor = CSRegistrationInfoFetcher_WIN_type_descriptor_name - 0x10;

        auto CSRegistrationInfoFetcher_WIN_rtti_type_descriptor_rva = m_libcc_interpreter.convert_ptr_to_rva(CSRegistrationInfoFetcher_WIN_rtti_type_descriptor);

        auto CSRegistrationInfoFetcher_WIN_rtti_complete_object_locator_pTypeDescriptor =
            m_libcc_interpreter.search_section<const uint8_t*>(
                ".rdata", 
                [this, CSRegistrationInfoFetcher_WIN_rtti_type_descriptor_rva](const uint8_t* p, size_t s) {
                    if (reinterpret_cast<uintptr_t>(p) % sizeof(uint32_t) != 0) {
                        return false;
                    }

                    if (s < sizeof(uint32_t)) {
                        return false;
                    }

                    if (*reinterpret_cast<const uint32_t*>(p) != CSRegistrationInfoFetcher_WIN_rtti_type_descriptor_rva) {
                        return false;
                    }

                    if (s < sizeof(uint32_t) * 2) {
                        return false;
                    }

                    auto maybe_CSRegistrationInfoFetcher_WIN_rtti_class_hierarchy_descriptor_rva = reinterpret_cast<const uint32_t*>(p)[1];

                    try {
                        return memcmp(m_libcc_interpreter.image_section_header_from_rva(maybe_CSRegistrationInfoFetcher_WIN_rtti_class_hierarchy_descriptor_rva)->Name, ".rdata\x00\x00", 8) == 0;
                    } catch (nkg::exception&) {
                        return false;
                    }
                }
            );

        if (CSRegistrationInfoFetcher_WIN_rtti_complete_object_locator_pTypeDescriptor == nullptr) {
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: RTTI info for CSRegistrationInfoFetcher_WIN is not found. (failure label 1)\n");
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: This patch solution will be suppressed.\n");
            return false;
        }

        auto CSRegistrationInfoFetcher_WIN_rtti_complete_object_locator = CSRegistrationInfoFetcher_WIN_rtti_complete_object_locator_pTypeDescriptor - 0xC;

        auto CSRegistrationInfoFetcher_WIN_rtti_complete_object_locator_va = m_libcc_interpreter.convert_ptr_to_va(CSRegistrationInfoFetcher_WIN_rtti_complete_object_locator);

        auto CSRegistrationInfoFetcher_WIN_vtable_before =
            m_libcc_interpreter.search_section<const uint8_t*>(
                ".rdata",
                [CSRegistrationInfoFetcher_WIN_rtti_complete_object_locator_va](const uint8_t* p, size_t s) {
                    if (reinterpret_cast<uintptr_t>(p) % sizeof(uint64_t) != 0) {
                        return false;
                    }

                    if (s < sizeof(uint64_t)) {
                        return false;
                    }

                    return *reinterpret_cast<const uint64_t*>(p) == CSRegistrationInfoFetcher_WIN_rtti_complete_object_locator_va;
                }
            );

        if (CSRegistrationInfoFetcher_WIN_vtable_before == nullptr) {
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: Vftable for CSRegistrationInfoFetcher_WIN is not found.\n");
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: This patch solution will be suppressed.\n");
            return false;
        }

        auto CSRegistrationInfoFetcher_WIN_vtable = 
            reinterpret_cast<const image_interpreter::va_t*>(CSRegistrationInfoFetcher_WIN_vtable_before + sizeof(image_interpreter::va_t));

        m_va_CSRegistrationInfoFetcher_WIN_vtable = m_libcc_interpreter.convert_ptr_to_va(CSRegistrationInfoFetcher_WIN_vtable);
        m_va_CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey = CSRegistrationInfoFetcher_WIN_vtable[6];
        wprintf(L"[*] patch_solution_since<16, 0, 7, 0>: m_va_CSRegistrationInfoFetcher_WIN_vtable = 0x%016llx\n", m_va_CSRegistrationInfoFetcher_WIN_vtable);
        wprintf(L"[*] patch_solution_since<16, 0, 7, 0>: m_va_CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey = 0x%016llx\n", m_va_CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey);

        amd64_emulator x64_emulator;

        x64_emulator.context_set("heap_base", uint64_t{ 0x00007fff00000000 });
        x64_emulator.context_set("heap_size", size_t{ 0x1000 * 32 });
        x64_emulator.context_set("heap_records", std::map<uint64_t, uint64_t>{});

        x64_emulator.context_set("stack_base", uint64_t{ 0x00007fffffff0000 });
        x64_emulator.context_set("stack_size", size_t{ 0x1000 * 32 });
        x64_emulator.context_set("stack_top", uint64_t{ x64_emulator.context_get<uint64_t>("stack_base") - x64_emulator.context_get<size_t>("stack_size") });        

        x64_emulator.context_set("dead_area_base", uint64_t{ 0xfffffffffffff000 });
        x64_emulator.context_set("dead_area_size", size_t{ 0x1000 });

        x64_emulator.context_set("iat_base", uint64_t{ m_libcc_interpreter.convert_rva_to_va(m_libcc_interpreter.image_nt_headers()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress) });
        x64_emulator.context_set("iat_size", size_t{ m_libcc_interpreter.image_nt_headers()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size });

        x64_emulator.context_set("external_api_stub_area_base", uint64_t{ 0xffff800000000000 });
        x64_emulator.context_set("external_api_stub_area_size", size_t{ (x64_emulator.context_get<size_t>("iat_size") / 8 + 0xfff) / 0x1000 * 0x1000 });

        x64_emulator.context_set("external_api_impl", std::map<std::string, uint64_t>{});
        x64_emulator.context_set("external_api_impl_area_base", uint64_t{ 0xffff900000000000 });
        x64_emulator.context_set("external_api_impl_area_size", size_t{ 0 });

        x64_emulator.context_set("gs_base", uint64_t{ 0xffffa00000000000 });
        x64_emulator.context_set("gs_size", size_t{ 0x1000 });

        x64_emulator.context_set("start_address", static_cast<uint64_t>(m_va_CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey));
        x64_emulator.context_set("dead_address", x64_emulator.context_get<uint64_t>("dead_area_base"));

        // allocate heap
        x64_emulator.mem_map(x64_emulator.context_get<uint64_t>("heap_base"), x64_emulator.context_get<size_t>("heap_size"), UC_PROT_READ | UC_PROT_WRITE);

        // allocate stack
        x64_emulator.mem_map(x64_emulator.context_get<uint64_t>("stack_top"), x64_emulator.context_get<size_t>("stack_size"), UC_PROT_READ | UC_PROT_WRITE);

        // allocate dead area
        x64_emulator.mem_map(x64_emulator.context_get<uint64_t>("dead_area_base"), x64_emulator.context_get<size_t>("dead_area_size"), UC_PROT_READ | UC_PROT_EXEC);

        // allocate and hook read access to IAT
        {
            auto iat_base = x64_emulator.context_get<uint64_t>("iat_base");
            auto iat_size = x64_emulator.context_get<size_t>("iat_size");
            auto external_api_stub_area_base = x64_emulator.context_get<uint64_t>("external_api_stub_area_base");

            auto iat_page_base = iat_base / 0x1000 * 0x1000;
            auto iat_page_count = (iat_base - iat_page_base + iat_size + 0xfff) / 0x1000;

            x64_emulator.mem_map(iat_page_base, iat_page_count * 0x1000, UC_PROT_READ);
            
            x64_emulator.hook_add<UC_HOOK_MEM_READ>(
                [this, &x64_emulator, iat_base, external_api_stub_area_base](uc_mem_type type, uint64_t address, size_t size, int64_t value) {
                    auto rva = m_libcc_interpreter.convert_va_to_rva(address);
                    auto import_lookup_entry = m_libcc_interpreter.import_lookup_entry_from_rva(rva);

                    if (import_lookup_entry && !IMAGE_SNAP_BY_ORDINAL(import_lookup_entry->u1.Ordinal)) {
                        auto import_by_name_entry = m_libcc_interpreter.convert_rva_to_ptr<PIMAGE_IMPORT_BY_NAME>(import_lookup_entry->u1.AddressOfData);
                        if (strcmp(import_by_name_entry->Name, "memcpy") == 0) {
                            uint64_t impl_address = x64_emulator.context_get<std::map<std::string, uint64_t>&>("external_api_impl")["memcpy"];
                            x64_emulator.mem_write(address, &impl_address, sizeof(impl_address));
                        } else if (strcmp(import_by_name_entry->Name, "memcmp") == 0) {
                            uint64_t impl_address = x64_emulator.context_get<std::map<std::string, uint64_t>&>("external_api_impl")["memcmp"];
                            x64_emulator.mem_write(address, &impl_address, sizeof(impl_address));
                        } else {
                            uint64_t stub_address = external_api_stub_area_base + (address - iat_base) / sizeof(IMAGE_THUNK_DATA);
                            x64_emulator.mem_write(address, &stub_address, sizeof(stub_address));
                        }
                    } else {
                        x64_emulator.emu_stop();
                    }
                },
                iat_base,
                iat_base + iat_size - 1
            );
        }

        // allocate and setup external api stub area
        {
            auto external_api_stub_area_base = x64_emulator.context_get<uint64_t>("external_api_stub_area_base");
            auto external_api_stub_area_size = x64_emulator.context_get<size_t>("external_api_stub_area_size");

            x64_emulator.mem_map(external_api_stub_area_base, external_api_stub_area_size, UC_PROT_READ | UC_PROT_EXEC);
            x64_emulator.mem_write(external_api_stub_area_base, std::vector<uint8_t>(external_api_stub_area_size, 0xc3));   // c3 -> ret

            x64_emulator.hook_add<UC_HOOK_CODE>(
                [this, &x64_emulator, external_api_stub_area_base](uint64_t address, size_t size) {
                    auto iat_base = x64_emulator.context_get<uint64_t>("iat_base");
                    auto from_va = iat_base + (address - external_api_stub_area_base) * sizeof(IMAGE_THUNK_DATA);
                    auto from_rva = m_libcc_interpreter.convert_va_to_rva(from_va);

                    auto import_lookup_entry = m_libcc_interpreter.import_lookup_entry_from_rva(from_rva);
                    if (import_lookup_entry && !IMAGE_SNAP_BY_ORDINAL(import_lookup_entry->u1.Ordinal)) {
                        auto import_by_name_entry = m_libcc_interpreter.convert_rva_to_ptr<PIMAGE_IMPORT_BY_NAME>(import_lookup_entry->u1.AddressOfData);
                        if (strcmp(import_by_name_entry->Name, "malloc") == 0) {
                            m_va_iat_entry_malloc = from_va;

                            uint64_t alloc_size;
                            x64_emulator.reg_read(UC_X86_REG_RCX, &alloc_size);

                            auto& heap_records = x64_emulator.context_get<std::map<uint64_t, uint64_t>&>("heap_records");

                            auto predecessor_chunk =
                                std::adjacent_find(
                                    heap_records.begin(), 
                                    heap_records.end(),
                                    [alloc_size](const auto& chunk0, const auto& chunk1) { return chunk1.first - (chunk0.first + chunk0.second) >= alloc_size; }
                                );

                            uint64_t alloc_p;
                            if (predecessor_chunk != heap_records.end()) {
                                alloc_p = predecessor_chunk->first + predecessor_chunk->second;
                            } else {
                                auto heap_base = x64_emulator.context_get<uint64_t>("heap_base");
                                auto heap_size = x64_emulator.context_get<uint64_t>("heap_size");

                                auto free_space_base = heap_records.size() > 0 ? heap_records.rbegin()->first + heap_records.rbegin()->second : heap_base;
                                auto free_space_size = heap_base + heap_size - free_space_base;

                                if (free_space_size < alloc_size) {
                                    auto heap_expand_base = heap_base + heap_size;
                                    auto heap_expand_size = (alloc_size - free_space_size + 0xfff) / 0x1000 * 0x1000;
                                    x64_emulator.mem_map(heap_expand_base, heap_expand_size, UC_PROT_READ | UC_PROT_WRITE);
                                }

                                alloc_p = free_space_base;
                            }

                            heap_records[alloc_p] = alloc_size;

                            x64_emulator.reg_write(UC_X86_REG_RAX, &alloc_p);
                        } else if (strcmp(import_by_name_entry->Name, "free") == 0) {
                            uint64_t alloc_p;
                            x64_emulator.reg_read(UC_X86_REG_RCX, &alloc_p);

                            auto& heap_records = x64_emulator.context_get<std::map<uint64_t, uint64_t>&>("heap_records");

                            auto chunk = heap_records.find(alloc_p);
                            if (chunk != heap_records.end()) {
                                heap_records.erase(chunk);
                            } else {
                                x64_emulator.emu_stop();
                            }
                        } else {
                            x64_emulator.emu_stop();
                        }
                    } else {
                        x64_emulator.emu_stop();
                    }
                },
                external_api_stub_area_base,
                external_api_stub_area_base + external_api_stub_area_size - 1
            );
        }

        // allocate and setup external api impl area
        {
            keystone_assembler x64_assembler{ KS_ARCH_X86, KS_MODE_64 };

            std::map<std::string, std::vector<uint8_t>> machine_code_list =
                {
                    std::make_pair(
                        "memcpy", 
                        x64_assembler.assemble(
                            "push rdi;"
                            "push rsi;"
                            "mov rdi, rcx;"
                            "mov rsi, rdx;"
                            "mov rcx, r8;"
                            "rep movs byte ptr [rdi], byte ptr [rsi];"
                            "pop rsi;"
                            "pop rdi;"
                            "ret;"
                        )
                    ),
                    std::make_pair(
                        "memcmp",
                        x64_assembler.assemble(
                            "    push rdi;"
                            "    push rsi;"
                            "    mov rsi, rcx;"
                            "    mov rdi, rdx;"
                            "    mov rcx, r8;"
                            "    cmp rcx, rcx;"
                            "    repe cmps byte ptr [rsi], byte ptr [rdi];"
                            "    jz cmp_eq;"
                            "cmp_not_eq:"
                            "    movsx eax, byte ptr [rsi - 1];"
                            "    movsx ecx, byte ptr [rdi - 1];"
                            "    sub eax, ecx;"
                            "    jmp final;"
                            "cmp_eq:"
                            "    xor eax, eax;"
                            "final:"
                            "    pop rsi;"
                            "    pop rdi;"
                            "    ret;"
                        )
                    ),
                    std::make_pair(
                        "memmove",
                        x64_assembler.assemble(
                            "    push rdi;"
                            "    push rsi;"
                            "    cmp rdx, rcx;"
                            "    jb reverse_copy;"
                            "copy:"
                            "    mov rdi, rcx;"
                            "    mov rsi, rdx;"
                            "    mov rcx, r8;"
                            "    rep movsb byte ptr[rdi], byte ptr[rsi];"
                            "    jmp final;"
                            "reverse_copy:"
                            "    std;"
                            "    lea rdi, qword ptr[rcx + r8 - 1];"
                            "    lea rsi, qword ptr[rdx + r8 - 1];"
                            "    mov rcx, r8;"
                            "    rep movsb byte ptr[rdi], byte ptr[rsi];"
                            "    cld;"
                            "final:"
                            "    pop rsi;"
                            "    pop rdi;"
                            "    ret;"
                        )
                    )
                };

            auto& external_api_impl = x64_emulator.context_get<std::map<std::string, uint64_t>&>("external_api_impl");
            auto& external_api_impl_area_base = x64_emulator.context_get<uint64_t&>("external_api_impl_area_base");
            auto& external_api_impl_area_size = x64_emulator.context_get<size_t&>("external_api_impl_area_size");

            auto p = external_api_impl_area_base;
            for (const auto& name_code_pair : machine_code_list) {
                external_api_impl[name_code_pair.first] = p;
                p = (p + name_code_pair.second.size() + 0xf) / 0x10 * 0x10;
            }

            external_api_impl_area_size = (p + 0xfff) / 0x1000 * 0x1000 - external_api_impl_area_base;

            x64_emulator.mem_map(external_api_impl_area_base, external_api_impl_area_size, UC_PROT_READ | UC_PROT_EXEC);
            for (const auto& name_code_pair : machine_code_list) {
                x64_emulator.mem_write(external_api_impl[name_code_pair.first], name_code_pair.second);
            }
        }

        // allocate and hook access to gs area
        x64_emulator.mem_map(x64_emulator.context_get<uint64_t>("gs_base"), x64_emulator.context_get<size_t>("gs_size"), UC_PROT_READ | UC_PROT_WRITE);
        x64_emulator.msr_write(0xC0000101, x64_emulator.context_get<uint64_t>("gs_base"));  // set gs base address

        x64_emulator.hook_add<UC_HOOK_MEM_READ>(
            [this, &x64_emulator](uc_mem_type access, uint64_t address, size_t size, int64_t value) {
                auto gs_base = x64_emulator.context_get<uint64_t>("gs_base");
                switch (address - gs_base) {
                    case 0x10:  // qword ptr gs:[0x10] -> Stack Limit / Ceiling of stack (low address)
                        {    
                            uint64_t val = x64_emulator.context_get<uint64_t>("stack_top");
                            x64_emulator.mem_write(address, &val, size);  
                        }
                        break;
                    default:
                        x64_emulator.emu_stop();
                        break;
                }
            },
            x64_emulator.context_get<uint64_t>("gs_base"),
            x64_emulator.context_get<uint64_t>("gs_base") + x64_emulator.context_get<size_t>("gs_size") - 1
        );

        // x64_emulator.hook_add<UC_HOOK_CODE>([](uint64_t address, size_t size) { wprintf_s(L"code_trace, address = 0x%016zx\n", address); });

        x64_emulator.hook_add<UC_HOOK_MEM_UNMAPPED>(
            [this, &x64_emulator](uc_mem_type access, uint64_t address, size_t size, int64_t value) -> bool {
                try {
                    auto fault_section = m_libcc_interpreter.image_section_header_from_va(address);

                    auto page_base = address / 0x1000 * 0x1000;
                    auto page_size = 0x1000;
                    uint32_t page_perms = UC_PROT_NONE;

                    if (fault_section->Characteristics & IMAGE_SCN_MEM_READ) {
                        page_perms |= UC_PROT_READ;
                    }
                    if (fault_section->Characteristics & IMAGE_SCN_MEM_WRITE) {
                        page_perms |= UC_PROT_WRITE;
                    }
                    if (fault_section->Characteristics & IMAGE_SCN_MEM_EXECUTE) {
                        page_perms |= UC_PROT_EXEC;
                    }

                    x64_emulator.mem_map(page_base, page_size, page_perms);
                    x64_emulator.mem_write(page_base, m_libcc_interpreter.convert_va_to_ptr<const void*>(page_base), page_size);

                    return true;
                } catch (::nkg::exception&) {
                    return false;
                }
            }
        );
        
        // set rbp, rsp
        uint64_t init_rbp = x64_emulator.context_get<uint64_t>("stack_base") - x64_emulator.context_get<size_t>("stack_size") / 4;
        uint64_t init_rsp = x64_emulator.context_get<uint64_t>("stack_base") - x64_emulator.context_get<size_t>("stack_size") / 2;

        x64_emulator.reg_write(UC_X86_REG_RBP, &init_rbp);
        x64_emulator.reg_write(UC_X86_REG_RSP, &init_rsp);

        // set return address
        auto retaddr = x64_emulator.context_get<uint64_t>("dead_address");
        x64_emulator.mem_write(init_rsp, &retaddr, sizeof(retaddr));

        // set argument registers
        uint64_t init_rcx = 0;                  // `this` pointer of CSRegistrationInfoFetcher_WIN, but we don't need it for now.
        uint64_t init_rdx = init_rsp + 0x40;    // a pointer to stack memory which stores return value
        x64_emulator.reg_write(UC_X86_REG_RCX, &init_rcx);
        x64_emulator.reg_write(UC_X86_REG_RDX, &init_rdx);

        // 
        // start emulate
        // 
        try {
            x64_emulator.emu_start(x64_emulator.context_get<uint64_t>("start_address"), x64_emulator.context_get<uint64_t>("dead_address"));
        } catch (nkg::exception&) {
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: Code emulation failed.\n");
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: This patch solution will be suppressed.\n");
            return false;
        }

        wprintf_s(L"[*] patch_solution_since<16, 0, 7, 0>: m_va_iat_entry_malloc = 0x%016llx\n", m_va_iat_entry_malloc);

        //
        // get result
        // 
        // on AMD64 platform, `std::string` has follow memory layout:
        //     ------------------------------
        //     | offset | size |
        //     ------------------------------
        //     | +0     | 0x10 | `char[16]: a small string buffer` OR `char*: a large string buffer pointer`
        //     ------------------------------
        //     | +0x10  | 0x8  | size_t: string length
        //     ------------------------------
        //     | +0x18  | 0x8  | size_t: capacity
        //     ------------------------------
        //
        uint64_t encoded_key_length;
        x64_emulator.mem_read(init_rdx + 0x10, &encoded_key_length, sizeof(encoded_key_length));
        if (encoded_key_length != official_encoded_key.length()) {
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: unexpected encoded key length(%llu).\n", encoded_key_length);
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: This patch solution will be suppressed.\n");
            return false;
        }

        uint64_t encoded_key_ptr;
        x64_emulator.mem_read(init_rdx, &encoded_key_ptr, sizeof(encoded_key_ptr));

        auto encoded_key = x64_emulator.mem_read(encoded_key_ptr, encoded_key_length);
        if (memcmp(encoded_key.data(), official_encoded_key.data(), encoded_key.size()) == 0) {
            wprintf_s(L"[+] patch_solution_since<16, 0, 7, 0>: official encoded key is found.\n");
            return true;
        } else {
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: official encoded key is not found.\n");
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: This patch solution will be suppressed.\n");
            return false;
        }
    }

    bool patch_solution_since<16, 0, 7, 0>::check_rsa_privkey(const rsa_cipher& cipher) {
        return true;    // no requirements
    }

    void patch_solution_since<16, 0, 7, 0>::make_patch(const rsa_cipher& cipher) {
        auto encoded_key = _build_encoded_key(cipher);

        auto CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey = 
            m_libcc_interpreter.convert_va_to_ptr<uint8_t*>(m_va_CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey);

        std::vector<std::string> patch_code_chunks;
        patch_code_chunks.emplace_back("push rdi;");
        patch_code_chunks.emplace_back("push rsi;");
        patch_code_chunks.emplace_back("push rbx;");
        patch_code_chunks.emplace_back("push rbp;");
        patch_code_chunks.emplace_back("mov rbp, rsp;");
        patch_code_chunks.emplace_back("mov rbx, rdx;");
        patch_code_chunks.emplace_back("sub rsp, 0x20;");
        patch_code_chunks.emplace_back(fmt::format("mov rcx, {:#x};", encoded_key.length() + 1));
        patch_code_chunks.emplace_back(fmt::format("call qword ptr [{:#016x}];", m_va_iat_entry_malloc));
        patch_code_chunks.emplace_back("add rsp, 0x20;");
        {
            std::vector<uint64_t> push_values((encoded_key.length() + 1 + 7) / 8, 0);
            memcpy(push_values.data(), encoded_key.data(), encoded_key.length());

            std::for_each(
                push_values.crbegin(), 
                push_values.crend(), 
                [&patch_code_chunks](uint64_t x) {
                    patch_code_chunks.emplace_back(fmt::format("mov rdx, {:#016x};", x));
                    patch_code_chunks.emplace_back("push rdx;");
                }
            );
        }
        patch_code_chunks.emplace_back("mov rdi, rax;");
        patch_code_chunks.emplace_back("mov rsi, rsp;");
        patch_code_chunks.emplace_back(fmt::format("mov rcx, {:#x};", encoded_key.length() + 1));
        patch_code_chunks.emplace_back("rep movs byte ptr [rdi], byte ptr [rsi];");
        patch_code_chunks.emplace_back("mov qword ptr [rbx], rax;");
        patch_code_chunks.emplace_back(fmt::format("mov qword ptr [rbx + 0x10], {:#x};", encoded_key.length()));
        patch_code_chunks.emplace_back(fmt::format("mov qword ptr [rbx + 0x18], {:#x};", encoded_key.length() + 1));
        patch_code_chunks.emplace_back("mov rax, rbx;");
        patch_code_chunks.emplace_back("leave;");
        patch_code_chunks.emplace_back("pop rbx;");
        patch_code_chunks.emplace_back("pop rsi;");
        patch_code_chunks.emplace_back("pop rdi;");
        patch_code_chunks.emplace_back("ret;");

        //auto patch_code = keystone_assembler{ KS_ARCH_X86, KS_MODE_64 }
        //    .assemble(
        //        fmt::format(
        //            "    push rdi;"
        //            "    push rsi;"
        //            "    push rbx;"
        //            "    mov rbx, rdx;"
        //            "allocate_string_buf:"
        //            "    mov rcx, {encoded_key_length:#x} + 1;"
        //            "    sub rsp, 0x20;"
        //            "    call qword ptr [{m_va_iat_entry_malloc:#x}];"
        //            "    add rsp, 0x20;"
        //            "write_our_own_key_to_string_buf:"
        //            "    mov rdi, rax;"
        //            "    lea rsi, qword ptr [end_of_code + rip];"
        //            "    mov rcx, 0x188;"
        //            "    rep movs byte ptr [rdi], byte ptr [rsi];"
        //            "    mov byte ptr [rdi], 0;"
        //            "craft_std_string:"
        //            "    mov qword ptr [rbx], rax;"
        //            "    mov qword ptr [rbx + 0x10], {encoded_key_length:#x};"
        //            "    mov qword ptr [rbx + 0x18], {encoded_key_length:#x} + 1;"
        //            "final:"
        //            "    mov rax, rbx;"
        //            "    pop rbx;"
        //            "    pop rsi;"
        //            "    pop rdi;"
        //            "    ret;"
        //            "end_of_code:",
        //            fmt::arg("encoded_key_length", encoded_key.length()),
        //            fmt::arg("m_va_iat_entry_malloc", m_va_iat_entry_malloc)
        //        ),
        //        m_va_CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey
        //    );

        std::vector<uint8_t> assembled_patch_code;
        {
            keystone_assembler x86_assembler{ KS_ARCH_X86, KS_MODE_64 };

            auto current_va = m_va_CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey;
            auto next_reloc = m_libcc_interpreter.relocation_distribute().lower_bound(m_libcc_interpreter.convert_va_to_rva(current_va));
            for (const auto& patch_code_chunk : patch_code_chunks) {
                auto assembled_patch_code_chunk = x86_assembler.assemble(patch_code_chunk, current_va);

                while (true) {
                    auto next_reloc_va = m_libcc_interpreter.convert_rva_to_va(next_reloc->first);
                    auto next_reloc_size = next_reloc->second;

                    if (current_va + assembled_patch_code_chunk.size() + 2 <= next_reloc_va) {     // 2 -> size of machine code "jmp rel8"
                        assembled_patch_code.insert(assembled_patch_code.end(), assembled_patch_code_chunk.begin(), assembled_patch_code_chunk.end());
                        current_va += assembled_patch_code_chunk.size();
                        break;
                    } else if (current_va + 2 <= next_reloc_va) {
                        auto next_va = next_reloc_va + next_reloc_size;
                        auto assembled_jmp = x86_assembler.assemble(fmt::format("jmp {:#016x};", next_va), current_va);
                        auto assembled_padding = std::vector<uint8_t>(next_va - (current_va + assembled_jmp.size()), 0xcc);     // 0xcc -> int3
                        assembled_patch_code.insert(assembled_patch_code.end(), assembled_jmp.begin(), assembled_jmp.end());
                        assembled_patch_code.insert(assembled_patch_code.end(), assembled_padding.begin(), assembled_padding.end());
                        current_va = next_va;
                        ++next_reloc;
                    } else {
                        __assume(false);    // impossible to reach here
                    }
                }
            }
        }

        memcpy(CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey, assembled_patch_code.data(), assembled_patch_code.size());
        wprintf_s(L"[*] patch_solution_since<16, 0, 7, 0>: Patch has been done.\n");
    }

}
