#include "StdAfx.h"
#include "LogStorage.h"
#include <fstream>
#include "ISOdate.h"

using namespace std;


TCHAR * GetComponentFile(const char * szDefaultFilename, const char * szExtension)
{
	USES_CONVERSION;

	static TCHAR	szRetValue [ _MAX_PATH ] = _T("");
	size_t			nRequiredSize;

	_tgetenv_s (&nRequiredSize, szRetValue, _MAX_PATH, _T("APPDATA"));
	if (nRequiredSize == 0)
	{
		_tcscpy_s(szRetValue,_T("C:"));
	}
	_tcscat_s(szRetValue,_T("\\"));
	_tcscat_s(szRetValue,A2T(szDefaultFilename));
	_tcscat_s(szRetValue,_T("."));
	_tcscat_s(szRetValue,A2T(szExtension));

	return szRetValue;
}


CLogStorage::CLogStorage(void)
{

}


CLogStorage::~CLogStorage(void)
{
}


// Sets the filename to use for the log
BOOL CLogStorage::SetLogName(const char * szName)
{
	CBuffer		objLastEntry;

	try {
		_tcscpy_s(szDataFilename, GetComponentFile(szName, "txt") );
		_tcscpy_s(szIndexFilename, GetComponentFile(szName, "ndx") );		
		
		if (! CheckIndexFile() )
		{
			return FALSE;
		}

		if( Count() == 0)
		{
			dLastEntry.SetDateTime(1970,1,1,0,0,0,0);

			return TRUE;
		}

		if ( ! GetLogEntry(1,dLastEntry,objLastEntry, NULL) )
		{ 
			return FALSE;
		}

		return TRUE;
	} catch(...) {
		return FALSE;
	}
}


// Returns the number of log entries
long CLogStorage::Count(void)
{	
	long		nRetValue;
	ofstream	fIndex;

	try {
		fIndex.open( szIndexFilename, ios_base::out | ios_base::app  | ios_base::binary);
		fIndex.seekp(0,ios_base::end);
		nRetValue = fIndex.tellp() / sizeof(RIndexRecord);
		fIndex.close();
	} catch(...) {
		nRetValue = -1;
	}

	return nRetValue;
}


// returns the size of the log
long CLogStorage::Size(void)
{
	long		nRetValue;
	ofstream	fData;

	try {
		fData.open( szDataFilename, ios_base::out | ios_base::app  | ios_base::binary);
		fData.seekp(0,ios_base::end);
		nRetValue = fData.tellp();
		fData.close();
	} catch(...) {
		nRetValue = -1;
	}

	return nRetValue;
}


// Adds new data to the log
BOOL CLogStorage::AppendToLog(CBuffer &objData, long nCategoryID)
{
	RIndexRecord	rNew;
	ofstream		fData, fIndex;
	CISOdate		dNow;
	char			szCatNSize[21];

	try {
		if (objData.Size() == 0)
		{
			return FALSE;
		}
		sprintf_s(szCatNSize,"[%08X][%08X]",nCategoryID,objData.Size());
		fData.open( szDataFilename, ios_base::out | ios_base::app  );
		fData.seekp(0,ios_base::end);
		rNew.nOffset = fData.tellp();	
		fData << "[" << dNow << "]" << szCatNSize << objData;
		rNew.nWidth = ((long) fData.tellp()) - rNew.nOffset;
		fData << endl;
		rNew.nCategoryID = nCategoryID;
		fData.flush();
		fData.close();
		
		if (rNew.nOffset < 0)
		{
			return FALSE;
		}

		fIndex.open( szIndexFilename, ios_base::out | ios_base::app  | ios_base::binary);
		fIndex.write((char *)&rNew, sizeof(rNew));
		fIndex.flush();
		fIndex.close();

		dLastEntry = dNow;

		return TRUE;
	} catch(...) {
		return FALSE;
	}
}


