#ifndef ENUMS_H
#define ENUMS_H

#include <QObject>

enum  PeremType {
	integer,
	kumString,
	charect,
	real,
	mass_integer,
	mass_charect,
	mass_string,
	mass_real,
	mass_bool,
	kumBoolean,
	none,
	bool_or_num,
	bool_or_lit,
	dummy
};

enum IspType {
	internal,
	instrument,
	kumIsp,
	network,
	KumSystem,
	friendIsp
};

namespace Kumir
{
enum InstructionType {
	Empty, // Just for convience
	VarDecl, // Variable(s) declaration line
	AlgDecl, // Algorhitm header line
	AlgBegin, // Algorhitm begin
	AlgEnd, // Algorhitm end
	LoopBegin, // Loop begin
	LoopEnd, // Loop end
	Else, // "else" in "if..then..else" or "switch..case..else" statement
	Exit, // Exit from loop or algorhitm
	If, // If statement
	Then, // Then statement
	Switch, // Switch statement
	Case, // Case statement
	Fin, // Finishs if..then..else... and switch..case..else..
	Assign, // Assign line (e.g. something := something other)
	Output, // Display output line
	Assert, // Assertion statement
	Pre, // Pre-assertion statement for algorhitm
	Post, // Post-assertion statement for algorhitm
	Input, // Display input
	LoopEndCond, // Loop end by condition (deprecated)
	ModuleBegin, // Begin of module; contatins module name
	ModuleEnd, // End of module
	UseModule, // Import module by name
	FileInput, // File input
	FileOutput, // File output
	Include, // Include raw string
	Doc, // Doxygen-style documentation
	Comment, // Just a comment (deprecated)
	CallAlg, // Call algorhitm without return value
	Undefined = 127// For various applications
};
}

Q_ENUMS(Kumir::InstructionType);


#endif // ENUMS_H
