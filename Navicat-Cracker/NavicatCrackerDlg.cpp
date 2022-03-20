
// NavicatCrackerDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "NavicatCracker.h"
#include "NavicatCrackerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define NKG_CURRENT_SOURCE_FILE() u8".\\Navicat-Cracker\\NavicatCrackerDlg.cpp"
#define NKG_CURRENT_SOURCE_LINE() __LINE__


void select_patch_solutions
(nkg::resource_wrapper<nkg::resource_traits::cxx_object_traits<nkg::patch_solution>>& solution0)
{
	return;
}

void load_rsa_privkey(nkg::rsa_cipher& cipher, std::filesystem::path& rsa_privkey_filepath, nkg::patch_solution* solution0) {
	if (!rsa_privkey_filepath.empty()) {
		cipher.import_private_key_file(rsa_privkey_filepath);

		if (solution0 && !solution0->check_rsa_privkey(cipher)) {
			throw nkg::exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"The RSA private key you provide cannot be used.");
		}
	}
	else {
		do {
			cipher.generate_key(2048);
		} while (solution0 && !solution0->check_rsa_privkey(cipher));   // re-generate RSA key if one of `check_rsa_privkey` returns false
	}
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

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

//class CAboutDlg : public CDialogEx
//{
//public:
//	CAboutDlg();
//
//// 对话框数据
//#ifdef AFX_DESIGN_TIME
//	enum { IDD = IDD_ABOUTBOX };
//#endif
//
//	protected:
//	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
//
//// 实现
//protected:
//	DECLARE_MESSAGE_MAP()
//};
//
//CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
//{
//}
//
//void CAboutDlg::DoDataExchange(CDataExchange* pDX)
//{
//	CDialogEx::DoDataExchange(pDX);
//}
//
//BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
//END_MESSAGE_MAP()


// CNavicatCrackerDlg 对话框



CNavicatCrackerDlg::CNavicatCrackerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NavicatCracker_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNavicatCrackerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ActivationCode, m_actcode);
	DDX_Control(pDX, IDC_CopyBTN, m_copy);
	DDX_Control(pDX, IDC_GenActBTN, m_genact);
	DDX_Control(pDX, IDC_GenerateBTN, m_genkey);
	DDX_Control(pDX, IDC_InstallPath, m_path);
	DDX_Control(pDX, IDC_Key, m_key);
	DDX_Control(pDX, IDC_LanguageBox, m_lang);
	DDX_Control(pDX, IDC_Organ, m_org);
	DDX_Control(pDX, IDC_PatchBTN, m_patch);
	DDX_Control(pDX, IDC_InstallPath, m_path);
	DDX_Control(pDX, IDC_ProductionBox, m_product);
	DDX_Control(pDX, IDC_RequestCode, m_reqcode);
	DDX_Control(pDX, IDC_UserName, m_name);
	DDX_Control(pDX, IDC_VersionBox, m_ver);
	DDX_Control(pDX, IDC_KeyFile, m_keyfile);
	DDX_Control(pDX, IDC_OpenKeyBTN, m_open);
	DDX_Control(pDX, IDC_CheckKey, m_checkkey);
	DDX_Control(pDX, IDC_ADVANCE, m_checkadv);
	DDX_Control(pDX, IDC_LangHex1, m_langhex1);
	DDX_Control(pDX, IDC_LangHex2, m_langhex2);
	DDX_Control(pDX, IDC_ProdHex, m_prodhex);
	DDX_Control(pDX, IDC_hosts, m_hosts);
	DDX_Control(pDX, IDC_Logo, m_logo);
}

BEGIN_MESSAGE_MAP(CNavicatCrackerDlg, CDialogEx)
	
	ON_BN_CLICKED(IDC_PatchBTN, &CNavicatCrackerDlg::OnClickedPatchbtn)
	ON_BN_CLICKED(IDC_GenerateBTN, &CNavicatCrackerDlg::OnClickedGeneratebtn)
	ON_BN_CLICKED(IDC_GenActBTN, &CNavicatCrackerDlg::OnClickedGenActbtn)
	ON_BN_CLICKED(IDC_CopyBTN, &CNavicatCrackerDlg::OnClickedCopybtn)
	ON_BN_CLICKED(IDC_CheckKey, &CNavicatCrackerDlg::OnClickedCheckKey)
	ON_BN_CLICKED(IDC_OpenKeyBTN, &CNavicatCrackerDlg::OnClickedOpenKeybtn)
	ON_BN_CLICKED(IDC_OpenPath, &CNavicatCrackerDlg::OnBnClickedOpenpath)
	ON_BN_CLICKED(IDC_ADVANCE, &CNavicatCrackerDlg::OnBnClickedAdvance)
	ON_BN_CLICKED(IDC_About, &CNavicatCrackerDlg::OnBnClickedAbout)
	ON_BN_CLICKED(IDC_hosts, &CNavicatCrackerDlg::OnBnClickedhosts)
