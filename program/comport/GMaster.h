#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_GIANT_BUFFER_SZ		20480
#define MAX_HUGE_BUFFER_SZ		10240
#define MAX_LARGE_BUFFER_SZ		2048
#define MAX_BIG_BUFFER_SZ		1024
#define MAX_MODEST_BUFFER_SZ	512
#define MAX_NORMAL_BUFFER_SZ	256
#define MAX_SMALL_BUFFER_SZ		128
#define MAX_LITTLE_BUFFER_SZ	64
#define MAX_TINY_BUFFER_SZ		32


#if defined (WIN32)

	#include <windows.h>

	// Macro Definitions

	#define	MEM_ALLOC(p,q,r)		p = r malloc (q); if (p) memset (p, 0x00, q)
	#define	MEM_ALLOC_EX(p,q,r)		p = r malloc (q)

	#define MEM_CLEAN(p)			if (p){free (p); p = 0;}


//#define	MEM_ALLOC(p,q,r)		p = r malloc (q); if (p) {memset (p, 0x00, q);printf ("A,0x%08X,%d,%s\n", p, __LINE__, __FILE__);}
//#define	MEM_ALLOC_EX(p,q,r)		p = r malloc (q); if (p) {printf ("A,0x%08X,%d,%s\n", p, __LINE__, __FILE__);}

//#define MEM_CLEAN(p)			if (p){free (p); printf ("C,0x%08X,%d,%s\n", p, __LINE__, __FILE__); p = 0;}
	#define DEL(p)					if (p){delete (p); p = 0;}

	#define STR_COPY(p,q)			MEM_ALLOC (p, strlen (q) + 1, (char *)); if (p) strcpy (p, q)
	#define STRN_COPY(p,q,r)		MEM_ALLOC (p, r + 1, (char *)); if (p) strncpy (p, q, r)
	
	#define GET_TICKCOUNT(x)		(x = GetTickCount ())
	#define MS						(((unsigned long) clock ()) % 999)

	#define TIMESTAMP(x)			{struct timeb stTime; ftime (&stTime); x = stTime.time;}

	#define SLEEP(p)				Sleep(p)
	#define MSLEEP(p)				Sleep(1)
	#define NSLEEP(p)				Sleep(1)
	#define SLEEPEX(p,q)			if (0 < p) Sleep(p); else Sleep (1)

	#define UDELAY(x)				Sleep(x)

	#if defined (IS_LITTLE_ENDIAN)

		#define hton_short(x)		(x)
		#define ntoh_short(x)		(x)

		#define hton_int(x)			(x)
		#define ntoh_int(x)			(x)

		#define hton_long(x)		(x)
		#define ntoh_long(x)		(x)

	#elif defined (IS_BIG_ENDIAN)

		#define hton_short(x)		((((x) >> 8)&0xffu) | (((x) << 8) & 0xff00u))
		#define ntoh_short(x)		hton_short (x)

		#define hton_int(x)			((((x) >> 24)&0xffuL) | (((x) >> 8) & 0xff00uL) | (((x) << 8) & 0xff0000uL) | (((x) << 24) & 0xff000000uL))
		#define ntoh_int(x)			hton_int (x)

		#define hton_long(x)		((((x) >> 24)&0xffuL) | (((x) >> 8) & 0xff00uL) | (((x) << 8) & 0xff0000uL) | (((x) << 24) & 0xff000000uL))
		#define ntoh_long(x)		hton_long (x)

	#else

		#error Please define the byte ordering (IS_BIG_ENDIAN or IS_LITTLE_ENDIAN)

	#endif

	// Type Definitions

	#define	INT64					__int64
	#define THREAD_DEF				unsigned _stdcall
	#define THREAD_HANDLER			HANDLE


	// Constant Definitions

	#define	OS_PATH_SEP				'\\'
	#define	OS_PATH_SEP_EX			'/'

	// Variant
	#define INT_TO_VAR(p,q)		VariantInit (&q); q.vt = VT_I2; q.iVal = p
	#define LNG_TO_VAR(p,q)		VariantInit (&q); q.vt = VT_I4; q.lVal = p
	#define DBL_TO_VAR(p,q)		VariantInit (&q); q.vt = VT_R8; q.dblVal = p
	#define BSTR_TO_VAR(p,q)	VariantInit (&q); q.vt = VT_BSTR; q.bstrVal = SysAllocString (p)
	#define BYTE_TO_VAR(p,q,r)	VariantInit (&q); q.vt = VT_BSTR; q.bstrVal = SysAllocStringByteLen ((char *) p, r)
	#define REF_TO_VAR(p,q)		VariantInit (&q); q.vt = VT_BYREF; q.byref = (void *) p

	#define CLEAN_VAR(p)		if (VT_BSTR == p.vt) SysFreeString (p.bstrVal); VariantClear (&p)

