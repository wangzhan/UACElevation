
// ManualElevationDlg.h : header file
//

#pragma once


// CManualElevationDlg dialog
class CManualElevationDlg : public CDialogEx
{
// Construction
public:
	CManualElevationDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MANUALELEVATION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
