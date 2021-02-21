#include <float.h>
#include <stdint.h>
#include "global.h"

const struct CharacterSets ChrSets = {
	"+-!^*/%.()1234567890E\'",	// Valid characters
	"+-!^*/%",					// Operators
	"+-!",						// Double operators
};
struct ProgramFlags Flags = {
	false,						// Significant digits	-d [INT]
	false,						// Show help			-h
	false						// Radian mode			-r
};
bool CmdLn;

const unsigned MantSize = 10;
const unsigned MaxDec = DBL_DIG;
const unsigned MaxExp = sizeof(float) != sizeof(double) /* double has double precision */ ?
						99 : 33;
const ssize_t MaxLn = SSIZE_MAX;	// SSIZE_MAX prevents signed-to-unsigned overflow