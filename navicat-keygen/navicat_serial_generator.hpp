#pragma once
#include <string>
#include <vector>
#include <openssl/des.h>

namespace nkg {

    enum class navicat_software_language {
        English,
        SimplifiedChinese,
        TraditionalChinese,
        Japanese,
        Polish,
        Spanish,
        French,
        German,
        Korean,
        Russian,
        Portuguese
    };

    enum class navicat_software_type {
        DataModeler,
        Premium,
        MySQL,
        PostgreSQL,
        Oracle,
        SQLServer,
        SQLite,
        MariaDB,
        MongoDB,
        ReportViewer
    };

    class navicat_serial_generator {
    private:
        static inline const DES_cblock s_des_key0 = { 0x64, 0xAD, 0xF3, 0x2F, 0xAE, 0xF2, 0x1A, 0x27 };
        static inline const DES_cblock s_des_key1 = { 0xE9, 0x7F, 0xB0, 0x60, 0x77, 0x45, 0x90, 0xAE };

        uint8_t m_data[10];
        DES_cblock m_des_key;
        std::string m_serial_number;
        std::string m_serial_number_formatted;

    public:
        navicat_serial_generator() noexcept;

        void set_software_language(navicat_software_language lang) noexcept;
        void set_software_language(uint8_t lang_sig0, uint8_t lang_sig1) noexcept;

        void set_software_type(navicat_software_type software_type) noexcept;
        void set_software_type(uint8_t software_type_sig) noexcept;

        void set_software_version(int Version);

        void generate();

        [[nodiscard]]
        const std::string& serial_number() const noexcept;

        [[nodiscard]]
        const std::string& serial_number_formatted() const noexcept;
    };

}

