#include "StdAfx.h"
#include "ISOdate.h"
#include "math.h"
#include <iomanip> 


CISOdate::CISOdate(void)
{
	Now(TRUE);
}


CISOdate::CISOdate(int nYear,int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nMilisecond, int nMinutesEast)
{
	int				nHdiff, nMdiff;
	char			cSign;

	if (nMinutesEast >= 0)
	{  // east
		cSign = '+';
		nMdiff = nMinutesEast;
	}
	else
	{  // west
		cSign = '-';
		nMdiff = 0 - nMinutesEast;
	}
	nHdiff = (nMdiff - (nMdiff % 60)) / 60;
	nMdiff = nMdiff - (nHdiff * 60);

	sprintf_s(m_szISO, ISODATELEN + 1, ISOFORMAT,
			nYear, nMonth, nDay, nHour,	nMinute, 
			nSecond, nMilisecond, cSign, nHdiff, nMdiff);
}


// sets to "now".  Local time if useLocal true, GMT if false  
void CISOdate::Now(BOOL useLocal)
{
	SYSTEMTIME		stLocal, stGMT;
	int				nDdiff, nHdiff, nMdiff;
	double			fMdiff;
	char			cSign;
	
	memset(m_szISO,'\0',ISODATELEN+1);

	GetSystemTime(&stGMT);
	if (useLocal)
	{
		GetLocalTime(&stLocal);
	}
	else
	{
		memcpy(&stLocal,&stGMT, sizeof(stGMT));
	}
	
	nDdiff = stLocal.wDay - stGMT.wDay;
	nDdiff = (nDdiff < -1) ?  1 : nDdiff;
	nDdiff = (nDdiff >  1) ? -1 : nDdiff;

	nMdiff = (nDdiff * 24 * 60) + (stLocal.wHour - stGMT.wHour  ) * 60 + (stLocal.wMinute - stGMT.wMinute); 
	nMdiff = (nMdiff < 0) ? (nMdiff + 24 * 60 ) : nMdiff;

	if (nMdiff >= 0)
	{  // east
		cSign = '+';
	}
	else
	{  // west
		cSign = '-';
		nMdiff = 0 - nMdiff;
	}

	fMdiff = (double) nMdiff;
	fMdiff = floor( (fMdiff / 5) + 0.5 ) * 5;
	nMdiff = (int) fMdiff; 
	
	nHdiff = (nMdiff - (nMdiff % 60)) / 60;
	nMdiff = nMdiff - (nHdiff * 60);

	sprintf_s(m_szISO, ISODATELEN + 1, ISOFORMAT,
			stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour,	stLocal.wMinute, 
			stLocal.wSecond, stLocal.wMilliseconds, cSign, nHdiff, nMdiff);
}


// sets the string
CISOdate::CISOdate(char * szISO)
{
	strcpy_s(m_szISO, ISODATELEN + 1,szISO);
}


// destroy object
CISOdate::~CISOdate(void)
{
}


// sets to given time
void CISOdate::SetDateTime(int nYear,int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nMilisecond)
{
	CISOdate	newValue(nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilisecond, MinutesEast());

	operator=(newValue);
}


// returns Year
int CISOdate::Year(void)
{
	int		nFields, nRetVal;

	try {
		nFields = sscanf_s(m_szISO, "%4d%*25s", &nRetVal);
		if (nFields = 1)
		{
			return nRetVal;
		}
		else
		{
			return -1;
		}
	} catch (...) {
		return -1;
	}
}

// returns month
int CISOdate::Month(void)
{
	int		nFields, nRetVal;

	try {
		nFields = sscanf_s(m_szISO, "%*5s%2d%*22s", &nRetVal);
		if (nFields = 1)
		{
			return nRetVal;
		}
		else
		{
			return -1;
		}
	} catch (...) {
		return -1;
	}
}

// returns day of month
int CISOdate::Day(void)
{
	int		nFields, nRetVal;

	try {
		nFields = sscanf_s(m_szISO, "%*8s%2d%*19s", &nRetVal);
		if (nFields = 1)
		{
			return nRetVal;
		}
		else
		{
			return -1;
		}
	} catch (...) {
		return -1;
	}
}

// returns hour
int CISOdate::Hour(void)
{
	int		nFields, nRetVal;

	try {
		nFields = sscanf_s(m_szISO, "%*11s%2d%*16s", &nRetVal);
		if (nFields = 1)
		{
			return nRetVal;
		}
		else
		{
			return -1;
		}
	} catch (...) {
		return -1;
	}
}

// returns minute
int CISOdate::Minute(void)
{
	int		nFields, nRetVal;

	try {
		nFields = sscanf_s(m_szISO, "%*14s%2d%*13s", &nRetVal);
		if (nFields = 1)
		{
			return nRetVal;
		}
		else
		{
			return -1;
		}
	} catch (...) {
		return -1;
	}
}

// returns second
int CISOdate::Second(void)
{
	int		nFields, nRetVal;

	try {
		nFields = sscanf_s(m_szISO, "%*17s%2d%*10s", &nRetVal);
		if (nFields = 1)
		{
			return nRetVal;
		}
		else
		{
			return -1;
		}
	} catch (...) {
		return -1;
	}
}

// returns milisecond
int CISOdate::Milisecond(void)
{
	int		nFields, nRetVal;

	try {
		nFields = sscanf_s(m_szISO, "%*20s%3d%*6s", &nRetVal);
		if (nFields = 1)
		{
			return nRetVal;
		}
		else
		{
			return -1;
		}
	} catch (...) {
		return -1;
	}
}

