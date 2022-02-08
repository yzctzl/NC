#include <stdio.h>
#include <windows.h>
#include <fmt/format.h>
#include <filesystem>
#include <optional>

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

#define NKG_CURRENT_SOURCE_FILE() u8".\\navicat-patcher\\wmain.cpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

void welcome() {
    _putws(L"***************************************************");
    _putws(L"*       navicat-patcher by @DoubleLabyrinth       *");
    _putws(L"*               version: 16.0.7.0                 *");
    _putws(L"***************************************************");
    _putws(L"");
}


void help() {
    _putws(L"usage:");
    _putws(L"    navicat-patcher.exe [-dry-run] <Navicat Install Path> [RSA-2048 PEM File Path]");
    _putws(L"");
    _putws(L"    [-dry-run]                   Run patcher without applying any patches.");
    _putws(L"                                 This parameter is optional.");
    _putws(L"");
    _putws(L"    <Navicat Install Path>       The folder path where Navicat is installed.");
    _putws(L"                                 This parameter must be specified.");
    _putws(L"");
    _putws(L"    [RSA-2048 PEM File Path]     The path to an RSA-2048 private key file.");
    _putws(L"                                 This parameter is optional.");
    _putws(L"                                 If not specified, an RSA-2048 private key file");
    _putws(L"                                 named \"RegPrivateKey.pem\" will be generated.");
    _putws(L"");
    _putws(L"example:");
    _putws(L"    navicat-patcher.exe \"C:\\Program Files\\PremiumSoft\\Navicat Premium 12\"");
    _putws(L"");
}

bool parse_cmdline(int argc, wchar_t* argv[], bool& dry_run, std::filesystem::path& navicat_install_path, std::filesystem::path& rsa_privkey_filepath) {
    if (argc == 2) {
        dry_run = false;
        navicat_install_path = argv[1];
        rsa_privkey_filepath.clear();
        return true;
    } else if (argc == 3) {
        if (_wcsicmp(argv[1], L"-dry-run") == 0) {
            dry_run = true;
            navicat_install_path = argv[2];
            rsa_privkey_filepath.clear();
            return true;
        } else {
            dry_run = false;
            navicat_install_path = argv[1];
            rsa_privkey_filepath = argv[2];
            return true;
        }
    } else if (argc == 4) {
        if (_wcsicmp(argv[1], L"-dry-run") == 0) {
            dry_run = true;
            navicat_install_path = argv[2];
            rsa_privkey_filepath = argv[3];
            return true;
        } else {
            return false;
        }
    } else {
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
    } else {
        wprintf_s(L"[*] Generating new RSA private key, it may take a long time...\n");

        do {
            cipher.generate_key(2048);
        } while (solution0 && !solution0->check_rsa_privkey(cipher));   // re-generate RSA key if one of `check_rsa_privkey` returns false
    }

    wprintf_s(L"[*] Your RSA private key:\n%s", nkg::cp_converter<CP_UTF8, -1>::convert(cipher.export_private_key_string()).c_str());
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
            } else if (select == TEXT('N') || select == TEXT('n')) {
                throw nkg::exceptions::operation_canceled_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Backup file still exists. Patch abort!");
            } else {
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
    } else {
        std::filesystem::copy_file(file_path, backup_path);
    }
}

int wmain(int argc, wchar_t* argv[]) {
    welcome();

    bool dry_run = false;
    std::filesystem::path navicat_install_path;
    std::filesystem::path rsa_privkey_filepath;

    if (parse_cmdline(argc, argv, dry_run, navicat_install_path, rsa_privkey_filepath)) {
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

            //
            // Open libcc.dll
            //
            libcc_handle.set(CreateFileW(libcc_filepath.native().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL));
            if (libcc_handle.is_valid()) {
                wprintf_s(L"[+] Try to open libcc.dll ... OK!\n");
            } else {
                if (GetLastError() == ERROR_FILE_NOT_FOUND) {
                    wprintf_s(L"[-] Try to open libcc.dll ... NOT FOUND!\n");
                } else {
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

            //
            // find patch
            //
            if (!solution0->find_patch()) {
                solution0.release();
            }

            _putws(L"");

            //
            // decide which solutions will be applied
            //
            select_patch_solutions(solution0);

            if (!solution0.is_valid()) {
                throw nkg::exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"No patch solution is ready. Patch abort!")
                    .push_hint(u8"Are you sure your navicat has not been patched/modified before?");
            }

            //
            // load key
            //
            load_rsa_privkey(cipher, rsa_privkey_filepath, solution0.get());

            _putws(L"");

            if (dry_run) {
                _putws(L"*******************************************************");
                _putws(L"*               DRY-RUN MODE ENABLE!                  *");
                _putws(L"*             NO PATCH WILL BE APPLIED!               *");
                _putws(L"*******************************************************");
            } else {
                //
                // save private key if not given
                //
                if (rsa_privkey_filepath.empty()) {
                    cipher.export_private_key_file(u8"RegPrivateKey.pem");
                }

                //
                // detecting backups
                //
                if (solution0.is_valid()) {
                    detect_backup(libcc_filepath);
                }

                //
                // make backup
                //
                if (solution0.is_valid()) {
                    make_backup(libcc_filepath);
                }

                //
                // make patch
                // no way to go back from here :-)
                //
                if (solution0.is_valid()) {
                    solution0->make_patch(cipher);
                }

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

            return 0;
        } catch (nkg::exception& e) {
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
    } else {
        help();
        return -1;
    }
}

#undef NKG_CURRENT_SOURCE_LINE
#undef NKG_CURRENT_SOURCE_FILE
