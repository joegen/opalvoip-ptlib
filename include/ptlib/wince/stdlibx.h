//
// (c) Yuri Kiryanov, openh323@kiryanov.com
// for Openh323, www.Openh323.org
//
// Windows CE Port
// Stdlib.h extension 
// 

#ifndef __STDLIBX_H__
#define __STDLIBX_H__

#include <stdlib.h>
#include <winsock.h>
#include <errno.h>
#include <io.h>
#include <tchar.h>
#include <wceatl.h>
#include <atlconv.h>
#include <afxwin.h>
#include <winnetwk.h>
#include <mmsystemx.h>
#include <winsock.h>

#define assert(e) 
inline void abort() { exit(3); }

#define _environ (NULL)
inline char *getenv( const char *varname ) { return NULL; };
inline int putenv( const char *envstring ) { return -1; };

inline void* calloc(size_t num, size_t size)
{
	void *ptr = malloc(num*size);
	if(ptr)
		memset(ptr, 0, num*size);
	return ptr;
}

int		_mkdir(const char *);
int		_rmdir(const char *);
int		_access(const char *, int);
inline int access(const char * s, int i) { return _access(s,i); }
inline int _chdrive(int d) { return 0; }
inline int _chdir(const char * s) { return 0; }
char *	_mktemp (char *temp);
int		remove(const char *);
int		_chmod( const char *filename, int pmode );
int		rename( const char *oldname, const char *newname );
int 		_sopen(const char *, int, int, ...);

#define _S_IREAD        0000400         /* read permission, owner */
#define _S_IWRITE       0000200         /* write permission, owner */

#define S_IREAD  _S_IREAD
#define S_IWRITE _S_IWRITE


#define _O_RDONLY       0x0000  /* open for reading only */
#define _O_WRONLY       0x0001  /* open for writing only */
#define _O_RDWR         0x0002  /* open for reading and writing */
#define _O_APPEND       0x0008  /* writes done at eof */

#define _O_CREAT        0x0100  /* create and open file */
#define _O_TRUNC        0x0200  /* open and truncate */
#define _O_EXCL         0x0400  /* open only if file doesn't already exist */

/* O_TEXT files have <cr><lf> sequences translated to <lf> on read()'s,
** and <lf> sequences translated to <cr><lf> on write()'s
*/

#define _O_TEXT         0x4000  /* file mode is text (translated) */
#define _O_BINARY       0x8000  /* file mode is binary (untranslated) */

/* macro to translate the C 2.0 name used to force binary mode for files */

#define _O_RAW  _O_BINARY

/* Open handle inherit bit */

#define _O_NOINHERIT    0x0080  /* child process doesn't inherit file */

/* Temporary file bit - file is deleted when last handle is closed */

#define _O_TEMPORARY    0x0040  /* temporary file bit */

/* temporary access hint */

#define _O_SHORT_LIVED  0x1000  /* temporary storage file, try not to flush */

/* sequential/random access hints */
#define _O_SEQUENTIAL   0x0020  /* file access is primarily sequential */
#define _O_RANDOM       0x0010  /* file access is primarily random */


#define O_RDONLY        _O_RDONLY
#define O_WRONLY        _O_WRONLY
#define O_RDWR          _O_RDWR
#define O_APPEND        _O_APPEND
#define O_CREAT         _O_CREAT
#define O_TRUNC         _O_TRUNC
#define O_EXCL          _O_EXCL
#define O_TEXT          _O_TEXT
#define O_BINARY        _O_BINARY
#define O_RAW           _O_BINARY
#define O_TEMPORARY     _O_TEMPORARY
#define O_NOINHERIT     _O_NOINHERIT
#define O_SEQUENTIAL    _O_SEQUENTIAL
#define O_RANDOM        _O_RANDOM

#define _SH_DENYRW      0x10    /* deny read/write mode */
#define _SH_DENYWR      0x20    /* deny write mode */
#define _SH_DENYRD      0x30    /* deny read mode */
#define _SH_DENYNO      0x40    /* deny none mode */

