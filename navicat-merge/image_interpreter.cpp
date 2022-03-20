#include "image_interpreter.hpp"
#include <fmt/format.h>
#include "exceptions/index_exception.hpp"

#define NKG_CURRENT_SOURCE_FILE() u8".\\navicat-patcher\\image_interpreter.cpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {

    image_interpreter::image_interpreter() :
        m_dos_header(nullptr),
        m_nt_headers(nullptr),
        m_section_header_table(nullptr),
        m_vs_fixed_file_info(nullptr) {}

    [[nodiscard]]
    image_interpreter image_interpreter::parse(void* image_base, bool parse_relocation) {
        image_interpreter new_image;

        new_image.m_dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(image_base);
        if (new_image.m_dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
            throw parse_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Invalid image: DOS signature check failure")
                .push_hint(u8"Are you sure you DO provide a valid WinPE file?");
        }

        new_image.m_nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uint8_t*>(image_base) + new_image.m_dos_header->e_lfanew);
        if (new_image.m_nt_headers->Signature != IMAGE_NT_SIGNATURE) {
            throw parse_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Invalid image: NT signature check failure")
                .push_hint(u8"Are you sure you DO provide a valid WinPE file?");
        }

#if defined(_M_AMD64)
        if (new_image.m_nt_headers->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            throw parse_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Invalid image: optional header magic check failure")
                .push_hint(u8"Are you sure you DO provide a valid 64-bits WinPE file?");
        }
        if (new_image.m_nt_headers->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
            throw parse_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Invalid image: machine check failure")
                .push_hint(u8"Are you sure you DO provide a valid 64-bits WinPE file?");
        }
