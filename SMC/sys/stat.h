
#ifndef _STAT_H_
#define _STAT_H_

#include <../include/sys/stat.h>

#ifndef _MSC_VER
#error "Use this header only with Microsoft Visual C++ compilers!"
#endif

#ifndef S_ISBLK
#define S_ISBLK(mode)  (((mode) & S_IFMT) == S_IFBLK)
#endif

#ifndef S_ISFIFO
#define S_ISFIFO(mode)  (((mode) & S_IFMT) == S_IFIFO)
#endif

#ifndef S_ISCHR
#define S_ISCHR(mode)  (((mode) & S_IFMT) == S_IFCHR)
#endif

#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif

#ifndef S_ISLNK
#define S_ISLNK(mode)  (((mode) & S_IFMT) == S_IFLNK)
#endif

#ifndef S_ISSOCK
#define S_ISSOCK(mode)  (((mode) & S_IFMT) == S_IFSOCK)
#endif

#define	_S_ISUID        0004000
#define	_S_ISGID        0002000
#define	_S_ISVTX        0001000

#define	S_ISUID        _S_ISUID
#define	S_ISGID        _S_ISGID
#define	S_ISVTX        _S_ISVTX

#define	_S_IRWXU	     (_S_IREAD | _S_IWRITE | _S_IEXEC)
#define	_S_IXUSR	     _S_IEXEC
#define	_S_IWUSR	     _S_IWRITE
#define	_S_IRUSR	     _S_IREAD
#define	_S_IRWXG        (_S_IRWXU >> 3)
#define	_S_IXGRP        (_S_IXUSR >> 3)
#define	_S_IWGRP        (_S_IWUSR >> 3)
#define	_S_IRGRP        (_S_IRUSR >> 3)
#define	_S_IRWXO        (_S_IRWXG >> 3) 
#define	_S_IXOTH        (_S_IXGRP >> 3)
#define	_S_IWOTH        (_S_IWGRP >> 3)
#define	_S_IROTH        (_S_IRGRP  >> 3)

#ifndef S_IRWXU
#define	S_IRWXU	     _S_IRWXU
#define	S_IXUSR	     _S_IXUSR
#define	S_IWUSR	     _S_IWUSR
#define	S_IRUSR	     _S_IRUSR
#endif
#define	S_IRWXG        _S_IRWXG
#define	S_IXGRP        _S_IXGRP
#define	S_IWGRP        _S_IWGRP
#define	S_IRGRP        _S_IRGRP
#define	S_IRWXO        _S_IRWXO
#define	S_IXOTH        _S_IXOTH
#define	S_IWOTH        _S_IWOTH
#define	S_IROTH        _S_IROTH

#endif