#ifdef  __cplusplus
extern "C" {
#endif

/* declare reference to errno */

#define errno			GetLastError()
#define set_errno(err)	SetLastError( err)

/* Error Codes */

#define EPERM           1
#define ENOENT          2
#define ESRCH           3
#define EINTR           4
#define EIO             5
#define ENXIO           6
#define E2BIG           7
#define ENOEXEC         8
#define EBADF           9
#define ECHILD          10
#define EAGAIN          11
#define ENOMEM          12
#define EACCES          13
#define EFAULT          14
#define EBUSY           16
#define EEXIST          17
#define EXDEV           18
#define ENODEV          19
#define ENOTDIR         20
#define EISDIR          21
#define EINVAL          22
#define ENFILE          23
#define EMFILE          24
#define ENOTTY          25
#define EFBIG           27
#define ENOSPC          28
#define ESPIPE          29
#define EROFS           30
#define EMLINK          31
#define EPIPE           32
#define EDOM            33
#define ERANGE          34
#define EDEADLK         36

#ifndef ENAMETOOLONG
#define ENAMETOOLONG    38
#endif

#define ENOLCK          39
#define ENOSYS          40
#ifndef ENAMETOOLONG
#define ENOTEMPTY       41
#endif

#define EILSEQ          42
#define EDEADLOCK       EDEADLK

#ifdef  __cplusplus
}
#endif

long	_lseek(int, long, int);
int		_close(int);
int		_read(int, void *, unsigned int);
int		_write(int, const void *, unsigned int);
int		_open( const char *filename, int oflag , int pmode );
int		_chsize( int handle, long size );

inline int isprint(int c) { return _istprint(c);}
inline int isxdigit(int c) { return _istxdigit(c); }
inline int isspace( int c ) { return _istspace(c); }
inline int isupper( int c ) { return _istupper(c); }
inline int islower( int c ) { return _istlower(c); }
inline int isalnum( int c ) { return _istalnum(c); }
inline int isalpha( int c ) { return _istalpha(c); }
inline int iscntrl( int c ) { return _istcntrl(c); }
inline int isdigit( int c ) { return _istdigit(c); }
inline int ispunct( int c ) { return _istpunct(c); }

long			strtol( const char *nptr, char **endptr, int base );
unsigned long	strtoul( const char *nptr, char **endptr, int base );
double			strtod( const char *nptr, char **endptr );
char *			_i64toa (__int64 val,char *buf,int radix);
char *			_ui64toa (unsigned __int64 val,char *buf,int radix);
__int64			_atoi64(const char *nptr);

const char *	strrchr(const char *, int);
size_t strspn( const char *string, const char *strCharSet );
int stricmp( const unsigned short*string1, const char* string2 );

LONG RegOpenKeyEx( HKEY hKey, const char* lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult );

LONG RegCreateKeyEx( HKEY hKey, const char* lpSub, DWORD dwr, LPSTR lpcls, DWORD dwo, 
	REGSAM sam, LPSECURITY_ATTRIBUTES lpsa, PHKEY phk, LPDWORD lpdw );

LONG RegEnumKey(HKEY hKey, DWORD dwIndex, const char* lpName, DWORD cbName);

LONG RegDeleteKey( HKEY hKey, const char* lpSubKey );

LONG RegEnumValue( HKEY hKey, DWORD dwIndex, const char* lpValueName, LPDWORD lpcbValueName, 
	LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData );

LONG RegQueryValueEx( HKEY hKey, char* lpValueName, 
	LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData );

LONG RegSetValueEx( HKEY hKey, const char* lpValueName, DWORD Reserved, DWORD dwType, 
	const BYTE *lpData, DWORD cbData );

LONG RegDeleteValue( HKEY hKey, const char* lpValueName );

UINT GetWindowsDirectory( char* lpBuffer, UINT uSize );

DWORD GetPrivateProfileString( const char* lpAppName, const char* lpKeyName,
  const char* lpDefault, char* lpReturned, DWORD nSize, const char*  );

BOOL WritePrivateProfileString(const char* lpAppName, const char* lpKeyName,
  const char* lpString, const char* );

#endif