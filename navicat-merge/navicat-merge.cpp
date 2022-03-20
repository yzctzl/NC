#include <stdio.h>
#include <windows.h>
#include <fmt/format.h>
#include <filesystem>
#include <optional>
#include <functional>

#include "cp_converter.hpp"

#include "resource_wrapper.hpp"
#include "resource_traits/cxx_object_traits.hpp"
#include "resource_traits/win32/file_handle.hpp"
#include "resource_traits/win32/generic_handle.hpp"
#include "resource_traits/win32/map_view_ptr.hpp"

#include "rsa_cipher.hpp"
#include "image_interpreter.hpp"
#include "patch_solution.hpp"
#include "patch_solution_since_16.0.7.0.hpp"

#include "exception.hpp"
#include "exceptions/operation_canceled_exception.hpp"
#include "exceptions/win32_exception.hpp"

#include "base64_rfc4648.hpp"
#include "navicat_serial_generator.hpp"
#include "rsa_cipher.hpp"

#define NKG_CURRENT_SOURCE_FILE() u8".\\navicat-merge\\navicat-merge.cpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {
    using fnCollectInformation = std::function<navicat_serial_generator()>;
    using fnGenerateLicense = std::function<void(const rsa_cipher& cipher, const navicat_serial_generator& generator)>;

    navicat_serial_generator CollectInformationNormal();
    navicat_serial_generator CollectInformationAdvanced();
    void GenerateLicenseText(const rsa_cipher& cipher, const navicat_serial_generator& sn_generator);
    void GenerateLicenseBinary(const rsa_cipher& cipher, const navicat_serial_generator& sn_generator);
}

void welcome() {
    _putws(L"***************************************************");
    _putws(L"*        navicat-merge by @DoubleLabyrinth        *");
    _putws(L"*               for ver.16.0.7.0+                 *");
    _putws(L"***************************************************");
    _putws(L"");
}


void help() {
    _putws(L"Usage:");
    _putws(L"    navicat-merge.exe [-h] [-adv] [Navicat Install Path]");
    _putws(L"");
    _putws(L"    [-h]                         Show help.");
    _putws(L"");
    _putws(L"    [-adv]                       Enable advance mode.");
    _putws(L"                                 This parameter is optional.");
    _putws(L"");
    _putws(L"    [Navicat Install Path]       Path to a directory where Navicat is installed.");
    _putws(L"                                 This parameter is \"C:\\Program Files\\PremiumSoft\\Navicat Premium 16\" by default.");
    _putws(L"                                 This parameter is optional.");
    _putws(L"");
    _putws(L"Example:");
    _putws(L"    navicat-merge.exe \"C:\\Program Files\\PremiumSoft\\Navicat Premium 16\"");
    _putws(L"");
}