// returns given entry, as measured from the end.  returns false if error
BOOL CLogStorage::GetLogEntry(long nIndex, CISOdate& objTimestamp, CBuffer& objDst, long * pnCategoryID)
{
	RIndexRecord	rIdx;
	ifstream		fData, fIndex;
	long			nFileSize, nOffset;
	char			szDate[ISODATELEN + 1];

	if (nIndex <= 0)
	{
		return FALSE; 
	}
	fIndex.open( szIndexFilename, ios_base::in | ios_base::binary);
	fIndex.seekg(0,ios_base::end);
	nFileSize = fIndex.tellg();
	nOffset = nFileSize  - (nIndex * sizeof(rIdx));
	if ((nFileSize <= 0) || (nOffset < 0) )
	{
		fIndex.close();
		return FALSE; 
	}
	fIndex.seekg(nOffset);
	fIndex.read((char *) &rIdx,sizeof(rIdx));
	if (fIndex.bad())
	{
		fIndex.close();
		return FALSE;
	}
	fIndex.close();

	if (pnCategoryID != NULL)
	{
		* pnCategoryID = rIdx.nCategoryID;
	}

	fData.open(szDataFilename, ios_base::in);
	fData.seekg(rIdx.nOffset);
	if (fData.bad())
	{
		fData.close();
		return FALSE; 
	}
	objDst.NewBuffer(rIdx.nWidth,FALSE); 
	if (!objDst.IsValid())
	{
		fData.close();
		return FALSE; 
	}
	fData >> objDst;
	if (fData.bad() || (!objDst.IsValid()) )
	{
		fData.close();
		return FALSE; 
	}	
	fData.close();

	objDst.GetPart(szDate,1,ISODATELEN,TRUE);
	if (!objDst.IsValid())
	{ 
		return FALSE;
	}
	CISOdate	objDate(szDate);
	if (objDate.FileTime() == INVALIDDATE)
	{
		return FALSE;
	}
	objDst.RemoveHeader(ISODATELEN + 22);
	if (!objDst.IsValid())
	{
		return FALSE; 
	}
	objTimestamp = objDate;

	return TRUE;
}



// returns the date of the last entry in the log
CISOdate& CLogStorage::LastEntry(void)
{
	return dLastEntry;
}


// returns the last entry belonging to a given category
BOOL CLogStorage::GetLastOfCategory(long nCategoryID, CISOdate& objTimestamp, CBuffer& objDst)
{
	long			nIndex;
	RIndexRecord	rIdx;
	ifstream		fData, fIndex;
	ULONG			nFileSize;
	char			szDate[ISODATELEN + 1];

	objTimestamp.SetDateTime(1970,1,1,0,0,0,0);
	fIndex.open( szIndexFilename, ios_base::in | ios_base::binary);
	fIndex.seekg(0,ios_base::end);
	nFileSize = fIndex.tellg();
	if ((nFileSize <= 0) || ((sizeof(rIdx)) > nFileSize) )
	{
		fIndex.close();
		return FALSE; 
	}
	nIndex = 1;
	do {
		if ( (nIndex * sizeof(rIdx)) > nFileSize)
		{
			fIndex.close();
			return FALSE;
		}
		fIndex.seekg(0 - (nIndex * sizeof(rIdx)), ios_base::end);
		fIndex.read((char *) &rIdx,sizeof(rIdx));
		if (fIndex.bad())
		{
			fIndex.close();
			return FALSE;
		}
		nIndex ++;
	} while( rIdx.nCategoryID != nCategoryID );
	fIndex.close();

	fData.open(szDataFilename, ios_base::in);
	fData.seekg(rIdx.nOffset);
	if (fData.bad())
	{
		fData.close();
		return FALSE; 
	}
	objDst.NewBuffer(rIdx.nWidth,FALSE); 
	if (!objDst.IsValid())
	{
		fData.close();
		return FALSE; 
	}
	fData >> objDst;
	if (fData.bad() || (!objDst.IsValid()) )
	{
		fData.close();
		return FALSE; 
	}	
	fData.close();

	objDst.GetPart(szDate,1,ISODATELEN,TRUE);
	if (!objDst.IsValid())
	{ 
		return FALSE;
	}
	CISOdate	objDate(szDate);
	if (objDate.FileTime() == INVALIDDATE)
	{
		return FALSE;
	}
	objDst.RemoveHeader(ISODATELEN + 22);
	if (!objDst.IsValid())
	{
		return FALSE; 
	}
	objTimestamp = objDate;

	return TRUE;
}


