#define _CRT_SECURE_NO_WARNINGS
#include "i386_emulator.hpp"
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

        auto CSRegistrationInfoFetcher_WIN_rtti_type_descriptor = CSRegistrationInfoFetcher_WIN_type_descriptor_name - 0x8;

        auto CSRegistrationInfoFetcher_WIN_rtti_type_descriptor_va = m_libcc_interpreter.convert_ptr_to_va(CSRegistrationInfoFetcher_WIN_rtti_type_descriptor);

        auto CSRegistrationInfoFetcher_WIN_rtti_complete_object_locator_pTypeDescriptor =
            m_libcc_interpreter.search_section<const uint8_t*>(
                ".rdata", 
                [this, CSRegistrationInfoFetcher_WIN_rtti_type_descriptor_va](const uint8_t* p, size_t s) {
                    if (reinterpret_cast<uintptr_t>(p) % sizeof(uint32_t) != 0) {
                        return false;
                    }

                    if (s < sizeof(uint32_t)) {
                        return false;
                    }

                    if (*reinterpret_cast<const uint32_t*>(p) != CSRegistrationInfoFetcher_WIN_rtti_type_descriptor_va) {
                        return false;
                    }

                    if (s < sizeof(uint32_t) * 2) {
                        return false;
                    }

                    auto maybe_CSRegistrationInfoFetcher_WIN_rtti_class_hierarchy_descriptor_va = reinterpret_cast<const uint32_t*>(p)[1];

                    try {
                        return memcmp(m_libcc_interpreter.image_section_header_from_va(maybe_CSRegistrationInfoFetcher_WIN_rtti_class_hierarchy_descriptor_va)->Name, ".rdata\x00\x00", 8) == 0;
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
                    if (reinterpret_cast<uintptr_t>(p) % sizeof(uint32_t) != 0) {
                        return false;
                    }

                    if (s < sizeof(uint32_t)) {
                        return false;
                    }

                    return *reinterpret_cast<const uint32_t*>(p) == CSRegistrationInfoFetcher_WIN_rtti_complete_object_locator_va;
                }
        );

        if (CSRegistrationInfoFetcher_WIN_vtable_before == nullptr) {
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: Vftable for CSRegistrationInfoFetcher_WIN is not found.\n");
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: This patch solution will be suppressed.\n");
            return false;
        }

        auto CSRegistrationInfoFetcher_WIN_vtable =
            reinterpret_cast<const uint32_t*>(CSRegistrationInfoFetcher_WIN_vtable_before + sizeof(uint32_t));

        m_va_CSRegistrationInfoFetcher_WIN_vtable = m_libcc_interpreter.convert_ptr_to_va(CSRegistrationInfoFetcher_WIN_vtable);
        m_va_CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey = CSRegistrationInfoFetcher_WIN_vtable[6];
        wprintf(L"[*] patch_solution_since<16, 0, 7, 0>: m_va_CSRegistrationInfoFetcher_WIN_vtable = 0x%08x\n", m_va_CSRegistrationInfoFetcher_WIN_vtable);
        wprintf(L"[*] patch_solution_since<16, 0, 7, 0>: m_va_CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey = 0x%08x\n", m_va_CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey);


        i386_emulator x86_emulator;

        x86_emulator.context_set("heap_base", uint32_t{ 0x7f000000 });
        x86_emulator.context_set("heap_size", size_t{ 0x1000 * 32 });
        x86_emulator.context_set("heap_records", std::map<uint32_t, uint32_t>{});

        x86_emulator.context_set("stack_base", uint32_t{ 0x7fff0000 });
        x86_emulator.context_set("stack_size", size_t{ 0x1000 * 32 });
        x86_emulator.context_set("stack_top", uint32_t{ x86_emulator.context_get<uint32_t>("stack_base") - x86_emulator.context_get<size_t>("stack_size") });

        x86_emulator.context_set("r0_to_r3_stub_area_base", uint32_t{ 0xffffe000 });
        x86_emulator.context_set("r0_to_r3_stub_area_size", size_t{ 0x1000 });

        x86_emulator.context_set("dead_area_base", uint32_t{ 0xfffff000 });
        x86_emulator.context_set("dead_area_size", size_t{ 0x1000 });

        x86_emulator.context_set("iat_base", uint32_t{ m_libcc_interpreter.convert_rva_to_va(m_libcc_interpreter.image_nt_headers()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress) });
        x86_emulator.context_set("iat_size", size_t{ m_libcc_interpreter.image_nt_headers()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size });

        x86_emulator.context_set("external_api_stub_area_base", uint32_t{ 0x80000000 });
        x86_emulator.context_set("external_api_stub_area_size", size_t{ (x86_emulator.context_get<size_t>("iat_size") / 8 + 0xfff) / 0x1000 * 0x1000 });

        x86_emulator.context_set("external_api_impl", std::map<std::string, uint32_t>{});
        x86_emulator.context_set("external_api_impl_area_base", uint32_t{ 0x90000000 });
        x86_emulator.context_set("external_api_impl_area_size", size_t{ 0 });

        x86_emulator.context_set("gdt_base", uint32_t{ 0xffff0000 });
        x86_emulator.context_set("gdt_size", size_t{ 0x1000 });

        x86_emulator.context_set("fs_base", uint32_t{ 0xa0000000 });
        x86_emulator.context_set("fs_size", size_t{ 0x1000 });

        x86_emulator.context_set("start_address", static_cast<uint32_t>(m_va_CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey));
        x86_emulator.context_set("dead_address", x86_emulator.context_get<uint32_t>("dead_area_base"));

        // allocate heap
        x86_emulator.mem_map(x86_emulator.context_get<uint32_t>("heap_base"), x86_emulator.context_get<size_t>("heap_size"), UC_PROT_READ | UC_PROT_WRITE);

        // allocate stack
        x86_emulator.mem_map(x86_emulator.context_get<uint32_t>("stack_top"), x86_emulator.context_get<size_t>("stack_size"), UC_PROT_READ | UC_PROT_WRITE);

        // allocate r0_to_r3_stub area
        x86_emulator.mem_map(x86_emulator.context_get<uint32_t>("r0_to_r3_stub_area_base"), x86_emulator.context_get<size_t>("r0_to_r3_stub_area_size"), UC_PROT_READ | UC_PROT_EXEC);
        x86_emulator.mem_write(x86_emulator.context_get<uint32_t>("r0_to_r3_stub_area_base"), keystone_assembler{ KS_ARCH_X86, KS_MODE_32 }.assemble("iretd;"));

        // allocate dead area
        x86_emulator.mem_map(x86_emulator.context_get<uint32_t>("dead_area_base"), x86_emulator.context_get<size_t>("dead_area_size"), UC_PROT_READ | UC_PROT_EXEC);

        // allocate and hook read access to IAT
        {
            auto iat_base = x86_emulator.context_get<uint32_t>("iat_base");
            auto iat_size = x86_emulator.context_get<size_t>("iat_size");
            auto external_api_stub_area_base = x86_emulator.context_get<uint32_t>("external_api_stub_area_base");

            auto iat_page_base = iat_base / 0x1000 * 0x1000;
            auto iat_page_count = (iat_base - iat_page_base + iat_size + 0xfff) / 0x1000;

            x86_emulator.mem_map(iat_page_base, iat_page_count * 0x1000, UC_PROT_READ);
            
            x86_emulator.hook_add<UC_HOOK_MEM_READ>(
                [this, &x86_emulator, iat_base, external_api_stub_area_base](uc_mem_type type, uint32_t address, size_t size, int32_t value) {
                    auto rva = m_libcc_interpreter.convert_va_to_rva(address);
                    auto import_lookup_entry = m_libcc_interpreter.import_lookup_entry_from_rva(rva);

                    if (import_lookup_entry && !IMAGE_SNAP_BY_ORDINAL(import_lookup_entry->u1.Ordinal)) {
                        auto import_by_name_entry = m_libcc_interpreter.convert_rva_to_ptr<PIMAGE_IMPORT_BY_NAME>(import_lookup_entry->u1.AddressOfData);
                        if (strcmp(import_by_name_entry->Name, "memcpy") == 0) {
                            uint32_t impl_address = x86_emulator.context_get<std::map<std::string, uint32_t>&>("external_api_impl")["memcpy"];
                            x86_emulator.mem_write(address, &impl_address, sizeof(impl_address));
                        } else {
                            uint32_t stub_address = external_api_stub_area_base + (address - iat_base) / sizeof(IMAGE_THUNK_DATA);
                            x86_emulator.mem_write(address, &stub_address, sizeof(stub_address));
                        }
                    } else {
                        x86_emulator.emu_stop();
                    }
                },
                iat_base,
                iat_base + iat_size - 1
            );
        }

        // allocate and setup external api stub area
        {
            auto external_api_stub_area_base = x86_emulator.context_get<uint32_t>("external_api_stub_area_base");
            auto external_api_stub_area_size = x86_emulator.context_get<size_t>("external_api_stub_area_size");

            x86_emulator.mem_map(external_api_stub_area_base, external_api_stub_area_size, UC_PROT_READ | UC_PROT_EXEC);
            x86_emulator.mem_write(external_api_stub_area_base, std::vector<uint8_t>(external_api_stub_area_size, 0xc3));   // c3 -> ret

            x86_emulator.hook_add<UC_HOOK_CODE>(
                [this, &x86_emulator, external_api_stub_area_base](uint32_t address, size_t size) {
                    auto iat_base = x86_emulator.context_get<uint32_t>("iat_base");
                    auto from_va = iat_base + (address - external_api_stub_area_base) * sizeof(IMAGE_THUNK_DATA);
                    auto from_rva = m_libcc_interpreter.convert_va_to_rva(from_va);

                    auto import_lookup_entry = m_libcc_interpreter.import_lookup_entry_from_rva(from_rva);
                    if (import_lookup_entry && !IMAGE_SNAP_BY_ORDINAL(import_lookup_entry->u1.Ordinal)) {
                        auto import_by_name_entry = m_libcc_interpreter.convert_rva_to_ptr<PIMAGE_IMPORT_BY_NAME>(import_lookup_entry->u1.AddressOfData);
                        if (strcmp(import_by_name_entry->Name, "malloc") == 0) {
                            m_va_iat_entry_malloc = from_va;

                            uint32_t esp;
                            x86_emulator.reg_read(UC_X86_REG_ESP, &esp);

                            uint32_t alloc_size;
                            x86_emulator.mem_read(esp + 4, &alloc_size, sizeof(alloc_size));

                            auto& heap_records = x86_emulator.context_get<std::map<uint32_t, uint32_t>&>("heap_records");

                            auto predecessor_chunk =
                                std::adjacent_find(
                                    heap_records.begin(), 
                                    heap_records.end(),
                                    [alloc_size](const auto& chunk0, const auto& chunk1) { return chunk1.first - (chunk0.first + chunk0.second) >= alloc_size; }
                                );

                            uint32_t alloc_p;
                            if (predecessor_chunk != heap_records.end()) {
                                alloc_p = predecessor_chunk->first + predecessor_chunk->second;
                            } else {
                                auto heap_base = x86_emulator.context_get<uint32_t>("heap_base");
                                auto heap_size = x86_emulator.context_get<uint32_t>("heap_size");

                                auto free_space_base = heap_records.size() > 0 ? heap_records.rbegin()->first + heap_records.rbegin()->second : heap_base;
                                auto free_space_size = heap_base + heap_size - free_space_base;

                                if (free_space_size < alloc_size) {
                                    auto heap_expand_base = heap_base + heap_size;
                                    auto heap_expand_size = (alloc_size - free_space_size + 0xfff) / 0x1000 * 0x1000;
                                    x86_emulator.mem_map(heap_expand_base, heap_expand_size, UC_PROT_READ | UC_PROT_WRITE);
                                }

                                alloc_p = free_space_base;
                            }

                            heap_records[alloc_p] = alloc_size;

                            x86_emulator.reg_write(UC_X86_REG_EAX, &alloc_p);
                        } else if (strcmp(import_by_name_entry->Name, "free") == 0) {
                            uint32_t esp;
                            x86_emulator.reg_read(UC_X86_REG_ESP, &esp);

                            uint32_t alloc_p;
                            x86_emulator.mem_read(esp + 4, &alloc_p, sizeof(alloc_p));

                            auto& heap_records = x86_emulator.context_get<std::map<uint32_t, uint32_t>&>("heap_records");

                            auto chunk = heap_records.find(alloc_p);
                            if (chunk != heap_records.end()) {
                                heap_records.erase(chunk);
                            } else {
                                x86_emulator.emu_stop();
                            }
                        } else {
                            x86_emulator.emu_stop();
                        }
                    } else {
                        x86_emulator.emu_stop();
                    }
                },
                external_api_stub_area_base,
                external_api_stub_area_base + external_api_stub_area_size - 1
            );
        }

        // allocate and setup external api impl area
        {
            keystone_assembler x86_assembler{ KS_ARCH_X86, KS_MODE_32 };

            std::map<std::string, std::vector<uint8_t>> machine_code_list =
                {
                    std::make_pair(
                        "memcpy",
                        x86_assembler.assemble(
                            "push edi;"
                            "push esi;"
                            "mov eax, dword ptr [esp + 0x8 + 0x4];"
                            "mov edi, eax;"
                            "mov esi, dword ptr [esp + 0x8 + 0x8];"
                            "mov ecx, dword ptr [esp + 0x8 + 0xc];"
                            "rep movs byte ptr [edi], byte ptr [esi];"
                            "pop esi;"
                            "pop edi;"
                            "ret;"
                        )
                    )
                };

            auto& external_api_impl = x86_emulator.context_get<std::map<std::string, uint32_t>&>("external_api_impl");
            auto& external_api_impl_area_base = x86_emulator.context_get<uint32_t&>("external_api_impl_area_base");
            auto& external_api_impl_area_size = x86_emulator.context_get<size_t&>("external_api_impl_area_size");

            auto p = external_api_impl_area_base;
            for (const auto& name_code_pair : machine_code_list) {
                external_api_impl[name_code_pair.first] = p;
                p = (p + name_code_pair.second.size() + 0xf) / 0x10 * 0x10;
            }

            external_api_impl_area_size = (p + 0xfff) / 0x1000 * 0x1000 - external_api_impl_area_base;

            x86_emulator.mem_map(external_api_impl_area_base, external_api_impl_area_size, UC_PROT_READ | UC_PROT_EXEC);
            for (const auto& name_code_pair : machine_code_list) {
                x86_emulator.mem_write(external_api_impl[name_code_pair.first], name_code_pair.second);
            }
        }

        // allocate and setup GDT, segment registers
        {
            auto gdt_base = x86_emulator.context_get<uint32_t>("gdt_base");
            auto gdt_size = x86_emulator.context_get<size_t>("gdt_size");
            x86_emulator.mem_map(gdt_base, gdt_size, UC_PROT_READ | UC_PROT_WRITE);
            
            x86_emulator.create_gdt_entry(gdt_base, 0, 0, 0, 0);    // null segment descriptor
            // -------------------------------------------------------- access_byte
            // 0x80     -> present bit
            // (0 << 5) -> DPL is set to 0
            // 0x10     -> code/data segment
            // 0x08     -> executable segment
            // !(0x4)   -> not conforming code segment
            // 0x02     -> code segment is readable
            // !(0x01)  -> accessed bit, this bit is managed by CPU
            // -------------------------------------------------------- flags
            // 0x08     -> 4k granularity  
            // 0x04     -> 32-bit protected mode segment
            // !(0x01)  -> AVL bit is not used
            x86_emulator.create_gdt_entry(gdt_base + 1 * 0x8, 0x00000000, 0xfffff, 0x80 | (0 << 5) | 0x10 | 0x08 | !(0x04) | 0x02 | !(0x01), 0x08 | 0x04 | !(0x01));    // kernel code segment
            // -------------------------------------------------------- access_byte
            // 0x80     -> present bit
            // (0 << 5) -> DPL is set to 0
            // 0x10     -> code/data segment
            // !(0x08)  -> data segment
            // !(0x4)   -> segment grows up
            // 0x02     -> data segment is writable
            // !(0x01)  -> accessed bit, this bit is managed by CPU
            // -------------------------------------------------------- flags
            // 0x08     -> 4k granularity  
            // 0x04     -> 32-bit protected mode segment
            // !(0x01)  -> AVL bit is not used
            x86_emulator.create_gdt_entry(gdt_base + 2 * 0x8, 0x00000000, 0xfffff, 0x80 | (0 << 5) | 0x10 | !(0x08) | !(0x04) | 0x02 | !(0x01), 0x08 | 0x04 | !(0x01)); // kernel data segment
            // -------------------------------------------------------- access_byte
            // 0x80     -> present bit
            // (3 << 5) -> DPL is set to 3
            // 0x10     -> code/data segment
            // 0x08     -> executable segment
            // !(0x4)   -> not conforming code segment
            // 0x02     -> code segment is readable
            // !(0x01)  -> accessed bit, this bit is managed by CPU
            // -------------------------------------------------------- flags
            // 0x08     -> 4k granularity  
            // 0x04     -> 32-bit protected mode segment
            // !(0x01)  -> AVL bit is not used
            x86_emulator.create_gdt_entry(gdt_base + 3 * 0x8, 0x00000000, 0xfffff, 0x80 | (3 << 5) | 0x10 | 0x08 | !(0x04) | 0x02 | !(0x01), 0x08 | 0x04 | !(0x01));    // user code segment
            // -------------------------------------------------------- access_byte
            // 0x80     -> present bit
            // (3 << 5) -> DPL is set to 3
            // 0x10     -> code/data segment
            // !(0x08)  -> data segment
            // !(0x4)   -> segment grows up
            // 0x02     -> data segment is writable
            // !(0x01)  -> accessed bit, this bit is managed by CPU
            // -------------------------------------------------------- flags
            // 0x08     -> 4k granularity  
            // 0x04     -> 32-bit protected mode segment
            // !(0x01)  -> AVL bit is not used
            x86_emulator.create_gdt_entry(gdt_base + 4 * 0x8, 0x00000000, 0xfffff, 0x80 | (3 << 5) | 0x10 | !(0x08) | !(0x04) | 0x02 | !(0x01), 0x08 | 0x04 | !(0x01)); // user data segment
            // -------------------------------------------------------- access_byte
            // 0x80     -> present bit
            // (3 << 5) -> DPL is set to 3
            // 0x10     -> code/data segment
            // !(0x08)  -> data segment
            // !(0x4)   -> segment grows up
            // 0x02     -> data segment is writable
            // !(0x01)  -> accessed bit, this bit is managed by CPU
            // -------------------------------------------------------- flags
            // !(0x08)  -> 1-byte granularity  
            // 0x04     -> 32-bit protected mode segment
            // !(0x01)  -> AVL bit is not used
            auto fs_base = x86_emulator.context_get<uint32_t>("fs_base");
            auto fs_size = x86_emulator.context_get<size_t>("fs_size");
            x86_emulator.create_gdt_entry(gdt_base + 7 * 0x8, fs_base, fs_size - 1, 0x80 | (3 << 5) | 0x10 | !(0x08) | !(0x04) | 0x02 | !(0x01), !(0x08) | 0x04 | !(0x01)); // user fs segment

            uc_x86_mmr gdtr = {};
            gdtr.base = gdt_base;
            gdtr.limit = gdt_base + gdt_size - 1;
            x86_emulator.reg_write(UC_X86_REG_GDTR, &gdtr);

            uint16_t cs, ds, es, fs, gs, ss;
            cs = (1 << 3) | (0 << 2) | (0);             // use kernel code segmet
            ss = (2 << 3) | (0 << 2) | (0);             // use kernel data segmet
            ds = es = (4 << 3) | (0 << 2) | (3);        // use user data segment
            fs = (7 << 3) | (0 << 2) | (3);             // use user fs segment
            gs = 0;                                     // not used

            uint32_t eflags;
            x86_emulator.reg_read(UC_X86_REG_EFLAGS, &eflags);
            x86_emulator.reg_write(UC_X86_REG_CS, &cs);
            x86_emulator.reg_write(UC_X86_REG_SS, &ss);
            x86_emulator.reg_write(UC_X86_REG_DS, &ds);
            x86_emulator.reg_write(UC_X86_REG_ES, &es);
            x86_emulator.reg_write(UC_X86_REG_FS, &fs);
            x86_emulator.reg_write(UC_X86_REG_GS, &gs);
            
        }

        // allocate and hook access to fs area
        {
            auto fs_base = x86_emulator.context_get<uint32_t>("fs_base");
            auto fs_size = x86_emulator.context_get<size_t>("fs_size");
            x86_emulator.mem_map(fs_base, fs_size, UC_PROT_READ | UC_PROT_WRITE);

            x86_emulator.hook_add<UC_HOOK_MEM_READ>(
                [this, &x86_emulator, fs_base](uc_mem_type access, uint32_t address, size_t size, int64_t value) {
                    switch (address - fs_base) {
                        case 0:
                            if (size == 4) {
                                // Current Structured Exception Handling (SEH) frame, leave it NULL
                            } else {
                                x86_emulator.emu_stop();
                            }
                            break;
                        default:
                            x86_emulator.emu_stop();
                            break;
                    }
                },
                fs_base, 
                fs_base + fs_size - 1
            );
        }

        // x86_emulator.hook_add<UC_HOOK_CODE>([](uint32_t address, uint32_t size) { wprintf_s(L"code_trace, address = 0x%08x\n", address); });

        x86_emulator.hook_add<UC_HOOK_MEM_UNMAPPED>(
            [this, &x86_emulator](uc_mem_type access, uint32_t address, size_t size, int64_t value) -> bool {
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

                    x86_emulator.mem_map(page_base, page_size, page_perms);
                    x86_emulator.mem_write(page_base, m_libcc_interpreter.convert_va_to_ptr<const void*>(page_base), page_size);

                    return true;
                } catch (::nkg::exception&) {
                    return false;
                }
            }
        );

        // set ebp, esp
        uint32_t init_ebp = x86_emulator.context_get<uint32_t>("stack_base") - x86_emulator.context_get<size_t>("stack_size") / 4;
        uint32_t init_esp = x86_emulator.context_get<uint32_t>("stack_base") - x86_emulator.context_get<size_t>("stack_size") / 2;

        x86_emulator.reg_write(UC_X86_REG_EBP, &init_ebp);
        x86_emulator.reg_write(UC_X86_REG_ESP, &init_esp);

        // setup iretd context
        uint32_t ring3_eip = x86_emulator.context_get<uint32_t>("start_address");
        uint32_t ring3_cs = (3 << 3) | (0 << 2) | (3);  // use user code segment
        uint32_t ring3_eflags; x86_emulator.reg_read(UC_X86_REG_EFLAGS, &ring3_eflags);
        uint32_t ring3_esp = init_esp + 5 * 4;
        uint32_t ring3_ss = (4 << 3) | (0 << 2) | (3);  // use user data segment
        x86_emulator.mem_write(init_esp, &ring3_eip, sizeof(ring3_eip));
        x86_emulator.mem_write(init_esp + 0x4, &ring3_cs, sizeof(ring3_cs));
        x86_emulator.mem_write(init_esp + 0x8, &ring3_eflags, sizeof(ring3_eflags));
        x86_emulator.mem_write(init_esp + 0xc, &ring3_esp, sizeof(ring3_esp));
        x86_emulator.mem_write(init_esp + 0x10, &ring3_ss, sizeof(ring3_ss));

        // set ring3 retaddr
        uint32_t ring3_retaddr = x86_emulator.context_get<uint32_t>("dead_address");
        x86_emulator.mem_write(ring3_esp, &ring3_retaddr, sizeof(ring3_retaddr));

        // set argument registers
        uint32_t init_ecx = 0;                              // `this` pointer of CSRegistrationInfoFetcher_WIN, but we don't need it for now.
        uint32_t retval_addr = ring3_esp + 0x40;            // a pointer to stack memory which stores return value
        x86_emulator.reg_write(UC_X86_REG_ECX, &init_ecx);
        x86_emulator.mem_write(ring3_esp + 4, &retval_addr, sizeof(retval_addr));   // write to dword ptr [ring3_esp + 4]

        // 
        // start emulate
        // 
        try {
            x86_emulator.emu_start(x86_emulator.context_get<uint32_t>("r0_to_r3_stub_area_base"), x86_emulator.context_get<uint32_t>("dead_address"));
        } catch (nkg::exception&) {
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: Code emulation failed.\n");
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: This patch solution will be suppressed.\n");
            return false;
        }

        wprintf_s(L"[*] patch_solution_since<16, 0, 7, 0>: m_va_iat_entry_malloc = 0x%08x\n", m_va_iat_entry_malloc);

        //
        // get result
        // 
        // on I386 platform, `std::string` has follow memory layout:
        //     ------------------------------
        //     | offset | size |
        //     ------------------------------
        //     | +0     | 0x10 | `char[16]: a small string buffer` OR `char*: a large string buffer pointer`
        //     ------------------------------
        //     | +0x10  | 0x4  | size_t: string length
        //     ------------------------------
        //     | +0x14  | 0x4  | size_t: capacity
        //     ------------------------------
        //
        uint32_t encoded_key_length;
        x86_emulator.mem_read(retval_addr + 0x10, &encoded_key_length, sizeof(encoded_key_length));
        if (encoded_key_length != official_encoded_key.length()) {
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: Unexpected encoded key length(%u).\n", encoded_key_length);
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: This patch solution will be suppressed.\n");
            return false;
        }

        uint32_t encoded_key_ptr;
        x86_emulator.mem_read(retval_addr, &encoded_key_ptr, sizeof(encoded_key_ptr));

        auto encoded_key = x86_emulator.mem_read(encoded_key_ptr, encoded_key_length);
        if (memcmp(encoded_key.data(), official_encoded_key.data(), encoded_key.size()) == 0) {
            wprintf_s(L"[+] patch_solution_since<16, 0, 7, 0>: Official encoded key is found.\n");
            return true;
        } else {
            wprintf_s(L"[-] patch_solution_since<16, 0, 7, 0>: Official encoded key is not found.\n");
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
        patch_code_chunks.emplace_back("push edi;");
        patch_code_chunks.emplace_back("push esi;");
        patch_code_chunks.emplace_back("push ebx;");
        patch_code_chunks.emplace_back("push ebp;");
        patch_code_chunks.emplace_back("mov ebp, esp;");
        patch_code_chunks.emplace_back("call label; label: pop ebx; sub ebx, label;");  // ebx <- relocation shift value
        patch_code_chunks.emplace_back(fmt::format("mov eax, {:#08x};", m_va_iat_entry_malloc));
        patch_code_chunks.emplace_back("add eax, ebx;");
        patch_code_chunks.emplace_back("mov eax, dword ptr [eax];");                    // eax <- address of `malloc`
        patch_code_chunks.emplace_back(fmt::format("push {:#x};", encoded_key.length() + 1));
        patch_code_chunks.emplace_back("call eax;");
        patch_code_chunks.emplace_back("add esp, 0x4;");
        {
            std::vector<uint32_t> push_values((encoded_key.length() + 1 + 3) / 4, 0);
            memcpy(push_values.data(), encoded_key.data(), encoded_key.length());
            std::for_each(push_values.crbegin(), push_values.crend(), [&patch_code_chunks](uint32_t x) { patch_code_chunks.emplace_back(fmt::format("push {:#08x};", x)); });
        }
        patch_code_chunks.emplace_back("mov edi, eax;");
        patch_code_chunks.emplace_back("mov esi, esp;");
        patch_code_chunks.emplace_back(fmt::format("mov ecx, {:#x};", encoded_key.length() + 1));
        patch_code_chunks.emplace_back("rep movs byte ptr [edi], byte ptr [esi];");
        patch_code_chunks.emplace_back("mov edx, dword ptr [ebp + 0x14];");
        patch_code_chunks.emplace_back("mov dword ptr [edx], eax;");
        patch_code_chunks.emplace_back(fmt::format("mov dword ptr [edx + 0x10], {:#x};", encoded_key.length()));
        patch_code_chunks.emplace_back(fmt::format("mov dword ptr [edx + 0x14], {:#x};", encoded_key.length() + 1));
        patch_code_chunks.emplace_back("mov eax, edx;");
        patch_code_chunks.emplace_back("leave;");
        patch_code_chunks.emplace_back("pop ebx;");
        patch_code_chunks.emplace_back("pop esi;");
        patch_code_chunks.emplace_back("pop edi;");
        patch_code_chunks.emplace_back("ret 4;");

        std::vector<uint8_t> assembled_patch_code;
        {
            keystone_assembler x86_assembler{ KS_ARCH_X86, KS_MODE_32 };

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
                        auto assembled_jmp = x86_assembler.assemble(fmt::format("jmp {:#08x};", next_va), current_va);
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
