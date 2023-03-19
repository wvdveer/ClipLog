#include "StdAfx.h"
#include "Buffer.h"
#include <iomanip> 

#define BASE64 "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

CBuffer::CBuffer(void)
{
	pvData = NULL;
	nDataSize = 0;
	bValid = TRUE;
}


// copy constructor
CBuffer::CBuffer(const CBuffer& objRHS)
{
	pvData = NULL;
	nDataSize = 0;
	if (!objRHS.bValid)
	{
		bValid = FALSE;
	}
	NewBuffer(objRHS.nDataSize,FALSE);
	if (bValid)
	{
		memcpy(pvData,objRHS.pvData,nDataSize);
	}
}


CBuffer::~CBuffer(void)
{
	Clear();
}


// Get rid of the old space and allocate new space.
LPVOID CBuffer::NewBuffer(unsigned long nNewSize, BOOL bZero)
{
	Clear();
	if (bValid)
	{
		pvData = HeapAlloc(GetProcessHeap(), 0, nNewSize);
	}
	else
	{
		pvData = NULL;
	}
	if (pvData == NULL)
	{
		bValid = FALSE;
		nDataSize = 0;
	}
	else
	{
		nDataSize = nNewSize;
		bValid = TRUE;
		if (bZero)
		{
			memset(pvData,0,nDataSize);
		}
	}
	return pvData;
}


// remove buffer
void CBuffer::Clear(void)
{
	if (nDataSize > 0)
	{
		HeapFree(GetProcessHeap(), 0, pvData);
	}
	nDataSize = 0;
	pvData = NULL;
	bValid = TRUE;
}


// returns the size of the data
unsigned long CBuffer::Size(void)
{
	return (bValid) ? nDataSize : 0;
}

// sets the buffer contents to the given value
void CBuffer::SetData(LPVOID pvSrc, long nNewSize)
{
	NewBuffer(nNewSize,FALSE);
	if (bValid)
	{
		memcpy(pvData,pvSrc,nNewSize);
	}
}


// mark the data as invalid
void CBuffer::MarkInvalid(void)
{
	Clear();
	bValid = FALSE;
}


// Sets content to value of passed in null terminated string 
void CBuffer::SetToString(const char * szContent)
{
	char *	cBuffer;

	cBuffer = (char *) NewBuffer(strlen(szContent)+1,FALSE);
	if (bValid)
	{
		strncpy_s((char *)pvData, nDataSize, szContent, nDataSize-1);
	}
}

// Adds a trailer to the buffer
BOOL CBuffer::Append(CBuffer & objAppendix)
{
	LPVOID		pvNew;
	char *		pvTail;
	
	if ((!bValid) || (!objAppendix.bValid))
	{
		return FALSE;
	}
	if (objAppendix.nDataSize == 0)
	{
		return TRUE;
	}
	if (nDataSize == 0)
	{
		operator=(objAppendix);
	}
	else
	{
		pvNew = HeapReAlloc(GetProcessHeap(),HEAP_REALLOC_IN_PLACE_ONLY,pvData,nDataSize + objAppendix.nDataSize);
		if (pvNew == NULL)
		{
			pvNew = HeapAlloc(GetProcessHeap(), 0, nDataSize + objAppendix.nDataSize);		
			if (pvNew != NULL)
			{
				memcpy(pvNew,pvData,nDataSize);
				pvData = pvNew;
			}
		}
		if (pvNew == NULL)
		{
			bValid = FALSE;
		}
		else
		{
			pvTail = (char *) pvData;
			pvTail += nDataSize;
			memcpy((void *)pvTail, objAppendix.pvData, objAppendix.nDataSize);
			nDataSize += objAppendix.nDataSize;
		}
	}
	return bValid;
}


// Removes a header from a buffer
BOOL CBuffer::RemoveHeader(unsigned long nHeaderLen)
{
	LPVOID		pvNew;
	char *		pvTail;
	long		nNewSize;

	if (!bValid)
	{
		return FALSE;
	}
	if (nHeaderLen > nDataSize)
	{
		bValid = FALSE;
		return FALSE;
	}
	if (nHeaderLen == nDataSize)
	{
		Clear();
		return bValid;
	}
	nNewSize = nDataSize - nHeaderLen;
	pvNew = HeapAlloc(GetProcessHeap(), 0, nNewSize);
	if (pvNew == NULL)
	{
		bValid = FALSE;
		return FALSE;
	}
	pvTail = (char *) pvData;
	pvTail += nHeaderLen;
	memcpy(pvNew,(void *) pvTail,nNewSize);
	Clear();
	pvData = pvNew;
	nDataSize = nNewSize;

	return bValid;
}


