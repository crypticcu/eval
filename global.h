#ifndef SHARED_H
#define SHARED_H

#define UNIX 	defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#define GNU		defined(__GNUC__)
#define WIN64	defined(_WIN64)

#include <stdbool.h>	// bool
#include <stddef.h>		// size_t
#include <stdint.h>		// intmax_t
#include <limits.h>		// INT_MAX

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

#define F_BLD	"\e[1m"	// Bold
#define F_UND	"\e[4m"	// Underline
#define F_CLR	"\e[0m"	// Clear formatting

enum Status		{PASS = -2, FAIL};
enum Direction	{LEFT, RIGHT, UP, DOWN};
struct CharacterSets {char *valid, *opers, *doubl;};
struct ProgramFlags  {bool help, round, radian;};
typedef enum Direction direct_t;

extern const struct CharacterSets ChrSets;
extern struct ProgramFlags Flags;
extern bool CmdLn;	// Using command-line interface?
extern unsigned MaxDec; // Maximum # of decimals allowed using '-d' flag
extern ssize_t MaxLn;

#endif // #ifndef SHARED_H