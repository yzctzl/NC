#pragma once
#include <string>
#include <windows.h>
#include "exceptions/win32_exception.hpp"

#define NKG_CURRENT_SOURCE_FILE() u8".\\common\\cp_converter.hpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {

    template<int from_cp, int to_cp>
    struct cp_converter {
        static std::string convert(std::string_view from_string) {
            if constexpr (from_cp == to_cp) {
                return from_string;
            } else {
                if (from_cp == CP_ACP && GetACP() == to_cp) {
                    return from_string;
                } else {
                    return cp_converter<-1, to_cp>::convert(cp_converter<from_cp, -1>::convert(from_string));
                }
            }
        }
    };

    template<int from_cp>
    struct cp_converter<from_cp, -1> {
        static std::wstring convert(std::string_view from_string) {
            int len;

            len = MultiByteToWideChar(from_cp, 0, from_string.data(), -1, NULL, 0);
            if (len <= 0) {
                throw ::nkg::exceptions::win32_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), GetLastError(), u8"MultiByteToWideChar failed.");
            }

            std::wstring to_string(len, 0);

            len = MultiByteToWideChar(from_cp, 0, from_string.data(), -1, to_string.data(), len);
            if (len <= 0) {
                throw ::nkg::exceptions::win32_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), GetLastError(), u8"MultiByteToWideChar failed.");
            }

            while (to_string.length() > 0 && to_string.back() == 0) {
                to_string.pop_back();
            }

            return to_string;
        }
    };

    template<int to_cp>
    struct cp_converter<-1, to_cp> {
        static std::string convert(std::wstring_view from_string) {
            int len;
            
            len = WideCharToMultiByte(to_cp, 0, from_string.data(), -1, NULL, 0, NULL, NULL);
            if (len <= 0) {
                throw ::nkg::exceptions::win32_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), GetLastError(), u8"WideCharToMultiByte failed.");
            }

            std::string to_string(len, 0);

            len = WideCharToMultiByte(to_cp, 0, from_string.data(), -1, to_string.data(), len, NULL, NULL);
            if (len <= 0) {
                throw ::nkg::exceptions::win32_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), GetLastError(), u8"WideCharToMultiByte failed.");
            }

            while (to_string.length() > 0 && to_string.back() == 0) {
                to_string.pop_back();
            }

            return to_string;
        }
    };

}

#undef NKG_CURRENT_SOURCE_LINE
#undef NKG_CURRENT_SOURCE_FILE