// Removes a trailer from a buffer
BOOL CBuffer::Truncate(unsigned long nTrailerOffset)
{
	LPVOID		pvNew;

	if (!bValid)
	{
		return FALSE;
	}
	if (nTrailerOffset == nDataSize)
	{
		return bValid;
	}	
	if (nTrailerOffset > nDataSize)
	{
		bValid = FALSE;
		return FALSE;
	}	
	if (nTrailerOffset == 0)
	{
		Clear();
		return bValid;
	}	
	pvNew = HeapReAlloc(GetProcessHeap(),HEAP_REALLOC_IN_PLACE_ONLY,pvData,nTrailerOffset);
	if (pvNew == NULL)
	{
		pvNew = HeapAlloc(GetProcessHeap(), 0, nTrailerOffset);		
		if (pvNew != NULL)
		{
			memcpy(pvNew,pvData,nTrailerOffset);
		}
	}
	if (pvNew == NULL)
	{
		bValid = FALSE;
	}
	else
	{
		pvData = pvNew;
		nDataSize = nTrailerOffset;
	}
	return bValid;
}


// assignment
CBuffer CBuffer::operator=(CBuffer objRHS)
{
	Clear();
	if (!objRHS.bValid)
	{
		bValid = FALSE;
		return *this;
	}
	NewBuffer(objRHS.nDataSize,FALSE);
	if (bValid)
	{
		memcpy(pvData,objRHS.pvData,nDataSize);
	}
	return *this;
}


// returns bValid
BOOL CBuffer::IsValid(void)
{
	return bValid;
}


// copy a part of the buffer into dst.  returns number of characters copied, or -1 if error
long CBuffer::GetPart(void * pvDst, unsigned long nOffset, unsigned long nWidth, BOOL bAddNull)
{	
	char *		pvOffset;

	if ((!bValid) || ( (nOffset) > nDataSize) || (pvDst == NULL))
	{
		return -1;
	}
	try {
		if ((nOffset + nWidth) > nDataSize)
		{
			memset(pvDst,0,(bAddNull) ? (nWidth + 1) : nWidth);
			nWidth = nDataSize - nOffset;
		}
		if (nWidth == 0)
		{
			return 0;
		}
		else
		{
			pvOffset = (char *) pvData;
			pvOffset += nOffset;

			memcpy(pvDst, (void *) pvOffset, nWidth);
		}
		if (bAddNull)
		{
			pvOffset = (char *) pvDst;
			pvOffset[ nWidth ] = '\0';
		}

		return nWidth;
	} catch (...) {
		return -1;
	}
}


// Output a CBuffer
ostream& operator<<(ostream &stream, const CBuffer& objOutput)
{
	stream.write((char *)objOutput.pvData, objOutput.nDataSize);

	return stream;
}


// Input a CBuffer
istream& operator>>(istream &stream, CBuffer& objInput)
{
	if ( (objInput.bValid) && (stream.good()) && (objInput.nDataSize > 0) ) 
	{
		stream.read((char *)objInput.pvData, objInput.nDataSize);
	}
	if ( stream.bad() )
	{
		objInput.bValid = FALSE;
	}
	return stream;
}


// returns true if the contents of two buffers is the same
BOOL CBuffer::operator==(CBuffer objRHS)
{
	if (!bValid)
	{
		return FALSE;
	}
	if (! objRHS.IsValid() )
	{
		return FALSE;
	}
	if (objRHS.nDataSize != nDataSize)
	{
		return FALSE;
	}
	if ( memcmp(objRHS.pvData, pvData, nDataSize) != 0 )
	{
		return FALSE;
	}
	return TRUE;
}

