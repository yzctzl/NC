
// NavicatCrackerDlg.h: 头文件
//

#pragma once
#include <filesystem>

#include <stdio.h>
#include <windows.h>
#include <fmt/format.h>
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


namespace nkg {
	using fnCollectInformation = std::function<navicat_serial_generator(int procution_type, int language, int version)>;
	using fnGenerateLicense = std::function<std::wstring(const rsa_cipher& cipher, const navicat_serial_generator& sn_generator, std::wstring _username,
		std::wstring _organization, std::wstring _redcode)>;

	navicat_serial_generator CollectInformationNormal(int procution_type, int language, int version);
	navicat_serial_generator CollectInformationAdvanced(std::uint8_t procution_type, std::uint8_t lang1, std::uint8_t lang2, int version);
	std::wstring GenerateLicenseText(const rsa_cipher& cipher, const navicat_serial_generator& sn_generator, std::wstring _username,
		std::wstring _organization, std::wstring _redcode);
	//void GenerateLicenseBinary(const rsa_cipher& cipher, const navicat_serial_generator& sn_generator);
}

// CNavicatCrackerDlg 对话框
class CNavicatCrackerDlg : public CDialogEx
{
// 构造
public:
	CNavicatCrackerDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NavicatCracker_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	std::filesystem::path navicat_install_path;
	std::filesystem::path rsa_privkey_filepath;

	nkg::rsa_cipher cipher;
	nkg::navicat_serial_generator sn_generator;
	nkg::fnCollectInformation lpfnCollectInformation;
	nkg::fnGenerateLicense lpfnGenerateLicense;
	CString catched_message{  };

	CString defaultpath{ "C:\\Program Files\\PremiumSoft\\Navicat Premium 16" };
	CString username{ "tgMrZ" };
	CString organization{ "DoubleSine" };
	CString reqcode;
	CString actcode;
	CString keyfile{ "Navicat_2048bit_rsa_private_key.pem" };
	BOOL PATCH{ FALSE }; // patched
	BOOL KEYGEN{ FALSE };  // keygened

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	//afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_actcode;
	CButton m_copy;
	CButton m_genact;
	CButton m_genkey;
	CEdit m_key;
	CComboBox m_lang;
	CEdit m_org;
	CButton m_patch;
	CEdit m_path;
	CComboBox m_product;
	CEdit m_reqcode;
	CEdit m_name;
	CComboBox m_ver;
	afx_msg void OnClickedPatchbtn();
	afx_msg void OnClickedGeneratebtn();
	afx_msg void OnClickedGenActbtn();
	afx_msg void OnClickedCopybtn();

	CEdit m_keyfile;
	CBitmapButton m_open;
	CButton m_checkkey;
	CButton m_checkadv;
	CEdit m_langhex1;
	CEdit m_langhex2;
	CEdit m_prodhex;
	CButton m_hosts;

	afx_msg void OnClickedCheckKey();
	afx_msg void OnClickedOpenKeybtn();
	afx_msg void OnBnClickedOpenpath();
	afx_msg void OnBnClickedAdvance();
	afx_msg void OnBnClickedAbout();
	afx_msg void OnBnClickedhosts();
	CStatic m_logo;
};
