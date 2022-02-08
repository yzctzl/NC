#include "navicat_serial_generator.hpp"
#include "exception.hpp"
#include "base32_rfc4648.hpp"
#include <fmt/format.h>
#include <openssl/rand.h>
#include <algorithm>

#define NKG_CURRENT_SOURCE_FILE() u8".\\navicat-keygen\\navicat_serial_generator.cpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {

    navicat_serial_generator::navicat_serial_generator() noexcept :
        m_data{ 0x68 , 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32 }, m_des_key{} {}

    void navicat_serial_generator::set_software_language(navicat_software_language lang) noexcept {
        switch (lang) {
            case navicat_software_language::English:
                m_data[5] = 0xAC;       // Must be 0xAC for English version.
                m_data[6] = 0x88;       // Must be 0x88 for English version.
                break;
            case navicat_software_language::SimplifiedChinese:
                m_data[5] = 0xCE;       // Must be 0xCE for Simplified Chinese version.
                m_data[6] = 0x32;       // Must be 0x32 for Simplified Chinese version.
                break;
            case navicat_software_language::TraditionalChinese:
                m_data[5] = 0xAA;       // Must be 0xAA for Traditional Chinese version.
                m_data[6] = 0x99;       // Must be 0x99 for Traditional Chinese version.
                break;
            case navicat_software_language::Japanese:
                m_data[5] = 0xAD;       // Must be 0xAD for Japanese version. Discoverer: @dragonflylee
                m_data[6] = 0x82;       // Must be 0x82 for Japanese version. Discoverer: @dragonflylee
                break;
            case navicat_software_language::Polish:
                m_data[5] = 0xBB;       // Must be 0xBB for Polish version. Discoverer: @dragonflylee
                m_data[6] = 0x55;       // Must be 0x55 for Polish version. Discoverer: @dragonflylee
                break;
            case navicat_software_language::Spanish:
                m_data[5] = 0xAE;       // Must be 0xAE for Spanish version. Discoverer: @dragonflylee
                m_data[6] = 0x10;       // Must be 0x10 for Spanish version. Discoverer: @dragonflylee
                break;
            case navicat_software_language::French:
                m_data[5] = 0xFA;       // Must be 0xFA for French version. Discoverer: @Deltafox79
                m_data[6] = 0x20;       // Must be 0x20 for French version. Discoverer: @Deltafox79
                break;
            case navicat_software_language::German:
                m_data[5] = 0xB1;       // Must be 0xB1 for German version. Discoverer: @dragonflylee
                m_data[6] = 0x60;       // Must be 0x60 for German version. Discoverer: @dragonflylee
                break;
            case navicat_software_language::Korean:
                m_data[5] = 0xB5;       // Must be 0xB5 for Korean version. Discoverer: @dragonflylee
                m_data[6] = 0x60;       // Must be 0x60 for Korean version. Discoverer: @dragonflylee
                break;
            case navicat_software_language::Russian:
                m_data[5] = 0xEE;       // Must be 0xB5 for Russian version. Discoverer: @dragonflylee
                m_data[6] = 0x16;       // Must be 0x60 for Russian version. Discoverer: @dragonflylee
                break;
            case navicat_software_language::Portuguese:
                m_data[5] = 0xCD;       // Must be 0xCD for Portuguese version. Discoverer: @dragonflylee
                m_data[6] = 0x49;       // Must be 0x49 for Portuguese version. Discoverer: @dragonflylee
                break;
            default:
                break;
        }
    }

    void navicat_serial_generator::set_software_language(uint8_t lang_sig0, uint8_t lang_sig1) noexcept {
        m_data[5] = lang_sig0;
        m_data[6] = lang_sig1;
    }

    void navicat_serial_generator::set_software_type(navicat_software_type software_type) noexcept {
        switch (software_type) {
            case navicat_software_type::DataModeler:
                m_data[7] = 0x84;
                break;
            case navicat_software_type::Premium:
                m_data[7] = 0x65;
                break;
            case navicat_software_type::MySQL:
                m_data[7] = 0x68;
                break;
            case navicat_software_type::PostgreSQL:
                m_data[7] = 0x6C;
                break;
            case navicat_software_type::Oracle:
                m_data[7] = 0x70;
                break;
            case navicat_software_type::SQLServer:
                m_data[7] = 0x74;
                break;
            case navicat_software_type::SQLite:
                m_data[7] = 0x78;
                break;
            case navicat_software_type::MariaDB:
                m_data[7] = 0x7C;
                break;
            case navicat_software_type::MongoDB:
                m_data[7] = 0x80;
                break;
            case navicat_software_type::ReportViewer:
                m_data[7] = 0xb;
                break;
            default:
                break;
        }
    }

    void navicat_serial_generator::set_software_type(uint8_t software_type_sig) noexcept {
        m_data[7] = software_type_sig;
    }

    void navicat_serial_generator::set_software_version(int ver) {
        if (11 <= ver && ver < 16) {
            m_data[8] = static_cast<std::uint8_t>((ver << 4) | (m_data[8] & 0x0f));
            memcpy(m_des_key, s_des_key0, sizeof(s_des_key0));
        } else if (16 <= ver && ver < 32) {
            m_data[8] = static_cast<std::uint8_t>(((ver - 16) << 4) | (m_data[8] & 0x0f));
            memcpy(m_des_key, s_des_key1, sizeof(s_des_key1));
        } else {
            throw exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Invalid navicat version.");
        }
    }

    void navicat_serial_generator::generate() {
        RAND_bytes(m_data + 2, 3);

        DES_key_schedule schedule;
        DES_set_key_unchecked(&m_des_key, &schedule);
        DES_ecb_encrypt(reinterpret_cast<const_DES_cblock*>(m_data + 2), reinterpret_cast<const_DES_cblock*>(m_data + 2), &schedule, DES_ENCRYPT);

        m_serial_number = base32_rfc4648::encode(m_data, sizeof(m_data));
        std::transform(
            m_serial_number.begin(), m_serial_number.end(), m_serial_number.begin(), 
            [](char c) -> char {
                if (c == 'I') {
                    return '8';
                } else if (c == 'O') {
                    return '9';
                } else {
                    return c;
                }
            }
        );

        std::string_view sn = m_serial_number;
        m_serial_number_formatted = fmt::format("{}-{}-{}-{}", sn.substr(0, 4), sn.substr(4, 4), sn.substr(8, 4), sn.substr(12, 4));
    }

    [[nodiscard]]
    const std::string& navicat_serial_generator::serial_number() const noexcept {
        return m_serial_number;
    }

    [[nodiscard]]
    const std::string& navicat_serial_generator::serial_number_formatted() const noexcept {
        return m_serial_number_formatted;
    }
}

#undef NKG_CURRENT_SOURCE_LINE
#undef NKG_CURRENT_SOURCE_FILE