bool parse_cmdline(int argc, wchar_t* argv[], std::filesystem::path& navicat_install_path, std::filesystem::path& rsa_privkey_filepath, nkg::fnCollectInformation& lpfnCollectInformation) {
    if (argc == 1) {
        lpfnCollectInformation = nkg::CollectInformationNormal;
        navicat_install_path = "C:\\Program Files\\PremiumSoft\\Navicat Premium 16";
        rsa_privkey_filepath.clear();
        return true;
    }
    else if (argc == 2) {
        if (_wcsicmp(argv[1], L"-h") == 0) {
            return false;
        }
        else if (_wcsicmp(argv[1], L"-adv") == 0) {
            lpfnCollectInformation = nkg::CollectInformationAdvanced;
            navicat_install_path = "C:\\Program Files\\PremiumSoft\\Navicat Premium 16";
            rsa_privkey_filepath.clear();
            return true;
        }
        else {
            lpfnCollectInformation = nkg::CollectInformationNormal;
            navicat_install_path = argv[1];
            rsa_privkey_filepath.clear();
            return true;
        }
    }
    else if (argc == 3) {
        if (_wcsicmp(argv[1], L"-adv") == 0) {
            lpfnCollectInformation = nkg::CollectInformationAdvanced;
            navicat_install_path = argv[2];
            rsa_privkey_filepath.clear();
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

void select_patch_solutions
(nkg::resource_wrapper<nkg::resource_traits::cxx_object_traits<nkg::patch_solution>>& solution0)
{
    return;
}

void load_rsa_privkey(nkg::rsa_cipher& cipher, std::filesystem::path& rsa_privkey_filepath, nkg::patch_solution* solution0) {
    if (!rsa_privkey_filepath.empty()) {
        wprintf_s(L"[*] Import RSA-2048 private key from\n");
        wprintf_s(L"    %s\n", rsa_privkey_filepath.native().c_str());

        cipher.import_private_key_file(rsa_privkey_filepath);

        if (solution0 && !solution0->check_rsa_privkey(cipher)) {
            throw nkg::exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"The RSA private key you provide cannot be used.");
        }
    }
    else {
        wprintf_s(L"[*] Generating new RSA private key, it may take a long time...\n");

        do {
            cipher.generate_key(2048);
        } while (solution0 && !solution0->check_rsa_privkey(cipher));   // re-generate RSA key if one of `check_rsa_privkey` returns false
    }

    wprintf_s(L"[*] Your RSA private key:\n%s\n", nkg::cp_converter<CP_UTF8, -1>::convert(cipher.export_private_key_string()).c_str());
}

template<typename... args_t>
bool all_patch_solutions_are_suppressed(args_t&&... args) {
    return (!args.is_valid() && ...);
}

void detect_backup(const std::filesystem::path& file_path) {
    std::filesystem::path backup_path = file_path.native() + L".bak";
    if (std::filesystem::is_regular_file(backup_path)) {
        while (true) {
            wprintf_s(L"[*] Previous backup %s is detected. Delete? (y/n)", backup_path.native().c_str());

            auto select = getwchar();
            while (select != L'\n' && getwchar() != L'\n') {}

            if (select == L'Y' || select == L'y') {
                std::filesystem::remove(backup_path);
                break;
            }
            else if (select == TEXT('N') || select == TEXT('n')) {
                throw nkg::exceptions::operation_canceled_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Backup file still exists. Patch abort!");
            }
            else {
                continue;
            }
        }
    }
}

void make_backup(const std::filesystem::path& file_path) {
    std::filesystem::path backup_path = file_path.native() + L".bak";
    if (std::filesystem::exists(backup_path)) {
        throw nkg::exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Previous backup is detected.")
            .push_hint(fmt::format(u8"Please delete {} and try again.", nkg::cp_converter<-1, CP_UTF8>::convert(backup_path.native())));
    }
    else {
        std::filesystem::copy_file(file_path, backup_path);
    }
}

int wmain(int argc, wchar_t* argv[]) {
    std::locale::global(std::locale(""));
    welcome();

    bool dry_run = false;
    std::filesystem::path navicat_install_path;
    std::filesystem::path rsa_privkey_filepath;

    nkg::fnCollectInformation lpfnCollectInformation;
    nkg::fnGenerateLicense lpfnGenerateLicense{nkg::GenerateLicenseText};

    if (parse_cmdline(argc, argv, navicat_install_path, rsa_privkey_filepath, lpfnCollectInformation)) {
        try {
            if (!std::filesystem::is_directory(navicat_install_path)) {
                throw nkg::exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Navicat install path doesn't point to a directory.")
                    .push_hint(u8"Are you sure the path you specified is correct?")
                    .push_hint(fmt::format(u8"The path you specified: {}", nkg::cp_converter<-1, CP_UTF8>::convert(navicat_install_path.native())));
            }

            if (!rsa_privkey_filepath.empty() && !std::filesystem::is_regular_file(rsa_privkey_filepath)) {
                throw nkg::exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"RSA key file path doesn't point to a file.")
                    .push_hint(u8"Are you sure the path you specified is correct?")
                    .push_hint(fmt::format(u8"The path you specified: {}", nkg::cp_converter<-1, CP_UTF8>::convert(rsa_privkey_filepath.native())));
            }

            nkg::rsa_cipher cipher;

            std::filesystem::path libcc_filepath = navicat_install_path / "libcc.dll";
            nkg::resource_wrapper libcc_handle{ nkg::resource_traits::win32::file_handle{} };
            nkg::resource_wrapper libcc_map_handle{ nkg::resource_traits::win32::generic_handle{} };
            nkg::resource_wrapper libcc_map_view{ nkg::resource_traits::win32::map_view_ptr{} };
            std::optional<nkg::image_interpreter> libcc_interpreter;

            nkg::resource_wrapper solution0{ nkg::resource_traits::cxx_object_traits<nkg::patch_solution>{} };

            // open libcc.dll
            libcc_handle.set(CreateFileW(libcc_filepath.native().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL));
            if (libcc_handle.is_valid()) {
                wprintf_s(L"[+] Try to open libcc.dll ... OK!\n");
            }
            else {
                if (GetLastError() == ERROR_FILE_NOT_FOUND) {
                    wprintf_s(L"[-] Try to open libcc.dll ... NOT FOUND!\n");
                }
                else {
                    throw nkg::exceptions::win32_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), GetLastError(), u8"Failed to open libcc.dll");
                }
            }

            if (libcc_handle.is_valid()) {
                libcc_map_handle.set(CreateFileMapping(libcc_handle.get(), NULL, PAGE_READWRITE, 0, 0, NULL));
                if (!libcc_map_handle.is_valid()) {
                    throw nkg::exceptions::win32_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), GetLastError(), u8"CreateFileMapping failed.");
                }

                libcc_map_view.set(MapViewOfFile(libcc_map_handle.get(), FILE_MAP_ALL_ACCESS, 0, 0, 0));
                if (!libcc_map_view.is_valid()) {
                    throw nkg::exceptions::win32_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), GetLastError(), u8"MapViewOfFile failed.");
                }

                libcc_interpreter = nkg::image_interpreter::parse(libcc_map_view.get(), true);

                solution0.set(new nkg::patch_solution_since<16, 0, 7, 0>(libcc_interpreter.value()));
            }

            _putws(L"");

            // find patch and decide which solution will be applied
            if (solution0.is_valid()) {
                auto patch_found = solution0->find_patch();
                _putws(L"");

                if (!patch_found) {
                    solution0.release();
                }
            }

            select_patch_solutions(solution0);

            if (all_patch_solutions_are_suppressed(solution0)) {
                throw nkg::exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"All patch solutions are suppressed. Patch abort!")
                    .push_hint(u8"Are you sure your navicat has not been patched/modified before?");
            }

            // load key
            load_rsa_privkey(cipher, rsa_privkey_filepath, solution0.get());

            // apply patch solutions
            if (dry_run) {
                _putws(L"*******************************************************");
                _putws(L"*               DRY-RUN MODE ENABLE!                  *");
                _putws(L"*             NO PATCH WILL BE APPLIED!               *");
                _putws(L"*******************************************************");
            }
            else {
                // save private key if not given
                if (rsa_privkey_filepath.empty()) {
                    cipher.export_private_key_file(u8"RegPrivateKey.pem");
                }

                // detecting backups
                if (solution0.is_valid()) {
                    detect_backup(libcc_filepath);
                }

                // make backup
                if (solution0.is_valid()) {
                    make_backup(libcc_filepath);
                }

                // make patch
                // no way to go back from here :-)
                if (solution0.is_valid()) {
                    solution0->make_patch(cipher);
                }

                // print new key file path
                if (rsa_privkey_filepath.empty()) {
                    wprintf_s(L"[*] New RSA-2048 private key has been saved to\n");
                    wprintf_s(L"    %s\n", (std::filesystem::current_path() / L"RegPrivateKey.pem").c_str());
                    wprintf_s(L"\n");
                }

                _putws(L"");
                _putws(L"*******************************************************");
                _putws(L"*           PATCH HAS BEEN DONE SUCCESSFULLY!         *");
                _putws(L"*                  HAVE FUN AND ENJOY~                *");
                _putws(L"*******************************************************");
            }
            // generator
            auto sn_generator = lpfnCollectInformation();
            sn_generator.generate();

            _putws(L"[*] Serial number:");
            _putws(nkg::cp_converter<CP_UTF8, -1>::convert(sn_generator.serial_number_formatted()).c_str());
            _putws(L"");

            lpfnGenerateLicense(cipher, sn_generator);

            return 0;
        }
        catch (nkg::exceptions::operation_canceled_exception&) {
            return -1;
        }
        catch (nkg::exception& e) {
            wprintf_s(L"[-] %s:%d ->\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.source_file()).c_str(), e.source_line());
            wprintf_s(L"    %s\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.custom_message()).c_str());
            if (e.error_code_exists()) {
                wprintf_s(L"    %s (0x%zx)\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.error_string()).c_str(), e.error_code());
            }

            for (auto& hint : e.hints()) {
                wprintf_s(L"    HINT: %s\n", nkg::cp_converter<CP_UTF8, -1>::convert(hint).c_str());
            }

            return -1;
        }
        catch (std::exception& e) {
            wprintf_s(L"[-] %s\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.what()).c_str());
            return -1;
        }

    }
    else {
        help();
        return -1;
    }
}

#undef NKG_CURRENT_SOURCE_LINE
#undef NKG_CURRENT_SOURCE_FILE
