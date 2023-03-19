#pragma once
#include <iostream>
using namespace std;

#define ISODATELEN 29
#define ISOFORMAT	 "%04d-%02d-%02dT%02d:%02d:%02d.%03d%c%02d:%02d"


const unsigned long	__INVALIDDATE[2] = {0x00000000, 0x80000000};
// constant returned by FileTime when date is invalid
const __int64 INVALIDDATE = *( __int64* ) __INVALIDDATE;

class CISOdate
{
private:
	char m_szISO[ISODATELEN + 1];
public:
	// sets the string
	CISOdate(char * szISO);
	// sets to "now"
	CISOdate(void); 
	// sets all components
	CISOdate(int nYear,int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nMilisecond, int nMinutesEast);
	// clears the structure
	~CISOdate(void);	
	// sets to "now".  Local time if useLocal true, GMT if false  
	void Now(BOOL useLocal);
	// sets to given time
	void SetDateTime(int nYear,int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nMilisecond);
	// returns GMT offset
	int MinutesEast(void);
	// set to special "end of day" value, note: this is not ISO standard
	void SetEndOfDay(int nYear,int nMonth, int nDay);
	// return if end of day value
	BOOL IsEndOfDay(void);
	//  returns number of 100 nanosecond intervals since 1/1/1970 GMT.  Should be cast for FILETIME.  returns INVALIDDATE if error
	__int64 FileTime(void);
	// assignment
	CISOdate operator=(CISOdate dRHS);
	CISOdate operator=(COleDateTime dRHS);
	// date difference
	double operator-(CISOdate dRHS);
	// Output a CISOdate
	friend ostream& operator<<(ostream &stream, const CISOdate& dOutput);
	// Input a CISOdate
	friend istream& operator>>(istream &stream, CISOdate& dInput);
	// returns Year
	int Year(void);
	// returns month
	int Month(void);
	// returns day of month
	int Day(void);
	// returns hour
	int Hour(void);
	// returns minute
	int Minute(void);
	// returns second
	int Second(void);
	// returns milisecond
	int Milisecond(void);
	// copies ISO string into a given string
	void WriteToString(char * szDst);

};