#elif defined(_M_IX86)
        if (new_image.m_nt_headers->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
            throw parse_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Invalid Image. (Optional header magic check failure)")
                .push_hint(u8"Are you sure you DO provide a valid 32-bits WinPE file?");
        }
        if (new_image.m_nt_headers->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) {
            throw parse_error(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Invalid Image. (Machine check failure)")
                .push_hint(u8"Are you sure you DO provide a valid 32-bits WinPE file?");
        }
#else
#error "image_interpreter.cpp: unsupported architecture."
#endif

        new_image.m_section_header_table = 
            reinterpret_cast<PIMAGE_SECTION_HEADER>(reinterpret_cast<char*>(&new_image.m_nt_headers->OptionalHeader) + new_image.m_nt_headers->FileHeader.SizeOfOptionalHeader);

        for (WORD i = 0; i < new_image.m_nt_headers->FileHeader.NumberOfSections; ++i) {
            auto section_name = make_section_name(new_image.m_section_header_table[i].Name);

            if (new_image.m_section_header_name_lookup_table.find(section_name) == new_image.m_section_header_name_lookup_table.end()) {
                new_image.m_section_header_name_lookup_table[section_name] = &new_image.m_section_header_table[i];
            }

            new_image.m_section_header_rva_lookup_table[new_image.m_section_header_table[i].VirtualAddress] = &new_image.m_section_header_table[i];
            new_image.m_section_header_fo_lookup_table[new_image.m_section_header_table[i].PointerToRawData] = &new_image.m_section_header_table[i];
        }

        if (parse_relocation && new_image.m_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress != 0) {
            auto relocation_table = 
                new_image.convert_rva_to_ptr<PIMAGE_BASE_RELOCATION>(new_image.m_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);

            while (relocation_table->VirtualAddress != 0) {
                rva_t rva = relocation_table->VirtualAddress;
                auto reloc_items = reinterpret_cast<WORD*>(relocation_table + 1);
                auto reloc_items_count = (relocation_table->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);

                for (DWORD i = 0; i < reloc_items_count; ++i) {
                    auto reloc_type = reloc_items[i] >> 12;

                    switch (reloc_type) {
                        case IMAGE_REL_BASED_ABSOLUTE:
                            break;
                        case IMAGE_REL_BASED_HIGH:
                        case IMAGE_REL_BASED_LOW:
                        case IMAGE_REL_BASED_HIGHADJ:
                            new_image.m_relocation_rva_lookup_table[rva + (reloc_items[i] & 0x0fff)] = 2;
                            break;
                        case IMAGE_REL_BASED_HIGHLOW:
                            new_image.m_relocation_rva_lookup_table[rva + (reloc_items[i] & 0x0fff)] = 4;
                            break;
#if defined(IMAGE_REL_BASED_DIR64)
                        case IMAGE_REL_BASED_DIR64:
                            new_image.m_relocation_rva_lookup_table[rva + (reloc_items[i] & 0x0fff)] = 8;
                            break;
#endif
                        default:
                            break;
                    }
                }

                relocation_table = reinterpret_cast<PIMAGE_BASE_RELOCATION>(&reloc_items[reloc_items_count]);
            }
        }

        if (new_image.m_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress) {
            rva_t import_rva = new_image.m_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

            auto import_descriptors = new_image.convert_rva_to_ptr<PIMAGE_IMPORT_DESCRIPTOR>(import_rva);
            for (size_t i = 0; import_descriptors[i].OriginalFirstThunk != 0; ++i) {
                auto import_lookup_table = new_image.convert_rva_to_ptr<PIMAGE_THUNK_DATA>(import_descriptors[i].OriginalFirstThunk);
                rva_t import_address_table_rva = import_descriptors[i].FirstThunk;

                for (size_t j = 0; import_lookup_table[j].u1.Ordinal != 0; ++j) {
                    new_image.m_iat_rva_lookup_table[import_address_table_rva + j * sizeof(IMAGE_THUNK_DATA)] = std::make_pair(&import_descriptors[i], &import_lookup_table[j]);
                }
            }
        }

        if (new_image.m_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress) {
            rva_t resource_rva = new_image.m_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;

            auto res_type_directory = new_image.convert_rva_to_ptr<PIMAGE_RESOURCE_DIRECTORY>(resource_rva);
            auto res_type_name_entries = reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY_ENTRY>(res_type_directory + 1);
            auto res_type_id_entries = res_type_name_entries + res_type_directory->NumberOfNamedEntries;

            for (WORD i = 0; i < res_type_directory->NumberOfIdEntries && new_image.m_vs_fixed_file_info == nullptr; ++i) {
                if (res_type_id_entries[i].Id == reinterpret_cast<uintptr_t>(RT_VERSION) && res_type_id_entries[i].DataIsDirectory) {
                    auto res_name_directory = new_image.convert_rva_to_ptr<PIMAGE_RESOURCE_DIRECTORY>(resource_rva + res_type_id_entries[i].OffsetToDirectory);
                    auto res_name_name_entries = reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY_ENTRY>(res_name_directory + 1);
                    auto res_name_id_entries = res_name_name_entries + res_name_directory->NumberOfNamedEntries;

                    for (WORD j = 0; j < res_name_directory->NumberOfIdEntries && new_image.m_vs_fixed_file_info == nullptr; ++j) {
                        if (res_name_id_entries[j].Id == VS_VERSION_INFO && res_name_id_entries[j].DataIsDirectory) {
                            auto res_lang_directory = new_image.convert_rva_to_ptr<PIMAGE_RESOURCE_DIRECTORY>(resource_rva + res_name_id_entries[j].OffsetToDirectory);
                            auto res_lang_name_entries = reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY_ENTRY>(res_lang_directory + 1);
                            auto res_lang_id_entries = res_lang_name_entries + res_lang_directory->NumberOfNamedEntries;

                            for (WORD k = 0; k < res_lang_directory->NumberOfIdEntries && new_image.m_vs_fixed_file_info == nullptr; ++k) {
                                constexpr WORD neutral_lang_id = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
                                constexpr WORD english_lang_id = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

                                if ((res_lang_id_entries[k].Id == neutral_lang_id || res_lang_id_entries[k].Id == english_lang_id) && !res_lang_id_entries[k].DataIsDirectory) {
                                    auto res_data_entry = new_image.convert_rva_to_ptr<PIMAGE_RESOURCE_DATA_ENTRY>(resource_rva + res_lang_id_entries[k].OffsetToData);

                                    auto vs_version_info = new_image.convert_rva_to_ptr<PBYTE>(res_data_entry->OffsetToData);
                                    auto vs_version_info_key = reinterpret_cast<PWSTR>(vs_version_info + 6); // vs_version_info->szKey
                                    if (_wcsicmp(vs_version_info_key, L"VS_VERSION_INFO") == 0) {
                                        auto p = reinterpret_cast<PBYTE>(vs_version_info_key + _countof(L"VS_VERSION_INFO"));
                                        while (new_image.convert_ptr_to_rva(p) % sizeof(DWORD)) {
                                            ++p;
                                        }

                                        auto vs_fixed_file_info = reinterpret_cast<VS_FIXEDFILEINFO*>(p);

                                        if (vs_fixed_file_info->dwSignature == VS_FFI_SIGNATURE) {
                                            new_image.m_vs_fixed_file_info = vs_fixed_file_info;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return new_image;
    }

    [[nodiscard]]
    PIMAGE_DOS_HEADER image_interpreter::image_dos_header() const noexcept {
        return m_dos_header;
    }

    [[nodiscard]]
    PIMAGE_NT_HEADERS image_interpreter::image_nt_headers() const noexcept {
        return m_nt_headers;
    }

    [[nodiscard]]
    PIMAGE_SECTION_HEADER image_interpreter::image_section_header_table() const noexcept {
        return m_section_header_table;
    }

    [[nodiscard]]
    PIMAGE_SECTION_HEADER image_interpreter::image_section_header(size_t n) const {
        if (n < m_nt_headers->FileHeader.NumberOfSections) {
            return m_section_header_table + n;
        } else {
            throw exceptions::index_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Section index is out of range.");
        }
    }

    [[nodiscard]]
    PIMAGE_SECTION_HEADER image_interpreter::image_section_header(std::string_view section_name) const {
        if (section_name.length() <= 8) {
            std::array<BYTE, 8> name{};

            std::copy(section_name.begin(), section_name.end(), name.begin());

            auto it = m_section_header_name_lookup_table.find(name);
            if (it != m_section_header_name_lookup_table.end()) {
                return it->second;
            } else {
                throw exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), fmt::format(u8"Target section header is not found: section_name = {}", section_name));
            }
        } else {
            throw exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Target section header is not found: section_name is too long.");
        }
    }

    [[nodiscard]]
    PIMAGE_SECTION_HEADER image_interpreter::image_section_header_from_rva(rva_t rva) const {
        auto it = m_section_header_rva_lookup_table.upper_bound(rva);
        if (it != m_section_header_rva_lookup_table.begin()) {
            --it;
        }

        rva_t section_rva_begin = it->second->VirtualAddress;
        rva_t section_rva_end = section_rva_begin + it->second->Misc.VirtualSize;

        if (section_rva_begin <= rva && rva < section_rva_end) {
            return it->second;
        } else {
            throw exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Target section header is not found.")
                .push_hint(fmt::format("rva = 0x{:x}", rva));
        }
    }

    [[nodiscard]]
    PIMAGE_SECTION_HEADER image_interpreter::image_section_header_from_va(va_t va) const {
        return image_section_header_from_rva(static_cast<rva_t>(va - m_nt_headers->OptionalHeader.ImageBase));
    }

    [[nodiscard]]
    PIMAGE_SECTION_HEADER image_interpreter::image_section_header_from_fo(fo_t file_offset) const {
        auto it = m_section_header_fo_lookup_table.upper_bound(file_offset);
        if (it != m_section_header_fo_lookup_table.begin()) {
            --it;
        }

        uintptr_t section_fo_begin = it->second->PointerToRawData;
        uintptr_t section_fo_end = section_fo_begin + it->second->SizeOfRawData;

        if (section_fo_begin <= file_offset && file_offset < section_fo_end) {
            return it->second;
        } else {
            throw exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Target section header is not found.")
                .push_hint(fmt::format(u8"file_offset = 0x{:x}", file_offset));
        }
    }

    [[nodiscard]]
    image_interpreter::va_t image_interpreter::convert_rva_to_va(rva_t rva) const noexcept {
        return rva + m_nt_headers->OptionalHeader.ImageBase;
    }

    [[nodiscard]]
    image_interpreter::fo_t image_interpreter::convert_rva_to_fo(rva_t rva) const {
        auto section_header = image_section_header_from_rva(rva);
        return section_header->PointerToRawData + (rva - static_cast<uintptr_t>(section_header->VirtualAddress));
    }

    [[nodiscard]]
    image_interpreter::rva_t image_interpreter::convert_fo_to_rva(fo_t file_offset) const {
        auto section_header = image_section_header_from_fo(file_offset);
        return section_header->VirtualAddress + (file_offset - section_header->PointerToRawData);
    }

    [[nodiscard]]
    image_interpreter::va_t image_interpreter::convert_fo_to_va(fo_t file_offset) const {
        return convert_fo_to_rva(file_offset) + m_nt_headers->OptionalHeader.ImageBase;
    }

    [[nodiscard]]
    image_interpreter::rva_t image_interpreter::convert_va_to_rva(va_t va) const noexcept {
        return va - m_nt_headers->OptionalHeader.ImageBase;
    }

    [[nodiscard]]
    image_interpreter::fo_t image_interpreter::convert_va_to_fo(va_t va) const {
        return image_section_header_from_va(va)->PointerToRawData;
    }

    [[nodiscard]]
    size_t image_interpreter::number_of_sections() const noexcept {
        return m_nt_headers->FileHeader.NumberOfSections;
    }

    PIMAGE_IMPORT_DESCRIPTOR image_interpreter::import_descriptor_from_rva(rva_t rva) {
        auto it = m_iat_rva_lookup_table.find(rva);
        return it != m_iat_rva_lookup_table.end() ? it->second.first : nullptr;
    }

    PIMAGE_THUNK_DATA image_interpreter::import_lookup_entry_from_rva(rva_t rva) {
        auto it = m_iat_rva_lookup_table.find(rva);
        return it != m_iat_rva_lookup_table.end() ? it->second.second : nullptr;
    }
}

#undef NKG_CURRENT_SOURCE_LINE
#undef NKG_CURRENT_SOURCE_FILE
