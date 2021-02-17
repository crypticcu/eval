#include "global.h"

const struct CharacterSets chrsets = {
	"+-!^*/%.()1234567890\'",	// Valid characters
	"+-!^*/%",					// Operators
	"+-!",						// Double operators
};

struct Flags flags = {
	false,						// Show help				-h
	false,						// Round to # of decimals	-d
	false						// Radian mode				-r
};

bool cmdln;
unsigned maxdec;
ssize_t maxln;