// convert contents to Base64
BOOL CBuffer::Base64Encode(void)
{
	if (nDataSize == 0) 
	{
		return TRUE;
	}
	if (!bValid)
	{
		return FALSE;
	}

	CBuffer objNew;
	long	nTriplets, nTail, nLoop;
	char	*pcNew, *pcOld;

	nTail = ((nDataSize % 3) == 0) ? 0 : (3 - (nDataSize % 3));
	nTriplets = (nDataSize + nTail) / 3;

	objNew.NewBuffer(nTriplets * 4,FALSE);

	if (! objNew.IsValid())
	{
		MarkInvalid();
		return FALSE;
	}

	if (nTail > 0)
	{
		CBuffer objTail;
		objTail.NewBuffer(nTail,TRUE);
		Append(objTail);
		if (!bValid)
		{
			return FALSE;
		}
	}

	pcOld = (char *) pvData;
	pcNew = (char *) objNew.pvData;

	for (nLoop = 0; nLoop < nTriplets; nLoop ++)
	{
		pcNew[0] = BASE64[((pcOld[0] & 252) /  4)                           ];
		pcNew[1] = BASE64[((pcOld[0] &   3) * 16) + ((pcOld[1] & 240) / 16) ];
		pcNew[2] = BASE64[((pcOld[1] &  15) *  4) + ((pcOld[2] & 192) / 64) ];
		pcNew[3] = BASE64[ (pcOld[2] &  63)                                 ];

		pcNew += 4;
		pcOld += 3;
	}
	pcNew -= 4;
	if (nTail >= 1)
	{
		pcNew[3] = '=';
	}
	if (nTail == 2)
	{
		pcNew[2] = '=';
	}
	SetData(objNew.pvData,objNew.nDataSize);

	return bValid;
}


// convert contents from Base64
BOOL CBuffer::Base64Decode(void)
{
	ClearTrailingNulls();
	if (nDataSize == 0)
	{
		return TRUE;
	}
	if ((!bValid) || ((nDataSize % 4) != 0))
	{
		MarkInvalid();
		return FALSE;
	}

	CBuffer objNew;
	long	nTriplets, nTail, nLoop;
	char	*pcNew, *pcOld;

	nTriplets = (nDataSize / 4);
	objNew.NewBuffer(nTriplets * 3, FALSE);
	if (! objNew.IsValid())
	{
		MarkInvalid();
		return FALSE;
	}
	nTail = 0;
	pcOld = (char *) pvData;
	pcOld = pcOld + nDataSize - 1;
	if (*pcOld == '=')
	{
		*pcOld = 'A';
		nTail ++;
	}
	pcOld --;
	if (*pcOld == '=')
	{
		*pcOld = 'A';
		nTail ++;
	}
	// decode to 0-63 in each byte
	pcOld = (char *) pvData;
	for (nLoop = 0; nLoop < (nTriplets * 4); nLoop++)
	{
		if ((*pcOld >= 'A') && (*pcOld <= 'Z'))
		{
			*pcOld = *pcOld - 'A';
		}
		else if ((*pcOld >= 'a') && (*pcOld <= 'z'))
		{
			*pcOld = *pcOld - 'a' + 26;
		}
		else if ((*pcOld >= '0') && (*pcOld <= '9'))
		{
			*pcOld = *pcOld - '0' + 52;
		}
		else if (*pcOld == '+')
		{
			*pcOld = 62;
		}
		else if (*pcOld == '/')
		{
			*pcOld = 63;
		}
		else
		{
			MarkInvalid();
			return FALSE;
		}
		pcOld ++;
	}
	// now shift bits
	pcOld = (char *) pvData;
	pcNew = (char *) objNew.pvData;
	for (nLoop=0; nLoop < nTriplets; nLoop ++)
	{
		pcNew[0] = ( pcOld[0]       << 2) + ((pcOld[1] & 48) >> 4);
		pcNew[1] = ((pcOld[1] & 15) << 4) + ((pcOld[2] & 60) >> 2);
		pcNew[2] = ((pcOld[2] &  3) << 6) +   pcOld[3]            ;

		pcNew += 3;
		pcOld += 4;
	}
	// copy excluding tail bytes
	SetData(objNew.pvData, objNew.nDataSize - nTail);

	return bValid;
}


// removes trailing nulls from a buffer
BOOL CBuffer::ClearTrailingNulls(void)
{
	if (!bValid)
	{
		return FALSE;
	}
	if (nDataSize == 0)
	{
		return FALSE;
	}
	char * pcData = (char *) pvData;
	long nLoop = nDataSize - 1;
	if (pcData[nLoop] != '\0')
	{
		return FALSE;
	}
	while ((nLoop > 0) && (pcData[nLoop] == '\0'))
	{
		nLoop --;
	}
	if (pcData[nLoop] != '\0')
	{
		nLoop ++;
	}
	Truncate(nLoop);

	return TRUE;
}
