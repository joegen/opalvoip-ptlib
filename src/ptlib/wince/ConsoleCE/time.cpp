//
// (c) Yuriy Gorvitovskiy
// for Openh323, www.Openh323.org
//
// Windows CE Port
//
// time routines implementation
//

#include <time.h>
#include <winbase.h>

static tm tb;
static int _lpdays[] = { -1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
static int _days[] = { -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364 };


#define _DAY_SEC           (24L * 60L * 60L)    /* secs in a day */
#define _YEAR_SEC          (365L * _DAY_SEC)    /* secs in a year */
#define _FOUR_YEAR_SEC     (1461L * _DAY_SEC)   /* secs in a 4 year interval */
#define _BASE_DOW          4                    /* 01-01-70 was a Thursday */

static time_t _inittime=time(NULL);

struct tm * __cdecl gmtime (const time_t *timp)
{
	long caltim = *timp;            /* calendar time to convert */
    int islpyr = 0;                 /* is-current-year-a-leap-year flag */
    int tmptim;
    int *mdays;                /* pointer to days or lpdays */

    struct tm *ptb = &tb;

    if ( caltim < 0L )
        return(NULL);

    /*
     * Determine years since 1970. First, identify the four-year interval
     * since this makes handling leap-years easy (note that 2000 IS a
     * leap year and 2100 is out-of-range).
     */
    tmptim = (int)(caltim / _FOUR_YEAR_SEC);
    caltim -= ((long)tmptim * _FOUR_YEAR_SEC);

    /*
     * Determine which year of the interval
     */
    tmptim = (tmptim * 4) + 70;         /* 1970, 1974, 1978,...,etc. */

    if ( caltim >= _YEAR_SEC ) 
	{
        tmptim++;                       /* 1971, 1975, 1979,...,etc. */
        caltim -= _YEAR_SEC;

        if ( caltim >= _YEAR_SEC ) 
		{

            tmptim++;                   /* 1972, 1976, 1980,...,etc. */
            caltim -= _YEAR_SEC;

            /*
             * Note, it takes 366 days-worth of seconds to get past a leap
             * year.
             */
            if ( caltim >= (_YEAR_SEC + _DAY_SEC) ) 
			{

                    tmptim++;           /* 1973, 1977, 1981,...,etc. */
                    caltim -= (_YEAR_SEC + _DAY_SEC);
            }
            else 
			{
                    /*
                     * In a leap year after all, set the flag.
                     */
                    islpyr++;
            }
        }
    }

    /*
     * tmptim now holds the value for tm_year. caltim now holds the
     * number of elapsed seconds since the beginning of that year.
     */
    ptb->tm_year = tmptim;

    /*
     * Determine days since January 1 (0 - 365). This is the tm_yday value.
     * Leave caltim with number of elapsed seconds in that day.
     */
    ptb->tm_yday = (int)(caltim / _DAY_SEC);
    caltim -= (long)(ptb->tm_yday) * _DAY_SEC;

    /*
     * Determine months since January (0 - 11) and day of month (1 - 31)
     */
    if ( islpyr )
        mdays = _lpdays;
    else
        mdays = _days;


    for ( tmptim = 1 ; mdays[tmptim] < ptb->tm_yday ; tmptim++ ) ;

    ptb->tm_mon = --tmptim;

    ptb->tm_mday = ptb->tm_yday - mdays[tmptim];

    /*
     * Determine days since Sunday (0 - 6)
     */
    ptb->tm_wday = ((int)(*timp / _DAY_SEC) + _BASE_DOW) % 7;

    /*
     *  Determine hours since midnight (0 - 23), minutes after the hour
     *  (0 - 59), and seconds after the minute (0 - 59).
     */
    ptb->tm_hour = (int)(caltim / 3600);
    caltim -= (long)ptb->tm_hour * 3600L;

    ptb->tm_min = (int)(caltim / 60);
    ptb->tm_sec = (int)(caltim - (ptb->tm_min) * 60);

    ptb->tm_isdst = 0;
    return( (struct tm *)ptb );

}

clock_t __cdecl clock (void)
{
    clock_t elapsed;

    /* Calculate the difference between the initial time and now. */
    time_t t = time(NULL);
    elapsed = (t - _inittime) * CLOCKS_PER_SEC;
    return(elapsed);
}

time_t FileTimeToTime(const FILETIME& FileTime)
{
	SYSTEMTIME SystemTime;
	FileTimeToSystemTime(&FileTime,&SystemTime);
	return SystemTimeToTime(&SystemTime);
}

time_t	SystemTimeToTime(const LPSYSTEMTIME pSystemTime)
{	
	tm aTm;
	aTm.tm_sec = pSystemTime->wSecond;  
	aTm.tm_min = pSystemTime->wMinute;  
	aTm.tm_hour = pSystemTime->wHour; 
	aTm.tm_mday = pSystemTime->wDay; 
	aTm.tm_mon = pSystemTime->wMonth;  
	aTm.tm_year = pSystemTime->wYear; 
	aTm.tm_wday = pSystemTime->wDayOfWeek; 	
	aTm.tm_isdst = 1;

	//[YG] Will work for a next 100 years
    if (((aTm.tm_year>>2)<<2)==aTm.tm_year)
        aTm.tm_yday = _lpdays[aTm.tm_mon] + aTm.tm_mday;
    else
        aTm.tm_yday = _days[aTm.tm_mon] + aTm.tm_mday;

	return mktime(&aTm);
}