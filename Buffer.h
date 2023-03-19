#pragma once
#include <iostream>
using namespace std;

class CBuffer
{
private:
	LPVOID			pvData;
	BOOL			bValid;
	unsigned long	nDataSize;
public:
	// default constructor
	CBuffer(void);
	// copy constructor
	CBuffer(const CBuffer& objRHS);
	// frees memory on destruction
	~CBuffer(void);
	// Get rid of the old space and allocate new space.
	LPVOID NewBuffer(unsigned long nNewSize, BOOL bZero);
	// remove buffer
	void Clear(void);
	// returns the size of the data
	unsigned long Size(void);
	// mark the data as invalid
	void MarkInvalid(void);
	// Sets content to value of passed in null terminated string 
	void SetToString(const char * szContent);
	// Adds a trailer to the buffer
	BOOL Append(CBuffer & objAppendix);
	// Removes a header from a buffer
	BOOL RemoveHeader(unsigned long nHeaderLen);
	// Removes a trailer from a buffer
	BOOL Truncate(unsigned long nTrailerOffset);
	// assignment
	CBuffer operator=(CBuffer objRHS);
	// returns bValid
	BOOL IsValid(void);
	// copy a part of the buffer into dst.  returns number of characters copied
	long GetPart(void * pvDst, unsigned long nOffset, unsigned long nWidth, BOOL bAddNull);
	// sets the buffer contents to the given value
	void SetData(LPVOID pvSrc, long nNewSize);
	// Output a CBuffer
	friend ostream& operator<<(ostream &stream, const CBuffer& objOutput);
	// Input a CBuffer
	friend istream& operator>>(istream &stream, CBuffer& objInput);
	// returns true if the contents of two buffers is the same
	BOOL operator==(CBuffer objRHS);
	// convert contents to Base64
	BOOL Base64Encode(void);
	// convert contents from Base64
	BOOL Base64Decode(void);
	// removes trailing nulls from a buffer
	BOOL ClearTrailingNulls(void);
};