// returns GMT offset
int CISOdate::MinutesEast(void)
{
	int			nFields, nHdiff, nMdiff;
	char		cSign;

	try {
		nFields = sscanf_s(m_szISO, "%*23s%c%d%*1s%d",  &cSign, 1, &nHdiff, &nMdiff);
		
		if (nFields = 3)
		{
			if ((nHdiff > 23) || (nMdiff > 59))
			{
				return -32768;
			}
			if ((cSign != '+') && (cSign != '-'))
			{
				return -32768;
			}
			nMdiff = nHdiff * 60 + nMdiff;
			nMdiff = ( cSign == '+' ) ? nMdiff : ( 0 - nMdiff );

			return nMdiff;
		}
		else
		{
			return -32768;
		}
	} catch (...) {
		return -32768;
	}
}


// set to special "end of day" value, note: this is not ISO standard
void CISOdate::SetEndOfDay(int nYear,int nMonth, int nDay)
{
	SetDateTime(nYear,nMonth,nDay,0,0,0,0);

	m_szISO[11] = '2';
	m_szISO[12] = '4';
}


// return if end of day value
BOOL CISOdate::IsEndOfDay(void)
{
	return ( (m_szISO[11] == '2') && (m_szISO[12] == '4') ) ? TRUE : FALSE;
}


// return value in seconds since 1/1/1970 GMT.  Should be cast for FILETIME
__int64 CISOdate::FileTime(void)
{
	FILETIME			ftLocal;
	LARGE_INTEGER		nRetValue;
	__int64				nRetValueCast;
	SYSTEMTIME			stValue;
	__int64				nMdiff;
	BOOL				bEndOfDay;
	
	try {
		stValue.wYear = Year();
		stValue.wMonth = Month();
		stValue.wDay = Day();
		stValue.wHour = Hour();
		stValue.wMinute = Minute();
		stValue.wSecond = Second();
		stValue.wMilliseconds = Milisecond();
		nMdiff = MinutesEast();
	} catch( ... ) {
		return INVALIDDATE;
	}

	if ((stValue.wYear > 9999) || (stValue.wMonth > 12) || (stValue.wDay > 31) || (stValue.wHour > 24) ||
		(stValue.wMinute > 59) || (stValue.wSecond > 59) || (stValue.wMilliseconds > 999) || (nMdiff == -32768))
	{
		return INVALIDDATE;
	}

	nMdiff = ( 0 - nMdiff ); // we actually need to subtract here

	bEndOfDay = IsEndOfDay();
	
	if (bEndOfDay)
	{
		// make it start of day, we'll add 24 hours later
		stValue.wHour = 0;
	}

	if( SystemTimeToFileTime(&stValue,&ftLocal) == 0 )
	{
		return INVALIDDATE;
	}

	// convert, respecting that ftLocal may have boundry issues (LARGE_INTEGER doesn't)
	nRetValue.HighPart = ftLocal.dwHighDateTime;
	nRetValue.LowPart = ftLocal.dwLowDateTime;

	// make it GMT 
	nRetValue.QuadPart += nMdiff * 60 * 10000000;

	// add back EndOfDay 24 hours
	if (bEndOfDay)
	{
		nMdiff = 60 * 10000000;
		nMdiff = 24 * 60 * nMdiff;
		nRetValue.QuadPart += nMdiff;
	}

	memcpy((void *) &nRetValueCast, (void *) &nRetValue, 8); 

	return nRetValueCast;
}

// copies ISO string into a given string
void CISOdate::WriteToString(char * szDst)
{
	memcpy(szDst,m_szISO, ISODATELEN + 1);
}

// assignment 
CISOdate CISOdate::operator=(CISOdate dRHS)
{
	strcpy_s(m_szISO, ISODATELEN + 1, dRHS.m_szISO);

	return *this;
}

// assignment from COleDateTime 
CISOdate CISOdate::operator=(COleDateTime dRHS)
{
	SetDateTime(dRHS.GetYear(),dRHS.GetMonth(),dRHS.GetDay(),dRHS.GetHour(),dRHS.GetMinute(),dRHS.GetSecond(),0);

	return *this;
}


// date difference
double CISOdate::operator-(CISOdate dRHS)
{
	__int64			nSecondDiff;
	double			fSecondDiff;
	double			fMilisecondDiff;
	unsigned long	mNaN[2]={0xffffffff, 0x7fffffff};
	double			fNaN = *( double* )mNaN;

	
	if ((FileTime() == INVALIDDATE) || (dRHS.FileTime() == INVALIDDATE))
	{
		return fNaN ;
	}	
	else
	{
		// use FileTime to do down to seconds
		nSecondDiff = FileTime() - dRHS.FileTime();
		fSecondDiff = (double) (nSecondDiff + 0.0);
		// now do miliseconds, since left out of filetime calculation
		fMilisecondDiff = (double) (m_szISO[20] - dRHS.m_szISO[20]) / 10;
		fMilisecondDiff = fMilisecondDiff + ((m_szISO[21] - dRHS.m_szISO[21]) / 100);
		fMilisecondDiff = fMilisecondDiff + ((m_szISO[22] - dRHS.m_szISO[22]) / 1000);
		fSecondDiff += fMilisecondDiff; 

		return (double) nSecondDiff / (24.0 * 60.0 * 60.0);
	}
}


// Output a CISOdate
ostream & operator<<(ostream &stream, const CISOdate& dOutput)
{
	stream << dOutput.m_szISO; 

	return stream;
}


// Input a CISOdate
istream & operator>>(istream &stream, CISOdate& dInput)
{
	memset(dInput.m_szISO,0,ISODATELEN+1);
	stream >> setw(ISODATELEN) >> dInput.m_szISO;

	return stream;
}




