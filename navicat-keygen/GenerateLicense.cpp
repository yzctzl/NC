#include "exception.hpp"
#include "exceptions/operation_canceled_exception.hpp"
#include "exceptions/win32_exception.hpp"

#include "resource_wrapper.hpp"
#include "resource_traits/win32/file_handle.hpp"

#include "cp_converter.hpp"
#include "base64_rfc4648.hpp"
#include "navicat_serial_generator.hpp"
#include "rsa_cipher.hpp"

#include <iostream>
#include <ctime>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#define NKG_CURRENT_SOURCE_FILE() u8".\\navicat-keygen\\GenerateLicense.cpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__

namespace nkg {

    void GenerateLicenseText(const rsa_cipher& cipher, const navicat_serial_generator& sn_generator) {
        std::wstring username;
        std::wstring organization;
        std::string u8_username;
        std::string u8_organization;

        std::wstring b64_request_code;
        std::vector<std::uint8_t> request_code;
        std::string u8_request_info;
        std::string u8_response_info;
        std::vector<std::uint8_t> response_code;
        std::wstring b64_response_code;

        std::wcout << L"[*] Your name: ";
        if (!std::getline(std::wcin, username)) {
            throw exceptions::operation_canceled_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Operation is canceled by user.");
        } else {
            u8_username = cp_converter<-1, CP_UTF8>::convert(username);
        }

        std::wcout << L"[*] Your organization: ";
        if (!std::getline(std::wcin, organization)) {
            throw exceptions::operation_canceled_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Operation is canceled by user.");
        } else {
            u8_organization = cp_converter<-1, CP_UTF8>::convert(organization);
        }

        std::wcout << std::endl;
        std::wcout << L"[*] Input request code in Base64: (Input empty line to end)" << std::endl;
        while (true) {
            std::wstring s;
            if (!std::getline(std::wcin, s)) {
                throw exceptions::operation_canceled_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Operation is canceled by user.");
            }

            if (s.empty()) {
                break;
            }

            b64_request_code.append(s);
        }

        if (b64_request_code.empty()) {
            throw exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Nothing inputs, abort!");
        }

        request_code = base64_rfc4648::decode(cp_converter<-1, CP_UTF8>::convert(b64_request_code));

        u8_request_info.resize((cipher.bits() + 7) / 8);
        u8_request_info.resize(cipher.private_decrypt(request_code.data(), request_code.size(), u8_request_info.data(), RSA_PKCS1_PADDING));
        while (u8_request_info.back() == '\x00') {
            u8_request_info.pop_back();
        }

        std::wcout << L"[*] Request Info:" << std::endl;
        std::wcout << cp_converter<CP_UTF8, -1>::convert(u8_request_info) << std::endl;
        std::wcout << std::endl;

        rapidjson::Document json;
        rapidjson::Value N_Key;
        rapidjson::Value N_Value;
        rapidjson::Value O_Key;
        rapidjson::Value O_Value;
        rapidjson::Value T_Key;
        rapidjson::Value T_Value;
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        //
        // begin to parse
        //
        json.Parse(u8_request_info.c_str());

        //
        // remove "Platform" info
        //
        json.RemoveMember(u8"P");

        //
        // set "Name" info
        //
        N_Key.SetString(u8"N", 1);
        N_Value.SetString(u8_username.c_str(), static_cast<rapidjson::SizeType>(u8_username.length()));

        //
        // set "Organization" info
        //
        O_Key.SetString(u8"O", 1);
        O_Value.SetString(u8_organization.c_str(), static_cast<rapidjson::SizeType>(u8_organization.length()));

        //
        // set "Time" info
        //
        T_Key.SetString(u8"T", 1);
        T_Value.SetUint(static_cast<unsigned int>(std::time(nullptr)));

        //
        // add "Name", "Organization" and "Time"
        //
        json.AddMember(N_Key, N_Value, json.GetAllocator());
        json.AddMember(O_Key, O_Value, json.GetAllocator());
        json.AddMember(T_Key, T_Value, json.GetAllocator());

        //
        // flush
        //
        json.Accept(writer);

        if (buffer.GetSize() > 240) {
            throw exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Response Info is too long.");
        }

        u8_response_info.assign(buffer.GetString(), buffer.GetSize());

        std::wcout << L"[*] Response Info:" << std::endl;
        std::wcout << cp_converter<CP_UTF8, -1>::convert(u8_response_info) << std::endl;
        std::wcout << std::endl;

        response_code.resize((cipher.bits() + 7) / 8);
        response_code.resize(cipher.private_encrypt(u8_response_info.data(), u8_response_info.size(), response_code.data(), RSA_PKCS1_PADDING));
        b64_response_code = cp_converter<CP_UTF8, -1>::convert(base64_rfc4648::encode(response_code));

        std::wcout << L"[*] Activation Code:" << std::endl;
        std::wcout << b64_response_code << std::endl;
        std::wcout << std::endl;
    }

