#include "StdAfx.h"
#include "Utility.h"
#import "c:\windows\system32\shell32.dll" rename("ShellExecute","_ShellExecute")
#include <shldisp.h>

// pointer to the singleton object
static PUtility pSingletonUtility	= NULL;


CUtility::CUtility(void)
{
	// initialise here
	hCOMSTATUS = CoInitialize(0);
	bThisStartedCOM = (hCOMSTATUS == S_OK);
	if (hCOMSTATUS == S_FALSE)
	{
		bThisStartedCOM = FALSE;
		hCOMSTATUS = S_OK;
	}
}


CUtility::~CUtility(void)
{
	Done();
}


// Returns the singleton object.
PUtility CUtility::Instance(void)
{
	if (pSingletonUtility == NULL)
	{
		pSingletonUtility = new CUtility;
	}

	return pSingletonUtility;
}


void CUtility::Done(void)
{
	if (pSingletonUtility != NULL)
	{
		// clear up here	
		if (pSingletonUtility->bThisStartedCOM)
		{
			CoUninitialize();
		}
		// remove singleton
		PUtility pobjTemp = pSingletonUtility;		
		pSingletonUtility = NULL;
		delete pobjTemp;
	}	
}

// returns true if sinlgeton is ok to use
BOOL CUtility::IsValid(void)
{
	if (pSingletonUtility == NULL)
	{
		return FALSE;
	}
	// other checks here
	if (pSingletonUtility->hCOMSTATUS != S_OK)
	{
		return FALSE;
	}

	return TRUE;
}


// gets the fullpath of the earliest recent file after objLaterThan date. returns true if successful
BOOL CUtility::GetRecentFile(CISOdate& objLastDate, CBuffer& objFullPath)
{
	USES_CONVERSION;
		
	CBuffer	    objEarliestName;  // full path of best file found, starts off as blank
	CISOdate	objCurrDate;	  // date of current list item
	CISOdate	objEarliestDate;  // date of best found, starts off as "now"
	_bstr_t		strTarget;
	char		szTarget[_MAX_PATH + 1];

	if (!IsValid())
	{
		return FALSE;
	}

	try {
		// initialise shell COM object
		Shell32::IShellDispatch2Ptr comShell;
		comShell.CreateInstance(__uuidof(Shell)); 
		// get recent files folder
		_variant_t varRecent ((short)Shell32::ssfRECENT);
		Shell32::FolderPtr comFolder = comShell->NameSpace(varRecent);
		// get folder items
		Shell32::FolderItemsPtr comItems = comFolder->Items();
		// loop through items
		objEarliestName.MarkInvalid(); // flag that we haven't found it
		for(int nCurrent = 0; nCurrent < comItems->Count; nCurrent ++ )
		{
			// get file # nCurrent 
			_variant_t varCurrent ((short)nCurrent);
			Shell32::FolderItemPtr comItem = comItems->Item(varCurrent);
			if (comItem->IsLink)
			{
				objCurrDate = comItem->ModifyDate; 
				if ( (( objCurrDate - objLastDate ) > 0) && ( (objEarliestDate - objCurrDate) > 0 ) )
				{
					Shell32::IShellLinkDual2Ptr comLink = comItem->GetLink;  
					Shell32::FolderItemPtr comTarget = comLink->Target;
					if (! comTarget->IsFolder)
					{
						objEarliestDate = objCurrDate;
						strTarget = comTarget->Path;
						strcpy_s(szTarget,_MAX_PATH + 1,W2A( static_cast<wchar_t*>(strTarget) ));
						objEarliestName.SetToString(szTarget);
					}
				}
			}
		}
		if (objEarliestName.IsValid())
		{
			objLastDate = objEarliestDate;
			objFullPath = objEarliestName;
			objFullPath.ClearTrailingNulls();
			return TRUE;
		}
	} catch (...) {
		objFullPath.MarkInvalid();
		return FALSE;
	}

	return FALSE;
}
