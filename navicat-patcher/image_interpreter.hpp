#pragma once
#include <type_traits>
#include <array>
#include <map>
#include <windows.h>

#include "exception.hpp"

namespace nkg {

    class image_interpreter {
    public:
        using va_t = uintptr_t;
        using rva_t = uintptr_t;
        using fo_t = uintptr_t;

    private:
        PIMAGE_DOS_HEADER m_dos_header;
        PIMAGE_NT_HEADERS m_nt_headers;
        PIMAGE_SECTION_HEADER m_section_header_table;

        std::map<std::array<BYTE, 8>, PIMAGE_SECTION_HEADER> m_section_header_name_lookup_table;
        std::map<rva_t, PIMAGE_SECTION_HEADER> m_section_header_rva_lookup_table;
        std::map<fo_t, PIMAGE_SECTION_HEADER> m_section_header_fo_lookup_table;

        std::map<rva_t, size_t> m_relocation_rva_lookup_table;

        std::map<rva_t, std::pair<PIMAGE_IMPORT_DESCRIPTOR, PIMAGE_THUNK_DATA>> m_iat_rva_lookup_table;

        VS_FIXEDFILEINFO* m_vs_fixed_file_info;

        image_interpreter();

        static std::array<BYTE, 8> make_section_name(const BYTE (&name)[8]) {
            std::array<BYTE, 8> retval;
            std::copy(std::begin(name), std::end(name), retval.begin());
            return retval;
        }

    public:
        class parse_error : public ::nkg::exception    {
        public:
            parse_error(std::string_view file, int line, std::string_view message) noexcept :
                ::nkg::exception(file, line, message) {}
        };

        [[nodiscard]]
        static image_interpreter parse(void* image_base, bool parse_relocation);

        template<typename ptr_t = void*>
        [[nodiscard]]
        ptr_t image_base() const noexcept {
            static_assert(std::is_pointer_v<ptr_t>);
            return reinterpret_cast<ptr_t>(m_dos_header);
        }

        [[nodiscard]]
        PIMAGE_DOS_HEADER image_dos_header() const noexcept;

        [[nodiscard]]
        PIMAGE_NT_HEADERS image_nt_headers() const noexcept;

        [[nodiscard]]
        PIMAGE_SECTION_HEADER image_section_header_table() const noexcept;

        [[nodiscard]]
        PIMAGE_SECTION_HEADER image_section_header(size_t n) const;

        [[nodiscard]]
        PIMAGE_SECTION_HEADER image_section_header(std::string_view name) const;

        [[nodiscard]]
        PIMAGE_SECTION_HEADER image_section_header_from_rva(rva_t rva) const;

        [[nodiscard]]
        PIMAGE_SECTION_HEADER image_section_header_from_va(va_t va) const;

        [[nodiscard]]
        PIMAGE_SECTION_HEADER image_section_header_from_fo(fo_t file_offset) const;

        [[nodiscard]]
        va_t convert_rva_to_va(rva_t rva) const noexcept;

        [[nodiscard]]
        fo_t convert_rva_to_fo(rva_t rva) const;

        template<typename ptr_t = void*>
        [[nodiscard]]
        ptr_t convert_rva_to_ptr(rva_t rva) const {
            static_assert(std::is_pointer_v<ptr_t>);
            return convert_fo_to_ptr<ptr_t>(convert_rva_to_fo(rva));
        }

        [[nodiscard]]
        rva_t convert_fo_to_rva(fo_t file_offset) const;

        [[nodiscard]]
        va_t convert_fo_to_va(fo_t file_offset) const;

        template<typename ptr_t>
        [[nodiscard]]
        ptr_t convert_fo_to_ptr(fo_t file_offset) const noexcept {
            static_assert(std::is_pointer_v<ptr_t>);
            return reinterpret_cast<ptr_t>(image_base<char*>() + file_offset);
        }

        [[nodiscard]]
        rva_t convert_va_to_rva(va_t va) const noexcept;

        [[nodiscard]]
        fo_t convert_va_to_fo(va_t va) const;

        template<typename ptr_t>
        [[nodiscard]]
        ptr_t convert_va_to_ptr(va_t va) const noexcept {
            return convert_rva_to_ptr<ptr_t>(convert_va_to_rva(va));
        }

        template<typename ptr_t>
        [[nodiscard]]
        fo_t convert_ptr_to_fo(ptr_t ptr) const noexcept {
            static_assert(std::is_pointer_v<ptr_t>);
            return reinterpret_cast<const volatile char*>(ptr) - image_base<const volatile char*>();
        }

        template<typename ptr_t>
        [[nodiscard]]
        rva_t convert_ptr_to_rva(ptr_t ptr) const {
            return convert_fo_to_rva(convert_ptr_to_fo(ptr));
        }

        template<typename ptr_t>
        [[nodiscard]]
        va_t convert_ptr_to_va(ptr_t ptr) const {
            return convert_fo_to_va(convert_ptr_to_fo(ptr));
        }

        [[nodiscard]]
        size_t number_of_sections() const noexcept;

        template<typename ptr_t = void*>
        [[nodiscard]]
        ptr_t image_section_view(size_t n, size_t offset = 0) const {
            static_assert(std::is_pointer_v<ptr_t>);
            return reinterpret_cast<ptr_t>(image_base<char*>() + image_section_header(n)->PointerToRawData + offset);
        }

        template<typename ptr_t = void*>
        [[nodiscard]]
        ptr_t image_section_view(std::string_view section_name, size_t offset = 0) const {
            static_assert(std::is_pointer_v<ptr_t>);
            return reinterpret_cast<ptr_t>(image_base<char*>() + image_section_header(section_name)->PointerToRawData + offset);
        }

        template<typename ptr_t, typename pred_func_t>
        [[nodiscard]]
        ptr_t search_section(size_t n, pred_func_t&& pred_func) const {
            static_assert(std::is_pointer_v<ptr_t>);

            auto section_header = image_section_header(n);

            auto begin = image_base<const uint8_t*>() + section_header->PointerToRawData;
            auto end = begin + section_header->Misc.VirtualSize;

            for (; begin < end; ++begin) {
                if (pred_func(begin, end - begin)) {
                    return reinterpret_cast<ptr_t>(const_cast<uint8_t*>(begin));
                }
            }

            return nullptr;
        }

        template<typename ptr_t, typename pred_func_t>
        [[nodiscard]]
        ptr_t search_section(std::string_view section_name, pred_func_t&& pred_func) const {
            static_assert(std::is_pointer_v<ptr_t>);

            auto section_header = image_section_header(section_name);

            auto begin = image_base<const uint8_t*>() + section_header->PointerToRawData;
            auto end = begin + section_header->Misc.VirtualSize;

            for (; begin < end; ++begin) {
                if (pred_func(begin, end - begin)) {
                    return reinterpret_cast<ptr_t>(const_cast<uint8_t*>(begin));
                }
            }

            return nullptr;
        }

        PIMAGE_IMPORT_DESCRIPTOR import_descriptor_from_rva(rva_t rva);

        PIMAGE_THUNK_DATA import_lookup_entry_from_rva(rva_t rva);

        auto& relocation_distribute() {
            return m_relocation_rva_lookup_table;
        }
    };

}
