#pragma once
#include "ISOdate.h"
#include "Buffer.h"

typedef struct RIndexRecord_struct
{
	long nOffset;		// from timestamp, add 24 for content
	long nWidth;		// full record, subtract 24 for content
	long nCategoryID;	// Category ID
} RIndexRecord;

class CLogStorage
{
	TCHAR szDataFilename[ _MAX_PATH ];	// The full path of the log data file on disk
	TCHAR szIndexFilename[ _MAX_PATH ]; // The full path of the log index file on disk
	CISOdate dLastEntry;				// date of last entry in the log
public:
	CLogStorage(void);
	~CLogStorage(void);
	// Sets the filename to use for the log
	BOOL SetLogName(const char * szName); 
	// Returns the number of log entries
	long Count(void);
	// returns the size of the log
	long Size(void);
	// returns the date of the last entry in the log
	CISOdate& LastEntry(void);
	// Adds new data to the log
	BOOL AppendToLog(CBuffer &objData, long nCategoryID);
	// returns given entry, as measured from the end.  returns false if error
	BOOL GetLogEntry(long nIndex, CISOdate& objTimestamp, CBuffer& objDst, long * pnCategoryID);
	// returns the last entry belonging to a given category
	BOOL GetLastOfCategory(long nCategoryID, CISOdate& objTimestamp, CBuffer& objDst);
	// checks the index file, then reindexes if needed.  returns false if work could not be completed.
	BOOL CheckIndexFile(void);
	// returns true if a valid header, with Date, Category and Width set
	BOOL ParseHeader(CBuffer objHeader, CISOdate& objDate, long *pnCategoryID, long *pnWidth);
};
