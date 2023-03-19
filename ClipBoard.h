#pragma once
#include "LogStorage.h"
#include "Buffer.h"
#include "ISOdate.h"

class CClipBoard;
typedef CClipBoard * PClipBoard;

class CClipBoard
{
private:
	CClipBoard(void);
	CClipBoard(const CClipBoard&);                 // Prevent copy-construction
	CClipBoard& operator=(const CClipBoard&);      // Prevent assignment
	CBuffer m_objSnapShot;						   // clipboard data
	BOOL	m_bFlushing;						   // clipboard is being flushed.  	
	UINT	m_nContentType;						   // the type of content stored on the clipboard
public:
	~CClipBoard(void);
	// Returns the singleton object.
	static PClipBoard Instance(void);
	// Removes singleton
	static void Done(void);
	// get data
	CBuffer& GetData(void);
	// set data
	void SetData(CBuffer &objData, UINT nContentType);
	// get a snapshot of the windows clipboard
	BOOL Refresh(HWND hWnd);
	// put data into the Windows clipboard
	BOOL Flush(HWND hWnd);
	// Writes clipboard data to the log
	BOOL AddToLog(CLogStorage& objLog);
	// returns true if Flush is executing
	BOOL IsFlushing(void);
	// sets to log entry, nIndex measured from end, returns true if successful
	BOOL SetToLogEntry(long nIndex, CLogStorage& objLog);
	// returns the content type
	UINT GetContentType(void);
	// sets the clipboard to the files in a HDROP structure
	BOOL SetToDropInfo(HDROP hDropInfo);
};



