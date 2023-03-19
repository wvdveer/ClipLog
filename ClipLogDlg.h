// ClipLogDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "Logstorage.h"

#define LE_CLIPBRD 0
#define LE_RECENTDOC 0x20000
#define WM_SHOWCLIPLOG (WM_USER + 636)

// CClipLogDlg dialog
class CClipLogDlg : public CDialog
{
// Construction
public:
	CClipLogDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CLIPLOG_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	HWND m_NextClipWindow;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	// Display of the log entries
	CListBox m_logDisplay;
	//Storage of the log entries
	CLogStorage m_logStorage; 
	afx_msg void OnLbnSelchangeLogdisplay();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnDestroy();
	afx_msg void OnDrawClipboard();
	afx_msg void OnChangeCbChain(HWND hWndRemove, HWND hWndAfter);
	afx_msg LRESULT OnShowClipLog(WPARAM wParam, LPARAM lParam);
	// Create an entry in the display
	BOOL AddDisplayLine(CISOdate& objTimeStamp, CBuffer& objContent, long nCategoryID, BOOL bSelect);
	// adds recent documents to the log
	void AddRecentDocuments(void);
	// load in (or refresh)  the display list
	void LoadList(void);
};
