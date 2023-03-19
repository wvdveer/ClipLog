// ClipLogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ClipLog.h"
#include "ClipLogDlg.h"
#include "ClipBoard.h"
#include "Utility.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CClipLogDlg dialog




CClipLogDlg::CClipLogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CClipLogDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


void CClipLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOGDISPLAY, m_logDisplay);
}

BEGIN_MESSAGE_MAP(CClipLogDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_LBN_SELCHANGE(IDC_LOGDISPLAY, &CClipLogDlg::OnLbnSelchangeLogdisplay)
	ON_WM_DROPFILES()
	ON_WM_DESTROY()
	ON_WM_DRAWCLIPBOARD()
	ON_WM_CHANGECBCHAIN()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SHOWCLIPLOG, &CClipLogDlg::OnShowClipLog) 
END_MESSAGE_MAP()


// CClipLogDlg message handlers
BOOL CClipLogDlg::OnInitDialog()
{

	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// initialise log storage and display
	m_logStorage.SetLogName("ClipBrd");
	LoadList();

	// capture recent docs
	AddRecentDocuments();
	
	// set clipboard listener
	m_NextClipWindow = NULL; // need to initialise because SetClipboardViewer will callback
	m_NextClipWindow = SetClipboardViewer( );
	
	// we want to start minimized, but let the dialog finish initialisation
	PostMessage(WM_SYSCOMMAND,SC_MINIMIZE,0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}


LRESULT CClipLogDlg::OnShowClipLog(WPARAM wParam, LPARAM lParam)
{
	if (lParam == WM_LBUTTONUP)
	{
		NOTIFYICONDATA rNID = { 0 };
		rNID.hWnd = m_hWnd;
		rNID.uID = 1;
		rNID.hIcon = m_hIcon;
		Shell_NotifyIcon(NIM_DELETE, &rNID);
		ShowWindow(SW_RESTORE);
		SetForegroundWindow();
	}

	return 1;
}


void CClipLogDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
		return;
	}
	
	CDialog::OnSysCommand(nID, lParam);

	if (nID == SC_MINIMIZE)
	{
		NOTIFYICONDATA rNID = { 0 };
		rNID.cbSize = sizeof(rNID);
		rNID.hWnd = m_hWnd;
		rNID.uID = 1;
		rNID.uFlags = NIF_ICON | NIF_MESSAGE;
		rNID.hIcon = m_hIcon;
		rNID.uCallbackMessage = WM_SHOWCLIPLOG;
		Shell_NotifyIcon(NIM_ADD, &rNID);
		ShowWindow(SW_HIDE);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CClipLogDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CClipLogDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CClipLogDlg::OnLbnSelchangeLogdisplay()
{
	int			nSelected;

	// determine which line and load it
	nSelected = m_logDisplay.GetCurSel();
	if ( CClipBoard::Instance()->SetToLogEntry(nSelected + 1,m_logStorage) )   
	{
		CClipBoard::Instance()->Flush(m_hWnd);
	}
}


void CClipLogDlg::OnDropFiles(HDROP hDropInfo)
{
	USES_CONVERSION;

	AddRecentDocuments();

	if(CClipBoard::Instance()->SetToDropInfo(hDropInfo))
	{
		CClipBoard::Instance()->Flush(m_hWnd);
		if (CClipBoard::Instance()->AddToLog(m_logStorage))
		{
			AddDisplayLine(m_logStorage.LastEntry(), CClipBoard::Instance()->GetData(),
				CClipBoard::Instance()->GetContentType(),TRUE);
		}
	}
}



void CClipLogDlg::OnDestroy()
{
	// stop listening to clipboard events
	ChangeClipboardChain(m_NextClipWindow); 

	// get rid of singletons
	CClipBoard::Instance()->Done();
	CUtility::Instance()->Done();

	// get rid of taskbar icon
	NOTIFYICONDATA rNID = { 0 };
	rNID.hWnd = m_hWnd;
	rNID.uID = 1;
	rNID.hIcon = m_hIcon;
	Shell_NotifyIcon(NIM_DELETE, &rNID);

	// super
	CDialog::OnDestroy();
}


void CClipLogDlg::OnDrawClipboard()
{
	TCHAR			szErrMsg[80];

	CDialog::OnDrawClipboard();

	// forward to next window if needed
	if (m_NextClipWindow)
	{
		// pass on to next in chain
		::SendMessage(m_NextClipWindow,WM_DRAWCLIPBOARD,0,0);
	}
	
	AddRecentDocuments();

	// add to log if required
	if (!CClipBoard::Instance()->IsFlushing())  
	{
		// capture clipboard content
		CClipBoard::Instance()->Refresh(m_hWnd);
		if (CClipBoard::Instance()->GetData().IsValid())
		{
			if (CClipBoard::Instance()->AddToLog(m_logStorage))
			{
				AddDisplayLine(m_logStorage.LastEntry(), CClipBoard::Instance()->GetData(),
					CClipBoard::Instance()->GetContentType(),TRUE);
			}
			if (! (CClipBoard::Instance()->GetData().IsValid()) )
			{
				_stprintf_s(szErrMsg,80,_T("*** Error adding to log file (rc=%d) ***"),GetLastError());
				m_logDisplay.InsertString(0,szErrMsg); 			
			}
		}
	}
}


void CClipLogDlg::OnChangeCbChain(HWND hWndRemove, HWND hWndAfter)
{
	CDialog::OnChangeCbChain(hWndRemove, hWndAfter);

	if (m_NextClipWindow == hWndRemove)
	{
		m_NextClipWindow = hWndAfter;	
	}
	else
	{
		// pass on to next in chain
		::SendMessage(m_NextClipWindow,WM_CHANGECBCHAIN,(WPARAM) hWndRemove, (LPARAM) hWndAfter);	
	}
}


// Create an entry in the display
BOOL CClipLogDlg::AddDisplayLine(CISOdate& objTimeStamp, CBuffer& objContent, long nCategoryID, BOOL bSelect)
{
	USES_CONVERSION;
	
	char		szDate[ISODATELEN + 1];
	char		szSample[51];
	TCHAR		szDisplayLine[81];
	BOOL		bValid;
	
	bValid = ((objTimeStamp.FileTime() != INVALIDDATE) && (objContent.IsValid()));
	if (bValid)
	{
		if (objContent.Size() == 0)
		{
			return TRUE;
		}
		objTimeStamp.WriteToString(szDate);
		szDate[19] = '\0'; // truncate miliseconds and zone
		strcpy_s(szSample,51,"[unknown]");
		if ((nCategoryID & 0xffff) == CF_TEXT)
		{
			objContent.GetPart(szSample,0,50,TRUE);
		}
		else
		{
			if((nCategoryID & 0xffff) == CF_DIB)
			{
				strcpy_s(szSample,51,"[bitmap]");
			}
		}
		_stprintf_s(szDisplayLine,81,_T("%19s           %-50s"),
					A2T(szDate), A2T(szSample) );
		m_logDisplay.InsertString(0,szDisplayLine);
	}
	else
	{
		m_logDisplay.InsertString(0,_T("*** Error adding to log ***"));
		bSelect = FALSE;
	}
	if (bSelect)
	{
		m_logDisplay.SelItemRange(TRUE,0,0);
	}

	return bValid;
}


// adds recent documents to the log
void CClipLogDlg::AddRecentDocuments(void)
{
	CISOdate	objLastDate;
	CBuffer		objLastFile;
	CBuffer		objFilename;

	m_logStorage.GetLastOfCategory(LE_RECENTDOC + CF_TEXT,objLastDate,objLastFile);
	while ( CUtility::Instance()->GetRecentFile(objLastDate,objFilename) )
	{
		if ( ! (objFilename == objLastFile) )
		{
			m_logStorage.AppendToLog(objFilename, LE_RECENTDOC + CF_TEXT);
			AddDisplayLine(objLastDate, objFilename, LE_RECENTDOC + CF_TEXT, FALSE);		
		}
	}
}

// load in (or refresh)  the display list
void CClipLogDlg::LoadList(void)
{
	long		nLog = m_logStorage.Count();
	long		nCategoryID;
	CBuffer		objContent;
	CISOdate	objDate;

	m_logDisplay.ResetContent();
	nLog = (nLog > 30000) ? 30000 : nLog;  // display is limited
	while (nLog > 0) 
	{
		if (m_logStorage.GetLogEntry(nLog,objDate,objContent, &nCategoryID))
		{
			AddDisplayLine(objDate, objContent, nCategoryID, FALSE);
			nLog --;
		}
		else
		{
			nLog = 0;
		}
	}
}
