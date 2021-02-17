#ifndef SHARED_H
#define SHARED_H

#include <stdbool.h>	// bool
#include <stddef.h>		// size_t
#include <stdint.h>		// intmax_t
#include <limits.h>		// INT_MAX
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <unistd.h>
#endif /* using UNIX */

/* Implement ssize_t if needed */
#ifndef __ssize_t_defined
typedef intmax_t	ssize_t;
#endif
#ifndef SSIZE_MAX
#define SSIZE_MAX	INTMAX_MAX
#endif

enum Statuses	{PASS = -2, FAIL};
enum Directions {RIGHT = 1, LEFT};

struct CharacterSets {
	char *valid, *opers, *doubl;
};
extern const struct CharacterSets chrsets;

struct Flags {
	bool help, round, radian;
};
extern struct Flags flags;

extern bool cmdln;	// Using command-line interface?
extern unsigned maxdec; // Maximum # of decimals allowed using '-d' flag
extern ssize_t maxln;

#endif /* #ifndef SHARED_H */