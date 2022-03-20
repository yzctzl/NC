#include "navicat_serial_generator.hpp"
#include <iostream>
#include "exceptions/operation_canceled_exception.hpp"

#define NKG_CURRENT_SOURCE_FILE() u8".\\navicat-keygen\\CollectInformation.cpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {

    [[nodiscard]]
    static int read_int(int min_val, int max_val, std::wstring_view prompt, std::wstring_view error_msg) {
        int val;

        for (std::wstring s;;) {
            std::wcout << prompt;
            if (!std::getline(std::wcin, s)) {
                throw exceptions::operation_canceled_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Operation is canceled by user.");
            }

            if (s.empty())
                continue;

            try {
                val = std::stoi(s, nullptr, 0);
                if (min_val <= val && val <= max_val) {
                    return val;
                } else {
                    throw std::invalid_argument(u8"");
                }
            } catch (std::invalid_argument&) {
                std::wcout << error_msg << std::endl;
            }
        }
    }

    [[nodiscard]]
    static int read_int(int min_val, int max_val, int default_val, std::wstring_view prompt, std::wstring_view error_msg) {
        int val;

        for (std::wstring s;;) {
            std::wcout << prompt;
            if (!std::getline(std::wcin, s)) {
                throw exceptions::operation_canceled_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Operation is canceled by user.");
            }

            if (s.empty()) {
                return default_val;
            }

            try {
                val = std::stoi(s, nullptr, 0);
                if (min_val <= val && val <= max_val) {
                    return val;
                } else {
                    throw std::invalid_argument(u8"");
                }
            } catch (std::invalid_argument&) {
                std::wcout << error_msg << std::endl;
            }
        }
    }

    [[nodiscard]]
    navicat_serial_generator CollectInformationNormal() {
        navicat_serial_generator sn_generator;

        std::wcout << L"[*] Select Navicat product:"    << std::endl;
        std::wcout << L" 0. DataModeler"                << std::endl;
        std::wcout << L" 1. Premium"                    << std::endl;
        std::wcout << L" 2. MySQL"                      << std::endl;
        std::wcout << L" 3. PostgreSQL"                 << std::endl;
        std::wcout << L" 4. Oracle"                     << std::endl;
        std::wcout << L" 5. SQLServer"                  << std::endl;
        std::wcout << L" 6. SQLite"                     << std::endl;
        std::wcout << L" 7. MariaDB"                    << std::endl;
        std::wcout << L" 8. MongoDB"                    << std::endl;
        std::wcout << L" 9. ReportViewer"               << std::endl;
        std::wcout << std::endl;
        sn_generator.set_software_type(static_cast<navicat_software_type>(read_int(0, 9, L"(Input index)> ", L"Invalid index.")));
        std::wcout << std::endl;

        std::wcout << L"[*] Select product language:"   << std::endl;
        std::wcout << L" 0. English"                    << std::endl;
        std::wcout << L" 1. Simplified Chinese"         << std::endl;
        std::wcout << L" 2. Traditional Chinese"        << std::endl;
        std::wcout << L" 3. Japanese"                   << std::endl;
        std::wcout << L" 4. Polish"                     << std::endl;
        std::wcout << L" 5. Spanish"                    << std::endl;
        std::wcout << L" 6. French"                     << std::endl;
        std::wcout << L" 7. German"                     << std::endl;
        std::wcout << L" 8. Korean"                     << std::endl;
        std::wcout << L" 9. Russian"                    << std::endl;
        std::wcout << L" 10. Portuguese"                << std::endl;
        std::wcout << std::endl;
        sn_generator.set_software_language(static_cast<navicat_software_language>(read_int(0, 10, L"(Input index)> ", L"Invalid index.")));
        std::wcout << std::endl;

        std::wcout << L"[*] Input major version number:" << std::endl;
        sn_generator.set_software_version(read_int(11, 16, 16, L"(range: 11 ~ 16, default: 16)> ", L"Invalid number."));
        std::wcout << std::endl;

        return sn_generator;
    }

    [[nodiscard]]
    navicat_serial_generator CollectInformationAdvanced() {
        navicat_serial_generator sn_generator;

        std::wcout << L"[*] Navicat Product Signature:" << std::endl;
        sn_generator.set_software_type(static_cast<std::uint8_t>(read_int(0x00, 0xff, L"(range: 0x00 ~ 0xFF)> ", L"Invalid number.")));
        std::wcout << std::endl;

        std::wcout << L"[*] Navicat Language Signature 0:" << std::endl;
        auto s1 = static_cast<std::uint8_t>(read_int(0x00, 0xff, L"(range: 0x00 ~ 0xFF)> ", L"Invalid number."));
        std::wcout << std::endl;

        std::wcout << L"[*] Navicat Language Signature 1:" << std::endl;
        auto s2 = static_cast<std::uint8_t>(read_int(0x00, 0xff, L"(range: 0x00 ~ 0xFF)> ", L"Invalid number."));
        sn_generator.set_software_language(s1, s2);
        std::wcout << std::endl;

        std::wcout << L"[*] Input major version number:" << std::endl;
        sn_generator.set_software_version(read_int(11, 16, 16, L"(range: 11 ~ 16, default: 16)> ", L"Invalid number."));
        std::wcout << std::endl;

        return sn_generator;
    }
}

#undef NKG_CURRENT_SOURCE_FILE
#undef NKG_CURRENT_SOURCE_LINE