#elif defined (LINUX) || defined (UCLINUX)

	#include <sys/time.h>

	// Macro Definitions

	#define	MEM_ALLOC(p,q,r)		p = r malloc (q); if (p) memset (p, 0x00, q)
	#define	MEM_ALLOC_EX(p,q,r)		p = r malloc (q)

	#define MEM_CLEAN(p)			if (p){free (p); p = 0;}
	#define DEL(p)					if (p){delete (p); p = 0;}

	#define STR_COPY(p,q)			MEM_ALLOC (p, strlen (q) + 1, (char *)); if (p) strcpy (p, q)
	#define STRN_COPY(p,q,r)		MEM_ALLOC (p, r + 1, (char *)); if (p) strncpy (p, q, r)

	#define GET_TICKCOUNT(x)		{struct timeval now; gettimeofday(&now, 0); x = now.tv_sec * 1000; x += now.tv_usec / 1000;}
	#define MS						(((unsigned long) clock ()) % 999)

	#define TIMESTAMP(x)			{struct timeb stTime; ftime (&stTime); x = stTime.time;}

	#define SLEEP(m)				{struct timespec cSleep; unsigned long ulTmp; cSleep.tv_sec = m / 1000; if(cSleep.tv_sec == 0){ulTmp = m * 10000; cSleep.tv_nsec = ulTmp * 100;}else{cSleep.tv_nsec = 0;}nanosleep(&cSleep, 0);}
	#define MSLEEP(m)				{struct timespec cSleep; cSleep.tv_sec = 0; cSleep.tv_nsec = m * 1000; nanosleep(&cSleep, 0);}
	#define NSLEEP(m)				{struct timespec cSleep; cSleep.tv_sec = 0; cSleep.tv_nsec = m; nanosleep(&cSleep, 0);}

	#define SLEEPEX(p,q)			nanosleep(&p, &q)

	#define UDELAY(x)				{struct timeval now; unsigned long ulStartTime, ulCurTime; gettimeofday (&now, 0); ulCurTime = now.tv_usec; ulStartTime = ulCurTime; while (ulCurTime - ulStartTime < x && ulCurTime >= ulStartTime){ gettimeofday (&now, 0); ulCurTime = now.tv_usec; }}

	#if defined (IS_LITTLE_ENDIAN)

		#define hton_short(x)		(x)
		#define ntoh_short(x)		(x)

		#define hton_int(x)			(x)
		#define ntoh_int(x)			(x)

		#define hton_long(x)		(x)
		#define ntoh_long(x)		(x)

	#elif defined (IS_BIG_ENDIAN)

		#define hton_short(x)		((((x) >> 8)&0xffu) | (((x) << 8) & 0xff00u))
		#define ntoh_short(x)		hton_short (x)

		#define hton_int(x)			((((x) >> 24)&0xffuL) | (((x) >> 8) & 0xff00uL) | (((x) << 8) & 0xff0000uL) | (((x) << 24) & 0xff000000uL))
		#define ntoh_int(x)			hton_int (x)

		#define hton_long(x)		((((x) >> 24)&0xffuL) | (((x) >> 8) & 0xff00uL) | (((x) << 8) & 0xff0000uL) | (((x) << 24) & 0xff000000uL))
		#define ntoh_long(x)		hton_long (x)

	#else

		#error Please define the byte ordering (IS_BIG_ENDIAN or IS_LITTLE_ENDIAN)

	#endif


	// Type Definitions

	#define	INT64					long long
	#define THREAD_DEF				static void *
	#define THREAD_HANDLER			pthread_t


	// Constant Definitions

	#define	OS_PATH_SEP				'/'
	#define	OS_PATH_SEP_EX			'\\'

#else

	#error Please define the target platform (WIN32, LINUX or UCLINUX)

#endif