END_MESSAGE_MAP()


// CNavicatCrackerDlg 消息处理程序

BOOL CNavicatCrackerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_NORMAL);

	// 初始化代码
	m_ver.SetCurSel(0);
	m_product.SetCurSel(1);
	m_lang.SetCurSel(1);
	
	m_path.SetWindowTextW(defaultpath);
	m_name.SetWindowTextW(username);
	m_org.SetWindowTextW(organization);

	m_keyfile.SetWindowTextW(keyfile);

	// setup logo png
	CBitmap bitmap_logo;
	CPngImage png_image;
	png_image.Load(IDB_PNG1, nullptr);
	bitmap_logo.Attach(png_image.Detach());
	m_logo.SetBitmap(bitmap_logo);



	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

//void CNavicatCrackerDlg::OnSysCommand(UINT nID, LPARAM lParam)
//{
//	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
//	{
//		CAboutDlg dlgAbout;
//		dlgAbout.DoModal();
//	}
//	else
//	{
//		CDialogEx::OnSysCommand(nID, lParam);
//	}
//}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CNavicatCrackerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CNavicatCrackerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CNavicatCrackerDlg::OnClickedPatchbtn()
{
	CString pathData;
	m_path.GetWindowTextW(pathData);
	// patcher
	navicat_install_path = pathData.GetString();
	try {
		if (!std::filesystem::is_directory(navicat_install_path)) {
			throw nkg::exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Navicat install path doesn't point to a directory.")
				.push_hint(u8"Are you sure the path you specified is correct?")
				.push_hint(fmt::format(u8"The path you specified: {}", nkg::cp_converter<-1, CP_UTF8>::convert(navicat_install_path.native())));
		}

		std::filesystem::path libcc_filepath = navicat_install_path / "libcc.dll";
		nkg::resource_wrapper libcc_handle{ nkg::resource_traits::win32::file_handle{} };
		nkg::resource_wrapper libcc_map_handle{ nkg::resource_traits::win32::generic_handle{} };
		nkg::resource_wrapper libcc_map_view{ nkg::resource_traits::win32::map_view_ptr{} };
		std::optional<nkg::image_interpreter> libcc_interpreter;
		nkg::resource_wrapper solution0{ nkg::resource_traits::cxx_object_traits<nkg::patch_solution>{} };

		// open libcc.dll
		libcc_handle.set(CreateFileW(libcc_filepath.native().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL));
		if (!libcc_handle.is_valid()) {
			if (GetLastError() == ERROR_FILE_NOT_FOUND) {
				throw nkg::exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Try to open libcc.dll ... NOT FOUND!");
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

		// find patch and decide which solution will be applied
		if (solution0.is_valid()) {
			auto patch_found = solution0->find_patch();

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
		if (m_checkkey.GetCheck()) {
			CString cs_keypath{};
			m_keyfile.GetWindowTextW(cs_keypath);
			rsa_privkey_filepath = cs_keypath.GetString();
		}
		load_rsa_privkey(cipher, rsa_privkey_filepath, solution0.get());

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

		// patch hosts
		if (m_hosts.GetCheck()) {
			LPCWSTR lpshosts = _T("C:\\Windows\\System32\\drivers\\etc\\hosts");
			SetFileAttributesW(lpshosts, GetFileAttributes(lpshosts) & ~FILE_ATTRIBUTE_READONLY);
			CStdioFile hosts;
			if (hosts.Open(lpshosts, CFile::modeWrite)) {
				hosts.SeekToEnd();
				hosts.WriteString(_T("127.0.0.1\tactivate.navicat.com\n"));
			}
			else {
				AfxMessageBox(L"Can't modify HOSTS file!\nPlease check your permission!", MB_ICONSTOP);
			}
			hosts.Close();
		}

		// private key export
		if (!m_checkkey.GetCheck()) {
			if(AfxMessageBox(L"Patch Success Applied to libcc.dll!\nDO You Want to Export Private Key?", MB_ICONINFORMATION | MB_YESNOCANCEL) == IDYES){
				cipher.export_private_key_file(u8"Navicat_2048bit_rsa_private_key.pem");
			}
		}
		else {
			AfxMessageBox(L"Patch Success Applied to libcc.dll!\n", MB_ICONINFORMATION | MB_OK);
		}
		PATCH = TRUE;
	}
	catch (nkg::exception& e) {
		catched_message.Format(L"[-] %s:%d ->\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.source_file()).c_str(), e.source_line());
		catched_message.AppendFormat(L"    %s\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.custom_message()).c_str());
		if (e.error_code_exists()) {
			catched_message.AppendFormat(L"    %s (0x%zx)\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.error_string()).c_str(), e.error_code());
		}
		for (auto& hint : e.hints()) {
			catched_message.AppendFormat(L"    HINT: %s\n", nkg::cp_converter<CP_UTF8, -1>::convert(hint).c_str());
		}

		AfxMessageBox(catched_message, MB_ICONSTOP);
	}
	catch (std::exception& e) {
		catched_message.Format(L"[-] %s\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.what()).c_str());
		AfxMessageBox(catched_message, MB_ICONSTOP);
	}
}


void CNavicatCrackerDlg::OnClickedGeneratebtn()
{
	int verData;
	CString strData;
	verData = 16 - m_ver.GetCurSel();
	try {
		if (!m_checkadv.GetCheck()) {
			int proData = m_product.GetCurSel();
			int langData = m_lang.GetCurSel();

			lpfnCollectInformation = nkg::CollectInformationNormal;
			sn_generator = lpfnCollectInformation(proData, langData, verData);
		}
		else {
			CString temp;
			std::uint8_t prod, lang1, lang2;
			m_prodhex.GetWindowTextW(temp);
			prod = static_cast<std::uint8_t>(std::stoul(temp.GetString(), nullptr, 16));
			m_langhex1.GetWindowTextW(temp);
			lang1 = static_cast<std::uint8_t>(std::stoul(temp.GetString(), nullptr, 16));
			m_langhex2.GetWindowTextW(temp);
			lang2 = static_cast<std::uint8_t>(std::stoul(temp.GetString(), nullptr, 16));

			sn_generator = nkg::CollectInformationAdvanced(prod, lang1, lang2, verData);
		}
		sn_generator.generate();
		m_key.SetWindowTextW(nkg::cp_converter<CP_UTF8, -1>::convert(sn_generator.serial_number_formatted()).c_str());
		KEYGEN = TRUE;
	}
	catch (nkg::exception& e) {
		catched_message.Format(L"[-] %s:%d ->\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.source_file()).c_str(), e.source_line());
		catched_message.AppendFormat(L"    %s\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.custom_message()).c_str());
		if (e.error_code_exists()) {
			catched_message.AppendFormat(L"    %s (0x%zx)\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.error_string()).c_str(), e.error_code());
		}
		for (auto& hint : e.hints()) {
			catched_message.AppendFormat(L"    HINT: %s\n", nkg::cp_converter<CP_UTF8, -1>::convert(hint).c_str());
		}

		AfxMessageBox(catched_message, MB_ICONSTOP);
	}
	catch (std::exception& e) {
		catched_message.Format(L"[-] %s\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.what()).c_str());
		if (m_checkadv.GetCheck()) {
			catched_message.AppendFormat(L"In ADV mode Production should be 0x00~0xFF.\n");
			catched_message.AppendFormat(L"In ADV mode Language(each part) should be 0x00~0xFF.");
		}
		AfxMessageBox(catched_message, MB_ICONSTOP);
	}
	catched_message.Empty();
}


void CNavicatCrackerDlg::OnClickedGenActbtn()
{
	try {
		if (!PATCH) {
			if (!m_checkkey.GetCheck()) {
				throw nkg::exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Can't Generate Activation Code.")
					.push_hint(u8"Please Patch first Or Specified RSA private key.");
			}
			else {
				CString private_path;
				m_keyfile.GetWindowTextW(private_path);
				cipher.import_private_key_file(nkg::cp_converter<-1, CP_UTF8>::convert(private_path.GetString()));
			}
		}
		if (!KEYGEN) {
			throw nkg::exception(NKG_CURRENT_SOURCE_FILE(), NKG_CURRENT_SOURCE_LINE(), u8"Can't Generate Activation Code.")
				.push_hint(u8"Please Generate a key!");
		}
		lpfnGenerateLicense = nkg::GenerateLicenseText;
		m_name.GetWindowTextW(username);
		m_org.GetWindowTextW(organization);
		m_reqcode.GetWindowTextW(reqcode);
		actcode = lpfnGenerateLicense(cipher, sn_generator, username.GetString(), organization.GetString(), reqcode.GetString()).c_str();
		m_actcode.SetWindowTextW(actcode);
	}
	catch (nkg::exception& e) {
		catched_message.Format(L"[-] %s:%d ->\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.source_file()).c_str(), e.source_line());
		catched_message.AppendFormat(L"    %s\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.custom_message()).c_str());
		if (e.error_code_exists()) {
			catched_message.AppendFormat(L"    %s (0x%zx)\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.error_string()).c_str(), e.error_code());
		}
		for (auto& hint : e.hints()) {
			catched_message.AppendFormat(L"    HINT: %s\n", nkg::cp_converter<CP_UTF8, -1>::convert(hint).c_str());
		}

		AfxMessageBox(catched_message, MB_ICONSTOP);
	}
	catch (std::exception& e) {
		catched_message.Format(L"[-] %s\n", nkg::cp_converter<CP_UTF8, -1>::convert(e.what()).c_str());
		AfxMessageBox(catched_message, MB_ICONSTOP);
	}
	catched_message.Empty();
}


void CNavicatCrackerDlg::OnClickedCopybtn()
{
	CString source;
	m_key.GetWindowTextW(source);
	if (OpenClipboard())
	{
		//防止非ASCII语言复制到剪切板为乱码
		int buff_size = source.GetLength();
		CStringW strWide = CStringW(source);
		int nLen = strWide.GetLength();
		HANDLE clipbuffer = ::GlobalAlloc(GMEM_MOVEABLE, (nLen + 1) * 2);
		if (!clipbuffer)
		{
			::CloseClipboard();
			return;
		}
		char* buffer = (char*)::GlobalLock(clipbuffer);
		if (buffer != 0) {
			memset(buffer, 0, (static_cast<size_t>(nLen) + 1) * 2);
			memcpy_s(buffer, nLen * 2, strWide.GetBuffer(0), nLen * 2);
			strWide.ReleaseBuffer();
			::GlobalUnlock(clipbuffer);
			::EmptyClipboard();
			::SetClipboardData(CF_UNICODETEXT, clipbuffer);
			::CloseClipboard();
		}
		else {
			::CloseClipboard();
			return;
		}
	}
}


void CNavicatCrackerDlg::OnClickedCheckKey()
{
	if (m_checkkey.GetCheck()) {
		m_keyfile.EnableWindow();
		m_open.EnableWindow();
	}
	else {
		m_keyfile.EnableWindow(0);
		m_open.EnableWindow(0);
	}
}


void CNavicatCrackerDlg::OnClickedOpenKeybtn()
{
	CFileDialog openFileDlg(FALSE, L".", L"PrivateKey.pem", OFN_HIDEREADONLY | OFN_READONLY, L"Private Key (*.pem)|*.pem||", NULL);
	INT_PTR result = openFileDlg.DoModal();
	if (result == IDOK) {
		m_keyfile.SetWindowTextW(openFileDlg.GetPathName());
	}
	// 失焦解决方法："https://kira-96.github.io/" 一些基本没什么用的MFC技巧
	::SetForegroundWindow(::GetDesktopWindow());
	this->SetForegroundWindow();
}



void CNavicatCrackerDlg::OnBnClickedOpenpath()
{
	CFolderPickerDialog openFolderDlg;

	openFolderDlg.m_ofn.lpstrTitle = L"Choose The Navicat Installation Path:";
	openFolderDlg.m_ofn.lpstrInitialDir = L".";

	INT_PTR result = openFolderDlg.DoModal();
	if (result == IDOK) {
		m_path.SetWindowTextW(openFolderDlg.GetPathName());
	}
	::SetForegroundWindow(::GetDesktopWindow());
	this->SetForegroundWindow();
}


void CNavicatCrackerDlg::OnBnClickedAdvance()
{
	if (m_checkadv.GetCheck()) {
		m_lang.ShowWindow(SW_HIDE);
		m_product.ShowWindow(SW_HIDE);
		m_prodhex.ShowWindow(SW_SHOW);
		m_langhex1.ShowWindow(SW_SHOW);
		m_langhex2.ShowWindow(SW_SHOW);
	}
	else {
		m_lang.ShowWindow(SW_SHOW);
		m_product.ShowWindow(SW_SHOW);
		m_prodhex.ShowWindow(SW_HIDE);
		m_langhex1.ShowWindow(SW_HIDE);
		m_langhex2.ShowWindow(SW_HIDE);
	}
}


void CNavicatCrackerDlg::OnBnClickedAbout()
{
	AfxMessageBox(L"Author :  tgMrZ\nBased on DoubleSine's work\n\n\nDoubleSine yyds!!!", MB_OK | MB_ICONQUESTION);
}


void CNavicatCrackerDlg::OnBnClickedhosts()
{
	if (m_hosts.GetCheck()) {
		AfxMessageBox(L"Please make sure you have permission to modify HOSTS file!");
	}
}
