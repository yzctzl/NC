#include "navicat_serial_generator.hpp"
#include <iostream>
#include "exceptions/operation_canceled_exception.hpp"

#define NKG_CURRENT_SOURCE_FILE() u8".\\navicat-keygen\\CollectInformation.cpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {
    [[nodiscard]]
    navicat_serial_generator CollectInformationNormal(int procution_type, int language, int version) {
        navicat_serial_generator sn_generator;

        sn_generator.set_software_type(static_cast<navicat_software_type>(procution_type));
        sn_generator.set_software_language(static_cast<navicat_software_language>(language));
        sn_generator.set_software_version(version);

        return sn_generator;
    }

    [[nodiscard]]
    navicat_serial_generator CollectInformationAdvanced(std::uint8_t procution_type, std::uint8_t lang1, std::uint8_t lang2, int version) {
        navicat_serial_generator sn_generator;

        sn_generator.set_software_type(procution_type);
        sn_generator.set_software_language(lang1, lang2);
        sn_generator.set_software_version(version);

        return sn_generator;
    }
}

#undef NKG_CURRENT_SOURCE_FILE
#undef NKG_CURRENT_SOURCE_LINE
