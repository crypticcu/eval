#include "global.h"

const struct CharacterSets ChrSets = {
	"+-!^*/%.()1234567890\'",	// Valid characters
	"+-!^*/%",					// Operators
	"+-!",						// Double operators
};

struct ProgramFlags Flags = {
	false,						// Show help				-h
	false,						// Round to # of decimals	-d
	false						// Radian mode				-r
};

bool CmdLn;
unsigned MaxDec;
ssize_t MaxLn;