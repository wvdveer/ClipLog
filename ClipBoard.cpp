#include "StdAfx.h"
#include "ClipBoard.h"

// pointer to the singleton object
static PClipBoard pSingletonClipboard = NULL;

CClipBoard::CClipBoard(void)
{
	m_bFlushing = FALSE;	
	m_nContentType = 0;
}

CClipBoard::~CClipBoard(void)
{
	Done();
}


// Returns the singleton object.
PClipBoard CClipBoard::Instance(void)
{
	if (pSingletonClipboard == NULL)
	{
		pSingletonClipboard = new CClipBoard;
	}

	return pSingletonClipboard;
}


void CClipBoard::Done(void)
{
	if (pSingletonClipboard != NULL)
	{
		pSingletonClipboard->m_objSnapShot.Clear();
		PClipBoard pobjTempClip = pSingletonClipboard;		
		pSingletonClipboard = NULL;
		delete pobjTempClip;
	}	
}


// get data
CBuffer& CClipBoard::GetData(void)
{
	return m_objSnapShot;
}


// set data
void CClipBoard::SetData(CBuffer &objData, UINT nContentType)
{
	m_objSnapShot = objData;
	m_nContentType = nContentType;
}


// capture system clipboard content.  true if successful
BOOL CClipBoard::Refresh(HWND hWnd)
{
	unsigned long		nDataSize;
	LPVOID				pvSrc;


	if (m_bFlushing)
	{
		return FALSE;
	}
	try {
		m_objSnapShot.Clear();
		m_nContentType = 0;
		// capture clipboard content
		OpenClipboard(hWnd); 
		HGLOBAL hg = GetClipboardData(CF_TEXT); // use text if available
		if (hg != NULL)
		{
			nDataSize = (hg == NULL) ? 0 : GlobalSize(hg); // Determine the data size 
			if  ((nDataSize != 0) && (nDataSize < 32000))
			{
				pvSrc = GlobalLock(hg);
				m_objSnapShot.SetData(pvSrc,nDataSize); // Grab the data 
				GlobalUnlock(hg); 			
				m_nContentType = CF_TEXT;
				m_objSnapShot.ClearTrailingNulls();
			}
		}
		else // its not text
		{
			// try DIB
			hg = GetClipboardData(CF_DIB);
			if (hg != NULL)
			{
				nDataSize = (hg == NULL) ? 0 : GlobalSize(hg); // Determine the data size 
				if  ((nDataSize != 0) && (nDataSize < 32000000))
				{
					pvSrc = GlobalLock(hg);
					m_objSnapShot.SetData(pvSrc,nDataSize); // Grab the data 
					GlobalUnlock(hg); 			
					m_nContentType = CF_DIB;
				}
			}	
			else 
			{
				hg = GetClipboardData(CF_HDROP);
				if (hg != NULL)
				{	
					SetToDropInfo((HDROP) hg);
				}
			}
		}
		if (m_nContentType == 0)
		{
				m_objSnapShot.Clear();
				m_objSnapShot.MarkInvalid();
		}
		CloseClipboard(); 	
	} catch(...) {
		return FALSE;
	}
	return TRUE;
}


// put m_objSnapShot into the Windows clipboard
BOOL CClipBoard::Flush(HWND hWnd)
{
//	unsigned long		nDataSize;
	LPVOID				pvDst;
	HANDLE				hClip; 

	if(! m_objSnapShot.IsValid())
	{ 
		return FALSE;
	}
	HLOCAL hl = LocalAlloc(LMEM_MOVEABLE,m_objSnapShot.Size() +1);		
	if (hl == NULL)
	{
		return FALSE;
	}
	pvDst = LocalLock(hl);
	if (pvDst == NULL)
	{
		return FALSE;
	}	
	m_objSnapShot.GetPart(pvDst,0,m_objSnapShot.Size(),FALSE); 
	LocalUnlock(hl);
	if((GetLastError() != NO_ERROR) || (! m_objSnapShot.IsValid()) )
	{ 
		LocalFree(hl);
		return FALSE;
	}
	hClip = NULL;
	try {
		m_bFlushing = TRUE;

		OpenClipboard(hWnd); 
		
		if( EmptyClipboard() )  // this clears then gets ownership, which we need
		{
			HANDLE hClip = SetClipboardData(m_nContentType,hl); 
		}
		CloseClipboard(); 	
	} catch(...) {
		m_bFlushing = FALSE;
		return FALSE;
	} 
	m_bFlushing = FALSE;
	return (hClip != NULL) ? TRUE : FALSE;
}


// Writes clipboard contents to the log
BOOL CClipBoard::AddToLog(CLogStorage& objLog)
{
	CISOdate	objLastDate;
	CBuffer		objLastEntry, objNewEntry;

	if (m_objSnapShot.Size() == 0)
	{
		return FALSE;
	}
	objLog.GetLogEntry(1,objLastDate,objLastEntry, NULL);
	if ( !objLastEntry.IsValid())
	{
		return FALSE;
	}
	if (m_objSnapShot == objLastEntry)
	{
		return FALSE;
	}
	objLastEntry.Clear();
	objNewEntry = m_objSnapShot;
	if (m_nContentType != CF_TEXT)
	{
		objNewEntry.Base64Encode();
	}
	return objLog.AppendToLog(objNewEntry,m_nContentType);
}



// returns true if Flush is executing
BOOL CClipBoard::IsFlushing(void)
{
	return m_bFlushing;
}

// sets to log entry, nIndex measured from end, returns true if successful
BOOL CClipBoard::SetToLogEntry(long nIndex, CLogStorage& objLog)
{
	CISOdate objTimeStamp;
	CBuffer	 objContent;
	long	 nCategoryID;

	if(objLog.GetLogEntry(nIndex,objTimeStamp,objContent, &nCategoryID))
	{
		SetData(objContent,nCategoryID & 0xffff);
		if (m_nContentType != CF_TEXT)
		{
			m_objSnapShot.Base64Decode();
		}		
		return m_objSnapShot.IsValid();
	}
	else
	{
		return FALSE;
	}
}

// returns the content type
UINT CClipBoard::GetContentType(void)
{
	return m_nContentType;
}


// sets the clipboard to the files in a HDROP structure
BOOL CClipBoard::SetToDropInfo(HDROP hDropInfo)
{
	USES_CONVERSION;

	TCHAR		szFilename[251];
	CBuffer		objFilename, objLF;
	long		nLoop, nCount;

	m_objSnapShot.Clear();
	objLF.SetToString("\x0D"); // carriage return
	objLF.ClearTrailingNulls();
	nCount = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	for(nLoop =0 ; nLoop < nCount; nLoop ++)
	{
		if (nLoop > 0)
		{
			m_objSnapShot.Append(objLF); // separate with cr
		}
		DragQueryFile(hDropInfo, nLoop, szFilename, 250);
		objFilename.SetToString(T2A(szFilename));
		objFilename.ClearTrailingNulls();
		m_objSnapShot.Append(objFilename);
	}
	if (m_objSnapShot.IsValid())
	{
		m_nContentType = CF_TEXT;
		return TRUE;
	}
	else
	{
		m_nContentType = 0;
		return FALSE;
	}
}
