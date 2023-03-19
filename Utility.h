#pragma once
#include "Buffer.h"
#include "ISOdate.h"

class CUtility;
typedef CUtility * PUtility;


class CUtility
{
	CUtility(void);							   // private constructor	
	CUtility(const CUtility&);                 // Prevent copy-construction
	CUtility& operator=(const CUtility&);      // Prevent assignment
	// member variables
	HRESULT							hCOMSTATUS;
	BOOL							bThisStartedCOM;
public:
	~CUtility(void);
	// Returns the singleton object.
	static PUtility Instance(void);
	// Removes singleton
	static void Done(void);
	// returns true if object is ok to use
	static BOOL IsValid(void);
	// gets the fullpath of the earliest recent file after objLastDate. returns true if successful, with objLastDate and objFullPath updated
	BOOL GetRecentFile(CISOdate& objLastDate, CBuffer& objFullPath);
};

