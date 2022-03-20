
// MFCApplication1Dlg.h: 头文件
//

#pragma once


// CMFCApplication1Dlg 对话框
class CMFCApplication1Dlg : public CDialogEx
{
// 构造
public:
	CMFCApplication1Dlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_actcode;
	CButton m_copy;
	CButton m_genact;
	CButton m_genkey;
//	CEdit m_path;
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
	afx_msg void OnClickedGenactbtn();
	afx_msg void OnClickedCopybtn();
};