    void GenerateLicenseBinary(const rsa_cipher& cipher, const navicat_serial_generator& sn_generator) {
        std::string utf8SerialNumber = sn_generator.serial_number();

        std::wstring username;
        std::wstring organization;
        std::string u8_username;
        std::string u8_organization;

        std::string u8_response_info;
        std::vector<std::uint8_t> response_code;

        std::wcout << L"[*] Your name: ";
        if (!std::getline(std::wcin, username)) {
            throw exceptions::operation_canceled_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Operation is canceled by user.");
        } else {
            u8_username = cp_converter<-1, CP_UTF8>::convert(username);
        }

        std::wcout << L"[*] Your organization: ";
        if (!std::getline(std::wcin, organization)) {
            throw exceptions::operation_canceled_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Operation is canceled by user.");
        } else {
            u8_organization = cp_converter<-1, CP_UTF8>::convert(organization);
        }

        rapidjson::Document json;
        rapidjson::Value N_Key;
        rapidjson::Value N_Value;
        rapidjson::Value O_Key;
        rapidjson::Value O_Value;
        rapidjson::Value T_Key;
        rapidjson::Value T_Value;
        rapidjson::Value K_Key;
        rapidjson::Value K_Value;
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        json.Parse("{}");
        K_Key.SetString("K", 1);
        K_Value.SetString(utf8SerialNumber.c_str(), static_cast<rapidjson::SizeType>(utf8SerialNumber.length()));
        N_Key.SetString("N", 1);
        N_Value.SetString(u8_username.c_str(), static_cast<rapidjson::SizeType>(u8_username.length()));
        O_Key.SetString("O", 1);
        O_Value.SetString(u8_organization.c_str(), static_cast<rapidjson::SizeType>(u8_organization.length()));
        T_Key.SetString("T", 1);
        T_Value.SetUint(static_cast<unsigned int>(std::time(nullptr)));

        json.AddMember(K_Key, K_Value, json.GetAllocator());
        json.AddMember(N_Key, N_Value, json.GetAllocator());
        json.AddMember(O_Key, O_Value, json.GetAllocator());
        json.AddMember(T_Key, T_Value, json.GetAllocator());

        //
        // flush
        //
        json.Accept(writer);

        if (buffer.GetSize() > 240) {
            throw exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Response Info is too long.");
        }

        u8_response_info.assign(buffer.GetString(), buffer.GetSize());
        
        std::wcout << L"[*] Response Info:" << std::endl;
        std::wcout << cp_converter<CP_UTF8, -1>::convert(u8_response_info) << std::endl;
        std::wcout << std::endl;

        response_code.resize((cipher.bits() + 7) / 8);
        response_code.resize(cipher.private_encrypt(u8_response_info.data(), u8_response_info.size(), response_code.data(), RSA_PKCS1_PADDING));

        resource_wrapper license_file{ resource_traits::win32::file_handle{}, CreateFileW(L"license_file", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) };
        if (license_file.is_valid() == false) {
            throw exceptions::win32_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), GetLastError(), u8"CreateFileW failed.");
        }

        if (DWORD _;  WriteFile(license_file.get(), response_code.data(), static_cast<DWORD>(response_code.size()), &_, NULL) == FALSE) {
            throw exceptions::win32_exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), GetLastError(), u8"WriteFile failed.");
        }

        std::wcout << L"[+] license_file has been generated." << std::endl;
    }

}

#undef NKG_CURRENT_SOURCE_FILE
#undef NKG_CURRENT_SOURCE_LINE
