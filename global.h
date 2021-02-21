#ifndef GLOBAL_H
#define GLOBAL_H

#define UNIX 	defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#define GNU		defined(__GNUC__)
#define WIN64	defined(_WIN64)

#include <float.h>		// DBL_MANT_DIG
#include <stdbool.h>	// bool
#include <stddef.h>		// size_t
#include <stdint.h>		// intmax_t
#include <limits.h>		// INT_MAX, UINT_MAX, ULONG_MAX, ULLONG_MAX

#if UNIX
#include <unistd.h>		// ssize_t, SSIZE_MAX
#endif // #if UNIX

/* ssize_t portability */
#if !UNIX
#if WIN64
typedef __int64	ssize_t;
#define SSIZE_MAX	INT64_MAX
#else
typedef long	ssize_t;
#define SSIZE_MAX	LONG_MAX
#endif // #if WIN64
#endif // #if !UNIX

/* Attribute portability */
#if GNU
#define attribute(...)	__attribute__((__VA_ARGS__))
#else
#define attribute(...)
#endif // #if GNU

/* size_t format */
#if SIZE_MAX == UINT_MAX
#define SIZE_FMT	"%u"
#elif SIZE_MAX == ULONG_MAX
#define SIZE_FMT	"%lu"
#elif SIZE_MAX == ULLONG_MAX
#define SIZE_FMT	"%llu"
#endif

#define F_BLD	"\e[1m"	// Bold
#define F_UND	"\e[4m"	// Underline
#define F_CLR	"\e[0m"	// Clear formatting

enum ReturnState     {PASS = INT_MIN, FAIL = INT_MAX};
enum Direction       {LEFT, RIGHT, UP, DOWN};
struct CharacterSets {char *valid, *opers, *doubl;};
struct ProgramFlags  {bool round, help, radian;};
typedef enum Direction direct_t;
typedef const char *format_t;

extern const struct CharacterSets ChrSets;
extern struct ProgramFlags Flags;
extern bool CmdLn;				// Using command-line interface?

extern const unsigned MantSize;
extern const unsigned MaxDec;	// Smallest accurate decimal place, 10^-x 
extern const unsigned MaxExp;	// Maximum double exponent
extern const ssize_t MaxLn;		// Maximum input buffer

#endif // #ifndef GLOBAL_H