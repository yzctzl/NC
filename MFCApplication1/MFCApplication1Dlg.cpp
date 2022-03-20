
// MFCApplication1Dlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCApplication1Dlg 对话框



CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
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
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	
	ON_BN_CLICKED(IDC_PatchBTN, &CMFCApplication1Dlg::OnClickedPatchbtn)
	ON_BN_CLICKED(IDC_GenerateBTN, &CMFCApplication1Dlg::OnClickedGeneratebtn)
	ON_BN_CLICKED(IDC_GenActBTN, &CMFCApplication1Dlg::OnClickedGenactbtn)
	ON_BN_CLICKED(IDC_CopyBTN, &CMFCApplication1Dlg::OnClickedCopybtn)
END_MESSAGE_MAP()


// CMFCApplication1Dlg 消息处理程序

BOOL CMFCApplication1Dlg::OnInitDialog()
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

	// TODO: 在此添加额外的初始化代码
	m_ver.SetCurSel(0);
	m_product.SetCurSel(1);
	m_lang.SetCurSel(1);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCApplication1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCApplication1Dlg::OnPaint()
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
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMFCApplication1Dlg::OnClickedPatchbtn()
{
	CString pathData;
	m_path.GetWindowTextW(pathData);
	AfxMessageBox(pathData);
}


void CMFCApplication1Dlg::OnClickedGeneratebtn()
{
	int verData, proData, langData;
	CString strData;
	verData = 16 - m_ver.GetCurSel();
	proData = m_product.GetCurSel();
	langData = m_lang.GetCurSel();
	strData.Format(_T("%d - %d - %d"), verData, proData, langData);
	m_key.SetWindowTextW(strData);
	AfxMessageBox(strData);
}


void CMFCApplication1Dlg::OnClickedGenactbtn()
{
	
}


void CMFCApplication1Dlg::OnClickedCopybtn()
{
	CString source;
	m_key.GetWindowTextW(source);
	if (OpenClipboard())
	{
		//防止非ASCII语言复制到剪切板为乱码
		int buff_size = source.GetLength();
		CStringW strWide = CStringW(source);
		int nLen = strWide.GetLength();
		//将剪切板置空
		::EmptyClipboard();
		HANDLE clipbuffer = ::GlobalAlloc(GMEM_DDESHARE, (nLen + 1) * 2);
		if (!clipbuffer)
		{
			::CloseClipboard();
			return;
		}
		char* buffer = (char*)::GlobalLock(clipbuffer);
		memset(buffer, 0, (nLen + 1) * 2);
		memcpy_s(buffer, nLen * 2, strWide.GetBuffer(0), nLen * 2);
		strWide.ReleaseBuffer();
		::GlobalUnlock(clipbuffer);
		::SetClipboardData(CF_UNICODETEXT, clipbuffer);
		::CloseClipboard();
	}
}
