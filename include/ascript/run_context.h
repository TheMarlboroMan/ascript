#pragma once

#include "ascript/host.h"
#include "ascript/variable.h"
#include "ascript/out_interface.h"

#include <map>
#include <optional>

namespace ascript {

//!The run context provides the neccesary interaction facilities between an 
//!interpreter and any instructions. Instructions run oblivious of the 
//!interpreter and the interpreter itself does not try to typecast them at all:
//!all instructions are supposed to be able to perform whatever action through
//!these run_context structures.
struct run_context {

	//!Different signals that can be read by an interpreter.
	enum class signals {none, sigbreak, sigreturn, sigreturnval, sigyield, sigjump, sigfail, sigcall, sigexit};

	//!Class construction.
	                                run_context(host*, out_interface*);

	//Clears signals and values for each new instruction.
	void                            reset();

	std::map<std::string, variable> symbol_table; //!< Current symbol table.
	host *                          host_ptr{nullptr}; //!< Pointer to the host object.
	out_interface *                 out_facility{nullptr}; //!< Pointer to the output facility.
	signals                         signal{signals::none}; //!< Currently signaled signal.
	variable                        value{false}; //!<A value produced by some function or the index of a block.
	std::optional<variable>         return_register; //!<The register where returned values are stored.
	std::vector<variable>           arguments; //!<Vector of arguments to be passed to a call from sigcall: the instruction will write them here, the interpreter will read them.

};

}