// returns true if a valid header, with Category and Width set
BOOL CLogStorage::ParseHeader(CBuffer objHeader, CISOdate& objDate, long *pnCategoryID, long *pnWidth)
{
	// more here
	char	szHeader[ISODATELEN + 22 + 1];	
	int		nFields;
	char	szDate[30];
	char	lb1, lb2, lb3, rb1, rb2, rb3;
	long	nCategoryID, nWidth;

	objHeader.GetPart(szHeader,0,ISODATELEN + 22,TRUE);
	if (! objHeader.IsValid() )
	{
		return FALSE;
	}
	nFields = sscanf_s(szHeader, "%c%29s%c%c%8X%c%c%8X%c", &lb1, 1, szDate, 30, &rb1, 1, 
		&lb2, 1, &nCategoryID, &rb2, 1, &lb3, 1, &nWidth, &rb3, 1);
	if (nFields != 9)
	{
		return FALSE;
	}
	if ((lb1 != '[') || (rb1 != ']') || (lb2 != '[') || (rb2 != ']') || (lb3 != '[') || (rb3 != ']'))
	{
		return FALSE;
	}
	CISOdate objTemp(szDate);
	if (objTemp.FileTime() == INVALIDDATE)
	{
		return FALSE;
	}
	objDate = objTemp;
	if (pnCategoryID != NULL)
	{
		*pnCategoryID = nCategoryID;
	}
	if (pnWidth != NULL)
	{
		*pnWidth = nWidth;
	}

	return TRUE;
}


// checks the index file, then reindexes if needed.  returns false if work could not be completed.
BOOL CLogStorage::CheckIndexFile(void)
{
	ofstream		fDataOut, fIndex;
	ifstream		fData;
	BOOL			bNeedReindex;
	CBuffer			objHeader, objNextChar;
	CISOdate		objDate;
	RIndexRecord	rIdx;

	bNeedReindex = FALSE;
	fDataOut.open( szDataFilename, ios_base::out | ios_base::app ); // app implies no create
	fDataOut.seekp(0,ios_base::end);
	fIndex.open( szIndexFilename, ios_base::out | ios_base::app );  // app implies no create
	fIndex.seekp(0,ios_base::end);

	if (fDataOut.bad())
	{
		fDataOut.close();
		fIndex.close();
		return FALSE;
	}
	if (fIndex.bad() && fDataOut.good()) 
	{
		bNeedReindex = TRUE;
	}
	if ((fIndex.tellp() <= 0) && (fDataOut.tellp() > 0))
	{
		bNeedReindex = TRUE;
	}
	fDataOut.close();
	fIndex.close();
	if ( ! bNeedReindex )
	{
		// everything was ok
		return TRUE;
	}

	// we need to reindex ...
	objHeader.NewBuffer(ISODATELEN + 22,TRUE);
	objNextChar.NewBuffer(1,TRUE);
	if ((! objHeader.IsValid() ) || (! objNextChar.IsValid()))
	{
		return FALSE;
	}
	// truncate the index file
	fIndex.open( szIndexFilename, ios_base::out | ios_base::trunc | ios_base::binary  );
	if (fIndex.bad())  // we could not create it
	{
		fIndex.close();

		return FALSE;
	}
	// read the data file.
	fData.open( szDataFilename, ios_base::in);
	fData >> objHeader;
	// loop through the file
	while ( objHeader.IsValid() ) 
	{
		// get a valid header

		while (objHeader.IsValid() && (!ParseHeader(objHeader, objDate, &rIdx.nCategoryID, &rIdx.nWidth)))
		{
			if( fData.eof() )
			{   // we're done here...
				objHeader.MarkInvalid();
			}
			else
			{   // advance one char
				fData >> objNextChar;
				objHeader.Append(objNextChar);
				objHeader.RemoveHeader(1);
			}
		}
		if ( objHeader.IsValid() ) 
		{
			// so we now have a valid header
			rIdx.nOffset = fData.tellg();
			rIdx.nOffset -= objHeader.Size();
			rIdx.nWidth += objHeader.Size();
			fIndex.write((char *)&rIdx, sizeof(rIdx));		
	
			fData.seekg(rIdx.nOffset + rIdx.nWidth);
			fData >> objHeader;
		}		
	}
	fData.close();
	fIndex.flush();
	fIndex.close();
	
	// true, we needed to reindex, but we did it.
	return TRUE;
}


