#include <stdio.h>
#include <functional>

#include "exception.hpp"
#include "exceptions/operation_canceled_exception.hpp"

#include "cp_converter.hpp"
#include "base64_rfc4648.hpp"
#include "navicat_serial_generator.hpp"
#include "rsa_cipher.hpp"

#define NKG_CURRENT_SOURCE_FILE() u8".\\navicat-keygen\\wmain.cpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {
    using fnCollectInformation = std::function<navicat_serial_generator()>;
    using fnGenerateLicense = std::function<void(const rsa_cipher& cipher, const navicat_serial_generator& generator)>;

    navicat_serial_generator CollectInformationNormal();
    navicat_serial_generator CollectInformationAdvanced();
    void GenerateLicenseText(const rsa_cipher& cipher, const navicat_serial_generator& sn_generator);
    void GenerateLicenseBinary(const rsa_cipher& cipher, const navicat_serial_generator& sn_generator);
}

static void welcome() {
    _putws(L"***************************************************");
    _putws(L"*       navicat-keygen by @DoubleLabyrinth        *");
    _putws(L"*                version: 16.0.7.0                *");
    _putws(L"***************************************************");
    _putws(L"");
}

static void help() {
    _putws(L"Usage:");
    _putws(L"    navicat-keygen.exe <-bin|-text> [-adv] <RSA-2048 Private Key File>");
    _putws(L"");
    _putws(L"    <-bin|-text>       Specify \"-bin\" to generate \"license_file\" used by Navicat 11.");
    _putws(L"                       Specify \"-text\" to generate base64-encoded activation code.");
    _putws(L"                       This parameter must be specified.");
    _putws(L"");
    _putws(L"    [-adv]             Enable advance mode.");
    _putws(L"                       This parameter is optional.");
    _putws(L"");
    _putws(L"    <RSA-2048 Private Key File>    A path to an RSA-2048 private key file.");
    _putws(L"                                   This parameter must be specified.");
    _putws(L"");
    _putws(L"Example:");
    _putws(L"    navicat-keygen.exe -text .\\RegPrivateKey.pem");
}

int wmain(int argc, wchar_t* argv[]) {
    welcome();

    if (argc == 3 || argc == 4) {
        nkg::fnCollectInformation lpfnCollectInformation;
        nkg::fnGenerateLicense lpfnGenerateLicense;

        if (_wcsicmp(argv[1], L"-bin") == 0) {
            lpfnGenerateLicense = nkg::GenerateLicenseBinary;
        } else if (_wcsicmp(argv[1], L"-text") == 0) {
            lpfnGenerateLicense = nkg::GenerateLicenseText;
        } else {
            help();
            return -1;
        }

        if (argc == 4) {
            if (_wcsicmp(argv[2], L"-adv") == 0) {
                lpfnCollectInformation = nkg::CollectInformationAdvanced;
            } else {
                help();
                return -1;
            }
        } else {
            lpfnCollectInformation = nkg::CollectInformationNormal;
        }

        try {
            nkg::rsa_cipher cipher;

            cipher.import_private_key_file(nkg::cp_converter<-1, CP_UTF8>::convert(argv[argc - 1]));
            if (cipher.bits() != 2048) {
                throw nkg::exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"RSA key length mismatches.")
                    .push_hint(u8"You must provide an RSA key whose modulus length is 2048 bits.");
            }

            auto sn_generator = lpfnCollectInformation();

            sn_generator.generate();

            _putws(L"[*] Serial number:");
            _putws(nkg::cp_converter<CP_UTF8, -1>::convert(sn_generator.serial_number_formatted()).c_str());
            _putws(L"");

            lpfnGenerateLicense(cipher, sn_generator);

            return 0;
        } catch (nkg::exceptions::operation_canceled_exception&) {
            return -1;
        } catch (nkg::exception& e) {
            wprintf_s(L"[-] %s:%d ->\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.source_file()).c_str(), e.source_line());
            wprintf_s(L"    %s\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.custom_message()).c_str());

            if (e.error_code_exists()) {
                wprintf_s(L"    %s (0x%zx)\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.error_string()).c_str(), e.error_code());
            }

            for (auto& hint : e.hints()) {
                wprintf_s(L"    Hints: %s\n", nkg::cp_converter<CP_UTF8, -1>::convert(hint).c_str());
            }

            return -1;
        } catch (std::exception& e) {
            wprintf_s(L"[-] %s\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.what()).c_str());
            return -1;
        }
    } else {
        help();
        return -1;
    }
}

#undef NKG_CURRENT_SOURCE_FILE
#undef NKG_CURRENT_SOURCE_LINE
